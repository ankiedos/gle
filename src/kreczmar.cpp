#include "kreczmar.hpp"

namespace GLERT::Kreczmar
{
ind_addr& ind_addr::operator =(const ind_addr& other)
{
	this->b = other.b;
	this->counter = other.counter;
	this->size = other.size;
	return *this;
}

void mem_init(memory& m, std::size_t heap_size, std::size_t data_size, std::size_t stack_size)
{
	m.stack.resize(stack_size);
	m.heap.resize(heap_size);
	m.data.resize(data_size);
	m.IAT.resize(heap_size / 2);
	m.head = m.tail = 0;
	m.free_frms = {};
	m.free_frms_sz = {};
}

bool mem_member(memory& m, word b, word counter, word* d)
{
	// is the pointer pointing at that IAT entry?
	if(counter != m.IAT[b + 1]) return false;
	// if yes, get the direct pointer
	*d = m.IAT[b];
	return true;
}

static void insert(memory& m, std::size_t s, word d)
{
	m.free_frms.push_back(d);
	m.free_frms_sz.push_back(s);
}
static bool search(memory& m, std::size_t s, word* d)
{
	for(std::size_t i = 0; i < m.free_frms.size(); i++)
	{
		if(m.free_frms_sz[i] > s)
		{
			auto diff = m.free_frms_sz[i] - s;
			*d = m.free_frms[i];
			m.free_frms_sz[i] = diff;
			m.free_frms[i] += diff;
			return true;
		}
		else if(m.free_frms_sz[i] == s)
		{
			*d = m.free_frms[i];
			m.free_frms_sz.erase(m.free_frms_sz.begin() + i);
			m.free_frms.erase(m.free_frms.begin() + i);
			return true;
		}
	}
	return false;
}
void mem_free(memory& m, word b, word counter, word size)
{
	word d;
	if(counter != m.IAT[b + 1]) return;
	m.IAT[b + 1]++;
	d = m.IAT[b];
	if(m.queue_empty)
	{
		m.head = b; m.IAT[b] = 0; m.tail = m.head;
	}
	else
	{
		m.IAT[m.tail] = b; m.IAT[b] = 0; m.tail = b;
	}
	if(d + size == m.heap.size())
	{
		m.heap.resize(m.heap.size() - size);
	}
	else insert(m, size, d);
	if(m.compactify_f == m.free_op_count)
	{
		mem_compactify(m, 5);
		m.free_op_count = 0;
	}
	else m.free_op_count++;
}

void mem_new(memory& m, word s, word* b, word* counter)
{
	word d, old_b = *b, old_gctr = m.IAT[*b + 1];
	if(m.queue_empty)
	{
		m.IAT.push_back(0);
		m.IAT.push_back(0);
		*b = m.IAT.size() - 1;
		m.IAT[*b + 1] = 0;
	}
	else
	{
		*b = m.head;
		if(m.tail == m.head) m.head = m.tail = 0;
		else m.head = m.IAT[*b];
	}
	if(search(m, s, &d))
	{
		m.IAT[*b] = d; *counter = m.IAT[*b + 1];
	}
	else
	{
		auto old_sz = m.heap.size();
		m.heap.resize(m.heap.size() + s, 0);
		d = old_sz;
		*counter = 0;
		m.heap[d + 1] = s;
	}
}

static word search_iat_by_d(memory& m, word d)
{
	for(std::size_t i = 0; i < m.IAT.size(); i += 2)
	{
		if(m.IAT[i] == d) return i;
	}
	return -1;
}
void compact_fs(memory& m, std::size_t i)
{
	// If the given frame shares a boundary with another free space frame
	if(m.free_frms[i] + m.free_frms_sz[i] == m.free_frms[i + 1])
	{
		// Increase the frame's size
		m.free_frms_sz[i] += m.free_frms_sz[i + 1];
		// And delete the next from the registry
		m.free_frms.erase(m.free_frms.begin() + i + 1);
		m.free_frms_sz.erase(m.free_frms_sz.begin() + i + 1);
	}
}
void compact_fs(memory& m)
{
	if(m.free_frms.size() == 0) return;
	for(std::size_t i = 0; i < m.free_frms.size() - 1; i++)
	{
		compact_fs(m, i);
	}
	// If the frame shares a boundary with unallocated space
	// Delete the frame from the registry
	auto size = m.free_frms.size();
	if(m.free_frms[size - 1] + m.free_frms_sz[size - 1] == m.heap.size())
	{
		m.heap.resize(m.free_frms[size - 1] - 1);
		m.free_frms.erase(m.free_frms.begin() + size - 1);
		m.free_frms_sz.erase(m.free_frms_sz.begin() + size - 1);
	}
}

void compact_ins(memory& m)
{
	for(std::size_t i = 0; i < m.free_frms.size(); i++)
	{
		// Get the pointer to the current free space frame
		auto freeptr = m.free_frms[i];
		// And its size
		auto freeptr_sz = m.free_frms_sz[i];

		// Get the pointer to the next INS frame (doesn't have to be in the free space), too
		auto obj_addr = freeptr + freeptr_sz;

		// If the current frame isn't last
		if(i < m.free_frms.size() - 1)
		{
			// And obj_addr is the beginning of the next free space frame
			if(obj_addr == m.free_frms[i + 1])
			{
				// Compactify free space
				compact_fs(m, i);
			}
		}
		// But if the current frame shares a boundary with unallocated space (because the location at ptr+size lies in it)
		else if(obj_addr == m.heap.size())
		{
			// Decrease the heap
			m.heap.resize(freeptr - 1);
			// And delete the frame from the registry
			m.free_frms.erase(m.free_frms.begin() + i);
			m.free_frms_sz.erase(m.free_frms_sz.begin() + i);
			return;
		}

		// If it is an object or a string
		if(m.heap[obj_addr] == LO_OBJ)
		{
			// Swap all elements of the object/string and the frame with each other
			auto frame_sz = m.heap[obj_addr + 1];
			// when the frame is larger:
			// [0 0 0 0 0 0] 0 1 2 a
			//  0 1 2 a
			// when the frame is of the same size:
			// [0 0 0 0] 0 1 2 a
			//  0 1 2 a
			// when the frame is smaller:
			// (frame_sz == 4)
			//  0 1 2  3 4 5 6    j (also the offsets for freeptr)
			// -3-2-1  0 1 2 3    offsets for obj_addr
			// [0 0 0] 0 1 2 a
			// dla j == 3
			// m.heap[freeptr + 3] = m.heap[obj_addr + 3]
			// therefor this subalgorithm is correct
			for(word j = 0; j < frame_sz; j++)
			{
				m.heap[freeptr + j] = m.heap[obj_addr + j];
			}
		}
		// However, if this is an indirect pointer
		else if(m.heap[obj_addr] == LO_PTR)
		{
			// Swap b and counter with the first two frame's elements
			m.heap[freeptr] = m.heap[obj_addr];
			m.heap[freeptr + 1] = m.heap[obj_addr + 1];
		}
		// However, if this is a raw value
		else if(m.heap[obj_addr] == LO_VAL)
		{
			// Swap it with the first frame's element
			m.heap[freeptr] = m.heap[obj_addr];
		}
		// Optionally, other conditions for
		// Your runtime follow

		// Swap the frame pointer and the one to the object/string/pointer/value
		freeptr ^= obj_addr ^= freeptr ^= obj_addr;
		// Update the free space frames registry
		m.free_frms[i] = freeptr;
		// If we have been working on an object or a string
		if(m.heap[obj_addr] == LO_OBJ)
		{
			// Update IAT
			auto b = search_iat_by_d(m, freeptr);
			if(b == -1) continue;
			m.IAT[b] = obj_addr;
		}
	}
}

void mem_compactify(Kreczmar::memory& m, std::size_t effectivity)
{
	// Does now, when compact_fs is being called in compact_ins, this call
	// is needed?
	// And is this loop?
	for(std::size_t i = 0; i < effectivity; i++)
	{
		compact_fs(m);
		compact_ins(m);
	}
	compact_fs(m);
}


}
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

void mem_init(memory& m, std::size_t ram_size, std::size_t stack_size)
{
	m.stack.resize(stack_size);
	m.M.resize(ram_size);
	m.IAT.resize(ram_size / 2);
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
	if(d + size == m.M.size())
	{
		m.M.resize(m.M.size() - size);
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
		auto old_sz = m.M.size();
		m.M.resize(m.M.size() + s, 0);
		d = old_sz;
		*counter = 0;
		m.M[d + 1] = s;
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
	// Jezeli dana ramka graniczy z inna ramka wolnej przestrzeni
	if(m.free_frms[i] + m.free_frms_sz[i] == m.free_frms[i + 1])
	{
		// Zwieksz rozmiar tej ramki
		m.free_frms_sz[i] += m.free_frms_sz[i + 1];
		// I usun z rejestru nastepna
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
	// Jezeli ramka graniczy z niezaalokowana przestrzenia
	// Usun ramke z rejestru i zmniejsz odpowiednio LastUsed
	auto size = m.free_frms.size();
	if(m.free_frms[size - 1] + m.free_frms_sz[size - 1] == m.M.size())
	{
		m.M.resize(m.free_frms[size - 1] - 1);
		m.free_frms.erase(m.free_frms.begin() + size - 1);
		m.free_frms_sz.erase(m.free_frms_sz.begin() + size - 1);
	}
}

void compact_ins(memory& m)
{
	for(std::size_t i = 0; i < m.free_frms.size(); i++)
	{
		// Uzyskaj wskaznik do obecnej ramki wolnej przestrzeni
		auto freeptr = m.free_frms[i];
		// Oraz jej rozmiar
		auto freeptr_sz = m.free_frms_sz[i];

		// Uzyskaj tez wskaznik na nastepna ramke w INS (niekoniecznie wolnej przestrzeni)
		auto obj_addr = freeptr + freeptr_sz;

		// Jezeli obecna ramka nie jest ostatnia
		if(i < m.free_frms.size() - 1)
		{
			// Oraz obj_addr jest poczatkiem nastepnej ramki wolnej przestrzeni
			if(obj_addr == m.free_frms[i + 1])
			{
				compact_fs(m, i);
			}
		}
		// Ale jesli obecna ramka graniczy z niezaalokowana przestrzenia (bo wskaznik ptr+size do niej nalezy)
		else if(obj_addr == m.M.size())
		{
			// Zmniejsz LastUsed
			m.M.resize(freeptr - 1);
			// I usun ramke z rejestru
			m.free_frms.erase(m.free_frms.begin() + i);
			m.free_frms_sz.erase(m.free_frms_sz.begin() + i);
			return;
		}

		// Jezeli mamy do czynienia z obiektem/ciagiem
		if(m.M[obj_addr] == LO_OBJ)
		{
			// Zeswapuj wszystkie elementy obiektu/ciagu z ramka
			auto frame_sz = m.M[obj_addr + 1];
			// gdy ramka jest wieksza:
			// [0 0 0 0 0 0] 0 1 2 a
			//  0 1 2 a
			// gdy ramka ma taki sam rozmiar:
			// [0 0 0 0] 0 1 2 a
			//  0 1 2 a
			// gdy ramka jest mniejsza:
			// (frame_sz == 4)
			//  0 1 2  3 4 5 6    j (zarazem offsety dla freeptr)
			// -3-2-1  0 1 2 3    offsety dla obj_addr
			// [0 0 0] 0 1 2 a
			// dla j == 3
			// m.M[freeptr + 3] = m.M[obj_addr + 3]
			// zatem ten podalgorytm jest poprawny
			for(word j = 0; j < frame_sz; j++)
			{
				m.M[freeptr + j] = m.M[obj_addr + j];
			}
		}
		// Jezeli natomiast mamy do czynienia z niebezposrednim wskaznikiem
		else if(m.M[obj_addr] == LO_PTR)
		{
			// Zeswapuj b i counter z dwoma pierwszymi elementami ramki
			m.M[freeptr] = m.M[obj_addr];
			m.M[freeptr + 1] = m.M[obj_addr + 1];
		}
		// Jezeli natomiast mamy do czynienia z wartoscia wielkosci slowa
		else if(m.M[obj_addr] == LO_VAL)
		{
			// Zeswapuj to slowo z pierwszym elementem ramki
			m.M[freeptr] = m.M[obj_addr];
		}
		// Nastepne warunki dla wartosci wiekszych niz slowo
		// tego srodowiska uruchomieniowego

		// Zeswapuj ze soba wskaznik na ramke i na obiekt/ciag/wskaznik/wartosc
		freeptr ^= obj_addr ^= freeptr ^= obj_addr;
		// Zaktualizuj odpowiednio rejestr ramek wolnej przestrzeni
		m.free_frms[i] = freeptr;
		// Jezeli pracowalismy na obiekcie/ciagu
		if(m.M[obj_addr] == LO_OBJ)
		{
			// Zaktualizuj IAT
			auto b = search_iat_by_d(m, freeptr);
			if(b == -1) continue;
			m.IAT[b] = obj_addr;
		}
	}
}

void mem_compactify(Kreczmar::memory& m, std::size_t effectivity)
{
	// Czy teraz, jak compact_fs jest wywolywane w compact_ins, to czy wywolywanie
	// go tutaj jest potrzebne?
	// I czy w ogole ta petla jest potrzebna?
	for(std::size_t i = 0; i < effectivity; i++)
	{
		compact_fs(m);
		compact_ins(m);
	}
	compact_fs(m);
}


}
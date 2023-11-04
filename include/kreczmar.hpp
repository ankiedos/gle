#ifndef GLE2_KRECZMAR_HPP
#define GLE2_KRECZMAR_HPP

#include<vector>

#include "common.hpp"

namespace GLERT::Kreczmar
{
using word = char;
enum layout
{
    LO_OBJ,
    LO_PTR,
    LO_VAL
};
struct ind_addr
{
    const layout lo = LO_PTR;
    word b;
    word counter;
    word size;
    ind_addr& operator=(const ind_addr& other);
};
struct memory
{
    std::vector<word> IAT;
    std::vector<word> heap;
    std::vector<word> data;
    std::vector<word> stack;
    bool queue_empty = true;
    std::size_t head = 0, tail = 0;
    std::vector<word> free_frms;
	std::vector<std::size_t> free_frms_sz;
	std::size_t compactify_f, free_op_count = 0;
};
/*
mutability\layout fixed mixed  variable
initialisable                  decl
immutable               blkchn
mutable                 obj,ai imp,reldb
*/

// imp_memory TRAITS: mutable, variable-size layout
// obj_memory TRAITS: mutable, mixed-size layout
// decl_memory TRAITS: initialisable, variable-size layout
// blockchain_memory TRAITS: immutable, mixed-size layout
// relationaldb_memory TRAITS: mutable, variable-size layout
// ai_memory (net + kv pairs) TRAITS: mutable, mixed-size layout


void mem_init(memory& m, std::size_t heap_size, std::size_t data_size, std::size_t stack_size);
bool mem_member(memory& m, word b, word counter, word* d);
void mem_new(memory& m, word s, word* b, word* counter);
void mem_free(memory& m, word b, word counter, word size);
void mem_compactify(memory& m, std::size_t effectivity);
}



#endif
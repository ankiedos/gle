#ifndef GLE2_VM_HPP
#define GLE2_VM_HPP

#include<vector>
#include "common.hpp"
#include "opcode.hpp"
#include "kreczmar.hpp"
#include "value.hpp"

namespace GLERT
{
struct VM
{
	enum resource
	{
		heap, data, code, reg
	};
	bool debug_mode;
	bool executing;
	Kreczmar::memory mem;
	std::vector<word> c;
	std::size_t ip;
	word regs[7]; // a8 b8 c8 d8 e8 f8 bp sp carry
	bool flags[8]; // eq gt ls gte lse zero overflow childproc
	resource current_resource;
};

constexpr word gle_true = 0xFF;
constexpr word gle_false = 0x80;
constexpr word gle_null = 0x00;
constexpr word gle_unit = 0x01;


word exec_argument_in(word val, word mode);
word exec_argument_out(word val, word mode);

void store(word dst, word src);
void store_real(word dst, word src);
void store_boolean(word dst, word src);


void vm_init(bool debug);
void vm_link(const std::vector<word>& code);
exec_res vm_run();
void vm_free();

void stpush();
void stpop();
void sttop();
void stget();
void ineg();
void fneg();
void lneg();
void iadd();
void fadd();
void ladd();
void isub();
void fsub();
void lsub();
void imul();
void fmul();
void lmul();
void idiv();
void fdiv();
void ldiv();
void imod();
void iinc();
void finc();
void linc();
void idec();
void fdec();
void ldec();
void isgn();
void fsgn();
void lsgn();
void iabs();
void fabs();
void labs();
void bor();
void band();
void bxor();
void ishl();
void fshl();
void lshl();
void ishr();
void fshr();
void lshr();
void bnot();

word get_reg(const std::string& reg);
word get_reg_addr(const std::string& reg);
std::size_t stack_size();
word get_stack(std::size_t idx);
void zero_regs();

std::string list_regs();
std::string list_stack();

}

#endif

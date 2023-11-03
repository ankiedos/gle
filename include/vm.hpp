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
		ram, code, reg
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

void stpush(word dst);
void stpop(word dst);
void sttop(word dst);
void stget(word dst, word idx);
void ineg(word dst, word src);
void fneg(word dst, word src);
void lneg(word dst, word src);
void iadd(word dst, word lhs, word rhs);
void fadd(word dst, word lhs, word rhs);
void ladd(word dst, word lhs, word rhs);
void isub(word dst, word lhs, word rhs);
void fsub(word dst, word lhs, word rhs);
void lsub(word dst, word lhs, word rhs);
void imul(word dst, word lhs, word rhs);
void fmul(word dst, word lhs, word rhs);
void lmul(word dst, word lhs, word rhs);
void idiv(word dst, word lhs, word rhs);
void fdiv(word dst, word lhs, word rhs);
void ldiv(word dst, word lhs, word rhs);
void imod(word dst, word lhs, word rhs);
void iinc(word dst, word src);
void finc(word dst, word src);
void linc(word dst, word src);
void idec(word dst, word src);
void fdec(word dst, word src);
void ldec(word dst, word src);
void isgn(word dst, word src);
void fsgn(word dst, word src);
void lsgn(word dst, word src);
void iabs(word dst, word src);
void fabs(word dst, word src);
void labs(word dst, word src);

word get_reg(const std::string& reg);
word get_reg_addr(const std::string& reg);
std::size_t stack_size();
word get_stack(std::size_t idx);
void zero_regs();

std::string list_regs();
std::string list_stack();

}

#endif

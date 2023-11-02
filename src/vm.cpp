#include<iostream>
#include<limits>

#include "common.hpp"
#include "vm.hpp"
//#include "debug.hpp"

namespace GLERT
{

VM vm;
auto a64 = get_reg_addr("a64");
auto b64 = get_reg_addr("b64");
auto c64 = get_reg_addr("c64");
auto d64 = get_reg_addr("d64");
auto e64 = get_reg_addr("e64");
auto f64 = get_reg_addr("f64");
auto carry64 = get_reg_addr("carry");

word exec_argument(word val, word mode)
{
	switch(mode)
	{
	case AMODE_IMM:      return val;
	case AMODE_REG:      return val;
	case AMODE_PTR:      return vm.mem.M[val];
	case AMODE_REMOTE:   return vm.mem.M[vm.mem.M[val]];
	case AMODE_LOCAL:    return vm.mem.M[1] + val; // b64 + val
	case AMODE_LOCALPTR: return vm.mem.M[1] + vm.mem.M[val]; // b64 + [val]
	}
}

void store(word dst, word src)
{
    vm.mem.M[dst] = src;
}
void store_real(word dst, word src)
{
    vm.mem.M[dst] = *(double*)(src);
}
void store_boolean(word dst, word src)
{
    vm.mem.M[dst] = *(unsigned char*)(src);
}

void vm_init(bool debug = false)
{
	vm.debug_mode = debug;
	mem_init(vm.mem, 50);
}
void vm_link(const std::vector<word>& code)
{
	vm.c = code;
	vm.ip = 0;
}

void stpush(word arg)
{
	vm.mem.stack[vm.mem.st_top++] = arg;
}
void stpop(word dst)
{
	// THROWS: SIG UNDERFLOW
	vm.mem.M[dst] = vm.mem.stack[--vm.mem.st_top];
}
void sttop(word dst)
{
	vm.mem.M[dst] = vm.mem.stack[stack_size() - 1];
}
void stget(word dst, word idx)
{
	// THROWS SIG BOUNDS
	vm.mem.M[dst] = get_stack(idx);
}
void ineg(word arg)
{
	vm.mem.M[arg] = -vm.mem.M[arg];
}
void fneg(word arg) // floating-point instructions must wait for the implementation of variable-sized layout
{
	// vm.mem.M[arg] = *(word)
}
void lneg(word arg)
{
	vm.mem.M[arg] = vm.mem.M[arg] ^ gle_true + gle_false;
}
void iadd(word dst, word lhs, word rhs)
{
	vm.mem.M[dst] = lhs + rhs;
	if(lhs + rhs > std::numeric_limits<word>::max())
	{
		vm.flags[6] = true;
	}
	else
	{
		vm.flags[6] = false;
	}
}
void fadd(word dst, word lhs, word rhs) // floating-point instructions must wait for the implementation of variable-sized layout
{
	//
}
void ladd(word dst, word lhs, word rhs)
{
	vm.mem.M[dst] = (((lhs << 1) & rhs) != 0) ? gle_true : gle_false;
}
void isub(word dst, word lhs, word rhs)
{
	vm.mem.M[dst] = lhs - rhs;
	if(lhs - rhs < std::numeric_limits<word>::min())
	{
		vm.flags[6] = true;
	}
	else
	{
		vm.flags[6] = false;
	}
}
void fsub(word dst, word lhs, word rhs) // floating-point instructions must wait for the implementation of variable-sized layout
{
	//
}
void lsub(word dst, word lhs, word rhs)
{
	vm.mem.M[dst] = lhs ^ rhs + gle_false;
}
void imul(word dst, word lhs, word rhs)
{
	vm.mem.M[dst] = lhs * rhs;
	if(lhs * rhs > std::numeric_limits<word>::max())
	{
		vm.flags[6] = true;
	}
	else
	{
		vm.flags[6] = false;
	}
}
void fmul(word dst, word lhs, word rhs) // floating-point instructions must wait for the implementation of variable-sized layout
{
	// ...
}
void lmul(word dst, word lhs, word rhs)
{
	vm.mem.M[dst] = lhs & rhs;
}
void idiv(word dst, word lhs, word rhs)
{
	// THROWS SIG DIVZERO
	vm.mem.M[dst] = lhs / rhs;
}
void fdiv(word dst, word lhs, word rhs) // floating-point instructions must wait for the implementation of variable-sized layout
{
	// ON rhs == 0 dst = Inf
	// ...
}
void ldiv(word dst, word lhs, word rhs)
{
	if(lhs == gle_false) vm.mem.M[dst] = gle_true;
	else vm.mem.M[dst] = rhs;
}
void imod(word dst, word lhs, word rhs)
{
	// THROWS SIG DIVZERO
	vm.mem.M[dst] = lhs % rhs;
}
void iinc(word arg)
{
	vm.mem.M[arg]++;
}
void finc(word arg) // floating-point instructions must wait for the implementation of variable-sized layout
{
	// ...
}
void linc(word arg)
{
	vm.mem.M[arg] = gle_true;
}
void idec(word arg)
{
	vm.mem.M[arg]--;
}
void fdec(word arg) // floating-point instructions must wait for the implementation of variable-sized layout
{
	// ...
}
void ldec(word arg)
{
	vm.mem.M[arg] = gle_false;
}
void isgn(word arg)
{
	if(arg > 0)
	{
		vm.mem.M[e64] = 1;
	}
	else if(arg < 0)
	{
		vm.mem.M[e64] = -1;
	}
	else vm.mem.M[e64] = 0;
}
void fsgn(word arg) // floating-point instructions must wait for the implementation of variable-sized layout
{
	// ...
}
void lsgn(word arg)
{
	if(arg == gle_true) vm.mem.M[e64] = 1;
	else vm.mem.M[e64] = -1;
}
void iabs(word arg)
{
	if(arg < 0) vm.mem.M[arg] = -vm.mem.M[arg];
}
void fabs(word arg) // floating-point instructions must wait for the implementation of variable-sized layout
{
	// ...
}
void labs(word arg)
{
	if(arg == gle_true) vm.mem.M[arg] = 1;
	else vm.mem.M[arg] = -1;
}

#define ADDR_MODE(name) auto name = vm.c[vm.ip++]
#define OPERAND(name, mode) auto name = exec_argument(vm.c[vm.ip++], mode)

void unary_instr(const std::string& mnemonic, word mode, word arg)
{
	std::cout << mnemonic << " " << (int)mode << " " << (int)arg << "\n";
}
void binary_instr(const std::string& mnemonic, word mode_1, word mode_2, word arg_1, word arg_2)
{
	std::cout << mnemonic << " " << (int)mode_1 << " " << (int)mode_2 << " " << ", " << (int)arg_1 << ", " << (int)arg_2 << "\n";
}
void ternary_instr(const std::string& mnemonic, word mode_1, word mode_2, word mode_3, word arg_1, word arg_2, word arg_3)
{
	std::cout << mnemonic << " " << (int)mode_1 << " " << (int)mode_2 << " " << (int)mode_3 << ", " << (int)arg_1 << ", " << (int)arg_2 << ", " << (int)arg_3 << "\n";
}
exec_res vm_run()
{
	vm.executing = true;
	while(vm.ip < vm.c.size())
	{
		switch(vm.c[vm.ip++])
		{
		case OP_STPUSH:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			stpush(arg);
			if(vm.debug_mode) unary_instr("stpush", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_STPOP:
		{
			ADDR_MODE(mode);
			OPERAND(dst, mode);
			stpop(dst);
			if(vm.debug_mode) unary_instr("stpop", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_STTOP:
		{
			ADDR_MODE(mode);
			OPERAND(dst, mode);
			sttop(dst);
			if(vm.debug_mode) unary_instr("sttop", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_STGET:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND(dst, mode_1);
			OPERAND(idx, mode_2);
			stget(dst, idx);
			if(vm.debug_mode) binary_instr("stget", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_INEG:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			ineg(arg);
			if(vm.debug_mode) unary_instr("ineg", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_FNEG:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			fneg(arg);
			if(vm.debug_mode) unary_instr("fneg", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_LNEG:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			lneg(arg);
			if(vm.debug_mode) unary_instr("lneg", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_IADD:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			iadd(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("iadd", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FADD:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			fadd(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("fadd", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LADD:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			ladd(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("ladd", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_ISUB:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			isub(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("isub", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FSUB:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			fsub(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("fsub", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LSUB:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			lsub(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("lsub", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IMUL:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			imul(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("imul", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FMUL:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			fmul(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("fmul", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LMUL:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			lmul(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("lmul", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IDIV:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			idiv(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("idikv", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FDIV:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			fdiv(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("fdiv", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LDIV:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			ldiv(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("ldiv", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IMOD:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND(dst, mode_1);
			OPERAND(lhs, mode_2);
			OPERAND(rhs, mode_3);
			imod(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("imod", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IINC:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			iinc(arg);
			if(vm.debug_mode) unary_instr("iinc", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_FINC:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			finc(arg);
			if(vm.debug_mode) unary_instr("finc", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_LINC:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			linc(arg);
			if(vm.debug_mode) unary_instr("linc", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_IDEC:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			idec(arg);
			if(vm.debug_mode) unary_instr("idec", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_FDEC:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			fdec(arg);
			if(vm.debug_mode) unary_instr("fdec", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_LDEC:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			ldec(arg);
			if(vm.debug_mode) unary_instr("ldec", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_ISGN:
		{
			ADDR_MODE(mode);
			OPERAND(arg, mode);
			isgn(arg);
			if(vm.debug_mode) unary_instr("isgn", mode, vm.c[vm.ip - 1]);
			break;
		}
		default:
		{
			if(vm.debug_mode) std::cout << "unknown opcode\n";
			return EXEC_UNKNOWN_OPCODE;
		}
		}
	}
	vm.executing = false;
	return EXEC_SUCC;
}
void vm_free()
{}

word get_reg(const std::string& reg)
{
	if(reg == "a64") return vm.mem.M[0];
	if(reg == "b64") return vm.mem.M[1];
	if(reg == "c64") return vm.mem.M[2];
	if(reg == "d64") return vm.mem.M[3];
	if(reg == "e64") return vm.mem.M[4];
	if(reg == "f64") return vm.mem.M[5];
	if(reg == "carry64") return vm.mem.M[6];
	return 0;
}
word get_reg_addr(const std::string& reg)
{
	if(reg == "a64") return 0;
	if(reg == "b64") return 1;
	if(reg == "c64") return 2;
	if(reg == "d64") return 3;
	if(reg == "e64") return 4;
	if(reg == "f64") return 5;
	if(reg == "carry64") return 6;
	return 127;
}
std::size_t stack_size()
{
	return vm.mem.st_top;
}
word get_stack(std::size_t idx)
{
	return vm.mem.stack[idx];
}
void zero_regs()
{
	for(std::size_t i = 0; i < 7; i++)
	{
		vm.mem.M[i] = 0;
	}
}
}

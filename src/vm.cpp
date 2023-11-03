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
auto bp = get_reg_addr("bp");
auto sp = get_reg_addr("sp");
auto carry64 = get_reg_addr("carry");

word exec_argument_in(word val, word mode)
{
	switch(mode)
	{
	case AMODE_IMM:      return val;
	case AMODE_REG:      return vm.regs[val];
	case AMODE_CODE:     return vm.c[val];
	case AMODE_PTR:      return vm.mem.M[val];
	case AMODE_REMOTE:   return vm.mem.M[vm.mem.M[val]];
	case AMODE_LOCAL:    return vm.mem.M[1] + val; // b64 + val
	case AMODE_LOCALPTR: return vm.mem.M[1] + vm.mem.M[val]; // b64 + [val]
	default: break; // THROWS SIG SEGV
	}
}
word exec_argument_out(word val, word mode)
{
	switch(mode)
	{
	case AMODE_PTR:      return val;
	case AMODE_REG:      return val;
	case AMODE_CODE:     return val;
	case AMODE_REMOTE:   return vm.mem.M[val];
	case AMODE_LOCAL:    return vm.mem.M[1] + val; // b64 + val
	case AMODE_LOCALPTR: return vm.mem.M[1] + vm.mem.M[val]; // b64 + [val]
	default: break; // THROWS SIG SEGV
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
	mem_init(vm.mem, 50, 50);
}
void vm_link(const std::vector<word>& code)
{
	vm.c = code;
	vm.ip = 0;
}

void stpush(word arg)
{
	vm.mem.stack[vm.regs[sp]++] = arg;
}
void stpop(word dst)
{
	// THROWS: SIG UNDERFLOW
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = vm.mem.stack[--vm.regs[sp]]; break;
	case VM::code: vm.c[dst] = vm.mem.stack[--vm.regs[sp]]; break;
	case VM::reg: vm.regs[dst] = vm.mem.stack[--vm.regs[sp]]; break;
	}
}
void sttop(word dst)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = vm.mem.stack[vm.regs[sp] - 1]; break;
	case VM::code: vm.c[dst] = vm.mem.stack[vm.regs[sp] - 1]; break;
	case VM::reg: vm.regs[dst] = vm.mem.stack[vm.regs[sp] - 1]; break;
	}
}
void stget(word dst, word idx)
{
	// THROWS SIG BOUNDS
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = get_stack(idx); break;
	case VM::code: vm.c[dst] = get_stack(idx); break;
	case VM::reg: vm.regs[dst] = get_stack(idx); break;
	}
}
void ineg(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = -src; break;
	case VM::code: vm.c[dst] = -src; break;
	case VM::reg: vm.regs[dst] = -src; break;
	}
}
void fneg(word dst, word src) // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::ram: break;// vm.mem.M[dst] = *(src)
	case VM::code: break;
	case VM::reg: break;
	}
}
void lneg(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = src ^ gle_true + gle_false; break;
	case VM::code: vm.c[dst] = src ^ gle_true + gle_false; break;
	case VM::reg: vm.regs[dst] = src ^ gle_true + gle_false; break;
	}
}
void iadd(word dst, word lhs, word rhs)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = lhs + rhs; break;
	case VM::code: vm.c[dst] = lhs + rhs; break;
	case VM::reg: vm.regs[dst] = lhs + rhs; break;
	}
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
	switch(vm.current_resource)
	{
	case VM::ram: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void ladd(word dst, word lhs, word rhs)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = (((lhs << 1) & rhs) != 0) ? gle_true : gle_false; break;
	case VM::code: vm.c[dst] = (((lhs << 1) & rhs) != 0) ? gle_true : gle_false; break;
	case VM::reg: vm.regs[dst] = (((lhs << 1) & rhs) != 0) ? gle_true : gle_false; break;
	}
}
void isub(word dst, word lhs, word rhs)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = lhs - rhs; break;
	case VM::code: vm.c[dst] = lhs - rhs; break;
	case VM::reg: vm.regs[dst] = lhs - rhs; break;
	}
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
	switch(vm.current_resource)
	{
	case VM::ram: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void lsub(word dst, word lhs, word rhs)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = lhs ^ rhs + gle_false; break;
	case VM::code: vm.c[dst] = lhs ^ rhs + gle_false; break;
	case VM::reg: vm.regs[dst] = lhs ^ rhs + gle_false; break;
	}
}
void imul(word dst, word lhs, word rhs)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = lhs * rhs; break;
	case VM::code: vm.c[dst] = lhs * rhs; break;
	case VM::reg: vm.regs[dst] = lhs * rhs; break;
	}
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
	switch(vm.current_resource)
	{
	case VM::ram: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void lmul(word dst, word lhs, word rhs)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = lhs & rhs; break;
	case VM::code: vm.c[dst] = lhs & rhs; break;
	case VM::reg: vm.regs[dst] = lhs & rhs; break;
	}
}
void idiv(word dst, word lhs, word rhs)
{
	// THROWS SIG DIVZERO
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = lhs / rhs; break;
	case VM::code: vm.c[dst] = lhs / rhs; break;
	case VM::reg: vm.regs[dst] = lhs / rhs; break;
	}
}
void fdiv(word dst, word lhs, word rhs) // floating-point instructions must wait for the implementation of variable-sized layout
{
	// ON rhs == 0 dst = Inf
	switch(vm.current_resource)
	{
	case VM::ram: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void ldiv(word dst, word lhs, word rhs)
{
	switch(vm.current_resource)
	{
	case VM::ram:
		if(lhs == gle_false) vm.mem.M[dst] = gle_true;
		else vm.mem.M[dst] = rhs;
		break;
	case VM::code:
		if(lhs == gle_false) vm.c[dst] = gle_true;
		else vm.c[dst] = rhs;
		break;
	case VM::reg:
		if(lhs == gle_false) vm.regs[dst] = gle_true;
		else vm.regs[dst] = rhs;
		break;
	}
}
void imod(word dst, word lhs, word rhs)
{
	// THROWS SIG DIVZERO
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = lhs % rhs; break;
	case VM::code: vm.c[dst] = lhs % rhs; break;
	case VM::reg: vm.regs[dst] = lhs % rhs; break;
	}
}
void iinc(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = src + 1; break;
	case VM::code: vm.c[dst] = src + 1; break;
	case VM::reg: vm.regs[dst] = src + 1; break;
	}
}
void finc(word dst, word src) // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::ram: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void linc(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = gle_true; break;
	case VM::code: vm.c[dst] = gle_true; break;
	case VM::reg: vm.regs[dst] = gle_true; break;
	}
}
void idec(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = src - 1; break;
	case VM::code: vm.c[dst] = src - 1; break;
	case VM::reg: vm.regs[dst] = src - 1; break;
	}
}
void fdec(word dst, word src) // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::ram: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void ldec(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram: vm.mem.M[dst] = gle_false; break;
	case VM::code: vm.c[dst] = gle_false; break;
	case VM::reg: vm.regs[dst] = gle_false; break;
	}
}
void isgn(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram:
		if(src > 0)
		{
			vm.mem.M[dst] = 1;
		}
		else if(src < 0)
		{
			vm.mem.M[dst] = -1;
		}
		else vm.mem.M[dst] = 0;
		break;
	case VM::code:
		if(src > 0)
		{
			vm.c[dst] = 1;
		}
		else if(src < 0)
		{
			vm.c[dst] = -1;
		}
		else vm.c[dst] = 0;
		break;
	case VM::reg:
		if(src > 0)
		{
			vm.regs[dst] = 1;
		}
		else if(src < 0)
		{
			vm.regs[dst] = -1;
		}
		else vm.regs[dst] = 0;
		break;
	}
}
void fsgn(word dst, word src) // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::ram: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void lsgn(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram:
		if(src == gle_true) vm.mem.M[dst] = 1;
		else vm.mem.M[dst] = -1;
		break;
	case VM::code:
		if(src == gle_true) vm.c[dst] = 1;
		else vm.c[dst] = -1;
		break;
	case VM::reg:
		if(src == gle_true) vm.regs[dst] = 1;
		else vm.regs[dst] = -1;
		break;
	}
}
void iabs(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram: if(src < 0) vm.mem.M[dst] = -src; break;
	case VM::code: if(src < 0) vm.c[dst] = -src; break;
	case VM::reg: if(src < 0) vm.regs[dst] = -src; break;
	}
}
void fabs(word dst, word src) // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::ram: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void labs(word dst, word src)
{
	switch(vm.current_resource)
	{
	case VM::ram:
		if(src == gle_true) vm.mem.M[dst] = 1;
		else vm.mem.M[dst] = -1;
		break;
	case VM::code:
		if(src == gle_true) vm.c[dst] = 1;
		else vm.c[dst] = -1;
		break;
	case VM::reg:
		if(src == gle_true) vm.regs[dst] = 1;
		else vm.regs[dst] = -1;
		break;
	}
}

#define ADDR_MODE(name) auto name = vm.c[vm.ip++]
#define OPERAND_IN(name, mode) auto name = exec_argument_in(vm.c[vm.ip++], mode)
#define OPERAND_OUT(name, mode) auto name = exec_argument_out(vm.c[vm.ip++], mode)

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
		switch(vm.c[vm.ip])
		{
		case 0xCC:
			vm.current_resource = VM::code;
			vm.ip++;
			break;
		case 0xFF:
			vm.current_resource = VM::reg;
			vm.ip++;
			break;
		default:
			vm.current_resource = VM::ram;
			break;
		}
		switch(vm.c[vm.ip++])
		{
		case OP_STPUSH:
		{
			ADDR_MODE(mode);
			OPERAND_IN(arg, mode);
			stpush(arg);
			if(vm.debug_mode) unary_instr("stpush", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_STPOP:
		{
			ADDR_MODE(mode);
			OPERAND_OUT(dst, mode);
			stpop(dst);
			if(vm.debug_mode) unary_instr("stpop", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_STTOP:
		{
			ADDR_MODE(mode);
			OPERAND_OUT(dst, mode);
			sttop(dst);
			if(vm.debug_mode) unary_instr("sttop", mode, vm.c[vm.ip - 1]);
			break;
		}
		case OP_STGET:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(idx, mode_2);
			stget(dst, idx);
			if(vm.debug_mode) binary_instr("stget", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_INEG:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			ineg(dst, src);
			if(vm.debug_mode) binary_instr("ineg", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FNEG:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			fneg(dst, src);
			if(vm.debug_mode) binary_instr("fneg", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LNEG:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			lneg(dst, src);
			if(vm.debug_mode) binary_instr("lneg", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IADD:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			iadd(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("iadd", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FADD:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			fadd(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("fadd", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LADD:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			ladd(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("ladd", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_ISUB:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			isub(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("isub", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FSUB:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			fsub(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("fsub", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LSUB:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			lsub(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("lsub", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IMUL:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			imul(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("imul", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FMUL:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			fmul(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("fmul", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LMUL:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			lmul(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("lmul", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IDIV:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			idiv(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("idikv", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FDIV:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			fdiv(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("fdiv", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LDIV:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			ldiv(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("ldiv", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IMOD:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			ADDR_MODE(mode_3);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(lhs, mode_2);
			OPERAND_IN(rhs, mode_3);
			imod(dst, lhs, rhs);
			if(vm.debug_mode) ternary_instr("imod", mode_1, mode_2, mode_3, vm.c[vm.ip - 3], vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IINC:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			iinc(dst, src);
			if(vm.debug_mode) binary_instr("iinc", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FINC:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			finc(dst, src);
			if(vm.debug_mode) binary_instr("finc", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LINC:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			linc(dst, src);
			if(vm.debug_mode) binary_instr("linc", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IDEC:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			idec(dst, src);
			if(vm.debug_mode) binary_instr("idec", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FDEC:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			fdec(dst, src);
			if(vm.debug_mode) binary_instr("fdec", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LDEC:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			ldec(dst, src);
			if(vm.debug_mode) binary_instr("ldec", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_ISGN:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			isgn(dst, src);
			if(vm.debug_mode) binary_instr("isgn", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FSGN:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			fsgn(dst, src);
			if(vm.debug_mode) binary_instr("isgn", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LSGN:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			lsgn(dst, src);
			if(vm.debug_mode) binary_instr("isgn", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_IABS:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			iabs(dst, src);
			if(vm.debug_mode) binary_instr("isgn", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_FABS:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			fabs(dst, src);
			if(vm.debug_mode) binary_instr("isgn", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
			break;
		}
		case OP_LABS:
		{
			ADDR_MODE(mode_1);
			ADDR_MODE(mode_2);
			OPERAND_OUT(dst, mode_1);
			OPERAND_IN(src, mode_2);
			labs(dst, src);
			if(vm.debug_mode) binary_instr("isgn", mode_1, mode_2, vm.c[vm.ip - 2], vm.c[vm.ip - 1]);
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
	if(reg == "bp") return 6;
	if(reg == "sp") return 7;
	if(reg == "carry64") return 8;
	return 127;
}
std::size_t stack_size()
{
	return vm.regs[sp];
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

std::string list_regs()
{
	std::string result{};
	result += "a64 = " + std::to_string((int)vm.regs[a64]) + "\n";
	result += "b64 = " + std::to_string((int)vm.regs[b64]) + "\n";
	result += "c64 = " + std::to_string((int)vm.regs[c64]) + "\n";
	result += "d64 = " + std::to_string((int)vm.regs[d64]) + "\n";
	result += "e64 = " + std::to_string((int)vm.regs[e64]) + "\n";
	result += "f64 = " + std::to_string((int)vm.regs[f64]) + "\n";
	result += "bp = " + std::to_string((int)vm.regs[bp]) + "\n";
	result += "sp = " + std::to_string((int)vm.regs[sp]) + "\n";
	result += "carry64 = " + std::to_string((int)vm.regs[carry64]) + "\n";
	return result;
}
std::string list_stack()
{
	std::string result{};
	for(std::size_t i = vm.regs[sp]; i > 0; i--)
	{
		result += std::to_string(i - 1) + ": " + std::to_string((int)vm.mem.stack[i - 1]) + "\n";
	}
	return result;
}
}

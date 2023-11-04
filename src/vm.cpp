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

word mode[3] = {};
word res[3] = {};
word arg[3] = {};

word exec_argument(word val, word mode, word res)
{
	switch(mode)
	{
	case AMODE_VAL:
		switch(res)
		{
		case RES_LIT:    return val;
		case RES_HEAP:   return vm.mem.heap[val];
		case RES_DATA:   return vm.mem.data[val];
		case RES_CODE:   return vm.c[val];
		case RES_REG:    return vm.regs[val];
		}
	case AMODE_PTR_H:
		switch(res)
		{
		case RES_LIT:    return vm.mem.heap[val];
		case RES_HEAP:   return vm.mem.heap[vm.mem.heap[val]];
		case RES_DATA:   return vm.mem.heap[vm.mem.data[val]];
		case RES_CODE:   return vm.mem.heap[vm.c[val]];
		case RES_REG:    return vm.mem.heap[vm.regs[val]];
		}
	case AMODE_PTR_D:
		switch(res)
		{
		case RES_LIT:    return vm.mem.data[val];
		case RES_HEAP:   return vm.mem.data[vm.mem.heap[val]];
		case RES_DATA:   return vm.mem.data[vm.mem.data[val]];
		case RES_CODE:   return vm.mem.data[vm.c[val]];
		case RES_REG:    return vm.mem.data[vm.regs[val]];
		}
	case AMODE_REMOTE_HH:
		switch(res)
		{
		case RES_LIT:    return vm.mem.heap[vm.mem.heap[val]];
		case RES_HEAP:   return vm.mem.heap[vm.mem.heap[vm.mem.heap[val]]];
		case RES_DATA:   return vm.mem.heap[vm.mem.heap[vm.mem.data[val]]];
		case RES_CODE:   return vm.mem.heap[vm.mem.heap[vm.c[val]]];
		case RES_REG:    return vm.mem.heap[vm.mem.heap[vm.regs[val]]];
		}
	case AMODE_REMOTE_HD:
		switch(res)
		{
		case RES_LIT:    return vm.mem.data[vm.mem.heap[val]];
		case RES_HEAP:   return vm.mem.data[vm.mem.heap[vm.mem.heap[val]]];
		case RES_DATA:   return vm.mem.data[vm.mem.heap[vm.mem.data[val]]];
		case RES_CODE:   return vm.mem.data[vm.mem.heap[vm.c[val]]];
		case RES_REG:    return vm.mem.data[vm.mem.heap[vm.regs[val]]];
		}
	case AMODE_REMOTE_DH:
		switch(res)
		{
		case RES_LIT:    return vm.mem.heap[vm.mem.data[val]];
		case RES_HEAP:   return vm.mem.heap[vm.mem.data[vm.mem.heap[val]]];
		case RES_DATA:   return vm.mem.heap[vm.mem.data[vm.mem.data[val]]];
		case RES_CODE:   return vm.mem.heap[vm.mem.data[vm.c[val]]];
		case RES_REG:    return vm.mem.heap[vm.mem.data[vm.regs[val]]];
		}
	case AMODE_REMOTE_DD:
		switch(res)
		{
		case RES_LIT:    return vm.mem.data[vm.mem.data[val]];
		case RES_HEAP:   return vm.mem.data[vm.mem.data[vm.mem.heap[val]]];
		case RES_DATA:   return vm.mem.data[vm.mem.data[vm.mem.data[val]]];
		case RES_CODE:   return vm.mem.data[vm.mem.data[vm.c[val]]];
		case RES_REG:    return vm.mem.data[vm.mem.data[vm.regs[val]]];
		}
	case AMODE_LOCAL:
		switch(res)
		{
		case RES_LIT:    return vm.regs[1] + val;
		case RES_HEAP:   return vm.regs[1] + vm.mem.heap[val];
		case RES_DATA:   return vm.regs[1] + vm.mem.data[val];
		case RES_CODE:   return vm.regs[1] + vm.c[val];
		case RES_REG:    return vm.regs[1] + vm.regs[val];
		}//return vm.mem.heap[1] + val; // b64 + val
	case AMODE_LOCALPTR_H:
		switch(res)
		{
		case RES_LIT:    return vm.regs[1] + vm.mem.heap[val];
		case RES_HEAP:   return vm.regs[1] + vm.mem.heap[vm.mem.heap[val]];
		case RES_DATA:   return vm.regs[1] + vm.mem.heap[vm.mem.data[val]];
		case RES_CODE:   return vm.regs[1] + vm.mem.heap[vm.c[val]];
		case RES_REG:    return vm.regs[1] + vm.mem.heap[vm.regs[val]];
		}//return vm.mem.heap[1] + vm.mem.heap[val]; // b64 + [val]
	case AMODE_LOCALPTR_D:
		switch(res)
		{
		case RES_LIT:    return vm.regs[1] + vm.mem.data[val];
		case RES_HEAP:   return vm.regs[1] + vm.mem.data[vm.mem.heap[val]];
		case RES_DATA:   return vm.regs[1] + vm.mem.data[vm.mem.data[val]];
		case RES_CODE:   return vm.regs[1] + vm.mem.data[vm.c[val]];
		case RES_REG:    return vm.regs[1] + vm.mem.data[vm.regs[val]];
		}//return vm.mem.heap[1] + vm.mem.heap[val]; // b64 + [val]
	default: break; // THROWS SIG SEGV
	}
}

/* void store(word dst, word src)
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
} */

void vm_init(bool debug = false)
{
	vm.debug_mode = debug;
	mem_init(vm.mem, 50, 50, 50);
}
void vm_link(const std::vector<word>& code)
{
	vm.c = code;
	vm.ip = 0;
}

void stpush()
{
	vm.mem.stack[vm.regs[sp]++] = arg[0];
}
void stpop()
{
	// THROWS: SIG UNDERFLOW
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = vm.mem.stack[--vm.regs[sp]]; break;
	case VM::data: vm.mem.data[arg[0]] = vm.mem.stack[--vm.regs[sp]]; break;
	case VM::code: vm.c[arg[0]] = vm.mem.stack[--vm.regs[sp]]; break;
	case VM::reg: vm.regs[arg[0]] = vm.mem.stack[--vm.regs[sp]]; break;
	}
}
void sttop()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = vm.mem.stack[vm.regs[sp] - 1]; break;
	case VM::data: vm.mem.data[arg[0]] = vm.mem.stack[vm.regs[sp] - 1]; break;
	case VM::code: vm.c[arg[0]] = vm.mem.stack[vm.regs[sp] - 1]; break;
	case VM::reg: vm.regs[arg[0]] = vm.mem.stack[vm.regs[sp] - 1]; break;
	}
}
void stget()
{
	// THROWS SIG BOUNDS
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = get_stack(arg[1]); break;
	case VM::data: vm.mem.data[arg[0]] = get_stack(arg[1]); break;
	case VM::code: vm.c[arg[0]] = get_stack(arg[1]); break;
	case VM::reg: vm.regs[arg[0]] = get_stack(arg[1]); break;
	}
}
void ineg()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = -arg[1]; break;
	case VM::data: vm.mem.data[arg[0]] = -arg[1]; break;
	case VM::code: vm.c[arg[0]] = -arg[1]; break;
	case VM::reg: vm.regs[arg[0]] = -arg[1]; break;
	}
}
void fneg() // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::heap: break;// vm.mem.heap[dst] = *(src)
	case VM::data: break;// vm.mem.data[dst] = *(src)
	case VM::code: break;
	case VM::reg: break;
	}
}
void lneg()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] ^ gle_true + gle_false; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] ^ gle_true + gle_false; break;
	case VM::code: vm.c[arg[0]] = arg[1] ^ gle_true + gle_false; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] ^ gle_true + gle_false; break;
	}
}
void iadd()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] + arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] + arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] + arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] + arg[2]; break;
	}
	if(arg[1] + arg[2] > std::numeric_limits<word>::max())
	{
		vm.flags[6] = true;
	}
	else
	{
		vm.flags[6] = false;
	}
}
void fadd() // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void ladd()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = (((arg[1] << 1) & arg[2]) != 0) ? gle_true : gle_false; break;
	case VM::data: vm.mem.data[arg[0]] = (((arg[1] << 1) & arg[2]) != 0) ? gle_true : gle_false; break;
	case VM::code: vm.c[arg[0]] = (((arg[1] << 1) & arg[2]) != 0) ? gle_true : gle_false; break;
	case VM::reg: vm.regs[arg[0]] = (((arg[1] << 1) & arg[2]) != 0) ? gle_true : gle_false; break;
	}
}
void isub()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] - arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] - arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] - arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] - arg[2]; break;
	}
	if(arg[1] - arg[2] < std::numeric_limits<word>::min())
	{
		vm.flags[6] = true;
	}
	else
	{
		vm.flags[6] = false;
	}
}
void fsub() // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void lsub()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] ^ arg[2] + gle_false; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] ^ arg[2] + gle_false; break;
	case VM::code: vm.c[arg[0]] = arg[1] ^ arg[2] + gle_false; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] ^ arg[2] + gle_false; break;
	}
}
void imul()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] * arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] * arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] * arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] * arg[2]; break;
	}
	if(arg[1] * arg[2] > std::numeric_limits<word>::max())
	{
		vm.flags[6] = true;
	}
	else
	{
		vm.flags[6] = false;
	}
}
void fmul() // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void lmul()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] & arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] & arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] & arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] & arg[2]; break;
	}
}
void idiv()
{
	// THROWS SIG DIVZERO
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] / arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] / arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] / arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] / arg[2]; break;
	}
}
void fdiv() // floating-point instructions must wait for the implementation of variable-sized layout
{
	// ON rhs == 0 dst = Inf
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void ldiv()
{
	switch(vm.current_resource)
	{
	case VM::heap:
		if(arg[1] == gle_false) vm.mem.heap[arg[0]] = gle_true;
		else vm.mem.heap[arg[0]] = arg[1];
		break;
	case VM::data:
		if(arg[1] == gle_false) vm.mem.data[arg[0]] = gle_true;
		else vm.mem.data[arg[0]] = arg[1];
		break;
	case VM::code:
		if(arg[1] == gle_false) vm.c[arg[0]] = gle_true;
		else vm.c[arg[0]] = arg[2];
		break;
	case VM::reg:
		if(arg[1] == gle_false) vm.regs[arg[0]] = gle_true;
		else vm.regs[arg[0]] = arg[2];
		break;
	}
}
void imod()
{
	// THROWS SIG DIVZERO
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] % arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] % arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] % arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] % arg[2]; break;
	}
}
void iinc()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] + 1; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] + 1; break;
	case VM::code: vm.c[arg[0]] = arg[1] + 1; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] + 1; break;
	}
}
void finc() // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void linc()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = gle_true; break;
	case VM::data: vm.mem.data[arg[0]] = gle_true; break;
	case VM::code: vm.c[arg[0]] = gle_true; break;
	case VM::reg: vm.regs[arg[0]] = gle_true; break;
	}
}
void idec()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] - 1; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] - 1; break;
	case VM::code: vm.c[arg[0]] = arg[1] - 1; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] - 1; break;
	}
}
void fdec() // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void ldec()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = gle_false; break;
	case VM::data: vm.mem.data[arg[0]] = gle_false; break;
	case VM::code: vm.c[arg[0]] = gle_false; break;
	case VM::reg: vm.regs[arg[0]] = gle_false; break;
	}
}
void isgn()
{
	switch(vm.current_resource)
	{
	case VM::heap:
		if(arg[1] > 0)
		{
			vm.mem.heap[arg[0]] = 1;
		}
		else if(arg[1] < 0)
		{
			vm.mem.heap[arg[0]] = -1;
		}
		else vm.mem.heap[arg[0]] = 0;
		break;
	case VM::data:
		if(arg[1] > 0)
		{
			vm.mem.data[arg[0]] = 1;
		}
		else if(arg[1] < 0)
		{
			vm.mem.data[arg[0]] = -1;
		}
		else vm.mem.data[arg[0]] = 0;
		break;
	case VM::code:
		if(arg[1] > 0)
		{
			vm.c[arg[0]] = 1;
		}
		else if(arg[1] < 0)
		{
			vm.c[arg[0]] = -1;
		}
		else vm.c[arg[0]] = 0;
		break;
	case VM::reg:
		if(arg[1] > 0)
		{
			vm.regs[arg[0]] = 1;
		}
		else if(arg[1] < 0)
		{
			vm.regs[arg[0]] = -1;
		}
		else vm.regs[arg[0]] = 0;
		break;
	}
}
void fsgn() // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void lsgn()
{
	switch(vm.current_resource)
	{
	case VM::heap:
		if(arg[1] == gle_true) vm.mem.heap[arg[0]] = 1;
		else vm.mem.heap[arg[0]] = -1;
		break;
	case VM::data:
		if(arg[1] == gle_true) vm.mem.data[arg[0]] = 1;
		else vm.mem.data[arg[0]] = -1;
		break;
	case VM::code:
		if(arg[1] == gle_true) vm.c[arg[0]] = 1;
		else vm.c[arg[0]] = -1;
		break;
	case VM::reg:
		if(arg[1] == gle_true) vm.regs[arg[0]] = 1;
		else vm.regs[arg[0]] = -1;
		break;
	}
}
void iabs()
{
	switch(vm.current_resource)
	{
	case VM::heap:
		if(arg[1] < 0) vm.mem.heap[arg[0]] = -arg[1];
		else vm.mem.heap[arg[0]] = arg[1];
		break;
	case VM::data:
		if(arg[1] < 0) vm.mem.data[arg[0]] = -arg[1];
		else vm.mem.data[arg[0]] = arg[1];
		break;
	case VM::code:
		if(arg[1] < 0) vm.c[arg[0]] = -arg[1];
		else vm.c[arg[0]] = arg[1];
		break;
	case VM::reg:
		if(arg[1] < 0) vm.regs[arg[0]] = -arg[1];
		else vm.regs[arg[0]] = arg[1];
		break;
	}
}
void fabs() // floating-point instructions must wait for the implementation of variable-sized layout
{
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void labs()
{
	switch(vm.current_resource)
	{
	case VM::heap:
		if(arg[1] == gle_true) vm.mem.heap[arg[0]] = 1;
		else vm.mem.heap[arg[0]] = -1;
		break;
	case VM::data:
		if(arg[1] == gle_true) vm.mem.data[arg[0]] = 1;
		else vm.mem.data[arg[0]] = -1;
		break;
	case VM::code:
		if(arg[1] == gle_true) vm.c[arg[0]] = 1;
		else vm.c[arg[0]] = -1;
		break;
	case VM::reg:
		if(arg[1] == gle_true) vm.regs[arg[0]] = 1;
		else vm.regs[arg[0]] = -1;
		break;
	}
}
void bor()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] | arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] | arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] | arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] | arg[2]; break;
	}
}
void band()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] & arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] & arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] & arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] & arg[2]; break;
	}
}
void bxor()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] ^ arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] ^ arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] ^ arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] ^ arg[2]; break;
	}
}
void ishl()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] << arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] << arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] << arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] << arg[2]; break;
	}
}
void fshl()
{
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void lshl()
{
	// Does it make sense?
}
void ishr()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = arg[1] >> arg[2]; break;
	case VM::data: vm.mem.data[arg[0]] = arg[1] >> arg[2]; break;
	case VM::code: vm.c[arg[0]] = arg[1] >> arg[2]; break;
	case VM::reg: vm.regs[arg[0]] = arg[1] >> arg[2]; break;
	}
}
void fshr()
{
	switch(vm.current_resource)
	{
	case VM::heap: break;
	case VM::data: break;
	case VM::code: break;
	case VM::reg: break;
	}
}
void lshr()
{
	// Does it make sense?
}
void bnot()
{
	switch(vm.current_resource)
	{
	case VM::heap: vm.mem.heap[arg[0]] = ~arg[1]; break;
	case VM::data: vm.mem.data[arg[0]] = ~arg[1]; break;
	case VM::code: vm.c[arg[0]] = ~arg[1]; break;
	case VM::reg: vm.regs[arg[0]] = ~arg[1]; break;
	}
}


#define ADDR_MODE(i)\
auto meta##i = vm.c[vm.ip++];\
res[i] = meta##i >> AMODE_SIZE;\
mode[i] = meta##i & 0b00011111;

#define OPERAND_IN(i) arg[i] = exec_argument(vm.c[vm.ip++], mode[i], res[i]);
#define OPERAND_OUT(i) arg[i] = exec_argument(vm.c[vm.ip++], mode[i], res[i]);

void unary_instr(const std::string& mnemonic)
{
	std::cout << mnemonic << " " << (int)res[0] << " " << (int)mode[0] << ", " << (int)arg[0] << "\n";
}
void binary_instr(const std::string& mnemonic)
{
	std::cout << mnemonic << " " << (int)res[0] << " " << (int)mode[0]
						  << " " << (int)res[1] << " " << (int)mode[1] << ", " << (int)arg[0] << ", " << (int)arg[1] << "\n";
}
void ternary_instr(const std::string& mnemonic)
{
	std::cout << mnemonic << " " << (int)res[0] << " " << (int)mode[0]
						  << " " << (int)res[1] << " " << (int)mode[1]
						  << " " << (int)res[2] << " " << (int)mode[2] << ", " << (int)arg[0] << ", " << (int)arg[1] << ", " << (int)arg[2] << "\n";
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
		case 0xDA:
			vm.current_resource = VM::data;
			vm.ip++;
			break;
		default:
			vm.current_resource = VM::heap;
			break;
		}
		switch(vm.c[vm.ip++])
		{
		case OP_STPUSH:
		{
			ADDR_MODE(0)
			OPERAND_IN(0)
			stpush();
			if(vm.debug_mode) unary_instr("stpush");
			break;
		}
		case OP_STPOP:
		{
			ADDR_MODE(0)
			OPERAND_OUT(0)
			stpop();
			if(vm.debug_mode) unary_instr("stpop");
			break;
		}
		case OP_STTOP:
		{
			ADDR_MODE(0)
			OPERAND_OUT(0)
			sttop();
			if(vm.debug_mode) unary_instr("sttop");
			break;
		}
		case OP_STGET:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			stget();
			if(vm.debug_mode) binary_instr("stget");
			break;
		}
		case OP_INEG:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			ineg();
			if(vm.debug_mode) binary_instr("ineg");
			break;
		}
		case OP_FNEG:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(1)
			OPERAND_IN(0)
			fneg();
			if(vm.debug_mode) binary_instr("fneg");
			break;
		}
		case OP_LNEG:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			lneg();
			if(vm.debug_mode) binary_instr("lneg");
			break;
		}
		case OP_IADD:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			iadd();
			if(vm.debug_mode) ternary_instr("iadd");
			break;
		}
		case OP_FADD:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			fadd();
			if(vm.debug_mode) ternary_instr("fadd");
			break;
		}
		case OP_LADD:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			ladd();
			if(vm.debug_mode) ternary_instr("ladd");
			break;
		}
		case OP_ISUB:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			isub();
			if(vm.debug_mode) ternary_instr("isub");
			break;
		}
		case OP_FSUB:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			fsub();
			if(vm.debug_mode) ternary_instr("fsub");
			break;
		}
		case OP_LSUB:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			lsub();
			if(vm.debug_mode) ternary_instr("lsub");
			break;
		}
		case OP_IMUL:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			imul();
			if(vm.debug_mode) ternary_instr("imul");
			break;
		}
		case OP_FMUL:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			fmul();
			if(vm.debug_mode) ternary_instr("fmul");
			break;
		}
		case OP_LMUL:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			lmul();
			if(vm.debug_mode) ternary_instr("lmul");
			break;
		}
		case OP_IDIV:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			idiv();
			if(vm.debug_mode) ternary_instr("idiv");
			break;
		}
		case OP_FDIV:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			fdiv();
			if(vm.debug_mode) ternary_instr("fdiv");
			break;
		}
		case OP_LDIV:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			ldiv();
			if(vm.debug_mode) ternary_instr("ldiv");
			break;
		}
		case OP_IMOD:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			imod();
			if(vm.debug_mode) ternary_instr("imod");
			break;
		}
		case OP_IINC:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			iinc();
			if(vm.debug_mode) binary_instr("iinc");
			break;
		}
		case OP_FINC:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			finc();
			if(vm.debug_mode) binary_instr("finc");
			break;
		}
		case OP_LINC:
		{
			ADDR_MODE(0)
			OPERAND_OUT(0)
			linc();
			if(vm.debug_mode) binary_instr("linc");
			break;
		}
		case OP_IDEC:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			idec();
			if(vm.debug_mode) binary_instr("idec");
			break;
		}
		case OP_FDEC:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			fdec();
			if(vm.debug_mode) binary_instr("fdec");
			break;
		}
		case OP_LDEC:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			ldec();
			if(vm.debug_mode) binary_instr("ldec");
			break;
		}
		case OP_ISGN:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			isgn();
			if(vm.debug_mode) binary_instr("isgn");
			break;
		}
		case OP_FSGN:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			fsgn();
			if(vm.debug_mode) binary_instr("isgn");
			break;
		}
		case OP_LSGN:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			lsgn();
			if(vm.debug_mode) binary_instr("isgn");
			break;
		}
		case OP_IABS:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			iabs();
			if(vm.debug_mode) binary_instr("isgn");
			break;
		}
		case OP_FABS:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			fabs();
			if(vm.debug_mode) binary_instr("isgn");
			break;
		}
		case OP_LABS:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			labs();
			if(vm.debug_mode) binary_instr("isgn");
			break;
		}
		case OP_BOR:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			bor();
			if(vm.debug_mode) ternary_instr("bor");
			break;
		}
		case OP_BAND:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			band();
			if(vm.debug_mode) ternary_instr("band");
			break;
		}
		case OP_BXOR:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			bxor();
			if(vm.debug_mode) ternary_instr("bxor");
			break;
		}
		case OP_ISHL:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			ishl();
			if(vm.debug_mode) ternary_instr("ishl");
			break;
		}
		case OP_FSHL:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			fshl();
			if(vm.debug_mode) ternary_instr("fshl");
			break;
		}
		case OP_LSHL:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			lshl();
			if(vm.debug_mode) ternary_instr("lshl");
			break;
		}
		case OP_ISHR:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			ishr();
			if(vm.debug_mode) ternary_instr("ishr");
			break;
		}
		case OP_FSHR:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			fshr();
			if(vm.debug_mode) ternary_instr("fshr");
			break;
		}
		case OP_LSHR:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			ADDR_MODE(2)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			OPERAND_IN(2)
			lshr();
			if(vm.debug_mode) ternary_instr("lshr");
			break;
		}
		case OP_BNOT:
		{
			ADDR_MODE(0)
			ADDR_MODE(1)
			OPERAND_OUT(0)
			OPERAND_IN(1)
			bnot();
			if(vm.debug_mode) binary_instr("bnot");
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
	if(reg == "a64") return vm.regs[0];
	if(reg == "b64") return vm.regs[1];
	if(reg == "c64") return vm.regs[2];
	if(reg == "d64") return vm.regs[3];
	if(reg == "e64") return vm.regs[4];
	if(reg == "f64") return vm.regs[5];
	if(reg == "carry64") return vm.regs[6];
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
		vm.regs[i] = 0;
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

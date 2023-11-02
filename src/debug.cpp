#include<iostream>

#include "debug.hpp"

namespace GLERT
{

void debug(VM& vm, const std::string& name)
{
//	std::cout << "'" << name << "':\n";

//	for(std::size_t offset = 0; offset < vm.c->count;)
//	{
//		offset = debug_instr(vm, offset);
//	}
}

static void debug_address(std::size_t addr, const std::string& mod)
{
	if(mod == "vm:stack") std::cout << "[";
	std::cout << addr;
	if(mod == "vm:stack") std::cout << "]";
}

static std::size_t binary_instr(const std::string& op,
								VM& vm,
								std::size_t offset,
								std::size_t arg0_size,
								std::size_t arg1_size,
								const std::string& mod0,
								const std::string& mod1)
{
	std::cout << op << " ";
	std::size_t idx0 = 0, idx1 = 0;

	for(std::size_t i = 1; i <= arg0_size; i++)
	{
		std::cout << vm.c->code[offset + i] << " ";
		idx0 += vm.c->code[offset + i] << (8 * (arg0_size - i));
	}
	for(std::size_t i = 1; i <= arg1_size; i++)
	{
		std::cout << vm.c->code[offset + i] << " ";
		idx1 += vm.c->code[offset + i] << (8 * (arg1_size - i));
	}

	debug_address(idx0, mod0);
	std::cout << " ";
	debug_address(idx1, mod1);

	std::cout << "(";
	if(mod0 == "vm:stack" && vm.executing)
	{
		std::cout << "'";
		symtab_print(vm_stack_get(vm, idx0));
		std::cout << "'";
	}
	else if(mod0 == "symtab")
	{
		std::cout << "'";
		symtab_print(symtab_get(vm.c->stab, idx0));
		std::cout << "'";
	}
	std::cout << " ";
	if(mod1 == "vm:stack" && vm.executing)
	{
		std::cout << "'";
		symtab_print(vm_stack_get(vm, idx1));
		std::cout << "'";
	}
	else if(mod1 == "symtab")
	{
		std::cout << "'";
		symtab_print(symtab_get(vm.c->stab, idx1));
		std::cout << "'";
	}
	std::cout << ")\n";

	return offset + 1 + arg0_size + arg1_size;
}
static std::size_t binary_aa_instr(const std::string& op,
								   VM& vm,
								   std::size_t offset,
								   std::size_t arg0_size,
								   std::size_t arg1_size)
{
	return binary_instr(op, vm, offset, arg0_size, arg1_size, "vm:stack", "vm:stack");
}

static std::size_t binary_as_instr(const std::string& op,
								   VM& vm,
								   std::size_t offset,
								   std::size_t arg0_size,
								   std::size_t arg1_size)
{
	return binary_instr(op, vm, offset, arg0_size, arg1_size, "vm:stack", "symtab");
}

static std::size_t binary_sa_instr(const std::string& op,
								   VM& vm,
								   std::size_t offset,
								   std::size_t arg0_size,
								   std::size_t arg1_size)
{
	return binary_instr(op, vm, offset, arg0_size, arg1_size, "symtab", "vm:stack");
}

static std::size_t binary_ss_instr(const std::string& op,
								   VM& vm,
								   std::size_t offset,
								   std::size_t arg0_size,
								   std::size_t arg1_size)
{
	return binary_instr(op, vm, offset, arg0_size, arg1_size, "symtab", "symtab");
}

static std::size_t unary_instr(const std::string& op,
							   VM& vm,
							   std::size_t offset,
							   std::size_t arg_size,
							   const std::string& mod)
{
	std::cout << op << " ";
	std::size_t idx = 0;
	for(std::size_t i = 1; i <= arg_size; i++)
	{
		std::cout << vm.c->code[offset + i] << " ";
		idx += vm.c->code[offset + i] << (8 * (arg_size - i));
	}
	debug_address(idx, mod);
	std::cout << " ";
	if(vm.executing && mod == "vm:stack")
	{
		std::cout << "'";
		symtab_print(vm_stack_get(vm, idx));
		std::cout << "'";
	}
	else if(mod == "symtab")
	{
		std::cout << "'";
		symtab_print(symtab_get(vm.c->stab, idx));
		std::cout << "'";
	}
	else std::cout << "";
	std::cout << "\n";
	return offset + 1 + arg_size;
}
static std::size_t unary_addr_instr(const std::string& op,
									VM& vm,
									std::size_t offset,
									std::size_t arg_size)
{
	return unary_instr(op, vm, offset, arg_size, "vm:stack");
}
static std::size_t unary_stab_instr(const std::string& op,
									VM& vm,
									std::size_t offset,
									std::size_t arg_size)
{
	return unary_instr(op, vm, offset, arg_size, "symtab");
}
static std::size_t nullary_instr(const std::string& op,
								 std::size_t offset)
{
	std::cout << op << "\n";
	return offset + 1;
}
static std::size_t unknown_instr(std::uint8_t op,
								 std::size_t offset)
{
	std::cout << "<Unknown> " << op << "\n";
	return offset + 1;
}
std::size_t debug_instr(VM& vm,
						std::size_t offset)
{
	std::cout << offset << " ";

	if(offset > 0 && vm.c->lines[offset] == vm.c->lines[offset - 1]) std::cout << "   | ";
	else std::cout << vm.c->lines[offset] << " ";

	std::uint8_t instr = vm.c->code[offset];
//	switch(instr)
//	{
//		case OP_RET:   return nullary_instr("RET", offset);
//		case OP_RETA:  return unary_addr_instr("RETA", vm, offset, 1);
//		case OP_RETC:  return unary_stab_instr("RETC", vm, offset, 1);
//		case OP_LOAD:  return unary_stab_instr("LOAD", vm, offset, 1);
//		case OP_ADDA:  return binary_aa_instr("ADDA", vm, offset, 1, 1);
//		case OP_SUBA:  return binary_aa_instr("SUBA", vm, offset, 1, 1);
//		case OP_MULTA: return binary_aa_instr("MULTA", vm, offset, 1, 1);
//		case OP_DIVA:  return binary_aa_instr("DIVA", vm, offset, 1, 1);
//		case OP_NEGA:  return nullary_instr("NEGA", offset);
//		case OP_NOTA:  return nullary_instr("NOTA", offset);
//		case OP_CMPA:  return binary_aa_instr("CMPA", vm, offset, 1, 1);
//		case OP_CMPC:  return binary_ss_instr("CMPC", vm, offset, 1, 1);
//		case OP_EQA:   return binary_aa_instr("EQA", vm, offset, 1, 1);
//		case OP_EQC:   return binary_ss_instr("EQC", vm, offset, 1, 1);
//		case OP_NEQA:  return binary_aa_instr("NEQA", vm, offset, 1, 1);
//		case OP_NEQC:  return binary_ss_instr("NEQC", vm, offset, 1, 1);
//		case OP_GTA:   return binary_aa_instr("GTA", vm, offset, 1, 1);
//		case OP_GTC:   return binary_ss_instr("GTC", vm, offset, 1, 1);
//		case OP_LSA:   return binary_aa_instr("LSA", vm, offset, 1, 1);
//		case OP_LSC:   return binary_ss_instr("LSC", vm, offset, 1, 1);
//		case OP_GTEA:  return binary_aa_instr("GTEA", vm, offset, 1, 1);
//		case OP_GTEC:  return binary_ss_instr("GTEC", vm, offset, 1, 1);
//		case OP_LSEA:  return binary_aa_instr("LSEA", vm, offset, 1, 1);
//		case OP_LSEC:  return binary_ss_instr("LSEC", vm, offset, 1, 1);
//		default:       return unknown_instr(instr, offset);
//	}
}
}

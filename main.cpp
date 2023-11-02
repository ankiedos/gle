#include<iostream>
#include<string>
#include "common.hpp"
#include "opcode.hpp"
//#include "debug.hpp"
#include "vm.hpp"

int main(int argc, char** argv)
{
	std::string is_debug_mode = "false";
	for(std::size_t i = 1; i < argc; i++)
	{
		if(argv[i] == std::string{"--debug"} && i != argc - 1)
		{
			is_debug_mode = argv[i + 1];
			i++;
		}
	}
	bool debug = (is_debug_mode == "true");
	GLERT::vm_init(debug);
	auto a64 = GLERT::get_reg_addr("a64");
	auto b64 = GLERT::get_reg_addr("b64");
	std::vector<GLERT::word> code = {
		GLERT::OP_STPUSH, GLERT::AMODE_IMM, 1,
		GLERT::OP_STTOP, GLERT::AMODE_REG, a64,
		GLERT::OP_STPOP, GLERT::AMODE_REG, b64,
		GLERT::OP_STPUSH, GLERT::AMODE_REG, a64,
		GLERT::OP_STGET, GLERT::AMODE_REG, GLERT::AMODE_IMM, 0, 0,
		GLERT::OP_INEG, GLERT::AMODE_REG, b64
	};
	GLERT::vm_link(code);
	GLERT::vm_run();
	GLERT::zero_regs();
	GLERT::iadd(a64, 0, 1);
	GLERT::isub(a64, GLERT::get_reg("a64"), 2);
	GLERT::isgn(GLERT::get_reg("a64"));
	GLERT::linc(a64);
	GLERT::ldec(b64);
	GLERT::ladd(a64, GLERT::get_reg("a64"), GLERT::get_reg("b64"));
	std::cout << GLERT::stack_size() << "\n";
	for(std::size_t i = GLERT::stack_size(); i > 0; i--)
	{
		std::cout << (int)GLERT::get_stack(i - 1) << "\n";
	}
	std::cout << "\n";
	std::cout << "a64 = " << (int)GLERT::get_reg("a64") << "\n";
	std::cout << "b64 = " << (int)GLERT::get_reg("b64") << "\n";
	std::cout << "c64 = " << (int)GLERT::get_reg("c64") << "\n";
	std::cout << "d64 = " << (int)GLERT::get_reg("d64") << "\n";
	std::cout << "e64 = " << (int)GLERT::get_reg("e64") << "\n";
	std::cout << "f64 = " << (int)GLERT::get_reg("f64") << "\n";
	std::cout << "carry = " << (int)GLERT::get_reg("carry64") << "\n";
	GLERT::vm_free();

	return 0;
}

#include<iostream>
#include<string>
#include<fstream>
#include<sstream>

#include "common.hpp"
#include "opcode.hpp"
//#include "debug.hpp"
#include "vm.hpp"

std::vector<GLERT::word> read_code(const std::string& fpath)
{
	std::ifstream ifs;
	std::string code_str{};
	ifs.open(fpath, std::ios::binary);
	std::stringstream str{};
	str << ifs.rdbuf();
	code_str = str.str();
	std::vector<GLERT::word> code{};
	for(const auto& ch : code_str)
	{
		code.push_back(ch);
	}
	return code;
}
int main(int argc, char** argv)
{
	std::string is_debug_mode = "false";
	std::string fpath = "main.gle";
	for(std::size_t i = 1; i < argc; i++)
	{
		if(argv[i] == std::string{"--debug"} && i != argc - 1)
		{
			is_debug_mode = argv[i + 1];
			i++;
		}
		else if(argv[i] == std::string{"--file"} && i != argc - 1)
		{
			fpath = argv[i + 1];
			i++;
		}
	}
	bool debug = (is_debug_mode == "true");
	GLERT::vm_init(debug);
	auto code = read_code(fpath);
	GLERT::vm_link(code);
	GLERT::vm_run();
	
	if(debug)
	{
		std::cout << "======================\n";
		std::cout << GLERT::list_regs();
		std::cout << "\nstack:\n";
		std::cout << GLERT::list_stack();
	}
	GLERT::vm_free();

	return 0;
}

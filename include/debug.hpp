#ifndef GLE2_DEBUG_HPP
#define GLE2_DEBUG_HPP

#include<string>
#include "opcode.hpp"
#include "vm.hpp"

namespace GLERT
{
void debug(VM& vm, const std::string& name);
std::size_t debug_instr(VM& vm, std::size_t offset);
}


#endif

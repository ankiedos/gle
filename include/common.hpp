#ifndef GLE2_COMMON_HPP
#define GLE2_COMMON_HPP

#include<cstddef>
#include<cstdint>
#include<cstdlib>

#define GROW_SIZE(size) ((size) < 8 ? 8 : (size) * 3 / 2)

namespace GLERT
{
using word = char;
enum exec_res
{
	EXEC_SUCC, EXEC_GNR_ERR, EXEC_OPRND_ERR,
	EXEC_ALLOCED_MEM, EXEC_VMEM_LACK, EXEC_MEM_LACK,
	EXEC_UNKNOWN_OPCODE
};
}

#endif

#include<iostream>
#include<string>

#include "common.hpp"
#include "value.hpp"
#include "kreczmar.hpp"
#include "vm.hpp"

namespace GLERT
{



bool value_equal(const value& val0, const value& val1)
{
	if(vm_typecheck(val0, val1)) return false;
	switch(val0.tp)
	{
	case T_BOOL: return AS_BOOL(val0) == AS_BOOL(val1);
	case T_NULL: return true;
	case T_U64:  return AS_U64(val0) == AS_U64(val1);
	case T_REAL: return AS_REAL(val0) == AS_REAL(val1);
	case T_WORD: return AS_WORD(val0) == AS_WORD(val1);
	case T_UNIT: return true;
	case T_ADDR: return AS_OBJ(val0).b == AS_OBJ(val1).b
	                    && AS_OBJ(val0).counter == AS_OBJ(val1).counter;
	default: return true; // unreachable
	}
}

}

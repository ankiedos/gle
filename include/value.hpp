#ifndef GLE2_VALUE_HPP
#define GLE2_VALUE_HPP

#include<vector>
#include "common.hpp"
#include "kreczmar.hpp"

namespace GLERT
{
struct object;
struct string;
enum type
{
	T_BOOL,
	T_UNIT,
	T_U64,
	T_REAL,
	T_WORD,
	T_NULL,
	T_ADDR
};

// REPRESENTATIONS IN MEMORY:
// o string:
// sizeptr chars...
// o array:
// sizeptr values...
// o int:
// value
// o real:
// value
// o bool:
// value
// o null:
// value
// o unit:
// doesn't have
// o object (address):
// sizeptr tpname fields and method ptrs...
// o subprogram:
// code...





struct value
{
	type tp;
	union
	{
		unsigned bool_null_unit : 8;
		word u8;
		word byte;
		double real;
		Kreczmar::ind_addr addr;
	} as;
};
#define BOOLV(val) (GLERT::value){GLERT::T_BOOL, {.bool_null_unit = val}}
#define NULLV      (GLERT::value){GLERT::T_NULL, {.bool_null_unit = 0}}
#define UNITV      (GLERT::value){GLERT::T_UNIT, {.bool_null_unit = 0b00000001}}
#define U64V(val)  (GLERT::value){GLERT::T_U64, {.u8 = val}}
#define REALV(val) (GLERT::value){GLERT::T_REAL, {.real = val}}
#define WORDV(val) (GLERT::value){GLERT::T_WORD, {.byte = val}}
#define OBJV(val)  (GLERT::value){GLERT::T_ADDR, {.addr = (ind_addr*)val}}

#define AS_BOOL(val) ((val).as.bool_null_unit)
#define AS_U64(val)  ((val).as.u8)
#define AS_REAL(val) ((val).as.real)
#define AS_WORD(val) ((val).as.byte)
#define AS_OBJ(val)  ((val).as.addr)

#define IS_BOOL(val) ((val).tp == T_BOOL)
#define IS_UNIT(val) ((val).tp == T_UNIT)
#define IS_NULL(val) ((val).tp == T_NULL)
#define IS_U64(val)  ((val).tp == T_U64)
#define IS_REAL(val) ((val).tp == T_REAL)
#define IS_WORD(val) ((val).tp == T_WORD)
#define IS_OBJ(val)  ((val).tp == T_ADDR)

}

#endif

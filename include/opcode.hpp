#ifndef GLE2_OPCODE_HPP
#define GLE2_OPCODE_HPP

#include<vector>
#include<string>
#include<iostream>

#include "common.hpp"
#include "value.hpp"

namespace GLERT
{
// extensions prefixed by 0xE0 - 0xEE
// operations can be prefixed with 0xCC (204) to make them modify the code
enum opcode // 0-138 instrs
{
	// STACK OPERATIONS
	OP_STPUSH = 0, OP_STPOP = 1, OP_STTOP = 2, OP_STGET = 3,

	// NUMBER OPERATIONS BINARY
	OP_INEG = 4, OP_FNEG = 5, OP_LNEG = 6, // LNEG=LNOT
	OP_IADD = 7, OP_FADD = 8, OP_LADD = 9, // LADD=LOR
	OP_ISUB = 10, OP_FSUB = 11, OP_LSUB = 12, // LSUB=|p - q|=LXOR
	OP_IMUL = 13, OP_FMUL = 14, OP_LMUL = 15, // LMUL=LAND
	OP_IDIV = 16, OP_FDIV = 17, OP_LDIV = 18, // LDIV=implication
	OP_IMOD = 19,
	// NUMBER OPERATIONS UNARY
	OP_IINC = 20, OP_FINC = 21, OP_LINC = 22, // LINC=LTRUE
	OP_IDEC = 23, OP_FDEC = 24, OP_LDEC = 25, // LDEC=LFALSE
	OP_ISGN = 26, OP_FSGN = 27, OP_LSGN = 28, // LSGN=L2I(p) - L2I(~p)
	OP_IABS = 29, OP_FABS = 30, OP_LABS = 31, // LABS=L2I


	// BITWISE & LOGICAL. BITWISE AND LOGICAL OPERATIONS BINARY
	// bitwise   boolean
	OP_BOR = 32,   OP_LOR  = OP_LADD,
	OP_BAND = 33,  OP_LAND = OP_LMUL,
	OP_BXOR = 34,  OP_LXOR = OP_LSUB,
	// arithm.   logical
	OP_ISHL = 35,  OP_FSHL = 36, OP_LSHL = 37,
	OP_ISHR = 38,  OP_FSHR = 39, OP_LSHR = 40,
	// BITWISE AND LOGICAL OPERATIONS UNARY
	// bitwise   boolean
	OP_BNOT = 41,  OP_LNOT = OP_LNEG,
	// boolean       boolean
	OP_LTRUE = OP_LINC, OP_LFALSE = OP_LDEC,

	// COMPOUND OPERATIONS (0xE0)
	// string    array     address
	OP_SNEW = 42,  OP_ARNEW = 43, OP_ANEW = 44,
	OP_SKLL = 45,  OP_ARKLL = 46, OP_AKLL = 47,
	OP_SGET = 48,  OP_ARGET = 49, OP_AOFF = 50,
	OP_SSET = 51,  OP_ARSET = 52, OP_ASET = 53,
	OP_SPRE = 54,  OP_ARPRE = 55,
	OP_SINS = 56,  OP_ARINS = 57,
	OP_SAPP = 58,  OP_ARAPP = 59,
	OP_SPOPF = 60, OP_ARPOPF = 61,
	OP_SDEL = 62,  OP_ARDEL = 63,
	OP_SPOPB = 64, OP_ARPOPB = 65,
	OP_SFND = 66,  OP_ARFND = 67, OP_ADEREF = 68,
	OP_SCCAT = 69, OP_ARCCAT = 70,         // concatenation
	OP_SSLC = 71,  OP_ARSLC = 72, OP_ASLC = 73, // slice; ASLC gives a direct ptr to the specified offset

	// MOVE OPERATIONS (assign by ref)
	// string    array     address
	OP_SMOV = 74,  OP_ARMOV = 75, OP_AMOV = 76,
	// int       real      boolean
	OP_IMOV = 77,  OP_FMOV = 78,  OP_LMOV = 79,

	// COPY OPERATIONS (assign by val)
	// string    array     address
	OP_SCP = 80,   OP_ARCP = 81,  OP_ACP = 82,
	// int       real      boolean
	OP_ICP = 83,   OP_FCP = 84,   OP_LCP = 85,

	// ARRAY OPERATIONS BINARY (0xE1)
	// int      real
	OP_ARIADD = 86,  OP_ARFADD = 87,
	OP_ARISUB = 88,  OP_ARFSUB = 89,
	OP_ARIMUL = 90,  OP_ARFMUL = 91,
	OP_ARIDIV = 92,  OP_ARFDIV = 93,
	OP_ARIMODI = 94,                // 2nd operand is an integer
	OP_ARIMODAR = 95,               // 2nd operand is an array
	OP_ARISCLI = 96,  OP_ARFSCLI = 97,   // scale all elements by k (k in either N or Z)
	OP_ARISCLF = 98,  OP_ARFSCLF = 99,   // scale all elements by k (k in R)
	// ARRAY OPERATIONS UNARY (0xE1)
	OP_ARIABS = 100,  OP_ARFABS = 101,
	OP_ARISGN = 102,  OP_ARFSGN = 103,     // get an array of signs of all elements
	OP_ARISCAL = 104, OP_ARFSCAL = 105,    // perform scalar product
	OP_ARIZER = 106,  OP_ARFZER = 107,

	// CONTROL FLOW OPERATIONS
	OP_JMP = 108,
	OP_CMP = 109,
	// truness    falsehood
	OP_JMPTRU = 110, OP_JMPFLS = 111,
	OP_JMPEQ = 112,  OP_JMPNEQ = 113,
	OP_JMPGT = 114,  OP_JMPLSE = 115,
	OP_JMPLS = 116,  OP_JMPGTE = 117,
	OP_JMPZR = 118,  OP_JMPNZR = 119,
	OP_RET = 120,    OP_THROW = 121,

	// TYPE CAST OPERATIONS (0xE2)
	OP_I2F = 122,
	OP_F2I = 123, OP_L2I = OP_LABS,
	OP_I2S = 124, OP_F2S = 125, OP_L2S = 126,
	OP_I2A = 127, OP_F2A = 128, OP_L2A = 129,
	// Instructions of the form A2T, T=type are nonexistent.
	// Use ADEREF instead.
	// Possibly for future:
	// OP_A2I = OP_ADEREF, OP_A2F = OP_ADEREF, OP_A2L=OP_ADEREF

	// SPECIAL OPERATIONS
	OP_SYSCALL = 130, OP_BREAKPOINT = 131, OP_NOP = 132, OP_HOST_ISA = 133, OP_HOST_OS = 134,
	OP_ISWP = 135, OP_FSWP = 136, OP_LSWP = 137,
	OP_ASWP = 138
};

enum addr_mode
{
	AMODE_IMM, AMODE_REG, AMODE_PTR, AMODE_REMOTE, AMODE_LOCAL, AMODE_LOCALPTR
};

}

#endif

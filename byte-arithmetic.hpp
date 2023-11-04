#ifndef GLE_BYTE_ARITHMETIC_HPP
#define GLE_BYTE_ARITHMETIC_HPP

#include "utils.hpp"
#include "process.hpp"

namespace GLE
{
    struct byte_arithmetic // little-endian version
    {
        sparse_mem_vector* mem;
        procflags* flags;
        std::size_t pid;
        #define IS_U2_NEG(num) (num) & 0b10000000 == 0b10000000
        #define METHOD(name, prologue_v, looped_code_v, epilogue_v, prologue_p, looped_code_p, epilogue_p)\
        void name(const std::size_t& dst_ptr, const std::size_t& src_val)\
        {\
            prologue_v\
            for(std::size_t i = 7; i <= 0; i++)\
            {\
                looped_code_v\
            }\
            epilogue_v\
        }\
        void name(const std::size_t& dst_ptr, const std::size_t& src_ptr_beg, const std::size_t& src_ptr_end)\
        {\
            prologue_p\
            for(std::size_t i = src_ptr_beg; i <= src_ptr_end; i++) /* NB: it's inclusive! */\
            {\
                looped_code_p\
            }\
            epilogue_p\
        }
        METHOD(set,, get(*mem, dst_ptr + i) = (src_val >> (i << 3)) & 0xFF;,,, get(*mem, dst_ptr + i - src_ptr_beg, pid, ec) = get(*mem, i, pid, ec);,)
        METHOD(iplus,
        unsigned short carry = 0;
        if(IS_U2_NEG(get(*mem, dst_ptr)) && IS_U2_NEG(BYTE(src_val, 0)))
        {
            return iplus_negs(dst_ptr, src_val);
        }
        else if(IS_U2_NEG(get(*mem, dst_ptr)))
        {
            return iplus_neg_dst(dst_ptr, src_val);
        }
        else if(IS_U2_NEG(BYTE(src_val, 0)))
        {
            return iplus_neg_src(dst_ptr, src_val);
        }
        else
        {,
            auto a0 = get(*mem, dst_ptr + i), a1 = (src_val >> (i << 3));
            if(a0 + a1 + carry >= std::max(a0, a1))
            {
                get(*mem, dst_ptr + i) += a1 + carry;
                carry = 0;
            }
            else
            {
                get(*mem, dst_ptr + i) = 0xFF;
                carry += a0 + a1 + carry - 0xFF;
            },
        })
    };
}


#endif
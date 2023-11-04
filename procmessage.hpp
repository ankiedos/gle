#ifndef GLE_PROCMESSAGE_HPP
#define GLE_PROCMESSAGE_HPP

#include<string>
#include<vector>

namespace GLE
{
    struct procmessage
    {
        enum msgtype { msg_i32, msg_i64, msg_u32, msg_u64, msg_f32, msg_f64, msg_c8, msg_str, msg_obj, msg_msg, msg_bit };
        union msgval
        {
            std::int32_t i32;
            std::uint32_t u32;
            std::int64_t i64;
            std::uint64_t u64;
            float f32;
            double f64;
            char c8;
            std::string str;
            void* obj;
            procmessage* procmsg;
            unsigned bit : 1;
        };
        msgtype type;
        std::vector<msgval> val;
    };
}


#endif
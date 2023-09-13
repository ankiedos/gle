#ifndef GLE_PROCESS_HPP
#define GLE_PROCESS_HPP

#include<limits>
#include<queue>

#include "procmessage.hpp"

namespace GLE
{
    struct procflags
    {
        unsigned zerof : 1;
        unsigned carryf : 1;
        unsigned greaterf : 1;
        unsigned lessf : 1;
        unsigned greatereqf : 1;
        unsigned lesseqf : 1;
        unsigned positivef : 1;
        unsigned stackoverf : 1;
        unsigned stackunderf : 1;
        unsigned syscallf : 1;
        unsigned callf : 1;
        unsigned jmpf : 1;
        unsigned reserved : 4;
    };
    struct process
    {
        std::size_t pid;
        std::size_t ip = 0;
        procflags flags;
        std::queue<procmessage> msgqueue;
        std::size_t file_descrs[std::numeric_limits<std::size_t>::max()];
        std::size_t sockets[std::numeric_limits<std::size_t>::max()];
    };
}


#endif
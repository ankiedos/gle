#ifndef GLE_EXEC_HPP
#define GLE_EXEC_HPP

#include<fstream>
#include<string>
#include<vector>
#include<stack>

#include "utils.hpp"
#include "process.hpp"
#include "byte-arithmetic.hpp"

namespace GLE
{
    enum opcode
    {
        OP_IPLUS, OP_IMINUS, OP_IMULT, OP_IDIV, OP_FPLUS, OP_FMINUS, OP_FMULT, OP_FDIV,
        OP_RET, OP_CALL, OP_JMP, OP_CMP, OP_SYSCALL, OP_LDR, OP_STR, OP_LD, OP_SPAWN, OP_PUSH, OP_POP,
        OP_INC, OP_DEC, OP_NEG, OP_NOT, OP_AND, OP_OR, OP_XOR,
        OP_CO_BREAK, OP_CO_CONTINUE // coroutine-related
    };
    struct RT
    {
        process* curr_proc;
        std::vector<process> processes;
        byte_arithmetic eval;
        int exec()
        {
            int ec = 0;
            uint64_t arg0 = 0, arg1 = 0, arg2 = 0;
            bool jumped = true;
            while(true)
            {
                #define AT(x) get(mem, x, curr_proc->pid, ec)
                #define LOOP(n) for(std::size_t i = instr_addr + 1 + (8 * (n + 1)); i >= instr_addr + 2 + (8 * n); i--)\
                        {\
                            arg0 += curr_proc->code[i] << (i - instr_addr - 9);\
                        }
                #define INSTR2_EPILOGUE(n) arg0 = arg1 = 0;\
                curr_proc->ip += (n);
                #define INSTR1_EPILOGUE(n) arg0 = 0;\
                curr_proc->ip += (n);

                auto instr_addr = curr_proc->ip;
                switch(curr_proc->code[instr_addr])
                {
                case OP_IPLUS:
                    break;
                case OP_IMINUS:
                    break;
                case OP_IMULT:
                    break;
                case OP_IDIV:
                    break;
                case OP_FPLUS:
                    break;
                case OP_FMINUS:
                    break;
                case OP_FMULT:
                    break;
                case OP_FDIV:
                    break;
                case OP_RET: // the result is stored on the top of the stack
                    for(std::size_t i = 0; i < 8; i++)
                    {
                        arg0 += (stack.top() << (8 * i));
                        stack.pop();
                    }
                    for(std::size_t i = 0; i < 8; i++)
                    {
                        arg1 += (stack.top() << (8 * i));
                        stack.pop();
                    }
                    stack.push(arg0);
                    curr_proc->ip = arg1;
                    INSTR2_EPILOGUE(0)
                    break;
                case OP_CALL:
                    LOOP(0)
                    for(std::size_t i = 0; i < 8; i++)
                    {
                        // Must start pushing from MSB for later RET properly interpreting this data
                        stack.push(((curr_proc->ip + 2) >> (8 * i)) & (0xFF << (8 * (8 - i))));
                    }
                    curr_proc->ip = arg0;
                    INSTR2_EPILOGUE(0)
                    break;
                case OP_JMP:
                    LOOP(0)
                    LOOP(1)
                    switch(arg0)
                    {
                    case 0x00: // short jump
                        curr_proc->ip += arg1;
                        break;
                    case 0x01: // long jump
                        curr_proc->ip = arg1;
                        break;
                    default: break;
                    }
                    jumped = true;
                    INSTR2_EPILOGUE(0)
                    break;
                case OP_CMP:
                    break;
                case OP_SYSCALL:
                    switch(arg0)
                    {
                    case 0x00: // printchr
                    case 0x01: // inputbyte
                    case 0x02: // inputbyte (with echo)
                    case 0x03: // timenow
                    case 0x04: // openfile
                    case 0x05: // closefile
                    case 0x06: // writefile
                    case 0x07: // readfile
                    case 0x08: // inetconnect
                    case 0x09: // inetdisconnect
                    case 0x0A: // inetsend
                    case 0x0B: // inetrecv
                    case 0x0C: // writepxl
                    case 0x0D: // writeline
                    case 0x0E: // writerect
                    case 0x0F: // writecircle
                    case 0x10: // getmousepos
                    case 0x11: // playsnd
                    case 0x12: // pause
                    case 0x13: // system/platform metadata
                    default: break;
                    }
                    break;
                case OP_LDR:
                case OP_STR:
                case OP_LD:
                case OP_SPAWN:
                case OP_PUSH:
                    break;
                case OP_INC:
                    break;
                case OP_DEC:
                case OP_NEG:
                case OP_NOT:
                case OP_AND:
                case OP_OR:
                case OP_XOR:
                case OP_CO_BREAK:
                case OP_CO_CONTINUE:
                default:
                }
                if(!jumped) curr_proc->ip++;
            }
        }
        int load(std::ifstream& ifs)
        {
            ifs.seekg(0, ifs.end);
            auto size = ifs.tellg();
            ifs.seekg(0, ifs.beg);
            std::string str{};
            ifs.read(str.data(), size);
            int ec = mem.set(curr_proc->pid, 0, size, str);
            eval.mem = mem;
            eval.flags = &(curr_proc->flags);
            eval.pid = curr_proc->pid;
            return ec;
        }
    };
}

#endif

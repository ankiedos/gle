#include<fstream>
#include "opcode.hpp"

std::ofstream ofs{"tests/calc.gle"};

// CMD FOR THE OUTPUT FILE:
// ./bin/glert --debug true --file ./tests/calc.gle
// IN THE ROOT OF THIS REPO

void write(GLERT::word byte)
{
    const char* b = reinterpret_cast<char*>(&byte);
    ofs.write(b, 1);
}


int main(int argc, char** argv)
{
    write(GLERT::OP_STPUSH); // STPUSH
    write(GLERT::AMODE_IMM); // AMODE_IMM
    write(1); // 1
    write(0xFF); // RES REG
    write(GLERT::OP_STTOP); // STTOP
    write(GLERT::AMODE_REG); // AMODE_REG
    write(0); // a64
    write(0xFF); // RES REG
    write(GLERT::OP_IADD); // IADD
    write(GLERT::AMODE_REG); // AMODE_REG
    write(GLERT::AMODE_IMM); // AMODE_IMM
    write(GLERT::AMODE_IMM); // AMODE_IMM
    write(0); // a64
    write(1); // 1
    write(2); // 2
    ofs.close();
    return 0;
}
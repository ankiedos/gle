#include<fstream>
#include "opcode.hpp"

std::ofstream ofs{"tests/calc.gle"};

// CMD FOR THE OUTPUT FILE:
// ./bin/glert --debug true --file ./tests/calc.gle
// IN THE ROOT OF THIS REPO

void write(GLERT::word byte)
{
    char c = static_cast<char>(byte);
    const char* b = &c;
    ofs.write(b, 1);
}

int main(int argc, char** argv)
{
    write(GLERT::OP_STPUSH); // STPUSH
    write((GLERT::RES_LIT << GLERT::AMODE_SIZE) | GLERT::AMODE_VAL); // AMODE_IMM
    write(1); // 1
    write(0xFF); // CURR RES REG
    write(GLERT::OP_STTOP); // STTOP
    write((GLERT::RES_REG << GLERT::AMODE_SIZE) | GLERT::AMODE_VAL); // AMODE_REG
    write(0); // a64
    write(0xFF); // RES REG
    write(GLERT::OP_IADD); // IADD
    write((GLERT::RES_REG << GLERT::AMODE_SIZE) | GLERT::AMODE_VAL); // AMODE_REG
    write((GLERT::RES_LIT << GLERT::AMODE_SIZE) | GLERT::AMODE_VAL); // AMODE_IMM
    write((GLERT::RES_LIT << GLERT::AMODE_SIZE) | GLERT::AMODE_VAL); // AMODE_IMM
    write(0); // a64
    write(1); // 1
    write(2); // 2
    ofs.close();
    return 0;
}
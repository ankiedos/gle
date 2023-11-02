#include<fstream>

int main(int argc, char** argv)
{
    std::ofstream ofs{"tests/calc.gle"};
    ofs.write("\0", 1); // STPUSH
    ofs.write("\0", 1); // AMODE_IMM
    ofs.write("\1", 1); // 1
    ofs.write("\2", 1); // STTOP
    ofs.write("\1", 1); // AMODE_REG
    ofs.write("\0", 1); // a64
    ofs.close();
    return 0;
}
#include "CrossLang.hpp"

int main(int argc, char** argv)
{
    std::vector<std::string> args(argc);
    for(int i = 0; i < argc; i++)
        args[i] = argv[i];
    return (int)Tesses::CrossLang::Programs::CrossArchiveCreate(args);
}
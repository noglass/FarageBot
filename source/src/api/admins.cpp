#include <string>
#include "api/admins.h"

Farage::AdminFlag Farage::getAdminFlagBits(const std::string &flags)
{
    static int FLAGS[26] = { 1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576,2097152,4194304,8388608,16777216,33554431 };
    Farage::AdminFlag bits = NOFLAG;
    for (auto it = flags.begin(), ite = flags.end();it != ite;++it)
        if (std::isalpha(*it))
            bits |= FLAGS[*it-97];
    return bits;
}

std::string Farage::getAdminFlagString(Farage::AdminFlag flagbits)
{
    static int FLAGS[25] = { 1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576,2097152,4194304,8388608,16777216 };
    std::string flags = "";
    if ((flagbits & ROOT) == ROOT)
        flags = "z";
    else for (int i = 0;i < 25;i++)
        if ((flagbits & FLAGS[i]) == FLAGS[i])
            flags.append(1,char(97+i));
    return flags;
}


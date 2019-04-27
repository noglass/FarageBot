#ifndef _FARAGE_ADMIN_
#define _FARAGE_ADMIN_

#include <string>

namespace Farage
{
    enum AdminFlag
    {
        NOFLAG                =        0,    // n/a
        GENERIC               =        1,    // a
        BAN                   =        2,    // b
        EDITCHAN              =        4,    // c
        DELCHAN               =        8,    // d
        EDITMSG               =       16,    // e
        NICK                  =       32,    // f
        ROLE                  =       64,    // g
        UNBAN                 =      128,    // h
        STATUS                =      256,    // i
        PIN                   =      512,    // j
        KICK                  =     1028,    // k
        VOICE                 =     2048,    // l
        GAG                   =     4096,    // m
        CONFIG                =     8192,    // n
        CUSTOM1               =    16384,    // o
        CUSTOM2               =    32768,    // p
        CUSTOM3               =    65536,    // q
        RCON                  =   131072,    // r
        CUSTOM4               =   262144,    // s
        CUSTOM5               =   524288,    // t
        CUSTOM6               =  1048576,    // u
        CUSTOM7               =  2097152,    // v
        CUSTOM8               =  4194304,    // w
        CUSTOM9               =  8388608,    // x
        CUSTOM10              = 16777216,    // y
        ROOT                  = 33554431     // z
    };
    inline AdminFlag operator~(AdminFlag a)
    {
        return static_cast<AdminFlag>(~static_cast<int>(a));
    }
    inline AdminFlag operator|(AdminFlag a, AdminFlag b)
    {
        return static_cast<AdminFlag>(static_cast<int>(a) | static_cast<int>(b));
    }
    inline AdminFlag operator|(AdminFlag a, int b)
    {
        return static_cast<AdminFlag>(static_cast<int>(a) | b);
    }
    inline int operator|(int a, AdminFlag b)
    {
        return a | static_cast<int>(b);
    }
    inline AdminFlag& operator|=(AdminFlag &a, AdminFlag b)
    {
        return reinterpret_cast<AdminFlag&>(reinterpret_cast<int&>(a) |= static_cast<int>(b));
    }
    inline AdminFlag& operator|=(AdminFlag &a, int b)
    {
        return reinterpret_cast<AdminFlag&>(reinterpret_cast<int&>(a) |= b);
    }
    inline int& operator|=(int &a, AdminFlag b)
    {
        return a |= static_cast<int>(b);
    }
    inline AdminFlag operator&(AdminFlag a, AdminFlag b)
    {
        return static_cast<AdminFlag>(static_cast<int>(a) & static_cast<int>(b));
    }
    inline AdminFlag operator&(AdminFlag a, int b)
    {
        return static_cast<AdminFlag>(static_cast<int>(a) & b);
    }
    inline int operator&(int a, AdminFlag b)
    {
        return a & static_cast<int>(b);
    }
    inline AdminFlag& operator&=(AdminFlag &a, AdminFlag b)
    {
        return reinterpret_cast<AdminFlag&>(reinterpret_cast<int&>(a) &= static_cast<int>(b));
    }
    inline AdminFlag& operator&=(AdminFlag &a, int b)
    {
        return reinterpret_cast<AdminFlag&>(reinterpret_cast<int&>(a) &= b);
    }
    inline int& operator&=(int &a, AdminFlag b)
    {
        return a &= static_cast<int>(b);
    }
    inline AdminFlag operator^(AdminFlag a, AdminFlag b)
    {
        return static_cast<AdminFlag>(static_cast<int>(a) ^ static_cast<int>(b));
    }
    inline AdminFlag operator^(AdminFlag a, int b)
    {
        return static_cast<AdminFlag>(static_cast<int>(a) ^ b);
    }
    inline int operator^(int a, AdminFlag b)
    {
        return a ^ static_cast<int>(b);
    }
    inline AdminFlag& operator^=(AdminFlag &a, AdminFlag b)
    {
        return reinterpret_cast<AdminFlag&>(reinterpret_cast<int&>(a) ^= static_cast<int>(b));
    }
    inline AdminFlag& operator^=(AdminFlag &a, int b)
    {
        return reinterpret_cast<AdminFlag&>(reinterpret_cast<int&>(a) ^= b);
    }
    inline int& operator^=(int &a, AdminFlag b)
    {
        return a ^= static_cast<int>(b);
    }
    inline bool operator==(AdminFlag a, int b)
    {
        return static_cast<int>(a) == b;
    }
    inline bool operator==(int a, AdminFlag b)
    {
        return a == static_cast<int>(b);
    }
    /*inline AdminFlag& operator=(AdminFlag &a, int b)
    {
        return static_cast<int&>(a) = b;
    }
    inline int& operator=(int &a, AdminFlag b)
    {
        return a = static_cast<int>(b);
    }
    inline operator AdminFlag(int a)
    {
        return static_cast<AdminFlag>(a);
    }
    inline operator int(AdminFlag a)
    {
        return static_cast<int>(a);
    }*/
    
    AdminFlag getAdminFlagBits(const std::string &flags);
    std::string getAdminFlagString(AdminFlag flagbits);
};

#endif


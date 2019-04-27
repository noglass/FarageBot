#ifndef _FARAGE_HOOK_
#define _FARAGE_HOOK_

#define HOOK_PRINT                          1
#define HOOK_BLOCK_EVENT                    2
#define HOOK_BLOCK_CMD                      4
#define HOOK_BLOCK_HOOK                     8

#include "api/containers.h"
#include "shared/regex.h"

namespace Farage
{
    class Handle;
    struct ChatHook;
    
    typedef int (*ChatHookCallback)(Handle&,ChatHook*,const rens::smatch&,const Message&);
    
    struct ChatHook
    {
        std::string name;
        rens::regex pattern;
        ChatHookCallback func;
        int flags;
    };
};

#endif


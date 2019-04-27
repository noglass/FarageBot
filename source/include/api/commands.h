#ifndef _FARAGE_COMMAND_
#define _FARAGE_COMMAND_

#include "api/containers.h"
#include "api/admins.h"

namespace Farage
{
    class Handle;
    
    typedef int (*ChatCmdCallback)(Handle&,int,const std::string[],const Message&);
    typedef int (*ConCmdCallback)(Handle&,int,const std::string[]);
    
    struct ChatCommand
    {
        std::string cmd;
        ChatCmdCallback func;
        AdminFlag flag;
        std::string desc;
    };
    
    struct ConsoleCommand
    {
        std::string cmd;
        ConCmdCallback func;
        std::string desc;
    };
};

#endif


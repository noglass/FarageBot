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
    struct ReactHook;
    
    enum ReactHookType
    {
        any,
        msg,
        guild,
        chan
    };
    
    typedef int (*ChatHookCallback)(Handle&,ChatHook*,const rens::smatch&,const Message&);
    typedef int (*ReactHookCallback)(Handle&,ReactHook*,const ServerMember&,const Channel&,const std::string&,const std::string&,const Emoji&);
    
    struct ChatHook
    {
        std::string name;
        rens::regex pattern;
        ChatHookCallback func;
        int flags;
    };
    
    struct ReactHook
    {
        std::string name;
        std::string user;
        std::string anyID;
        ReactHookType type;
        Emoji emoji;
        ReactHookCallback func;
        int flags;
        bool matches(const std::string &userID, const std::string &channelID, const std::string &messageID, const std::string &guildID, const Emoji &reaction)
        {
            if ((user.size() > 0) && (user != userID))
                return false;
            if (anyID.size() > 0)
            {
                switch (type)
                {
                    case msg:
                    {
                        if (anyID != messageID)
                            return false;
                        break;
                    }
                    case guild:
                    {
                        if (anyID != guildID)
                            return false;
                        break;
                    }
                    case chan:
                    {
                        if (anyID != channelID)
                            return false;
                        break;
                    }
                }
            }
            if (((emoji.id.size() > 0) || (emoji.name.size() > 0)) && (emoji != reaction))
                return false;
            return true;
        }
    };
};

#endif


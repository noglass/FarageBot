#ifndef _FARAGE_BASICS_
#define _FARAGE_BASICS_

#define GUILD_TEXT                         0
#define DM_TEXT                            1
#define GUILD_VOICE                        2
#define GROUP_DM                           3
#define GUILD_CATEGORY                     4

#define PLUGIN_CONTINUE                    0
#define PLUGIN_HANDLED                     1
#define PLUGIN_STOP                        2

#include "api/global.h"

std::string strlower(std::string text);
uint32_t mtrand(uint32_t lo = 0, uint32_t hi = -1);
namespace Farage
{
    Global* recallGlobal(Global *global = nullptr);
    GlobVar* findGlobVar(const std::string &name);
    void consoleOut(const std::string &msg);
    void errorOut(const std::string &msg);
    void debugOut(const std::string &msg);
    void verboseOut(const std::string &msg);
    int saveAdminRoles();
    int ignoreChannel(const std::string &ID, bool toggle = false);
    int ignoreUser(const std::string &ID, bool toggle = false);
    int saveIgnoredChannels();
    int saveIgnoredUsers();
    /*void getEventData(void *data, std::string &to);
    void getEventData(void *data, Server &to);
    void getEventData(void *data, Message &to);*/
};

#endif


#include <string>
#include <unordered_map>
#include <vector>
#include "api/global.h"
#include "api/handle.h"

std::string Farage::Global::prefix(const std::string &guild_id)
{
    std::string pre = prefixes[guild_id];
    if ((pre.size() < 1) && (guild_id != "default"))
        pre = prefixes[guild_id] = prefixes["default"];
    if (pre.size() < 1)
        pre = FARAGE_DEFAULT_PREFIX;
    return pre;
}

Farage::AdminFlag Farage::Global::getAdminFlags(const std::string &userID)
{
    Farage::AdminFlag flags = NOFLAG;
    auto flag = admins.find(userID);
    if (flag != admins.end())
        flags = flag->second;
    return flags;
}

Farage::AdminFlag Farage::Global::getAdminFlags(const std::string &guildID, const std::string &userID)
{
    Farage::AdminFlag flags = getAdminFlags(userID);
    auto guild = adminRoles.find(guildID);
    if (guild != adminRoles.end())
    {
        auto guilde = guild->second.end();
        Farage::ServerMember member = callbacks.getServerMember(guildID,userID);
        for (auto it = member.roles.begin(), ite = member.roles.end();it != ite;++it)
        {
            auto flag = guild->second.find(*it);
            if (flag != guilde)
                flags |= flag->second;
        }
    }
    return flags;
}

Farage::AdminFlag Farage::Global::getAdminRoleFlags(const std::string &guildID, const std::string &roleID)
{
    Farage::AdminFlag flags = NOFLAG;
    auto guild = adminRoles.find(guildID);
    if (guild != adminRoles.end())
    {
        auto flag = guild->second.find(roleID);
        if (flag != guild->second.end())
            flags = flag->second;
    }
    return flags;
}


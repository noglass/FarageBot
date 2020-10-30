#include "api/farage.h"
#include <unordered_map>
#include <unordered_set>
#include <fstream>
using namespace Farage;

#define VERSION "v0.0.4"

extern "C" Info Module
{
    "User Roles",
    "Madison",
    "User Role Granting System",
    VERSION,
    "http://farage.justca.me/",
    FARAGE_API_VERSION
};

namespace role
{
    std::unordered_map<std::string,std::unordered_set<std::string>> roles;
}

int addroleCmd(Handle&,int,const std::string[],const Message&);
int delroleCmd(Handle&,int,const std::string[],const Message&);
int giveroleCmd(Handle&,int,const std::string[],const Message&);

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("roles_version",VERSION,"User Roles Version",GVAR_CONSTANT);
    handle.regChatCmd("addrole",&addroleCmd,ROLE,"Allow an already existing role to the roster.");
    handle.regChatCmd("delrole",&delroleCmd,ROLE,"Remove a role from the roster.");
    handle.regChatCmd("role",&giveroleCmd,NOFLAG,"Grant a role for yourself.");
    return 0;
}

extern "C" int onReady(Handle &handle, Event event, void *data, void *nil, void *foo, void *bar)
{
    Ready* readyData = (Ready*)data;
    for (auto it = readyData->guilds.begin(), ite = readyData->guilds.end();it != ite;++it)
    {
        std::unordered_set<std::string> r;
        std::ifstream file ("roles/" + *it);
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file,line))
                r.emplace(line);
            file.close();
        }
        role::roles.emplace(*it,std::move(r));
    }
    return PLUGIN_CONTINUE;
}

int addroleCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global *global = recallGlobal();
    auto server = getGuildCache(message.guild_id);
    auto roles = role::roles.find(message.guild_id);
    if (roles == role::roles.end())
    {
        role::roles.emplace(message.guild_id,std::unordered_set<std::string>());
        roles = role::roles.find(message.guild_id);
    }
    if (argc < 2)
    {
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <role>`");
        return PLUGIN_HANDLED;
    }
    std::string role = argv[1];
    for (int i = 2;i < argc;++i)
        role.append(1,' ').append(argv[i]);
    if ((role.substr(0,3) == "<@&") && (role.back() == '>'))
        role = role.substr(3,role.size()-4);
    std::string name = strlower(role);
    if ((name.front() == '"') && (name.back() == '"'))
        name = name.substr(1,name.size()-2);
    bool found = false;
    for (auto it = server.roles.begin(), ite = server.roles.end();it != ite;++it)
    {
        if ((strlower(it->name) == name) || (it->id == role))
        {
            role = it->id;
            name = it->name;
            found = true;
            break;
        }
    }
    if ((found) && (roles->second.find(role) == roles->second.end()))
    {
        roles->second.emplace(role);
        std::ofstream file ("roles/" + message.guild_id);
        if (file.is_open())
        {
            for (auto it = roles->second.begin(), ite = roles->second.end();it != ite;++it)
                file<<*it<<std::endl;
            file.close();
        }
        sendMessage(message.channel_id,"Added the role: `" + name + "`!");
    }
    else if (found)
        sendMessage(message.channel_id,"Role is already registered: `" + name + "`!");
    else
        sendMessage(message.channel_id,"Not a valid role: `" + role + "`!");
    return PLUGIN_HANDLED;
}

int delroleCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global *global = recallGlobal();
    auto server = getGuildCache(message.guild_id);
    auto roles = role::roles.find(message.guild_id);
    if (argc < 2)
    {
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <role>`");
        return PLUGIN_HANDLED;
    }
    if ((roles == role::roles.end()) || (roles->second.size() == 0))
    {
        sendMessage(message.channel_id,"There are no registered roles!");
        return PLUGIN_HANDLED;
    }
    std::string role = argv[1];
    for (int i = 2;i < argc;++i)
        role.append(1,' ').append(argv[i]);
    if ((role.substr(0,3) == "<@&") && (role.back() == '>'))
        role = role.substr(3,role.size()-4);
    std::string name = strlower(role);
    if ((name.front() == '"') && (name.back() == '"'))
        name = name.substr(1,name.size()-2);
    for (auto it = server.roles.begin(), ite = server.roles.end();it != ite;++it)
    {
        if ((strlower(it->name) == name) || (it->id == role))
        {
            role = it->id;
            name = it->name;
            break;
        }
    }
    auto r = roles->second.find(role);
    if (r == roles->second.end())
    {
        sendMessage(message.channel_id,"Not a valid role: `" + role + "`!");
        return PLUGIN_HANDLED;
    }
    roles->second.erase(r);
    std::ofstream file ("roles/" + message.guild_id);
    if (file.is_open())
    {
        for (auto it = roles->second.begin(), ite = roles->second.end();it != ite;++it)
            file<<*it<<std::endl;
        file.close();
    }
    sendMessage(message.channel_id,"Removed the role: `" + name + "`!");
    return PLUGIN_HANDLED;
}

int giveroleCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global *global = recallGlobal();
    auto server = getGuildCache(message.guild_id);
    auto roles = role::roles.find(message.guild_id);
    if ((roles == role::roles.end()) || (roles->second.size() == 0))
    {
        sendMessage(message.channel_id,"There are no registered roles!");
        return PLUGIN_HANDLED;
    }
    if (argc < 2)
    {
        Embed out;
        out.color = 11614177;
        out.title = "Usage: `" + global->prefix(message.guild_id) + argv[0] + " <role>` to give yourself a role.";
        for (auto it = roles->second.begin(), ite = roles->second.end();it != ite;++it)
            for (auto sit = server.roles.begin(), site = server.roles.end();sit != site;++sit)
                if (sit->id == *it)
                    out.description.append(1,'\n').append(sit->name);
        sendEmbed(message.channel_id,out);
        return PLUGIN_HANDLED;
    }
    std::string role = argv[1];
    for (int i = 2;i < argc;++i)
        role.append(1,' ').append(argv[i]);
    std::string name = strlower(role);
    if ((role.substr(0,3) == "<@&") && (role.back() == '>'))
        role = role.substr(3,role.size()-4);
    else
    {
        if ((name.front() == '"') && (name.back() == '"'))
            name = name.substr(1,name.size()-2);
        for (auto it = server.roles.begin(), ite = server.roles.end();it != ite;++it)
        {
            if (strlower(it->name) == name)
            {
                role = it->id;
                name = it->name;
                break;
            }
        }
    }
    if (roles->second.find(role) == roles->second.end())
    {
        sendMessage(message.channel_id,"Not a valid role: `" + role + "`!");
        return PLUGIN_HANDLED;
    }
    else
    {
        auto mroles = message.member.roles;
        for (auto it = mroles.begin(), ite = mroles.end();it != ite;++it)
        {
            if (*it == role)
            {
                mroles.erase(it);
                editMember(message.guild_id,message.author.id,(message.member.nick.size() > 0 ? message.member.nick : message.author.username),mroles);
                sendMessage(message.channel_id,"You have left the `" + name + "` role.");
                return PLUGIN_HANDLED;
            }
        }
        mroles.push_back(role);
        editMember(message.guild_id,message.author.id,(message.member.nick.size() > 0 ? message.member.nick : message.author.username),mroles);
        sendMessage(message.channel_id,"You have joined the `" + name + "` role.");
    }
    return PLUGIN_HANDLED;
}


#include "api/farage.h"
#include <iostream>
#include <fstream>
//#include <regex>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
using namespace Farage;

#define SPLITSTRINGANY
#define REGSUBEX
#define MAKEMENTION
#include "common_func.h"

#define VERSION "v1.5.6"

#define UDEVAL

extern "C" Info Module
{
    "User Data",
    "Madison",
    "Get data on a user",
    VERSION,
    "http://farage.justca.me/",
    FARAGE_API_VERSION
};

int avatarCmd(Handle&,int,const std::string[],const Message&);
int bannerCmd(Handle&,int,const std::string[],const Message&);
#ifdef UDEVAL
int evalCmd(Handle&,int,const std::string[],const Message&);
#endif

void (*makeLewd)(std::string&,int[4]) = nullptr;
void (*getFaq)(std::string&) = nullptr;

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("udata_version",VERSION,"User Data Version",GVAR_CONSTANT);
    handle.regChatCmd("pfp",&avatarCmd,NOFLAG,"Get a user's avatar.");
    handle.regChatCmd("pfb",&bannerCmd,NOFLAG,"Get a user's banner.");
#ifdef UDEVAL
    handle.regChatCmd("udeval",&evalCmd,ROOT,"Evaluate an identifier{ID,...}[.prop]... string.");
#endif
    /*int found = 0;
    for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
    {
        if ((*it)->getModule() == "reproductive")
        {
            *(void **) (&makeLewd) = dlsym((*it)->getModulePtr(),"makeLewd");
            if (dlerror() != NULL)
                makeLewd = nullptr;
            found |= 1;
        }
        else if ((*it)->getModule() == "mettaton")
        {
            *(void **) (&getFaq) = dlsym((*it)->getModulePtr(),"getFaqValue");
            if (dlerror() != NULL)
                getFaq = nullptr;
            found |= 2;
        }
        if (found == 3)
            break;
    }*/
    return 0;
}

extern "C" int onModulesLoaded(Handle &handle, int event, void *iterator, void *position, void *foo, void *bar)
{
    Global* global = recallGlobal();
    int found = 0;
    for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
    {
        if ((*it)->getModule() == "reproductive")
        {
            *(void **) (&makeLewd) = dlsym((*it)->getModulePtr(),"makeLewd");
            if (dlerror() != NULL)
                makeLewd = nullptr;
            found |= 1;
        }
        else if ((*it)->getModule() == "mettaton")
        {
            *(void **) (&getFaq) = dlsym((*it)->getModulePtr(),"getFaqValue");
            if (dlerror() != NULL)
                getFaq = nullptr;
            found |= 2;
        }
        if (found == 3)
            break;
    }
    return PLUGIN_CONTINUE;
}

int fetchpfpurl(const User& who, std::string& avatar)
{
    /*if (who.avatar.size() > 0)
    {
        std::string line;
        std::string outfile = "pfp/" + who.avatar;
        std::string cmd = "curl -s -o ";
        avatar = "https://cdn.discordapp.com/avatars/" + who.id + '/' + who.avatar;
        system((cmd + outfile + ".gif " + avatar + ".gif").c_str());
        std::ifstream image (outfile + ".gif");
        if (image.is_open())
        {
            bool err = (image.peek() == std::char_traits<char>::eof());
            //image.close();
            std::getline(image,line);
            if ((err) || (line.find("image transform failed") == 0))
            {
                system(("rm " + outfile + ".gif").c_str());
                outfile += ".png";
                avatar += ".png";
                //system((cmd + outfile + ' ' + avatar).c_str());
                return 1;
            }
            else
            {
                outfile += ".gif";
                avatar += ".gif";
                system(("rm " + outfile).c_str());
                return 2;
            }
        }
        else
        {
            outfile += ".png";
            avatar += ".png";
            //system((cmd + outfile + ' ' + avatar).c_str());
            return 1;
        }
    }*/
    avatar = "https://cdn.discordapp.com/avatars/" + who.id + '/' + who.avatar;
    if (who.avatar.substr(0,2) == "a_")
        avatar += ".gif";
    else if (who.avatar.size() > 0)
        avatar += ".png";
    else
    {
        avatar = "https://cdn.discordapp.com/embed/avatars/" + std::to_string(std::stoi(who.discriminator) % 5) + ".png";
        return 2;
    }
    return 1;
}

std::string int2hex(const uint32_t color)
{
    std::string out(6,'0');
    for (uint8_t i = 0, j = 20;i<6;++i,j -= 4)
        out[i] = "0123456789ABCDEF"[(color>>j) & 0x0f];
    return std::move(out);
}

int fetchpfburl(const User& who, std::string& avatar)
{
    if (who.banner.size() > 0)
    {
        avatar = "https://cdn.discordapp.com/banners/" + who.id + '/' + who.banner;
        if (who.banner.substr(0,2) == "a_")
            avatar += ".gif";
        else
            avatar += ".png";
        return 1;
    }
    else if (who.accent_color)
    {
        avatar = "https://www.colorhexa.com/" + int2hex(who.accent_color) + ".png";
        return 2;
    }
    return 0;
}

int avatarCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <user_id>`");
    else
    {
        std::string id = argv[1];
        static rens::regex pingptrn ("<@!?([0-9]+)>");
        rens::smatch ml;
        if (regex_match(id,ml,pingptrn))
            id = ml[1].str();
        auto response = getUser(id);
        if (!response.response.error())
        {
            User who = response.object;
            std::string color = std::to_string(who.accent_color);
            std::string avatar;
            fetchpfpurl(who,avatar);
            //sendFile(message.channel_id,outfile,"**" + who.username + "**#" + who.discriminator + "'s avatar");
            if (color == "0")
            {
                color = "39835";
                ServerMember req = getServerMember(message.guild_id,id);
                Server guild = getGuildCache(message.guild_id);
                int position = 0;
                for (auto r = guild.roles.begin(), re = guild.roles.end();r != re;++r)
                {
                    for (auto it = req.roles.begin(), ite = req.roles.end();it != ite;++it)
                    {
                        if (r->id == *it)
                        {
                            if ((r->position > position) && (r->color > 0))
                            {
                                position = r->position;
                                std::string c = std::to_string(r->color);
                                if (c.size() > 0)
                                    color = c;
                            }
                        }
                    }
                }
            }
            std::string output = "{ \"color\":" + color + ", \"author\": { \"name\": \"" + who.username + "#" + who.discriminator + "'s avatar\", \"icon_url\": \"" + avatar.substr(0,avatar.size()-3) + "png\" }, \"image\": { \"url\": \"" + avatar + "?size=1024\" }, \"description\": \"" + makeMention(who.id) + "\" }";
            //std::cout<<output<<std::endl;
            sendEmbed(message.channel_id,output);
        }
        else
            reaction(message,"%E2%9D%93");
    }
    return PLUGIN_HANDLED;
}

int bannerCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <user_id>`");
    else
    {
        std::string id = argv[1];
        static rens::regex pingptrn ("<@!?([0-9]+)>");
        rens::smatch ml;
        if (regex_match(id,ml,pingptrn))
            id = ml[1].str();
        auto response = getUser(id);
        if (!response.response.error())
        {
            User who = response.object;
            std::string color = std::to_string(who.accent_color);
            std::string avatar, banner;
            if (fetchpfburl(who,banner))
            {
                //sendFile(message.channel_id,outfile,"**" + who.username + "**#" + who.discriminator + "'s avatar");
                if (!who.accent_color)
                {
                    color = "39835";
                    ServerMember req = getServerMember(message.guild_id,id);
                    Server guild = getGuildCache(message.guild_id);
                    int position = 0;
                    for (auto r = guild.roles.begin(), re = guild.roles.end();r != re;++r)
                    {
                        for (auto it = req.roles.begin(), ite = req.roles.end();it != ite;++it)
                        {
                            if (r->id == *it)
                            {
                                if ((r->position > position) && (r->color > 0))
                                {
                                    position = r->position;
                                    std::string c = std::to_string(r->color);
                                    if (c.size() > 0)
                                        color = c;
                                }
                            }
                        }
                    }
                }
                fetchpfpurl(who,avatar);
                std::string output = "{ \"color\":" + color + ", \"author\": { \"name\": \"" + who.username + "#" + who.discriminator + "'s banner\", \"icon_url\": \"" + avatar.substr(0,avatar.size()-3) + "png\" }, \"image\": { \"url\": \"" + banner + "?size=1024\" }, \"description\": \"" + makeMention(who.id) + "\" }";
                //std::cout<<output<<std::endl;
                sendEmbed(message.channel_id,output);
            }
            else
                reaction(message,"%E2%9D%8C");
            /*else if (who.accent_color)
            {
                fetchpfpurl(who,avatar);
                std::string output = "{ \"color\":" + color + ", \"author\": { \"name\": \"" + who.username + "#" + who.discriminator + "'s banner\", \"icon_url\": \"" + avatar.substr(0,avatar.size()-3) + "png\" }, \"image\": { \"url\": \"https://www.colorhexa.com/" + int2hex(who.accent_color) + ".png\" }, \"description\": \"" + makeMention(who.id) + "\" }";
                sendEmbed(message.channel_id,output);
            }*/
        }
        else
            reaction(message,"%E2%9D%93");
    }
    return PLUGIN_HANDLED;
}

#ifdef UDEVAL
std::string userData(const User& who, std::string prop)
{
    std::string out;
    size_t pos = 0;
    if (prop.front() == '.')
        prop.erase(0,1);
    if (who.discriminator.size() > 0)
    {
        if (prop.size() > 0)
        {
            if (!prop.find("id"))
            {
                out = who.id;
                pos = 2;
            }
            else if (!prop.find("username"))
            {
                out = who.username;
                pos = 8;
            }
            else if (!prop.find("discriminator"))
            {
                out = who.discriminator;
                pos = 13;
            }
            else if (!prop.find("avatar"))
            {
                out = who.avatar;
                pos = 6;
            }
            else if (!prop.find("pfp"))
            {
                fetchpfpurl(who,out);
                pos = 3;
            }
            else if (!prop.find("bot"))
            {
                if (who.bot)
                    out = "true";
                else
                    out = "false";
                pos = 3;
            }
            else if (!prop.find("mfa_enabled"))
            {
                if (who.mfa_enabled)
                    out = "true";
                else
                    out = "false";
                pos = 11;
            }
            else if (!prop.find("verified"))
            {
                if (who.verified)
                    out = "true";
                else
                    out = "false";
                pos = 8;
            }
            else if (!prop.find("email"))
            {
                out = who.email;
                pos = 5;
            }
            else if (!prop.find("banner"))
            {
                out = who.banner;
                pos = 6;
            }
            else if (!prop.find("accent_color"))
            {
                out = std::to_string(who.accent_color);
                pos = 12;
            }
            else if (!prop.find("pfb"))
            {
                fetchpfburl(who,out);
                pos = 3;
            }
            else
            {
                out = who.id + '.';
                pos = 0;
            }
        }
        else
            out = who.username + '#' + who.discriminator;
    }
    else
    {
        out = "<UNKNOWN>";
        if (prop.size() > 0)
            out += "." + prop;
        pos = prop.size();
    }
    if (pos < prop.size())
        out += prop.substr(pos);
    return out;
}

std::string roleData(const Role& role, std::string prop)
{
    std::string out;
    size_t pos = 0;
    if (prop.front() == '.')
        prop.erase(0,1);
    if (role.name.size() > 0)
    {
        if (prop.size() > 0)
        {
            if (!prop.find("id"))
            {
                out = role.id;
                pos = 2;
            }
            else if (!prop.find("name"))
            {
                out = role.name;
                pos = 4;
            }
            else if (!prop.find("color"))
            {
                out = std::to_string(role.color);
                pos = 5;
            }
            else if (!prop.find("hoist"))
            {
                if (role.hoist)
                    out = "true";
                else
                    out = "false";
                pos = 5;
            }
            else if (!prop.find("position"))
            {
                out = std::to_string(role.position);
                pos = 8;
            }
            else if (!prop.find("permissions"))
            {
                out = std::to_string(role.permissions);
                pos = 11;
            }
            else if (!prop.find("managed"))
            {
                if (role.managed)
                    out = "true";
                else
                    out = "false";
                pos = 7;
            }
            else if (!prop.find("mentionable"))
            {
                if (role.mentionable)
                    out = "true";
                else
                    out = "false";
                pos = 11;
            }
            else
            {
                out = role.id + '.';
                pos = 0;
            }
        }
        else
            out = role.id;
    }
    else
    {
        out = "<UNKNOWN>";
        if (prop.size() > 0)
            out += "." + prop;
        pos = prop.size();
    }
    if (pos < prop.size())
        out += prop.substr(pos);
    return out;
}

std::string memberData(const ServerMember& mem, std::string prop)//, const std::string& guild = "")
{
    std::string out;
    size_t pos = 0;
    if (prop.front() == '.')
        prop.erase(0,1);
    if (mem.joined_at.size() > 0)
    {
        if (prop.size() > 0)
        {
            if (!prop.find("roles"))
            {
                pos = 5;
                //Server server = getGuildCache(guild);
                if (pos < prop.size())
                {
                    if (prop.find("[]",pos) == pos)
                    {
                        //Role role;
                        //prop.erase(0,pos+2);
                        pos += 2;
                        for (auto it = mem.roles.begin(), ite = mem.roles.end();it != ite;)
                        {
                            /*for (auto rit = server.roles.begin(), rite = server.roles.end();rit != rite;++rit)
                            {
                                if (rit->id == *it)
                                {
                                    role = *rit;
                                    break;
                                }
                            }*/
                            //out += roleData(role,prop);
                            out += *it;
                            if (++it != ite)
                                out += ", ";
                        }
                        //pos = prop.size();
                    }
                    else if (prop.at(pos) == '[')
                    {
                        size_t npos;
                        if (((npos = prop.find("]",pos)) == std::string::npos) || ((prop.size() > pos+1) && (!std::isdigit(prop.at(pos+1)))))
                            out = std::to_string(mem.roles.size());
                        else
                        {
                            size_t n = std::stoull(prop.substr(pos+1));
                            if (n < mem.roles.size())
                            {
                                /*Role role;
                                for (auto it = server.roles.begin(), ite = server.roles.end();it != ite;++it)
                                {
                                    if (it->id == mem.roles[n])
                                    {
                                        role = *it;
                                        break;
                                    }
                                }
                                out = roleData(role,prop.substr(npos+1));
                                pos = prop.size();*/
                                out = mem.roles[n];
                                pos = npos+1;
                            }
                            else
                                out = std::to_string(mem.roles.size());
                        }
                    }
                    else
                        out = std::to_string(mem.roles.size());
                }
                else
                    out = std::to_string(mem.roles.size());
            }
            else if (!prop.find("nick"))
            {
                if ((out = mem.nick).size() == 0)
                    out = mem.user.username;
                pos = 4;
            }
            else if (!prop.find("joined_at"))
            {
                out = mem.joined_at;
                pos = 9;
            }
            else if (!prop.find("deaf"))
            {
                if (mem.deaf)
                    out = "true";
                else
                    out = "false";
                pos = 4;
            }
            else if (!prop.find("mute"))
            {
                if (mem.mute)
                    out = "true";
                else
                    out = "false";
                pos = 4;
            }
            else if (!prop.find("user"))
            {
                out = userData(mem.user,prop.substr(4));
                pos = prop.size();
            }
            else
            {
                out = mem.user.username + '.';
                pos = 0;
            }
        }
        else if ((out = mem.nick).size() == 0)
            out = mem.user.username;
    }
    else
    {
        out = "<UNKNOWN>";
        if (prop.size() > 0)
            out += "." + prop;
        pos = prop.size();
    }
    if (pos < prop.size())
        out += prop.substr(pos);
    return out;
}

std::string channelData(const Channel& chan, std::string prop, const std::string& request)
{
    std::string out;
    size_t pos = 0;
    if (prop.front() == '.')
        prop.erase(0,1);
    bool hidden = (chan.guild_id.size() == 0);
    for (auto it = chan.permission_overwrites.begin(), ite = chan.permission_overwrites.end();it != ite;++it)
    {
        if ((int64_t)it->deny & (int64_t)Permission::VIEW_CHANNEL)
        {
            hidden = true;
            break;
        }
    }
    if ((chan.id.size() > 0) && ((!hidden) || (chan.id == request) || (request.size() == 0)))
    {
        if (prop.size() > 0)
        {
            if (!prop.find("id"))
            {
                out = chan.id;
                pos = 2;
            }
            else if (!prop.find("type"))
            {
                out = std::to_string(chan.type);
                pos = 4;
            }
            else if (!prop.find("guild_id"))
            {
                out = chan.guild_id;
                pos = 8;
            }
            else if (!prop.find("position"))
            {
                out = std::to_string(chan.position);
                pos = 8;
            }
            else if (!prop.find("name"))
            {
                out = chan.name;
                pos = 4;
            }
            else if (!prop.find("topic"))
            {
                out = chan.topic;
                pos = 5;
            }
            else if (!prop.find("nsfw"))
            {
                if (chan.nsfw)
                    out = "true";
                else
                    out = "false";
                pos = 4;
            }
            else if (!prop.find("last_message_id"))
            {
                out = chan.last_message_id;
                pos = 15;
            }
            else if (!prop.find("bitrate"))
            {
                out = std::to_string(chan.bitrate);
                pos = 7;
            }
            else if (!prop.find("user_limit"))
            {
                out = std::to_string(chan.user_limit);
                pos = 10;
            }
            else if (!prop.find("recipients"))
            {
                pos = 10;
                if ((prop.size() > pos) && (prop.at(pos) == '['))
                {
                    size_t npos;
                    if (((npos = prop.find("]",pos)) == std::string::npos) || ((prop.size() > pos+1) && (!std::isdigit(prop.at(pos+1)))))
                        out = std::to_string(chan.recipients.size());
                    else
                    {
                        size_t n = std::stoull(prop.substr(pos+1));
                        if (n < chan.recipients.size())
                        {
                            out = userData(chan.recipients[n],prop.substr(npos+1));
                            pos = prop.size();
                        }
                        else
                            out = std::to_string(chan.recipients.size());
                    }
                }
                else
                    out = std::to_string(chan.recipients.size());
            }
            else if (!prop.find("icon"))
            {
                out = chan.icon;
                pos = 4;
            }
            else if (!prop.find("owner_id"))
            {
                out = chan.owner_id;
                pos = 8;
            }
            else if (!prop.find("parent_id"))
            {
                out = chan.parent_id;
                pos = 9;
            }
            else if (!prop.find("last_pin_timestamp"))
            {
                out = chan.last_pin_timestamp;
                pos = 18;
            }
            else
            {
                out = chan.name + '.';
                pos = 0;
            }
        }
        else
            out = chan.name;
    }
    else
    {
        out = "<UNKNOWN>";
        if (prop.size() > 0)
            out += "." + prop;
        pos = prop.size();
    }
    if (pos < prop.size())
        out += prop.substr(pos);
    return out;
}

std::string guildData(const Server& chan, std::string prop, const std::string& request)
{
    std::string out;
    size_t pos = 0;
    if (prop.front() == '.')
        prop.erase(0,1);
    if (chan.name.size() > 0)
    {
        if (prop.size() > 0)
        {
            if (!prop.find("id"))
            {
                out = chan.id;
                pos = 2;
            }
            else if (!prop.find("name"))
            {
                out = chan.name;
                pos = 4;
            }
            else if (!prop.find("icon"))
            {
                out = chan.icon;
                pos = 4;
            }
            else if (!prop.find("pfp"))
            {
                pos = 3;
                if (chan.icon.size() > 0)
                {
                    out = "https://cdn.discordapp.com/icons/" + chan.id + '/' + chan.icon;
                    if (chan.icon.find("a_") == 0)
                        out += ".gif";
                    else
                        out += ".png";
                }
            }
            else if (!prop.find("splash"))
            {
                out = chan.splash;
                pos = 6;
            }
            else if (!prop.find("owner_id"))
            {
                out = chan.owner_id;
                pos = 8;
            }
            else if (!prop.find("permissions"))
            {
                out = std::to_string(chan.permissions);
                pos = 11;
            }
            else if (!prop.find("region"))
            {
                out = chan.region;
                pos = 6;
            }
            else if (!prop.find("afk_channel_id"))
            {
                out = chan.afk_channel_id;
                pos = 14;
            }
            else if (!prop.find("afk_timeout"))
            {
                out = std::to_string(chan.afk_timeout);
                pos = 11;
            }
            else if (!prop.find("embed_enabled"))
            {
                if (chan.embed_enabled)
                    out = "true";
                else
                    out = "false";
                pos = 13;
            }
            else if (!prop.find("embed_channel_id"))
            {
                out = chan.embed_channel_id;
                pos = 16;
            }
            else if (!prop.find("verification_level"))
            {
                out = std::to_string(chan.verification_level);
                pos += 18;
            }
            else if (!prop.find("default_message_notifications"))
            {
                out = std::to_string(chan.default_message_notifications);
                pos = 29;
            }
            else if (!prop.find("roles"))
            {
                pos = 5;
                if ((prop.size() > pos) && (prop.at(pos) == '['))
                {
                    size_t npos;
                    if (((npos = prop.find("]",pos)) == std::string::npos) || ((prop.size() > pos+1) && (!std::isdigit(prop.at(pos+1)))))
                        out = std::to_string(chan.roles.size());
                    else
                    {
                        size_t n = std::stoull(prop.substr(pos+1));
                        if (n < chan.roles.size())
                        {
                            out = roleData(chan.roles[n],prop.substr(npos+1));
                            pos = prop.size();
                        }
                        else
                            out = std::to_string(chan.roles.size());
                    }
                }
                else
                    out = std::to_string(chan.roles.size());
            }
            else if (!prop.find("mfa_level"))
            {
                out = std::to_string(chan.mfa_level);
                pos += 9;
            }
            else if (!prop.find("joined_at"))
            {
                out = chan.joined_at;
                pos += 9;
            }
            else if (!prop.find("large"))
            {
                if (chan.large)
                    out = "true";
                else
                    out = "false";
                pos += 5;
            }
            else if (!prop.find("unavailable"))
            {
                if (chan.unavailable)
                    out = "true";
                else
                    out = "false";
                pos += 11;
            }
            else if (!prop.find("members"))
            {
                pos = 7;
                if ((prop.size() > pos) && (prop.at(pos) == '['))
                {
                    size_t npos;
                    if (((npos = prop.find("]",pos)) == std::string::npos) || ((prop.size() > pos+1) && (!std::isdigit(prop.at(pos+1)))))
                        out = std::to_string(chan.members.size());
                    else
                    {
                        size_t n = std::stoull(prop.substr(pos+1));
                        if (n < chan.members.size())
                        {
                            out = memberData(chan.members[n],prop.substr(npos+1));//,chan.id);
                            pos = prop.size();
                        }
                        else
                            out = std::to_string(chan.members.size());
                    }
                }
                else
                    out = std::to_string(chan.members.size());
            }
            else if (!prop.find("channels"))
            {
                pos = 8;
                if ((prop.size() > pos) && (prop.at(pos) == '['))
                {
                    size_t npos;
                    if (((npos = prop.find("]",pos)) == std::string::npos) || ((prop.size() > pos+1) && (!std::isdigit(prop.at(pos+1)))))
                        out = std::to_string(chan.channels.size());
                    else
                    {
                        size_t n = std::stoull(prop.substr(pos+1));
                        if (n < chan.channels.size())
                        {
                            out = channelData(chan.channels[n],prop.substr(npos+1),request);
                            pos = prop.size();
                        }
                        else
                            out = std::to_string(chan.channels.size());
                    }
                }
                else
                    out = std::to_string(chan.channels.size());
            }
            else
            {
                out = chan.name + '.';
                pos = 0;
            }
        }
        else
            out = chan.name;
    }
    else
    {
        out = "<UNKNOWN>";
        if (prop.size() > 0)
            out += "." + prop;
        pos = prop.size();
    }
    if (pos < prop.size())
        out += prop.substr(pos);
    return out;
}

std::string embedData(const Embed& embed, std::string prop)
{
    std::string out;
    size_t pos = 0;
    if (prop.front() == '.')
        prop.erase(0,1);
    //if (prop.size() == 0)
    //{
        out = "```json\n\
{\n\
    \"title\": \"" + embed.title + "\",\n\
    \"type\": \"" + embed.type + "\",\n\
    \"description\": \"" + embed.description + "\",\n\
    \"url\": \"" + embed.url + "\",\n\
    \"timestamp\": \"" + embed.timestamp + "\",\n\
    \"color\": " + std::to_string(embed.color) + ",\n\
    \"footer\": {\n\
        \"text\": \"" + embed.footer.text + "\",\n\
        \"icon_url\": \"" + embed.footer.icon_url + "\",\n\
        \"proxy_icon_url\": \"" + embed.footer.proxy_icon_url + "\"\n\
    },\n\
    \"image\": {\n\
        \"url\": \"" + embed.image.url + "\",\n\
        \"proxy_url\": \"" + embed.image.proxy_url + "\",\n\
        \"width\": " + std::to_string(embed.image.width) + ",\n\
        \"height\": " + std::to_string(embed.image.height) + "\n\
    },\n\
    \"thumbnail\": {\n\
        \"url\": \"" + embed.thumbnail.url + "\",\n\
        \"proxy_url\": \"" + embed.thumbnail.proxy_url + "\",\n\
        \"width\": " + std::to_string(embed.thumbnail.width) + ",\n\
        \"height\": " + std::to_string(embed.thumbnail.height) + "\n\
    },\n\
    \"video\": {\n\
        \"url\": \"" + embed.video.url + "\",\n\
        \"width\": " + std::to_string(embed.video.width) + ",\n\
        \"height\": " + std::to_string(embed.video.height) + "\n\
    },\n\
    \"provider\": {\n\
        \"name\": \"" + embed.provider.name + "\",\n\
        \"url\": \"" + embed.provider.url + "\"\n\
    },\n\
    \"author\": {\n\
        \"name\": \"" + embed.author.name + "\",\n\
        \"url\": \"" + embed.author.url + "\",\n\
        \"icon_url\": \"" + embed.author.icon_url + "\",\n\
        \"proxy_icon_url\": \"" + embed.author.proxy_icon_url + "\"\n\
    },\n\
    \"fields\": [\n";
    for (auto& field : embed.fields)
    {
        out += "        {\n\
            \"name\": \"" + field.name + "\",\n\
            \"value\": \"" + field.value + "\",\n\
            \"inline\": ";
            if (field._inline)
                out += "true";
            else
                out += "false";
            out += "\n        },";
    }
    if (out.back() == ',')
        out.erase(out.size()-1);
    out += "    ]\n}\n```";
    //}
    return out;
}

std::string messageData(const Message& chan, std::string prop, const std::vector<Overwrite>& perms, const std::string& request)
{
    std::string out;
    size_t pos = 0;
    if (prop.front() == '.')
        prop.erase(0,1);
    bool hidden = (chan.guild_id.size() == 0);
    for (auto it = perms.begin(), ite = perms.end();it != ite;++it)
    {
        if ((int64_t)it->deny & (int64_t)Permission::VIEW_CHANNEL)
        {
            hidden = true;
            break;
        }
    }
    if ((chan.id.size() > 0) && ((!hidden) || (chan.channel_id == request) || (request.size() == 0)))
    {
        if (prop.size() > 0)
        {
            if (!prop.find("id"))
            {
                out = chan.id;
                pos = 2;
            }
            else if (!prop.find("channel_id"))
            {
                out = chan.channel_id;
                pos = 10;
            }
            else if (!prop.find("guild_id"))
            {
                out = chan.guild_id;
                pos = 8;
            }
            else if (!prop.find("author"))
            {
                out = userData(chan.author,prop.substr(6));
                pos = prop.size();
            }
            else if (!prop.find("member"))
            {
                out = memberData(chan.member,prop.substr(6));//,chan.guild_id);
                pos = prop.size();
            }
            else if (!prop.find("content"))
            {
                out = chan.content;
                pos = 7;
            }
            else if (!prop.find("timestamp"))
            {
                out = chan.timestamp;
                pos = 9;
            }
            else if (!prop.find("edited_timestamp"))
            {
                out = chan.edited_timestamp;
                pos = 16;
            }
            else if (!prop.find("tts"))
            {
                if (chan.tts)
                    out = "true";
                else
                    out = "false";
                pos = 3;
            }
            else if (!prop.find("mention_everyone"))
            {
                if (chan.mention_everyone)
                    out = "true";
                else
                    out = "false";
                pos = 16;
            }
            else if (!prop.find("mentions"))
            {
                pos = 8;
                if (prop.size() > pos)
                {
                    if (prop.find("[]",pos) == pos)
                    {
                        prop.erase(0,pos+2);
                        for (auto it = chan.mentions.begin(), ite = chan.mentions.end();it != ite;)
                        {
                            out += userData(*it,prop);
                            if (++it != ite)
                                out += ", ";
                        }
                        pos = prop.size();
                    }
                    else if (prop.at(pos) == '[')
                    {
                        size_t npos;
                        if (((npos = prop.find("]",pos)) == std::string::npos) || ((prop.size() > pos+1) && (!std::isdigit(prop.at(pos+1)))))
                            out = std::to_string(chan.mentions.size());
                        else
                        {
                            size_t n = std::stoull(prop.substr(pos+1));
                            if (n < chan.mentions.size())
                            {
                                out = userData(chan.mentions[n],prop.substr(npos+1));
                                pos = prop.size();
                            }
                            else
                                out = std::to_string(chan.mentions.size());
                        }
                    }
                    else
                        out = std::to_string(chan.mentions.size());
                }
                else
                    out = std::to_string(chan.mentions.size());
            }
            else if (!prop.find("mention_roles"))
            {
                pos = 13;
                if (prop.size() > pos)
                {
                    if (prop.find("[]",pos) == pos)
                    {
                        prop.erase(0,pos+2);
                        for (auto it = chan.mention_roles.begin(), ite = chan.mention_roles.end();it != ite;)
                        {
                            //out += userData(*it,prop);
                            out += *it;
                            if (++it != ite)
                                out += ", ";
                        }
                        pos = prop.size();
                    }
                    else if (prop.at(pos) == '[')
                    {
                        size_t npos;
                        if (((npos = prop.find("]",pos)) == std::string::npos) || ((prop.size() > pos+1) && (!std::isdigit(prop.at(pos+1)))))
                            out = std::to_string(chan.mention_roles.size());
                        else
                        {
                            size_t n = std::stoull(prop.substr(pos+1));
                            if (n < chan.mention_roles.size())
                            {
                                out = chan.mention_roles[n];
                                pos = npos+1;
                            }
                            else
                                out = std::to_string(chan.mention_roles.size());
                        }
                    }
                    else
                        out = std::to_string(chan.mention_roles.size());
                }
                else
                    out = std::to_string(chan.mention_roles.size());
            }
            else if (!prop.find("pinned"))
            {
                if (chan.pinned)
                    out = "true";
                else
                    out = "false";
                pos = 6;
            }
            else if (!prop.find("webhook_id"))
            {
                out = chan.webhook_id;
                pos = 10;
            }
            else if (!prop.find("type"))
            {
                out = std::to_string(chan.type);
                pos = 4;
            }
            else if (!prop.find("embeds"))
            {
                pos = 6;
                if (prop.size() > pos)
                {
                    size_t n = 0;
                    if (prop.find("[]",pos) == pos)
                        pos += 2;
                    else if (prop.at(pos) == '[')
                    {
                        size_t npos;
                        if (((npos = prop.find("]",pos)) == std::string::npos) || ((prop.size() > pos+1) && (!std::isdigit(prop.at(pos+1)))))
                            out = std::to_string(chan.embeds.size());
                        else
                        {
                            n = std::stoull(prop.substr(pos+1));
                            pos = npos+1;
                        }
                    }
                    if (out.size() == 0)
                    {
                        if (n < chan.embeds.size())
                        {
                            out = embedData(chan.embeds[n],prop.substr(pos));
                            pos = prop.size();
                        }
                        else
                            out = std::to_string(chan.embeds.size());
                    }
                }
                else
                    out = std::to_string(chan.embeds.size());
            }
            else
            {
                out = chan.id + '.';
                pos = 0;
            }
        }
        else
            out = "<" + userData(chan.author,prop) + "> " + chan.content;
            //out = chan.content;
    }
    else
    {
        out = "<UNKNOWN>";
        if (prop.size() > 0)
            out += "." + prop;
        pos = prop.size();
    }
    if (pos < prop.size())
        out += prop.substr(pos);
    return out;
}

std::string messagesData(const ArrayResponse<Message>& msgs, std::string prop, const Channel& chan, const std::string& request)
{
    std::string out;
    size_t pos = 0;
    if (prop.front() == '.')
        prop.erase(0,1);
    bool hidden = (chan.guild_id.size() == 0);
    for (auto it = chan.permission_overwrites.begin(), ite = chan.permission_overwrites.end();it != ite;++it)
    {
        if ((int64_t)it->deny & (int64_t)Permission::VIEW_CHANNEL)
        {
            hidden = true;
            break;
        }
    }
    if ((!msgs.response.error()) && ((!hidden) || (chan.id == request) || (request.size() == 0)))
    {
        if (prop.size() > 0)
        {
            if (!prop.find("[]"))
            {
                prop.erase(0,2);
                for (auto it = msgs.array.rbegin(), ite = msgs.array.rend();it != ite;)
                {
                    out += messageData(*it,prop,chan.permission_overwrites,request);
                    if (++it != ite)
                        out += "\n";
                }
                pos = prop.size();
            }
            else if (prop.front() == '[')
            {
                size_t npos;
                if (((npos = prop.find("]",1)) == std::string::npos) || ((prop.size() > 1) && (!std::isdigit(prop.at(1)))))
                    out = std::to_string(msgs.array.size());
                else
                {
                    size_t n = std::stoull(prop.substr(1));
                    prop.erase(0,npos+1);
                    if (n < msgs.array.size())
                    {
                        out = messageData(msgs.array[n],prop,chan.permission_overwrites,request);
                        pos = prop.size();
                    }
                    else
                        out = std::to_string(msgs.array.size());
                }
            }
            else
                out = std::to_string(msgs.array.size());
        }
        else
            out = std::to_string(msgs.array.size());
    }
    else
    {
        out = "<UNKNOWN>";
        if (prop.size() > 0)
            out += "." + prop;
        pos = prop.size();
    }
    if (pos < prop.size())
        out += prop.substr(pos);
    return out;
}


rens::regex rem ("[\\\\\"\\$`';\n]|\\\\n");
std::string exec(const std::string& command, const int size = 128, bool getAll = false)
{
    std::string output;
    char buffer[size];
    FILE* outstream = popen(command.c_str(),"r");
    if (outstream)
    {
        if (fgets(buffer,size,outstream) != NULL)
            output = buffer;
        if (getAll)
            while (!feof(outstream))
                if (fgets(buffer,size,outstream) != NULL)
                    output.append(buffer);
        pclose(outstream);
    }
    for (auto it = output.end(), ite = output.begin();(it-- != ite) && (*it == '\n');it = output.erase(it));
    return output;
}

std::string calc(const std::string& expression)
{
    return exec("exec bash -c 'let \"foo=" + rens::regex_replace(expression,rem,"") + "\";echo $foo'");
}

std::string calcf(const std::string& expression)
{
    return exec("exec bash -c 'awk \"BEGIN { ; print " + rens::regex_replace(expression,rem,"") + "}\"'");
}

std::string duration(int64_t seconds)
{
    std::string output;
    if (seconds < 0)
    {
        seconds *= -1;
        output = "Negative ";
    }
    size_t minutes = 0, hours = 0, days = 0, weeks = 0;//, months = 0, years = 0;
    minutes = (hours = seconds / 60) % 60;
    seconds %= 60;
    hours = (days = hours / 60) % 24;
    days = (weeks = days / 24) % 7;
    weeks /= 7;
    /*months = (years = months / 30) % 12;
    years /= 12;
    if (years)
        output += std::to_string(years) + " year" + (years > 1 ? "s, " : ", ");
    if (months)
        output += std::to_string(months) + " month" + (months > 1 ? "s, " : ", ");*/
    if (weeks)
        output += std::to_string(weeks) + " week" + (weeks > 1 ? "s, " : ", ");
    if (days)
        output += std::to_string(days) + " day" + (days > 1 ? "s, " : ", ");
    if (hours)
        output += std::to_string(hours) + " hour" + (hours > 1 ? "s, " : ", ");
    if (minutes)
        output += std::to_string(minutes) + " minute" + (minutes > 1 ? "s, " : ", ");
    output += std::to_string(seconds) + " second";
    if ((seconds > 1) || (seconds == 0))
        output += "s";
    return std::move(output);
}

bool isLeap(int year)
{
    if (year % 4 == 0)
    {
        if (year % 100 == 0)
        {
            if (year % 400 == 0)
                return true;
            else
                return false;
        }
        return true;
    }
    return false;
}

std::string timedur(time_t start, time_t end)
{
    static const int monthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    std::string output;
    bool future = (start < end);
    tm st, ed;
    if (future)
    {
        gmtime_r(&start,&st);
        gmtime_r(&end,&ed);
    }
    else
    {
        gmtime_r(&end,&st);
        gmtime_r(&start,&ed);
    }
    bool leap = false;
    int seconds = 0, minutes = 0, hours = 0, days = 0, months = 0, years = 0;
    years = ed.tm_year - st.tm_year;
    if ((years) && ((st.tm_mon > ed.tm_mon) || ((st.tm_mon == ed.tm_mon) && (st.tm_mday > ed.tm_mday))))
        --years;
    if (st.tm_year == ed.tm_year)
        months = ed.tm_mon - st.tm_mon;
    else if (st.tm_mon < ed.tm_mon)
        months = ed.tm_mon - st.tm_mon;
    else if (st.tm_mon != ed.tm_mon)
        months = 12 - st.tm_mon + ed.tm_mon;
    if (st.tm_mday > ed.tm_mday)
        --months;
    if (months < 0)
        months = 11;
    if (st.tm_mday > ed.tm_mday)
    {
        int m = st.tm_mon;
        if (st.tm_mon == ed.tm_mon)
            if (--m < 0)
                m = 11;
        if (m == 1)
            leap = isLeap(1900 + st.tm_year);
        days = monthDays[m] - st.tm_mday + leap + ed.tm_mday;
    }
    else
        days = ed.tm_mday - st.tm_mday;
    
    int sTime = (st.tm_hour * 60 + st.tm_min) * 60 + st.tm_sec;
    int eTime = (ed.tm_hour * 60 + ed.tm_min) * 60 + ed.tm_sec;
    if (eTime < sTime)
    {
        --days;
        seconds = (24*60*60) - sTime + eTime;
    }
    else
        seconds = eTime - sTime;
    
    if (days < 0)
    {
        int m = st.tm_mon - 1;
        if (m < 0)
            m = 11;
        if (m == 1)
            leap = isLeap(1900 + st.tm_year);
        days = monthDays[m] - st.tm_mday + leap + ed.tm_mday;
        --months;
    }
    if (months < 0)
    {
        months = 11;
        --years;
    }
    
    minutes = (hours = seconds / 60) % 60;
    hours = (hours / 60) % 24;
    seconds %= 60;
    
    if (years)
        output += std::to_string(years) + " year" + (years > 1 ? "s, " : ", ");
    if (months)
        output += std::to_string(months) + " month" + (months > 1 ? "s, " : ", ");
    if (days)
        output += std::to_string(days) + " day" + (days > 1 ? "s, " : ", ");
    if (hours)
        output += std::to_string(hours) + " hour" + (hours > 1 ? "s, " : ", ");
    if (minutes)
        output += std::to_string(minutes) + " minute" + (minutes > 1 ? "s, " : ", ");
    if (seconds)
    {
        output += std::to_string(seconds) + " second";
        if ((seconds > 1) || (seconds == 0))
            output += "s";
    }
    else
       output.erase(output.size()-2);
    /*if (future)
        output += " from <t:" + std::to_string(start) + ">";
    else
        output += " before <t:" + std::to_string(start) + ">";*/
    return std::move(output);
}

void evaluateDataRe(std::string& eval, const std::string& guildID, const std::string& channelID, const std::string& authorID, const std::string& messageID)
{
    static const rens::regex specialptrn ("[\\.\\{\\}\\[\\]\\(\\)]");
    //static const rens::regex inputptrn ("(?<!\\\\)\\$(\\d+)");
    static const rens::regex identptrn ("(?i)(user|member|guild|channel|role|message|messages\\((\\d+|before|after|around),\\d+\\))\\{(\\d+|this)(\\}|,\\d+\\})(\\.[\\w\\[\\]\\d\\.]+)?");
    static const rens::regex randptrn ("(?i)rand\\((\\d+),?(\\d*)\\)");
    static const rens::regex rand2ptrn ("(?i)rand\\(([^)]*)\\)");
    static const rens::regex argptrn ("^([^,]*)(,|$)");
    static const rens::regex inputptrn ("&(\\d+)");
    static const rens::regex balanced ("^(\\((?>[^()]+|(?1))*\\))");
    static const rens::regex lewdptrn ("(?i)lewd\\((\\d*),?(\\d*),?(\\d*),?(\\d*)\\)");
    static const rens::regex timeptrn ("(?i)\\btime\\(\\)");
    static const rens::regex time2ptrn ("(?i)\\btime\\(([^,]+),([^)]+)\\)");
    static const rens::regex strftimeptrn ("(?i)strftime\\((-?\\d+),([^)]+)\\)");
    static const rens::regex durationptrn ("(?i)duration\\((-?\\d+)\\)");
    static const rens::regex timediffptrn ("(?i)timediff\\((-?\\d+),?(-?\\d+)?\\)");
    static const rens::regex faqptrn ("(?i)faq\\(([^)]+)\\)");
    Global* global = recallGlobal();
    rens::smatch ml;
    std::string request = channelID;
    if ((guildID.size() == 0) && ((global->getAdminFlags(authorID) & AdminFlag::ROOT) == AdminFlag::ROOT))
        request.clear();
    while (rens::regex_search(eval,ml,inputptrn))
    {
        int argc = 0, n = std::stoi(ml[1].str());
        std::string* argv = splitStringAny(nospace(eval)," \t\n",argc);
        eval.erase(ml[0].position(),ml[0].length());
        if (--n < 0)
            eval.insert(ml[0].position(),std::to_string(argc));
        else if ((n < argc) && (argv[n] != ml[0].str()))
        {
            size_t ipos = ml[0].position();
            //for (int i = 0;i < n;++i)
            //    t += argv[i].size()+1;
            size_t pos = eval.find(argv[n]);
            eval.erase(pos,argv[n].size());
            rens::smatch ml2;
            while (rens::regex_search(argv[n],ml2,inputptrn))
            {
                int m = std::stoi(ml2[1].str());
                argv[n].erase(ml2[0].position(),ml2[0].length());
                if ((m < argc) && (argv[m] != ml2[0].str()))
                    argv[n].insert(ml2[0].position(),argv[m]);
            }
            bool forward = false;
            if (pos < ipos)
                ipos -= argv[n].size();
            else
                forward = true;
            evaluateDataRe(argv[n],guildID,channelID,authorID,messageID);
            if (forward)
                pos += argv[n].size();
            eval.insert(ipos,argv[n]);
            eval.insert(pos,argv[n]);
        }
        delete[] argv;
    }
    size_t pos;
    while ((pos = eval.find("calc")) != std::string::npos)
    {
        bool integer = false, matched = false;
        std::string sub = eval.substr(pos);
        sub.erase(0,4);
        if ((sub.size() > 0) && (sub.front() == 'i'))
        {
            integer = true;
            sub.erase(0,1);
        }
        eval.erase(pos);
        while (rens::regex_search(sub,ml,balanced))
        {
            matched = true;
            std::string str = ml[1].str();
            evaluateDataRe(str,guildID,channelID,authorID,messageID);
            if (integer)
                eval += ml.prefix().str() + calc(str);
            else
                eval += ml.prefix().str() + calcf(str);
            sub = ml.suffix().str();
        }
        if ((!matched) || (sub.size() > 0))
            eval += sub;
        /*if (integer)
            eval += regsubex(sub,balanced,"$1",&calc);
        else
            eval += regsubex(sub,balanced,"$1",&calcf);*/
    }
    while (rens::regex_search(eval,ml,timeptrn))
    {
        eval.erase(ml[0].pos,ml[0].length());
        eval.insert(ml[0].pos,std::to_string(time(NULL)));
    }
    while (rens::regex_search(eval,ml,strftimeptrn))
    {
        char foo[300];
        time_t u = std::stoll(ml[1].str());
        tm* t = gmtime(&u);
        strftime(foo,300,ml[2].str().c_str(),t);
        eval.erase(ml[0].pos,ml[0].length());
        eval.insert(ml[0].pos,foo);
    }
    while (rens::regex_search(eval,ml,time2ptrn))
    {
        tm t = {0};
        std::string str = ml[1].str();
        evaluateDataRe(str,guildID,channelID,authorID,messageID);
        strptime(str.c_str(),ml[2].str().c_str(),&t);
        eval.erase(ml[0].pos,ml[0].length());
        time_t tt = mktime(&t) + 60*60;
        eval.insert(ml[0].pos,std::to_string(tt));
    }
    while (rens::regex_search(eval,ml,randptrn))
    {
        uint32_t r[2];
        r[0] = 0;
        if (ml[2].length() > 0)
        {
            r[0] = std::stoull(ml[1].str());
            r[1] = std::stoull(ml[2].str());
        }
        else
            r[1] = std::stoull(ml[1].str())-1;
        eval.erase(ml[0].pos,ml[0].length());
        eval.insert(ml[0].pos,std::to_string(mtrand(r[0],r[1])));
    }
    while (rens::regex_search(eval,ml,strftimeptrn))
    {
        char foo[300];
        time_t u = std::stoll(ml[1].str());
        tm* t = gmtime(&u);
        strftime(foo,300,ml[2].str().c_str(),t);
        eval.erase(ml[0].pos,ml[0].length());
        eval.insert(ml[0].pos,foo);
    }
    while (rens::regex_search(eval,ml,durationptrn))
    {
        eval.insert(ml[0].pos + ml[0].length(),duration(std::stoll(ml[1].str())));
        eval.erase(ml[0].pos,ml[0].length());
    }
    while (rens::regex_search(eval,ml,timediffptrn))
    {
        time_t t;
        if (ml[2].length() > 0)
            t = std::stoll(ml[2].str());
        else
            t = time(NULL);
        eval.insert(ml[0].pos + ml[0].length(),timedur(t,std::stoll(ml[1].str())));
        eval.erase(ml[0].pos,ml[0].length());
    }
    /*while (rens::regex_search(eval,ml,rand2ptrn))
    {
        std::string str = ml[1].str();
        std::vector<std::string> opts;
        eval.erase(ml[0].pos,ml[0].length());
        if (str.size() > 0)
        {
            //for (size_t pos[2] = {0};pos[1] != std::string::npos;opts.emplace_back(str.substr(pos[0],(pos[1] = str.find(',',pos[0]))-pos[0])))
            //    if (pos[1] > 0)
            //        pos[0] = pos[1]+1;
            rens::smatch mll;
            //std::cout<<str<<std::endl;
            while ((str.size() > 0) && (rens::regex_search(str,mll,argptrn)))
            {
                //std::cout<<mll[0].str()<<' '<<mll[1].str()<<std::endl;
                opts.emplace_back(mll[1].str());
                str = mll.suffix().str();
            }
            //std::cout<<opts.size()<<std::endl;
            eval.insert(ml[0].pos,opts.at(mtrand(0,opts.size()-1)));
        }
        else
            eval.insert(ml[0].pos,std::to_string(mtrand(0,-1)));
    }*/
    while (rens::regex_search(eval,ml,rand2ptrn))
    {
        std::string str = ml[1].str();
        evaluateDataRe(str,guildID,channelID,authorID,messageID);
        eval.erase(ml[1].position(),ml[1].length());
        eval.insert(ml[1].position(),str);
        if (rens::regex_search(eval,ml,randptrn))
        {
            uint32_t r[2];
            r[0] = 0;
            if (ml[2].length() > 0)
            {
                r[0] = std::stoull(ml[1].str());
                r[1] = std::stoull(ml[2].str());
            }
            else
                r[1] = std::stoull(ml[1].str())-1;
            eval.erase(ml[0].pos,ml[0].length());
            eval.insert(ml[0].pos,std::to_string(mtrand(r[0],r[1])));
        }
        if (rens::regex_search(eval,ml,rand2ptrn))
        {
            std::string str = ml[1].str();
            std::vector<std::string> opts;
            eval.erase(ml[0].pos,ml[0].length());
            if (str.size() > 0)
            {
                //for (size_t pos[2] = {0};pos[1] != std::string::npos;opts.emplace_back(str.substr(pos[0],(pos[1] = str.find(',',pos[0]))-pos[0])))
                //    if (pos[1] > 0)
                //        pos[0] = pos[1]+1;
                rens::smatch mll;
                //std::cout<<str<<std::endl;
                while ((str.size() > 0) && (rens::regex_search(str,mll,argptrn)))
                {
                    //std::cout<<mll[0].str()<<' '<<mll[1].str()<<std::endl;
                    opts.emplace_back(mll[1].str());
                    str = mll.suffix().str();
                }
                //std::cout<<opts.size()<<std::endl;
                eval.insert(ml[0].pos,opts.at(mtrand(0,opts.size()-1)));
            }
            else
                eval.insert(ml[0].pos,std::to_string(mtrand(0,-1)));
        }
    }
    if (makeLewd != nullptr) while (rens::regex_search(eval,ml,lewdptrn))
    {
        int in[4] = {-1};
        for (int i = 0;i < 4;++i)
        {
            if (ml[i+1].str().size() == 0)
                break;
            in[i] = std::stoi(ml[i+1].str());
        }
        eval.erase(ml[0].pos,ml[0].length());
        std::string lewd;
        makeLewd(lewd,in);
        eval.insert(ml[0].pos,lewd);
    }
    if (getFaq != nullptr) while (rens::regex_search(eval,ml,faqptrn))
    {
        std::string faq = ml[1].str();
        getFaq(faq);
        eval.erase(ml[0].pos,ml[0].length());
        eval.insert(ml[0].pos,faq);
    }
    User who;
    Message msg;
    Channel chan;
    while (rens::regex_search(eval,ml,identptrn))
    {
        std::cout<<"ml.size(): "<<ml.size()<<std::endl;
        for (auto it = ml.begin(), ite = ml.end();it != ite;++it)
            std::cout<<":: "<<it->str()<<std::endl;
        std::string out;
        std::string ident = ml[1].str();
        std::string id = ml[3].str();
        std::string guild = guildID;
        bool customGuild = false;
        if (ml[4].str().front() == ',')
        {
            guild = ml[4].str().substr(1,ml[4].str().size()-2);
            customGuild = true;
        }
        std::string prop = strlower(ml[5].str());
        if (ident == "user")
        {
            if (id == "this")
                id = authorID;
            if (id != who.id)
                who = getUser(id).object;
            out = userData(who,prop);
        }
        else if (ident == "member")
        {
            if (id == "this")
                id = authorID;
            out = memberData(getServerMember(guild,id),prop);//,guild);
        }
        else if (ident == "guild")
        {
            if (id == "this")
                id = guildID;
            out = guildData(getGuildCache(id),prop,request);
        }
        else if (ident == "channel")
        {
            if (id == "this")
                id = channelID;
            out = channelData(getChannelCache(guild,id),prop,request);
        }
        else if (ident == "role")
        {
            Server server = getGuildCache(guild);
            Role role;
            for (auto it = server.roles.begin(), ite = server.roles.end();it != ite;++it)
            {
                if (it->id == id)
                {
                    role = *it;
                    break;
                }
            }
            out = roleData(role,prop);
        }
        else if (ident == "message")
        {
            if (id == "this")
                id = messageID;
            if (!customGuild)
                guild = channelID;
            if (chan.id != guild)
                chan = getChannel(guild).object;
            if (msg.id != id)
                msg = getMessage(chan.id,id).object;
            out = messageData(msg,prop,chan.permission_overwrites,request);
        }
        else // messages
        {
            if (id == "this")
                id = messageID;
            if (!customGuild)
                guild = channelID;
            if (chan.id != guild)
                chan = getChannel(guild).object;
            int when;
            std::string foo = ml[2].str();
            if (std::isdigit(foo.front()))
                when = std::stoi(foo);
            else if (foo == "after")
                when = 3;
            else if (foo == "around")
                when = 1;
            else // before
                when = 2;
            if ((when > 3) || (when < 1))
                when = 3;
            uint8_t limit = std::stoi(ident.substr(10 + foo.size()));
            out = messagesData(getMessages(chan.id,GetMessagesKey(when),id,limit),prop,chan,request);
        }
        eval = rens::regex_replace(eval,rens::regex(rens::regex_replace(ml[0].str(),specialptrn,"\\$0")),out,0);
    }
}

extern "C" void evaluateData(std::string& eval, const std::string& guildID, const std::string& channelID, const std::string& authorID, const std::string& messageID)
{
    static rens::regex joinptrn ("\\s*\\&\\+\\s*");
    evaluateDataRe(eval,guildID,channelID,authorID,messageID);
    eval = rens::regex_replace(eval,joinptrn,"");
}

int evalCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    //static rens::regex inputptrn ("\\$(\\d+)");
    Global* global = recallGlobal();
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <evaluate>`");
    std::string eval = message.content.substr(message.content.find(argv[0]) + argv[0].size() + 1);
    /*rens::smatch ml;
    while (rens::regex_search(eval,ml,inputptrn))
    {
        int arg = (int)std::stoull(ml[1].str());
        if ((arg < argc) && (arg >= 0))
        {
            if (argv[arg].front() == '$')// || (argv[arg].front() == '\\'))
                eval = rens::regex_replace(eval,inputptrn,argv[arg].substr(1),0);
            else
                eval = rens::regex_replace(eval,inputptrn,argv[arg],0);
        }
        else
            eval = rens::regex_replace(eval,inputptrn,"",0);
    }*/
    evaluateData(eval,message.guild_id,message.channel_id,message.author.id,message.id);
    sendMessage(message.channel_id,"**Eval**: " + eval);
    return PLUGIN_HANDLED;
}
#endif


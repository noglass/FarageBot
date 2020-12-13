#include "api/farage.h"
#include <iostream>
#include <fstream>
//#include <regex>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdio>
#include <cstring>
using namespace Farage;

#define SPLITSTRINGANY
#define REGSUBEX
#define MAKEMENTION
#include "common_func.h"

#define VERSION "v0.9.6"

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
#ifdef UDEVAL
int evalCmd(Handle&,int,const std::string[],const Message&);
#endif

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("udata_version",VERSION,"User Data Version",GVAR_CONSTANT);
    handle.regChatCmd("pfp",&avatarCmd,NOFLAG,"Get a user's avatar.");
#ifdef UDEVAL
    handle.regChatCmd("udeval",&evalCmd,ROOT,"Evaluate an identifier{ID,...}[.prop]... string.");
#endif
    return 0;
}

int avatarCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <user_id>`");
    else
    {
        std::string color = "39835";
        std::string id = argv[1];
        static rens::regex pingptrn ("<@!?([0-9]+)>");
        rens::smatch ml;
        if (regex_match(id,ml,pingptrn))
            id = ml[1].str();
        User who = getUser(id).object;
        if (who.avatar.size() > 0)
        {
            std::string outfile = "pfp/" + who.id + message.timestamp;
            std::string cmd = "curl -s -o ";
            std::string avatar = "https://cdn.discordapp.com/avatars/" + who.id + '/' + who.avatar;
            system((cmd + outfile + ".gif " + avatar + ".gif?size=1024").c_str());
            std::ifstream image (outfile + ".gif");
            if (image.is_open())
            {
                bool err = (image.peek() == std::char_traits<char>::eof());
                image.close();
                if (err)
                {
                    system(("rm " + outfile + ".gif").c_str());
                    outfile += ".png";
                    avatar += ".png?size=1024";
                    //system((cmd + outfile + ' ' + avatar).c_str());
                }
                else
                {
                    outfile += ".gif";
                    avatar += ".gif?size=1024";
                    system(("rm " + outfile).c_str());
                }
            }
            else
            {
                outfile += ".png";
                avatar += ".png?size=1024";
                //system((cmd + outfile + ' ' + avatar).c_str());
            }
            //sendFile(message.channel_id,outfile,"**" + who.username + "**#" + who.discriminator + "'s avatar");
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
            std::string output = "{ \"color\":" + color + ", \"author\": { \"name\": \"" + who.username + "#" + who.discriminator + "'s avatar\", \"icon_url\": \"" + avatar + "\" }, \"image\": { \"url\": \"" + avatar + "\" }, \"description\": \"" + makeMention(who.id) + "\" }";
            //std::cout<<output<<std::endl;
            sendEmbed(message.channel_id,output);
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
            else if (!prop.find("bot"))
            {
                if (who.bot)
                    out = "true";
                else
                    out = "false";
                pos = 8;
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
    bool hidden = false;
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
    bool hidden = false;
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
    bool hidden = false;
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
                    prop.erase(npos+1);
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

void evaluateDataRe(std::string& eval, const std::string& guildID, const std::string& channelID, const std::string& authorID, const std::string& messageID)
{
    static rens::regex specialptrn ("[\\.\\{\\}\\[\\]\\(\\)]");
    //static rens::regex inputptrn ("(?<!\\\\)\\$(\\d+)");
    static rens::regex identptrn ("(?i)(user|member|guild|channel|role|message|messages\\(\\d+,\\d+\\))\\{(\\d+|this)(\\}|,\\d+\\})(\\.[\\w\\[\\]\\d\\.]+)?");
    static rens::regex randptrn ("(?i)rand\\((\\d+),?(\\d*)\\)");
    static rens::regex rand2ptrn ("(?i)rand\\(([^)]*)\\)");
    static rens::regex argptrn ("^([^,]*)(,|$)");
    static rens::regex inputptrn ("&(\\d+)");
    static rens::regex balanced ("(\\((?>[^()]+|(?1))*\\))");
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
    size_t pos;
    while ((pos = eval.find("calc")) != std::string::npos)
    {
        bool integer = false;
        std::string sub = eval.substr(pos);
        sub.erase(0,4);
        if ((sub.size() > 0) && (sub.front() == 'i'))
        {
            integer = true;
            sub.erase(0,1);
        }
        eval.erase(pos);
        if (integer)
            eval += regsubex(sub,balanced,"$1",&calc);
        else
            eval += regsubex(sub,balanced,"$1",&calcf);
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
        std::string id = ml[2].str();
        std::string guild = guildID;
        bool customGuild = false;
        if (ml[3].str().front() == ',')
        {
            guild = ml[3].str().substr(1,ml[3].str().size()-2);
            customGuild = true;
        }
        std::string prop = strlower(ml[4].str());
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
                guild = guildID;
            if (chan.id != guild)
                chan = getChannel(guild).object;
            int when = std::stoi(ident.substr(9));
            uint8_t limit = std::stoi(ident.substr(10 + std::to_string(when).size()));
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


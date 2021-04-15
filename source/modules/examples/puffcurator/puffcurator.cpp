#include "api/farage.h"
#include <iostream>
#include <fstream>
using namespace Farage;

#define MAKEMENTION
#include "common_func.h"

#define VERSION "v0.1.2"

extern "C" Info Module
{
    "Puffer Curator",
    "Madison",
    "PPP Curator Helper",
    VERSION,
    "http://you.justca.me/",
    FARAGE_API_VERSION
};

int curate(const std::string& userID, ObjectResponse<Message>& response, const std::string& title = "")
{
    if (response.response.error())
        return 1;
    std::string out = "> Credit: " + response.object.author.username + " Shared via " + makeMention(userID,response.object.guild_id);
    if (title.size() == 0)
        out = response.object.content + '\n' + out;
    else
        out = title + '\n' + out;
    if (response.object.attachments.size() > 0)
    {
        system(("curl -s -o " + response.object.attachments.front().filename + ' ' + response.object.attachments.front().url.substr(0,response.object.attachments.front().url.find_last_of('?'))).c_str());
        system(("rm '" + response.object.attachments.front().filename + "' $(sleep 30) &").c_str());
        sendFile("650027620624236556",response.object.attachments.front().filename,out);
            //return 2;
    }
    else
    {
        static const rens::regex url ("(?i)\\b(https?://[a-z0-9.-]+[^\\s]*)");
        rens::smatch ml;
        if (rens::regex_search(response.object.content,ml,url))
        {
            if (sendMessage("650027620624236556",out + '\n' + ml[1].str()).response.error())
                return 2;
        }
    }
    return 0;
}

int curateHook(Handle& handle, ReactHook* hook, const ServerMember& member, const Channel& channel, const std::string& messageID, const std::string& guildID, const Emoji& emoji)
{
    if ((channel.id == "650027620624236556") || (member.user.id == recallGlobal()->self.id))
        return PLUGIN_CONTINUE;
    bool isCurator = false;
    for (auto& role : member.roles)
    {
        if ((role == "644904718681899019") || (role == "769208795385561088") || (role == "556169463951785987"))
        {
            isCurator = true;
            break;
        }
    }
    if (!isCurator)
        return PLUGIN_CONTINUE;
    ObjectResponse<Message> response = getMessage(channel.id,messageID);
    for (auto& react : response.object.reactions)
    {
        if (react.emoji == emoji)
        {
            if (react.me)
                return PLUGIN_HANDLED;
            break;
        }
    }
    int result = curate(member.user.id,response);
    if (result == 1)
    {
        reactToID(channel.id,messageID,"%E2%9D%93");
        return PLUGIN_HANDLED;
    }
    if (!result)
        reactToID(channel.id,messageID,emoji.encoded());
    else
        reactToID(channel.id,messageID,"%E2%9D%8C");
    return PLUGIN_HANDLED;
}

int curateCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    static rens::regex linkptrn ("^https://(\\w+\\.)?discord(app)?\\.com/channels/556168952506482699/([0-9]+)/([0-9]+)$");
    if (argc < 2)
    {
        sendMessage(message.channel_id,"Usage: `" + argv[0] + " <message_id> [title]`");
        return PLUGIN_HANDLED;
    }
    std::string title, id = argv[1], channel = message.channel_id;
    if (argc > 2)
        title = argv[2];
    rens::smatch ml;
    if (rens::regex_search(argv[1],ml,linkptrn))
    {
        channel = ml[3].str();
        id = ml[4].str();
    }
    ObjectResponse<Message> response = getMessage(channel,id);
    int result = curate(message.author.id,response,title);
    if (result == 1)
    {
        reactToID(message.channel_id,message.id,"%E2%9D%93");
        return PLUGIN_HANDLED;
    }
    if (!result)
        reactToID(message.channel_id,message.id,"%E2%9C%85");
    else
        reactToID(message.channel_id,message.id,"%E2%9D%8C");
    return PLUGIN_HANDLED;
}

int recurateCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    static rens::regex linkptrn ("^https://(\\w+\\.)?discord(app)?\\.com/channels/556168952506482699/650027620624236556/([0-9]+)$");
    if (argc < 3)
    {
        sendMessage(message.channel_id,"Usage: `" + argv[0] + " <message_id> <title>`");
        return PLUGIN_HANDLED;
    }
    std::string id = argv[1];
    rens::smatch ml;
    if (rens::regex_search(argv[1],ml,linkptrn))
        id = ml[3].str();
    ObjectResponse<Message> response = getMessage("650027620624236556",id);
    size_t pos;
    if ((response.response.error()) || ((pos = response.object.content.find("> Credit: ")) > response.object.content.size()))
    {
        reactToID(message.channel_id,message.id,"%E2%9D%93");
        return PLUGIN_HANDLED;
    }
    if (pos)
        response.object.content.erase(0,pos);

    if (editMessage("650027620624236556",id,argv[2] + '\n' + response.object.content).response.error())
        reactToID(message.channel_id,message.id,"%E2%9D%8C");
    else
        reactToID(message.channel_id,message.id,"%E2%9C%85");
    return PLUGIN_HANDLED;
}

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("curate_version",VERSION,"Puffer Curator Version",GVAR_CONSTANT);
    handle.hookReactionGuild("curateHook",&curateHook,0,"556168952506482699","🖼️");
    handle.regChatCmd("curate",&curateCmd,CUSTOM4,"Curate a puffer!");
    handle.regChatCmd("recurate",&recurateCmd,CUSTOM4,"Change the title for a curated message.");
    return 0;
}


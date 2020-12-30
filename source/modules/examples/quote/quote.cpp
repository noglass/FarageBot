#include "api/farage.h"
#include <iostream>
using namespace Farage;

#define REGSUBEX
#define MAKEMENTION
#define STRREPLACE
#include "common_func.h"

#define VERSION "v0.6.9"

extern "C" Info Module
{
    "Quote",
    "Madison",
    "Quote System",
    VERSION,
    "http://farage.justca.me/",
    FARAGE_API_VERSION
};

int quoteCmd(Handle&,int,const std::string[],const Message&);
int quoteReact(Handle&,ReactHook*,const ServerMember&,const Channel&,const std::string&,const std::string&,const Emoji&);
int deleteQuote(Handle&,ReactHook*,const ServerMember&,const Channel&,const std::string&,const std::string&,const Emoji&);
int updateQuote(Handle&,EditHook*,const Message&);

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("quote_version",VERSION,"Quote Version",GVAR_CONSTANT);
    handle.regChatCmd("quote",&quoteCmd,NOFLAG,"Quote a message.");
    handle.hookReaction("quote",&quoteReact,0,"🗒️");
    handle.hookReaction("quoteMobile",&quoteReact,0,"🗒");
    return 0;
}

std::string replacement(const std::string& in)
{
    if (in == "\"")
        return "\\\"";
    if (in == "\t")
        return "    ";
    return "\\n";
}

ObjectResponse<Message> createQuote(int &err, const ObjectResponse<Message>& response, Handle& handle, const std::string& serv, const std::string& chan, const std::string& mes, const std::string& outChannel, const std::string& requester, const std::string& reply = "")
{
    if (response.response.error())
        err = 1;
    else
    {
        err = 0;
        std::string nick = getServerMember(serv,response.object.author.id).nick;
        if (nick.size() == 0)
            nick = response.object.author.username;
        static rens::regex rep ("[\"\\n\t]");
        static rens::regex image ("(?i)\\.(jpe?g|png|gif|webp)$");
        //static rens::regex thumb ("(?i)\\b(https?://[a-z0-9.-]+/[^\\s]+\\.(jpe?g|png|gif|webp))\\b");
        static rens::regex url ("(?i)\\b(https?://[a-z0-9.-]+[^\\s]*)");
        //static rens::regex video ("(?i)\\.(mp(eg)?4|webm)$");
        std::string msgText = makeMention(requester,serv) + " https://discordapp.com/channels/" + serv + '/' + chan + '/' + mes;
        std::string linkMsg;
        if (reply.size() > 0)
            msgText += " " + makeMention(response.object.author.id,serv) + "\n> " + strreplace(std::move(reply),"\n","\n> ");
        std::string fields, content = "{ \"color\": 4934475, \"author\": { \"name\": \"" + nick + " (" + response.object.author.username + '#' + response.object.author.discriminator + ")\", \"icon_url\": \"https://cdn.discordapp.com/avatars/" + response.object.author.id + '/' + response.object.author.avatar + ".png\" }, ";
        bool hasImage = false;
        if (response.object.attachments.size() > 0)
        {
            bool hasAttach = false;
            for (auto it = response.object.attachments.begin(), ite = response.object.attachments.end();it != ite;++it)
            {
                if ((!hasImage) && (rens::regex_search(it->url,image)))
                {
                    hasImage = true;
                    content += "\"image\": { \"url\": \"" + it->url + "\" }, ";
                }
                //else if (rens::regex_search(it->url,video))
                //    content += ", \"video\": { \"url\": \"" + it->url + "\" }";
                if (!hasAttach)
                {
                    hasAttach = true;
                    fields = "\"fields\": [{ \"name\": \"Attachments\", \"value\": \"";
                }
                fields += "[" + it->filename + "](" + it->url + ")\\n";
            }
            if (hasAttach)
            {
                fields.erase(fields.size()-2);
                fields += "\" }], ";
            }
        }
        if (response.object.content.size() > 0)
        {
            rens::smatch ml;
            std::string str = response.object.content;
            bool thumbed = false;
            while (regex_search(str,ml,url))
            {
                if (regex_search(ml[1].str(),image))
                {
                    if (!thumbed)
                    {
                        thumbed = true;
                        content += "\"thumbnail\": { \"url\": \"" + ml[1].str() + "\" }, ";
                    }
                    else if (!hasImage)
                    {
                        hasImage = true;
                        content += "\"image\": { \"url\": \"" + ml[1].str() + "\" }, ";
                    }
                    else if (linkMsg.size() == 0)
                        linkMsg = ml[1].str();
                    else
                        linkMsg += "\n" + ml[1].str();
                }
                else if (ml[1].str().find("https://discordapp.com/channels/") != 0)
                {
                    if (linkMsg.size() == 0)
                        linkMsg = ml[1].str();
                    else
                        linkMsg += "\n" + ml[1].str();
                }
                str = ml.suffix().str();
            }
            content += "\"description\": \"" + regsubex(response.object.content,rep,"$0",&replacement);
            if (response.object.edited_timestamp.size() > 0)
                content += "\\n(edited)";
            content += "\", ";
        }
        else
        {
            content += "\"description\": \"** **\", ";
        }
        
        content += fields + "\"timestamp\": \"" + response.object.timestamp + "\", \"footer\": { \"icon_url\": \"https://cdn.discordapp.com/icons/" + serv + '/' + getGuildCache(serv).icon + ".png\", \"text\": \"#" + getChannelCache(serv,chan).name + "\" } }";
        std::cout<<content<<std::endl;
        ObjectResponse<Message> resp = sendEmbed(outChannel,content,msgText);
        if (resp.response.error())
            err = 2;
        else
        {
            ObjectResponse<Message> follow;
            if (response.object.embeds.size() > 0)
                follow = sendEmbed(outChannel,response.object.embeds.front(),linkMsg);
            else
                follow = sendMessage(outChannel,linkMsg);
            std::string n = resp.object.id;
            if (!follow.response.error())
                n = follow.object.id;
            handle.hookReactionMessage(n,&deleteQuote,0,resp.object.id,"❌",requester);
        }
        return std::move(resp);
    }
    return ObjectResponse<Message>(Response(4000),Message());
}

ObjectResponse<Message> createQuote(int &err, Handle& handle, const std::string& serv, const std::string& chan, const std::string& mes, const std::string& outChannel, const std::string& requester, const std::string& reply = "")
{
    ObjectResponse<Message> response = getMessage(chan,mes);
    return createQuote(err,response,handle,serv,chan,mes,outChannel,requester,reply);
}

int quoteCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global* global = recallGlobal();
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <link|id> [reply_text]`");
    else
    {
        static rens::regex linkptrn ("^https://(\\w+\\.)?discord(app)?\\.com/channels/([0-9]+)/([0-9]+)/([0-9]+)|[0-9]+$");
        rens::smatch ml;
        if (!rens::regex_search(argv[1],ml,linkptrn))
            sendMessage(message.channel_id,"Error: No message link or ID provided!");
        else
        {
            std::string serv, chan, mes;
            if (ml[0].str().front() != 'h')
            {
                serv = message.guild_id;
                chan = message.channel_id;
                mes = argv[1];
            }
            else
            {
                serv = ml[3].str();
                chan = ml[4].str();
                mes = ml[5].str();
            }
            std::string reply;
            if (argc > 2)
                reply = message.content.substr(message.content.find(argv[1]) + argv[1].size() + 1);
            int err;
            ObjectResponse<Message> response = createQuote(err,handle,serv,chan,mes,message.channel_id,message.author.id,reply);
            if (err == 1)
                reaction(message,"%E2%9D%8C");
            else if (err == 2)
                reaction(message,"%E2%9D%93");
            else if (argc > 2)
                //handle.hookReactionMessage(response.object.id,&updateQuote,0,message.id,"🔄",message.author.id);
                handle.hookEditMessage(response.object.id,&updateQuote,0,message.id);
        }
    }
    return PLUGIN_HANDLED;
}

int quoteReact(Handle& handle, ReactHook* hook, const ServerMember& member, const Channel& channel, const std::string& messageID, const std::string& guildID, const Emoji& emoji)
{
    if (member.user.id != recallGlobal()->self.id)
    {
        ObjectResponse<Message> response = getMessage(channel.id,messageID);
        if (!response.response.error())
        {
            for (auto it = response.object.reactions.begin(), ite = response.object.reactions.end();it != ite;++it)
            {
                if (it->emoji == emoji)
                {
                    if (it->count == 1)
                    {
                        int err;
                        createQuote(err,handle,guildID,channel.id,messageID,channel.id,member.user.id);
                        reactToID(channel.id,messageID,emoji.encoded());
                    }
                }
            }
        }
        else
            removeReaction(channel.id,messageID,emoji.encoded(),member.user.id);
    }
    return PLUGIN_HANDLED;
}

int deleteQuote(Handle& handle, ReactHook* hook, const ServerMember& member, const Channel& channel, const std::string& messageID, const std::string& guildID, const Emoji& emoji)
{
    deleteMessage(channel.id,messageID);
    if (hook->name != messageID)
        deleteMessage(channel.id,hook->name);
    return PLUGIN_ERASE | PLUGIN_HANDLED;
}

int updateQuote(Handle& handle, EditHook* hook, const Message& message)
{
    ObjectResponse<Message> oldmsg = getMessage(message.channel_id,hook->name);
    if (!oldmsg.response.error())
    {
        static rens::regex header ("[^\\n]*");
        rens::smatch ml;
        regex_search(oldmsg.object.content,ml,header);
        std::string newmsg = ml[0].str();
        if (regex_match(message.content,ml,"(?ms)^" + recallGlobal()->prefix(message.guild_id) + "\\s?\\S+\\s+\\S+\\s+(.*)"))
            newmsg += "\n> " + strreplace(ml[1].str(),"\n","\n> ");
        editMessage(message.channel_id,hook->name,newmsg);
    }
    return PLUGIN_HANDLED;
}


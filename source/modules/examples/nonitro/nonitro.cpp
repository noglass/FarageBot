#include "api/farage.h"
#include <unordered_map>
#include <fstream>
#include "shared/libini.h"
#include "shared/regex.h"
using namespace Farage;

#define VERSION "v0.3.2"

extern "C" Info Module
{
    "No Nitro Bypass",
    "Madison",
    "React with nitro emoji's without nitro",
    VERSION,
    "http://you.justca.me/",
    FARAGE_API_VERSION
};

namespace NoNitro
{
    struct EmojiHook
    {
        EmojiHook() {}
        EmojiHook(const std::string& code)
        {
            size_t pos = code.find(' ');
            emoji = code.substr(0,pos);
            if (pos != std::string::npos)
                name = code.substr(pos+1);
        }
        std::string emoji;
        std::string name;
    };
    struct timerEmojiStruct
    {
        Global* global;
        std::string emoji;
        std::string channel;
        std::string message;
        std::string user;
    };
    struct timerDataStruct
    {
        std::vector<std::string> reactions;
        std::string channel;
        std::string message;
    };
    static rens::regex nitrous ("<?(a:|:)?([^:]+):([0-9]+)>?");
    std::unordered_map<std::string,EmojiHook> emojis;
    std::unordered_map<std::string,std::vector<Emoji>> guildEmoji;
    void loadHooks(Handle&);
    int saveHooks();
    void hookEmoji(Handle&,const std::string&,const std::string&);
    int reactCB(Handle&,ReactHook*,const ServerMember&,const Channel&,const std::string&,const std::string&,const Emoji&);
    int reactUndoCB(Handle&,ReactHook*,const ServerMember&,const Channel&,const std::string&,const std::string&,const Emoji&);
    int listEmoji(Handle&,int,const std::string[],const Message&);
    int addEmojiHook(Handle&,int,const std::string[],const Message&);
    int remEmojiHook(Handle&,int,const std::string[],const Message&);
    int listEmojiHook(Handle&,int,const std::string[],const Message&);
    int emojiInfo(Handle&,int,const std::string[],const Message&);
    int reactTimer(Handle&,Timer*,void*);
    int unreactTimer(Handle&,Timer*,void*);
    long getVoteCount(Global* global)
    {
        long seconds = 0;
        for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
        {
            if ((*it)->getModule() == "mettaton")
            {
                for (auto tim = (*it)->timers.begin(), time = (*it)->timers.end();tim != time;++tim)
                    if ((*tim)->name == "vote")
                        seconds += ((timerDataStruct*)((*tim)->args))->reactions.size();
            }
            else if ((*it)->getModule() == "manager")
            {
                for (auto tim = (*it)->timers.begin(), time = (*it)->timers.end();tim != time;++tim)
                    if ((*tim)->name == "react")
                        seconds += ((timerDataStruct*)((*tim)->args))->reactions.size();
            }
        }
        return seconds;
    }
};

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("nonitro_version",VERSION,"No Nitro Version",GVAR_CONSTANT);
    //handle.regChatCmd("hookreaction",&chatHookReaction,GENERIC,"Testing some stuff...");
    //handle.regChatCmd("hookdelete",&chatHookDelete,GENERIC,"Testing some stuff...");
    handle.regChatCmd("animate",&NoNitro::listEmoji,CUSTOM3,"List animated guild emojis.");
    handle.regChatCmd("nnadd",&NoNitro::addEmojiHook,CUSTOM3,"Link a unicode emoji to a nitro emoji.");
    handle.regChatCmd("nnrem",&NoNitro::remEmojiHook,CUSTOM3,"Remove a linked unicode emoji.");
    handle.regChatCmd("nnlist",&NoNitro::listEmojiHook,CUSTOM3,"List all linked emojis.");
    handle.regChatCmd("einfo",&NoNitro::emojiInfo,NOFLAG,"Get info on an emoji.");
    NoNitro::loadHooks(handle);
    return 0;
}

extern "C" int onEditEmojis(Handle &handle, Event event, void *guild, void *emoji, void *foo, void *bar)
{
    std::string *guildID = (std::string*)(guild);
    std::vector<Emoji> *emojis = (std::vector<Emoji>*)emoji;
    //sendMessage("566218807790665731","Guild `" + *guildID + "` has " + std::to_string(emojis->size()) + " emojis.");
    std::vector<Emoji> animated;
    for (auto it = emojis->begin(), ite = emojis->end();it != ite;++it)
    {
        if (it->animated)
            animated.push_back(*it);
    }
    if (animated.size())
        NoNitro::guildEmoji[*guildID] = std::move(animated);
    return PLUGIN_CONTINUE;
}

void NoNitro::loadHooks(Handle &handle)
{
    for (auto &i : handle.reactHooks)
        handle.unhookReaction(i);
    INIObject list ("nonitro.ini");
    if (list.exists("placeholders"))
    {
        auto i = list.topic_it("placeholders");
        for (auto it = i->begin(), ite = i->end();it != ite;++it)
            NoNitro::hookEmoji(handle,it->item,it->value);
    }
}

int NoNitro::saveHooks()
{
    std::ofstream ini ("nonitro.ini",std::ios::out|std::ios::trunc);
    if (ini.is_open())
    {
        ini<<"[placeholders]\n";
        for (auto &it : NoNitro::emojis)
            ini<<it.first<<'='<<it.second.emoji<<' '<<it.second.name<<'\n';
        ini.close();
        return 0;
    }
    return 1;
}

void NoNitro::hookEmoji(Handle &handle, const std::string& hook, const std::string& replace)
{
    auto succ = NoNitro::emojis.emplace(hook,replace);//NoNitro::EmojiHook(replace));
    if (!succ.second)
        succ.first->second = NoNitro::EmojiHook(replace);
    handle.hookReaction(hook,&NoNitro::reactCB,0,hook);
    handle.hookReaction(replace,&NoNitro::reactUndoCB,0,succ.first->second.emoji);
}

int NoNitro::reactTimer(Handle& handle, Timer* timer, void* data)
{
    NoNitro::timerEmojiStruct* emoji = (NoNitro::timerEmojiStruct*)data;
    //if ((NoNitro::getVoteCount(emoji->global)) || (handle.findTimer("reactHook") != timer))
    if ((NoNitro::getVoteCount(emoji->global)) || (handle.findTimer("unreactHook") != nullptr) || (handle.findTimer("removeHook") != nullptr))
        return PLUGIN_CONTINUE;
    //auto response = reactToID(emoji->channel,emoji->message,emoji->emoji);
    reactToID(emoji->channel,emoji->message,emoji->emoji);
    //if (response.result)
    //{
        delete emoji;
        return PLUGIN_HANDLED;
    //}
    //return PLUGIN_CONTINUE;
}

int NoNitro::unreactTimer(Handle& handle, Timer* timer, void* data)
{
    NoNitro::timerEmojiStruct* emoji = (NoNitro::timerEmojiStruct*)data;
    //auto remove = handle.findTimer("removeHook");
    auto unreact = handle.findTimer("unreactHook");
    //if ((NoNitro::getVoteCount(emoji->global)) || (handle.findTimer("reactHook") != nullptr) || ((remove != nullptr) && (remove != timer)) || ((unreact != nullptr) && (unreact != timer)))
    if ((NoNitro::getVoteCount(emoji->global)) || ((unreact != nullptr) && (unreact != timer)))
        return PLUGIN_CONTINUE;
    //auto response = reactToID(emoji->channel,emoji->message,emoji->emoji);
    removeReaction(emoji->channel,emoji->message,emoji->emoji,emoji->user);
    //if (response.result)
    //{
        delete emoji;
        return PLUGIN_HANDLED;
    //}
    //return PLUGIN_CONTINUE;
}

int NoNitro::reactCB(Handle &handle, ReactHook *hook, const ServerMember &member, const Channel &channel, const std::string &messageID, const std::string &guildID, const Emoji &emoji)
{
    auto moji = NoNitro::emojis.find(emoji.display());
    if (moji != NoNitro::emojis.end())
    {
        timerEmojiStruct* data1 = new timerEmojiStruct;
        data1->channel = channel.id;
        data1->message = messageID;
        data1->emoji = emoji.encoded();
        data1->user = member.user.id;
        data1->global = recallGlobal();
        handle.createTimer("removeHook",1,&NoNitro::unreactTimer,(void*)data1);
        //removeReaction(channel.id,messageID,emoji.encoded(),member.user.id);
        //reactToID(channel.id,messageID,moji->second.emoji);
        timerEmojiStruct* data = new timerEmojiStruct;
        data->channel = channel.id;
        data->message = messageID;
        data->emoji = moji->second.emoji;
        data->global = recallGlobal();
        handle.createTimer("reactHook",1,&NoNitro::reactTimer,(void*)data);
        //handle.hookReactionMessage(messageID,&NoNitro::reactUndoCB,0,messageID,moji->second,member.user.id);
    }
    return PLUGIN_CONTINUE;
}

int NoNitro::reactUndoCB(Handle &handle, ReactHook *hook, const ServerMember &member, const Channel &channel, const std::string &messageID, const std::string &guildID, const Emoji &emoji)
{
    if (member.user.id != recallGlobal()->self.id)
    {
        timerEmojiStruct* data = new timerEmojiStruct;
        data->channel = channel.id;
        data->message = messageID;
        data->emoji = emoji.encoded();
        data->user = "@me";
        data->global = recallGlobal();
        handle.createTimer("unreactHook",1,&NoNitro::unreactTimer,(void*)data);
        //removeReaction(channel.id,messageID,emoji.encoded());
    }
    return PLUGIN_CONTINUE;//PLUGIN_ERASE;
}

int NoNitro::listEmoji(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    auto v = NoNitro::guildEmoji.find(message.guild_id);
    if (v != NoNitro::guildEmoji.end())
    {
        std::string mess;
        for (auto it = v->second.begin(), ite = v->second.end();it != ite;++it)
            mess = mess + it->id + ": " + it->display() + '\n';
        messageReply(message,mess);
    }
    else
        messageReply(message,"Because of SleepyDiscord's lack of Emoji endpoint support, please edit or create a new emoji and repeat this command.");
    return PLUGIN_HANDLED;
}

int NoNitro::addEmojiHook(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    rens::smatch ml;
    if (argc < 3)
        messageReply(message,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <unicode_emoji> <nitro_emoji|[a:]name:id> [unicode_emoji_name]`");
    else if (!rens::regex_match(argv[2],ml,NoNitro::nitrous))
        reactToID(message.channel_id,message.id,"%E2%9D%8C");
    else
    {
        std::string e = ml[1].str();
        if (e.size() == 1)
            e.clear();
        e += ml[2].str() + ':' + ml[3].str();
        if (argc > 3)
            e = e + ' ' + argv[3];
        NoNitro::hookEmoji(handle,argv[1],e);
        NoNitro::saveHooks();
        reactToID(message.channel_id,message.id,"%E2%9C%85");
    }
    return PLUGIN_HANDLED;
}

int NoNitro::remEmojiHook(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    if (argc < 2)
        messageReply(message,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <unicode_emoji>`");
    else
    {
        auto moji = NoNitro::emojis.find(argv[1]);
        if (moji != NoNitro::emojis.end())
        {
            handle.unhookReaction(moji->first);
            handle.unhookReaction(moji->second.emoji);
            NoNitro::emojis.erase(moji);
            NoNitro::saveHooks();
            reactToID(message.channel_id,message.id,"%E2%9C%85");
        }
        else
            messageReply(message,"Error: No emoji link exists for `" + argv[1] + '`');
    }
    return PLUGIN_HANDLED;
}

int NoNitro::listEmojiHook(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    std::string mess;
    for (auto &it : NoNitro::emojis)
    {
        if (mess.size() > 1700)
        {
            messageReply(message,mess);
            mess.clear();
        }
        mess = mess + it.first + ((it.second.name.size() > 0) ? (" (`" + it.second.name + "`) = ") : (" = ")) + Emoji(it.second.emoji).display() + '\n';
    }
    messageReply(message,mess);
    consoleOut(mess);
    return PLUGIN_HANDLED;
}

int NoNitro::emojiInfo(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    rens::smatch ml;
    bool nitromatch = true;
    if (argc < 2)
        messageReply(message,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <emoji>`");
    else if (!rens::regex_match(argv[1],ml,NoNitro::nitrous))
    {
        nitromatch = false;
        bool isID = true;
        for (size_t i = 0, l = argv[1].size();i < l;++i)
        {
            if (!std::isdigit(argv[1].at(i)))
            {
                isID = false;
                break;
            }
        }
        if (!isID)
        {
            auto linked = NoNitro::emojis.find(argv[1]);
            if ((linked != NoNitro::emojis.end()) && (rens::regex_match(linked->second.emoji,ml,nitrous)))
                nitromatch = true;
            else
                reactToID(message.channel_id,message.id,"%E2%9D%8C");
        }
        else
        {
            bool animated = false;
            std::string outfile = "temp" + argv[1] + message.timestamp + ".gif";
            system(("curl -s -o " + outfile + " https://cdn.discordapp.com/emojis/" + argv[1] + ".gif >/dev/null").c_str());
            std::ifstream image (outfile);
            if (image.is_open())
            {
                bool err = (image.peek() == std::char_traits<char>::eof());
                image.close();
                if (!err)
                    animated = true;
            }
            system(("rm " + outfile + " >/dev/null").c_str());
            std::string mess = "Name: `unknown`, ID: `" + argv[1] + "`\n";
            if (animated)
                mess += "<a:placeholder:" + argv[1] + ">\nhttps://cdn.discordapp.com/emojis/" + argv[1] + ".gif";
            else
                mess += "<:placeholder:" + argv[1] + ">\nhttps://cdn.discordapp.com/emojis/" + argv[1] + ".png";
            messageReply(message,mess);
        }
    }
    if (nitromatch)
    {
        std::string mess = "Name: `" + ml[2].str() + "`, ID: `" + ml[3].str() + "`\nhttps://cdn.discordapp.com/emojis/" + ml[3].str();
        if (ml[1].str().size() > 1)
            mess += ".gif";
        else
            mess += ".png";
        messageReply(message,mess);
    }
    return PLUGIN_HANDLED;
}








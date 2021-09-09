#include "api/farage.h"
#include <iostream>
#include <chrono>
#include <random>
#include <unordered_map>
#include "shared/libini.h"
using namespace Farage;

#define MAKEMENTION
#define SPLITSTRING
#include "common_func.h"

#define VERSION "v0.3.0"

extern "C" Info Module
{
    "I-R-C",
    "Madison",
    "I-R-C Bot Helper - Game Created By Bahlph",
    VERSION,
    "http://farage.justca.me/",
    FARAGE_API_VERSION
};

namespace irc
{
    //float rollcheck = 0.25;
    //int dicemax = 6;
    GlobVar *rollcheck = nullptr;
    GlobVar *customroll = nullptr;
    GlobVar *dicemax = nullptr;
    GlobVar *customdice = nullptr;
    GlobVar *metasendmsg = nullptr;
    GlobVar *storychan = nullptr;
    std::unordered_map<std::string,INIObject> rollsets;
};

/*int rollcheckChange(Handle&,GlobVar*,const std::string&,const std::string&,const std::string&);
int dicemaxChange(Handle&,GlobVar*,const std::string&,const std::string&,const std::string&);
int customrollChange(Handle&,GlobVar*,const std::string&,const std::string&,const std::string&);
int customdiceChange(Handle&,GlobVar*,const std::string&,const std::string&,const std::string&);*/
int diceCmd(Handle&,int,const std::string[],const Message&);
int rollCmd(Handle&,int,const std::string[],const Message&);
int addrollsetCmd(Handle&,int,const std::string[],const Message&);
int delrollsetCmd(Handle&,int,const std::string[],const Message&);
int rollsetCmd(Handle&,int,const std::string[],const Message&);
int rollsetsCmd(Handle&,int,const std::string[],const Message&);

ObjectResponse<Message> (*sendMessageOriginal)(const std::string&, const std::string&, const std::string&, const MessageReference&, bool) = nullptr;
ObjectResponse<Message> sendMessageMeta(const std::string& channel, const std::string& message, const std::string& json, const MessageReference& ref, bool tts = false)
{
    if (channel == irc::storychan->getAsString())//"719678495877234749")
    {
        std::string meta = irc::metasendmsg->getAsString();
        int n = 0;
        char word = ' ';
        if ((meta.size() > 0) && (std::isdigit(meta.at(0))))
            n = std::stoi(meta);
        if (n < 1)
            return ObjectResponse<Message>();
        int i = 0;
        for (size_t s = meta.size();i < s;++i)
            if ((!std::isdigit(meta.at(i))) && (meta.at(i) == 's'))
                word = '.';
        meta.clear();
        i = 0;
        for (size_t pos = 0;(i < n) && ((pos = message.find(word,pos)+1) != std::string::npos);++i)
            meta = message.substr(0,pos);
        if ((n > 0) && (meta.size() == 0))
            meta = message + word;
        return sendMessageOriginal(channel,meta,json,{},tts);
    }
    return sendMessageOriginal(channel,message,json,{},tts);
}

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("irc_version",VERSION,"I-R-C Version",GVAR_CONSTANT);
    irc::rollcheck = handle.createGlobVar("rollcheck","0.25","Default percent chance of passing a roll check",GVAR_DUPLICATE|GVAR_STORE,true,0.0,true,1.0);//->hookChange(&rollcheckChange);
    irc::dicemax = handle.createGlobVar("dice","6","Default sides of the dice",GVAR_DUPLICATE|GVAR_STORE,true,0.0);//->hookChange(&dicemaxChange);
    irc::customroll = handle.createGlobVar("customroll","true","Allow rollchecks with a custom percentage",GVAR_DUPLICATE|GVAR_STORE,true,0.0,true,1.0);//->hookChange(&customrollChange);
    irc::customdice = handle.createGlobVar("customdice","true","Allow rolling of a custom sided die",GVAR_DUPLICATE|GVAR_STORE,true,0.0,true,1.0);//->hookChange(&customdiceChange);
    irc::metasendmsg = handle.createGlobVar("meta_sendmsg","1w","Set how many words or sentences to allow being sent to the #story channel. Format: '\\d[ws]', 'w' = words, 's' = sentences.",GVAR_STORE);
    irc::storychan = handle.createGlobVar("story_chan","719678495877234749","Set the #story channel.",GVAR_STORE);
    handle.regChatCmd("dice",&diceCmd,NOFLAG,"Roll the dice.");
    handle.regChatCmd("roll",&rollCmd,NOFLAG,"Perform a roll check.");
    handle.regChatCmd("addrollset",&addrollsetCmd,GLOBVAR,"Add a roll set.");
    handle.regChatCmd("delrollset",&delrollsetCmd,GLOBVAR,"Delete a roll set.");
    handle.regChatCmd("rollset",&rollsetCmd,NOFLAG,"Roll for a random outcome of a set.");
    handle.regChatCmd("rollsets",&rollsetsCmd,NOFLAG,"View a list of the rollsets.");
    sendMessageOriginal = global->callbacks.sendMessage;
    global->callbacks.sendMessage = &sendMessageMeta;
    return 0;
}

extern "C" int onReady(Handle &handle, Event event, void *data, void *nil, void *foo, void *bar)
{
    Ready* readyData = (Ready*)data;
    for (auto it = readyData->guilds.begin(), ite = readyData->guilds.end();it != ite;++it)
        irc::rollsets.emplace(*it,"irc/" + *it);
    return PLUGIN_CONTINUE;
}

/*int rollcheckChange(Handle &handle, GlobVar *gvar, const std::string &newvalue, const std::string &oldvalue, const std::string &guild)
{
    irc::rollcheck = gvar->getAsFloat();
    return PLUGIN_CONTINUE;
}*/

int diceCmd(Handle &handle, int argc, const std::string args[], const Message &message)
{
    uint32_t lo = 1, hi = irc::dicemax->getAsInt(message.guild_id), sides = hi;
    if (irc::customdice->getAsBool(message.guild_id))
    {
        if ((argc == 2) && (std::isdigit(args[1].front())))
            sides = hi = std::stoull(args[1]);
        else if ((argc > 2) && (std::isdigit(args[1].front())) && (std::isdigit(args[2].front())))
        {
            lo = std::stoull(args[1]);
            hi = std::stoull(args[2]);
            sides = hi - lo + 1;
        }
    }
    int dice = sides/6;
    if (sides%6)
        dice++;
    if (!dice)
        dice = 1;
    if (dice > 5)
        dice = 5;
    std::string die;
    for (int i = 0;i < dice;i++)
        die += ":game_die: ";
    messageReply(message,die + "[**" + std::to_string(sides) + "**]: " + std::to_string(mtrand(lo,hi)));
    return PLUGIN_HANDLED;
}

//roll <chance>
int rollCmd(Handle &handle, int argc, const std::string args[], const Message &message)
{
    float chance = irc::rollcheck->getAsFloat(message.guild_id);
    if ((irc::customroll->getAsBool(message.guild_id)) && (argc > 1))
    {
        size_t pos = args[1].find('/');
        if ((((pos != 0) && (pos != std::string::npos)) || (((pos = args[1].find(':')) != std::string::npos) && (pos != 0))) && (pos+1 < args[1].size()))
        {
            std::string num = args[1].substr(0,pos);
            std::string den = args[1].substr(pos+1);
            if ((std::isdigit(num.front())) && (std::isdigit(den.front())))
                chance = std::stof(num) / std::stof(den);
        }
        else if (std::isdigit(args[1].front()))
        {
            chance = std::stof(args[1]);
            if (chance > 1)
                chance /= 100.0;
        }
    }
    float roll = float(mtrand(0,100)) / 100.0;
    //debugOut("[rollcheck] <chance=" + std::to_string(chance) + "> <roll=" + std::to_string(roll) + '>');
    std::string result;
    if (chance >= roll)
        result = "*Success!* You have passed the roll check!";
    else
        result = "*Oh no!* You have failed the roll check!";
    messageReply(message,result);
    return PLUGIN_HANDLED;
}

int addrollsetCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global* global = recallGlobal();
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <set_name> [outcomes ...]`");
    else
    {
        auto guild = irc::rollsets.find(message.guild_id);
        if (guild == irc::rollsets.end())
            guild = irc::rollsets.emplace(message.guild_id,INIObject()).first;
        auto set = guild->second.end(), setend = set;
        std::string name = argv[1];
        for (size_t pos = name.find('\n');pos != std::string::npos;pos = name.find('\n'))
            name[pos] = ' ';
        std::string ref = name;
        try
        {
            if (argc != 3)
            {
                std::unordered_map<std::string,bool> backrefs;
                while (true)
                {
                    backrefs.emplace(ref,true);
                    if ((set = guild->second.topic_it(ref)) == setend)
                        break;
                    if (backrefs.find((ref = set->find("references"))) != backrefs.end())
                    {
                        sendMessage(message.channel_id,"Infinite reference recursion detected!");
                        return PLUGIN_HANDLED;
                    }
                }
            }
            else
                set = guild->second.topic_it(ref);
        }
        catch (...)
        {
            
        }
        if (argc < 3)
        {
            if ((guild->second.topics() > 1) && (ref == "default") && (set != setend) && (set->topic() != ref))
            {
                set = guild->second.begin();
                ref = set->topic();
            }
            if (set == setend)
                sendMessage(message.channel_id,"No rollset named `" + ref + "`.");
            else if (set->exists("outcomes"))
            {
                int n;
                std::string* outcomes = splitString(set->find("outcomes")," ",n);
                sendMessage(message.channel_id,"`" + set->topic() + "` has the following outcomes (" + std::to_string(n) + "): " + set->find("outcomes"));
                delete[] outcomes;
            }
            else
            {
                if (set->exists("references"))
                    sendMessage(message.channel_id,"`" + set->find("references") + "` does not resolve to a valid rollset!");
                else
                    sendMessage(message.channel_id,"No rollset named `" + ref + "`.");
            }
            return PLUGIN_HANDLED;
        }
        else if (argc == 3)
        {
            std::string reference = argv[2];
            for (size_t pos = reference.find('\n');pos != std::string::npos;pos = reference.find('\n'))
                reference[pos] = ' ';
            if (!guild->second.exists(reference))
                sendMessage(message.channel_id,"Cannot create reference to nonexistant rollset `" + reference + "`.");
            else if (name == reference)
                sendMessage(message.channel_id,"Cannot reference self!");
            else
            {
                guild->second(name,"references") = reference;
                guild->second.erase(name,"outcomes");
                sendMessage(message.channel_id,"Rollset `" + name + "` now references `" + reference + "`.");
                if (guild->second.write("irc/" + message.guild_id))
                    sendMessage(message.channel_id,"An error occurred saving the database. The rollset will be lost when I inevitably crash.");
            }
        }
        else
        {
            std::string outcomes = "\"" + argv[2] + '"';
            for (int i = 3;i < argc;++i)
                outcomes += " \"" + argv[i] + '"';
            for (size_t pos = outcomes.find('\n');pos != std::string::npos;pos = outcomes.find('\n'))
                outcomes[pos] = ' ';
            if (set == setend)
                guild->second(name,"outcomes") = outcomes;
            else
            {
                (*set)("outcomes") = outcomes;
                set->erase("references");
            }
            reaction(message,"%E2%9C%85");
            if (guild->second.write("irc/" + message.guild_id))
                sendMessage(message.channel_id,"An error occurred saving the database. The rollset will be lost when I inevitably crash.");
        }
    }
    return PLUGIN_HANDLED;
}

int delrollsetCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global* global = recallGlobal();
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <set_name>`");
    else
    {
        auto guild = irc::rollsets.find(message.guild_id);
        if ((guild == irc::rollsets.end()) || (guild->second.topics() < 1))
        {
            sendMessage(message.channel_id,"No rollsets found!");
            return PLUGIN_HANDLED;
        }
        auto set = guild->second.end();
        try
        {
            set = guild->second.topic_it(argv[1]);
            std::string ref;
            size_t refs = 0;
            for (auto it = guild->second.begin(), ite = guild->second.end();it != ite;++it)
            {
                if ((it->exists("references")) && ((*it)("references") == argv[1]))
                {
                    refs++;
                    if (it->topic() == "default")
                        guild->second.erase(it);
                    else
                        (*it)("references") = "default";
                }
            }
            std::string out;
            if (set != guild->second.end())
            {
                guild->second.erase(set);
                out = "Successfully removed rollset `" + argv[1] + "`.";
            }
            else
                out = "No rollset named `" + argv[1] + "`.";
            if (refs > 1)
                out += "\nAlso redirected " + std::to_string(refs) + " references to this rollset.";
            else if (refs > 0)
                out += "\nAlso redirected 1 reference to this rollset.";
            if (guild->second.write("irc/" + message.guild_id))
                out += "\nAn error occurred saving the database. The rollset will return when I inevitably crash.";
            sendMessage(message.channel_id,out);
        }
        catch (...)
        {
            sendMessage(message.channel_id,"No rollset named `" + argv[1] + "`.");
            return PLUGIN_HANDLED;
        }
    }
    return PLUGIN_HANDLED;
}

int rollsetCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global* global = recallGlobal();
    auto guild = irc::rollsets.find(message.guild_id);
    if ((guild == irc::rollsets.end()) || (guild->second.topics() < 1))
    {
        sendMessage(message.channel_id,"No rollsets found!");
        return PLUGIN_HANDLED;
    }
    auto set = guild->second.end(), setend = set;
    std::string ref = "default";
    if (argc > 1)
        ref = argv[1];
    try
    {
        std::unordered_map<std::string,bool> backrefs;
        while (true)
        {
            backrefs.emplace(ref,true);
            set = guild->second.topic_it(ref);
            if (set == setend)
            {
                sendMessage(message.channel_id,"No rollset named `" + ref + "`.");
                return PLUGIN_HANDLED;
            }
            ref.clear();
            if (backrefs.find((ref = set->find("references"))) != backrefs.end())
            {
                sendMessage(message.channel_id,"Infinite reference recursion detected!");
                return PLUGIN_HANDLED;
            }
        }
    }
    catch (...)
    {
        if (ref.size() > 0)
        {
            if (ref == "default")
            {
                set = guild->second.begin();
                ref = set->topic();
            }
            else
            {
                sendMessage(message.channel_id,"No rollset named `" + ref + "`.");
                return PLUGIN_HANDLED;
            }
        }
    }
    if (ref.size() < 1)
        ref = set->topic();
    if (!set->exists("outcomes"))
    {
        sendMessage(message.channel_id,"No default rollset!");
        return PLUGIN_HANDLED;
    }
    int n;
    std::string* outcomes = splitString(set->find("outcomes")," ",n);
    n = mtrand(1,n)-1;
    if ((outcomes[n].front() == '"') && (outcomes[n].back() == '"'))
        outcomes[n] = outcomes[n].substr(1,outcomes[n].size()-2);
    sendMessage(message.channel_id,"[**" + ref + "**] *" + makeMention(message.author.id,message.guild_id) + " rolls " + outcomes[n] + '*');
    delete[] outcomes;
    return PLUGIN_HANDLED;
}

int rollsetsCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    std::string out;
    auto guild = irc::rollsets.find(message.guild_id);
    if (guild != irc::rollsets.end()) for (auto it = guild->second.begin(), ite = guild->second.end();it != ite;++it)
        out += it->topic() + '\n';
    if (out.size() > 0)
        sendMessage(message.channel_id,"The following rollsets exist:\n```\n" + out + "```");
    else
        sendMessage(message.channel_id,"No rollsets found!");
    return PLUGIN_HANDLED;
}


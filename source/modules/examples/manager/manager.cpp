#include "api/farage.h"
#include <vector>
#include <locale>
#include <codecvt>
#include <unordered_map>
#include <dlfcn.h>
using namespace Farage;

#define HEXIFY
#define SPLITSTRINGANY
#include "common_func.h"

#define VERSION "v0.3.3"

extern "C" Info Module
{
    "Manager",
    "Madison",
    "General Server Manager",
    VERSION,
    "http://server.management.justca.me/",
    FARAGE_API_VERSION
};

namespace Manager
{
    rens::regex reSpec ("(\\d+[smdh])(,(?1))*");
    rens::regex reSpecGet ("(\\d+)([smdh]),?");
    rens::regex reUrl ("https?://[\\w-.]+");
    rens::regex reMsgUrl ("^https://discord(app)?\\.com/channels/([0-9]+)/([0-9]+)/([0-9]+)$");
    rens::regex reEmoji ("<?a?:([-\\w]+:\\d+)>?");
    struct timerDataStruct
    {
        std::vector<std::string> reactions;
        std::string channel;
        std::string message;
    };
    struct hookData
    {
        int retVal;
        int flags;
        bool chat;
        std::string command;
        std::string from;
        std::string guild;
        std::string channel;
    };
    std::unordered_map<std::string,hookData> hookDatas;
    int callChatCmd(Global* global, const Message& message)
    {
        int ret = PLUGIN_CONTINUE;
        int argc = 0;
        std::string *argv = splitStringAny(nospace(message.content)," \t\n",argc);
        if (argc > 0)
        {
            auto plug = global->plugins.begin(), pluge = global->plugins.end();
            bool done = false;
            for (;plug != pluge;++plug)
            {
                if ((*plug)->getLoadPriority() != 0)
                    break;
                if ((ret = (*plug)->callChatCmd(argv[0],ROOT,argc,argv,message)) == PLUGIN_HANDLED)
                {
                    done = true;
                    break;
                }
            }
            if (!done) for (;plug != pluge;++plug)
                if ((ret = (*plug)->callChatCmd(argv[0],ROOT,argc,argv,message)) == PLUGIN_HANDLED)
                    break;
        }
        delete[] argv;
        return ret;
    }
    void (*evalData)(std::string&,const std::string&,const std::string&,const std::string&,const std::string&) = nullptr;
};

std::time_t convertTimestamp(std::string timestamp)
{
    size_t pos = timestamp.find('.');
    if (pos != std::string::npos)
        timestamp.erase(pos);
    tm timem;
    strptime(timestamp.c_str(),"%FT%T",&timem);
    return mktime(&timem);
}

int setPermission(Handle&,int,const std::string[]);
int chatPurge(Handle&,int,const std::string[],const Message&);
int chatRespond(Handle&,int,const std::string[],const Message&);
int chatReact(Handle&,int,const std::string[],const Message&);
//int chatUnreact(Handle&,int,const std::string[],const Message&);
int chatHookMsg(Handle&,int,const std::string[]);
/*int chatHookReaction(Handle&,int,const std::string[],const Message&);
int chatHookEdit(Handle&,int,const std::string[],const Message&);
int chatHookDelete(Handle&,int,const std::string[],const Message&);*/

extern "C" int onModuleStart(Handle& handle, Global* global)
{
    recallGlobal(global);
    handle.createGlobVar("gm_version",VERSION,"General Server Manager Version",GVAR_CONSTANT);
    handle.regConsoleCmd("setpermission",&setPermission,"Change the permissions for a chat command.");
    handle.regChatCmd("purge",&chatPurge,BAN,"Delete messages within a channel.");
    handle.regChatCmd("respond",&chatRespond,GENERIC,"Send a message in response to the command.");
    handle.regChatCmd("react",&chatReact,GENERIC,"React to a message.");
    //handle.regChatCmd("unreact",&chatUnreact,EDITMSG,"Remove reactions from a message.");
    handle.regConsoleCmd("hookchat",&chatHookMsg,"Hook a chat message to a chat or console command.");
    for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
    {
        if ((*it)->getModule() == "udata")
        {
            *(void **) (&Manager::evalData) = dlsym((*it)->getModulePtr(),"evaluateData");
            if (dlerror() != NULL)
                Manager::evalData = nullptr;
            break;
        }
    }
    return 0;
}

extern "C" int onModulesLoaded(Handle &handle, int event, void *iterator, void *position, void *foo, void *bar)
{
    Global* global = recallGlobal();
    if (Manager::evalData == nullptr)
    {
        for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
        {
            if ((*it)->getModule() == "udata")
            {
                *(void **) (&Manager::evalData) = dlsym((*it)->getModulePtr(),"evaluateData");
                if (dlerror() != NULL)
                    Manager::evalData = nullptr;
                break;
            }
        }
    }
    return PLUGIN_CONTINUE;
}

int chatPurge(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global *global = recallGlobal();
    std::string prefix = global->prefix(message.guild_id);
    if ((argc < 2) || (argv[1] == "--help"))
    {
        sendEmbed(
            message.channel_id,
            "{ \"color\": 4565417, \"title\": \"Manager Purge Help\", \"description\": \"Delete messages from a channel.\", \"fields\": [{ \"name\": \"Usage:\", \"value\": \"`" + prefix + "purge [OPTIONS ...] [match_text ...]`\" }, { \"name\": \"OPTIONS:\", \"value\": \"" +
            "`--`                        Marks the end of the OPTIONS. Useful if\\n" + 
            "                             `match_text` begins with `--`.\\n" +
            "`--regex`                   Declares the `match_text` as a regex pattern.\\n" +
        #ifdef FARAGE_USE_PCRE2
            "                             Uses PCRE2 regex style.\\n" +
        #else
            "                             Uses ECMAScript-like regex style with recursion+.\\n" +
        #endif
            //"`--force`                   Skip the verification message.\\n" +
            "`--in <channel>`            Set the channel. Defaults to the current channel.\\n" +
            "`--count <count>`           Set the number of matching messages to delete.\\n" +
            "                             Defaults to 50.\\n" +
            "`--from <user>`             Only find messages from user.\\n" +
            "`--has 'embed|file`         Only find messages that contain the option.\\n" +
            "       `link|text'`          This option can be used multiple times.\\n" +
            "`--lacks 'embed|file`       Only find messages that do not contain the option.\\n" +
            "         `link|text'`        This option can be used multiple times.\" }, { \"name\": \"Time Options:\", \"value\": \"" +
            "`--ref <message_id>`        Use a message as a point to begin the search from.\\n" +
            "                             The reference will also be included in the search\\n" +
            "                             unless prefixed with a `!`.\\n" +
            "                             Defaults to the message containing the\\n" +
            "                             command.\\n" +
            "`--before [time]`           Get only messages before the reference.\\n" +
            "                             Default.\\n" +
            "`--after [time]`            Get only messages after the reference.\\n" +
            "                             Can be combined with `--before`.\\n" +
            "`--around`                  Get messages around the reference.\\n" +
            "                             Cannot be combined with `--before`\\n" +
            "                             or `--after`.\\n\\n" +
            "`time` allows an optional time duration to span till.\\nThis can be in a format of series of `Nt` seperated by commas.\\nWhere `N` is a number and `t` is a time specification of the following:\\n`s` for seconds.\\n`m` for minutes.\\n`h` for hours.\\n`d` for days.\" }, { \"name\": \"Examples:\", \"value\": \"`" + prefix + "purge --before 1h,30m --from @nigel#9203`\\nThis will span all messages 1.5 hours ago within the current channel and delete all messages from nigel (Maximum of 50 messages).\\n\\n`" + prefix + "purge --count 100`\\nThis will delete the last 100 messages from all users in the current channel.\\n\\n`" + prefix + "purge --ref 574476211275956224 --before 30m --after 30m --count 0`\\nThis will delete all messages from all users in the current channel that were sent 30 minutes before or 30 minutes after the referenced message, including the refenced message. (No maximum number of messages).\\n\\n`" + prefix + "purge --in #general --ref 574476211275956224 --after --from @nigel#9203 --count 20 no u`\\nThis will delete the first 20 messages from nigel sent after the referenced message in #general that contain the text `no u`.\" }]}"
        );
    }
    else
    {
        bool regex = false;
        bool force = true;
        std::string channel = message.channel_id;
        int count = 50;
        std::string user;
        int has = 0, lacks = 0;
        std::string ref;
        int when = 1;
        std::string timespec[2], match;
        int a = 1;
        bool lastArg;
        for (;(a < argc) && (argv[a].find("--") == 0);a++)
        {
            std::string arg = strlower(argv[a].substr(2));
            if (a+1 >= argc)
                lastArg = true;
            else
                lastArg = false;
            if (arg.size() < 1)
            {
                a++;
                break;
            }
            else if (arg == "regex")
                regex = true;
            else if (arg == "force")
                force = true;
            else if (arg == "in")
            {
                if (lastArg)
                {
                    sendMessage(message.channel_id,"Error: `--in` expects a channel!");
                    return PLUGIN_HANDLED;
                }
                bool found = false;
                channel = argv[++a];
                if ((channel.back() == '>') && (channel.find("<#") == 0))
                    channel = channel.substr(2,channel.size()-3);
                if (channel.front() == '#')
                    channel.erase(0,1);
                Server cache = getGuildCache(message.guild_id);
                for (auto it = cache.channels.begin(), ite = cache.channels.end();it != ite;++it)
                {
                    if ((it->id == channel) || (it->name == channel))
                    {
                        channel = it->id;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    sendMessage(message.channel_id,"Error: Unknown channel `" + channel + "`!");
                    return PLUGIN_HANDLED;
                }
            }
            else if (arg == "count")
            {
                if (lastArg)
                {
                    sendMessage(message.channel_id,"Error: `--count` expects a number!");
                    return PLUGIN_HANDLED;
                }
                arg = argv[++a];
                if (std::isdigit(arg.front()))
                    count = std::stoi(arg);
                else
                {
                    sendMessage(message.channel_id,"Error: `--count` expects a number!");
                    return PLUGIN_HANDLED;
                }
            }
            else if (arg == "from")
            {
                if (lastArg)
                {
                    sendMessage(message.channel_id,"Error: `--from` expects a mention or user ID!");
                    return PLUGIN_HANDLED;
                }
                arg = argv[++a];
                if ((arg.back() == '>') && (arg.find("<@") == 0))
                {
                    user = arg.substr(2,arg.size()-3);
                    if (user.front() == '!')
                        user.erase(0,1);
                }
                else if (std::isdigit(arg.front()))
                    user = arg;
                else
                {
                    sendMessage(message.channel_id,"Error: `--from` expects a mention or user ID!");
                    return PLUGIN_HANDLED;
                }
            }
            else if (arg == "has")
            {
                if (lastArg)
                {
                    sendMessage(message.channel_id,"Error: `--has` expects one of the following `embed|file|link|text`!");
                    return PLUGIN_HANDLED;
                }
                arg = argv[++a];
                if (arg == "embed")
                    has |= 1;
                else if (arg == "file")
                    has |= 2;
                else if (arg == "link")
                    has |= 4;
                else if (arg == "text")
                    has |= 8;
                else
                {
                    sendMessage(message.channel_id,"Error: `--has` expects one of the following `embed|file|link|text`!");
                    return PLUGIN_HANDLED;
                }
            }
            else if (arg == "lacks")
            {
                if (lastArg)
                {
                    sendMessage(message.channel_id,"Error: `--lacks` expects one of the following `embed|file|link|text`!");
                    return PLUGIN_HANDLED;
                }
                arg = argv[++a];
                if (arg == "embed")
                    lacks |= 1;
                else if (arg == "file")
                    lacks |= 2;
                else if (arg == "link")
                    lacks |= 4;
                else if (arg == "text")
                    lacks |= 8;
                else
                {
                    sendMessage(message.channel_id,"Error: `--lacks` expects one of the following `embed|file|link|text`!");
                    return PLUGIN_HANDLED;
                }
            }
            else if (arg == "ref")
            {
                if (lastArg)
                {
                    sendMessage(message.channel_id,"Error: `--ref` expects a message ID or `!`!");
                    return PLUGIN_HANDLED;
                }
                ref = argv[++a];
            }
            else if (arg == "before")
            {
                if (!lastArg)
                {
                    arg = argv[a+1];
                    if ((arg.find("--") != 0) && (rens::regex_match(arg,Manager::reSpec)))
                    {
                        timespec[0] = arg;
                        a++;
                    }
                }
                when |= 1;
            }
            else if (arg == "after")
            {
                if (!lastArg)
                {
                    arg = argv[a+1];
                    if ((arg.find("--") != 0) && (rens::regex_match(arg,Manager::reSpec)))
                    {
                        timespec[1] = arg;
                        a++;
                    }
                }
                when |= 2;
            }
            else if (arg == "around")
            {
                when = 0;
            }
        }
        if (a < argc)
            match = argv[a];
        while (++a < argc)
            match = match + ' ' + argv[a];
        //sendMessage(message.channel_id,"Argument processing:\nregex == `" + std::to_string(regex) + "`\nchannel == `" + channel + "`\ncount == `" + std::to_string(count) + "`\nuser == `" + user + "`\nhas == `" + std::to_string(has) + "`\nlacks == `" + std::to_string(lacks) + "`\nref == `" + ref + "`\nwhen == `" + std::to_string(when) + "`\ntimespec[0] == `" + timespec[0] + "`\ntimespec[1] == `" + timespec[1] + "`\nmatch == `" + match + '`');
        std::vector<std::string> messageIDs;
        std::vector<Message> search;
        std::time_t refTime, fromTime[2] = {0,0};
        std::string timestamp;
        for (int i = 0;i < 2;i++)
        {
            rens::smatch ml;
            while (rens::regex_search(timespec[i],ml,Manager::reSpecGet))
            {
                long frame = std::stol(ml[1].str());
                switch (ml[2].str().front())
                {
                    case 's':
                    {
                        fromTime[i] += frame;
                        break;
                    }
                    case 'm':
                    {
                        fromTime[i] += frame*60;
                        break;
                    }
                    case 'h':
                    {
                        fromTime[i] += frame*60*60;
                        break;
                    }
                    case 'd':
                    {
                        fromTime[i] += frame*24*60*60;
                        break;
                    }
                }
                timespec[i] = ml.suffix().str();
            }
        }
        bool includeRef = true;
        if (ref.front() == '!')
        {
            includeRef = false;
            ref.erase(0,1);
        }
        if (ref.size() == 0)
            ref = message.id;
        ObjectResponse<Message> reference = getMessage(channel,ref);
        if (reference.response.error())
        {
            sendMessage(message.channel_id,"Unable to get reference message!");
            return PLUGIN_HANDLED;
        }
        refTime = convertTimestamp(reference.object.timestamp);
        //sendMessage(message.channel_id,"Reference time: `" + timestamp + "`, `" + std::to_string(refTime) + '`');
        fromTime[0] = refTime - fromTime[0];
        fromTime[1] = refTime + fromTime[1];
        //sendMessage(message.channel_id,"Getting messages between `" + std::to_string(fromTime[0]) + "` and `" + std::to_string(fromTime[1]) + "`\n[GMT] From: `" + ctime(&fromTime[0]) + "` Until: `" + ctime(&fromTime[1]) + '`');
        bool timeRange = (fromTime[0] != fromTime[1]);
        bool hasMax = ((count > 0) || (!timeRange));
        if ((hasMax) && (count < 1))
        {
            sendMessage(message.channel_id,"Error: `--count` must not be 0 unless a complete time range is provided.");
            return PLUGIN_HANDLED;
        }
        bool fromUser = (user.size() > 0);
        bool needsMatch = (match.size() > 0);
        rens::regex ptrn;
        if ((regex) && (needsMatch))
            ptrn = match;
        bool foundAll = false;
        bool isRef;
        bool refMatched = false;
        size_t deletedMessages = 0;
        std::string output;
        //uint8_t qlimit = (((count > 50) || (count == 0)) ? 50 : count);
        std::string cref = ref;
        while (!foundAll)
        {
            for (auto it = search.rbegin(), ite = search.rend();it != ite;++it)
            {
                bool found = false;
                for (auto& m : messageIDs)
                {
                    if (it->id == m)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    cref = it->id;
                    break;
                }
            }
            messageIDs.clear();
            if (when < 3)
            {
                search = std::move(getMessages(channel,GetMessagesKey(1+when),cref,30).array);
                if (when == 0)
                    foundAll = true;
            }
            else
            {
                search = std::move(getMessages(channel,before,cref,15).array);
                std::vector<Message> msgs = getMessages(channel,after,cref,15).array;
                search.reserve(search.size()+msgs.size());
                for (auto it = msgs.begin(), ite = msgs.end();it != ite;++it)
                    search.push_back(std::move(*it));
            }
            
            for (auto it = search.begin(), ite = search.end();it != ite;++it)
            {
                consoleOut(it->author.username + ' ' + it->content);
                if (((isRef = (it->id == ref))) && ((!includeRef) || (refMatched)))
                    continue;
                std::time_t stamp = convertTimestamp(it->timestamp);
                if ((timeRange) && ((fromTime[0] > stamp) || (stamp > fromTime[1])))
                    continue;
                if ((fromUser) && (it->author.id != user))
                    continue;
                if (needsMatch)
                {
                    if ((regex) && (!rens::regex_search(it->content,ptrn)))
                        continue;
                    else if ((!regex) && (it->content.find(match) == std::string::npos))
                        continue;
                }
                if ((has & 1) && (it->embeds.size() == 0))
                    continue;
                if ((has & 2) && (it->attachments.size() == 0))
                    continue;
                if ((has & 4) && (!rens::regex_search(it->content,Manager::reUrl)))
                    continue;
                if ((has & 8) && (it->content.size() == 0))
                    continue;
                if ((lacks & 1) && (it->embeds.size() > 0))
                    continue;
                if ((lacks & 2) && (it->attachments.size() > 0))
                    continue;
                if ((lacks & 4) && (rens::regex_search(it->content,Manager::reUrl)))
                    continue;
                if ((lacks & 8) && (it->content.size() > 0))
                    continue;
                if (isRef)
                    refMatched = true;
                else
                    messageIDs.push_back(it->id);
                if ((hasMax) && (messageIDs.size() + deletedMessages >= count))
                {
                    foundAll = true;
                    break;
                }
            }
            if ((search.size() < 1) || ((search.size() == 1) && (search[0].id == ref)))
            {
                foundAll = true;
                break;
            }
            if (messageIDs.size() > 0)
            {
                if (!bulkDeleteMessages(channel,messageIDs).result)
                    output += "Failed to delete " + std::to_string(messageIDs.size()) + " message(s)...\n";
                else
                    deletedMessages += messageIDs.size();
                count -= messageIDs.size();
            }
            else
                break;
        }
        if ((includeRef) && (refMatched))
        {
            if (!deleteMessage(channel,ref).result)
                output += "Failed to delete the reference message...\n";
            else
                deletedMessages++;
        }
        sendMessage(message.channel_id,output + "Successfully deleted " + std::to_string(deletedMessages) + " message(s).");
    }
    return PLUGIN_HANDLED;
}

int setPermission(Handle& handle, int argc, const std::string argv[])
{
    if (argc < 2)
        consoleOut("Usage: " + argv[0] + " <command> [permissions]");
    else
    {
        AdminFlag perms = NOFLAG;
        bool view = true;
        if (argc > 2)
        {
            if (std::isdigit(argv[2].front()))
                perms = AdminFlag(std::stoi(argv[2]));
            else
                perms = getAdminFlagBits(argv[2]);
            view = false;
        }
        Global* global = recallGlobal();
        bool found = false;
        for (auto& mod : global->plugins)
        {
            for (auto& cmd : mod->chatCommands)
            {
                if (cmd.cmd == argv[1])
                {
                    if (view)
                        consoleOut("  [" + mod->getModule() + "]: " + cmd.cmd + " has flags: " + getAdminFlagString(cmd.flag) + '(' + std::to_string(cmd.flag) + ')');
                    else
                    {
                        cmd.flag = perms;
                        consoleOut("  [" + mod->getModule() + "]: " + cmd.cmd + " now has flags: " + getAdminFlagString(cmd.flag) + '(' + std::to_string(cmd.flag) + ')');
                    }
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
        if (!found)
            consoleOut("  Unable to find command: " + argv[1]);
    }
    return PLUGIN_HANDLED;
}

int chatRespond(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <message ...>`");
    else
        sendMessage(message.channel_id,message.content.substr(message.content.find(argv[0]) + argv[0].size() + 1));
    return PLUGIN_HANDLED;
}

int reactTimer(Handle& handle, Timer* timer, void* data)
{
    Manager::timerDataStruct* reactions = (Manager::timerDataStruct*)data;
    if (reactions->reactions.size())
    {
        auto it = reactions->reactions.begin();
        reactToID(reactions->channel,reactions->message,*it);
        reactions->reactions.erase(it);
    }
    if (reactions->reactions.size())
        return PLUGIN_CONTINUE;
    delete reactions;
    return PLUGIN_HANDLED;
}

int chatReact(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " [message ID|link] <emojis ...>`");
    else
    {
        Manager::timerDataStruct* data = new Manager::timerDataStruct;
        data->channel = message.channel_id;
        data->message = message.id;
        rens::smatch ml;
        int i = 1;
        if (std::isdigit(argv[1].front()))
        {
            data->message = argv[1];
            ++i;
        }
        else if (rens::regex_match(argv[1],ml,Manager::reMsgUrl))
        {
            data->channel = ml[3].str();
            data->message = ml[4].str();
            ++i;
        }
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        for (;i < argc;++i)
        {
            std::string foo = argv[i];
            std::wstring test = converter.from_bytes(foo);
            if (foo.size() > test.size())
                data->reactions.push_back(hexify(foo));
            else while (rens::regex_search(foo,ml,Manager::reEmoji))
            {
                data->reactions.push_back(ml[1].str());
                foo = ml.suffix().str();
            }
        }
        if (data->reactions.size() < 1)
        {
            sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " [message ID|link] <emojis ...>`");
            delete data;
        }
        else
        {
            auto it = data->reactions.begin();
            reactToID(data->channel,data->message,*it);
            data->reactions.erase(it);
            if (data->reactions.size() > 0)
                handle.createTimer("react",1,&reactTimer,(void*)data);
            else
                delete data;
        }
    }
    return PLUGIN_HANDLED;
}

/*int chatUnreact(Handle &handle, int argc, const std::string argv[], const Message& message)
{
    
    return PLUGIN_HANDLED;
}*/

int chatHookCB(Handle& handle, ChatHook* hook, const rens::smatch& ml, const Message& message)
{
    auto data = Manager::hookDatas.find(hook->name);
    if (data == Manager::hookDatas.end())
        return PLUGIN_ERASE;
    hook->flags = 0;
    if (data->second.from.size() > 0)
    {
        if (data->second.from.front() == '!')
        {
            if (data->second.from.find(message.author.id,1) == 1)
                return PLUGIN_CONTINUE;
        }
        else if (message.author.id != data->second.from)
            return PLUGIN_CONTINUE;
    }
    if (data->second.guild.size() > 0)
    {
        if (data->second.guild.front() == '!')
        {
            if (data->second.guild.find(message.guild_id,1) == 1)
                return PLUGIN_CONTINUE;
        }
        else if (message.guild_id != data->second.guild)
            return PLUGIN_CONTINUE;
    }
    if (data->second.channel.size() > 0)
    {
        if (data->second.channel.front() == '!')
        {
            if (data->second.channel.find(message.channel_id,1) == 1)
                return PLUGIN_CONTINUE;
        }
        else if (message.channel_id != data->second.channel)
            return PLUGIN_CONTINUE;
    }
    hook->flags = data->second.flags;
    std::string cmd = data->second.command;
    cmd = ml.format(cmd);
    if (Manager::evalData != nullptr)
        Manager::evalData(cmd,message.guild_id,message.channel_id,message.author.id,message.id);
    if (cmd.size() > 0)
    {
        if (data->second.chat)
        {
            Message modified = message;
            modified.content = std::move(cmd);
            Manager::callChatCmd(recallGlobal(),modified);
        }
        else
            serverCommand(cmd);
    }
    return data->second.retVal;
}

int chatHookMsg(Handle& handle, int argc, const std::string argv[])
{
    // Usage: hookmsg [options] <name> <command> [pattern]
    std::string name, ptrn;
    Manager::hookData data;
    bool options = true;
    data.chat = false;
    data.retVal = PLUGIN_CONTINUE;
    data.flags = 0;
    int i = 1;
    for (;i < argc;++i)
    {
        bool last = (i+1 == argc);
        if (argv[i].find("--") == 0)
        {
            if (argv[i] == "--chat")
                data.chat = true;
            else if (argv[i] == "--console")
                data.chat = false;
            else if (argv[i] == "--flags")
            {
                if ((last) || (!std::isdigit(argv[++i].front())))
                {
                    consoleOut("Error: '--flags' requires a value: '--flags <int>'");
                    return PLUGIN_HANDLED;
                }
                data.flags = std::stoi(argv[i]);
            }
            else if (argv[i] == "--return")
            {
                if ((last) || (!std::isdigit(argv[++i].front())))
                {
                    consoleOut("Error: '--return' requires a value: '--return <int>'");
                    return PLUGIN_HANDLED;
                }
                data.retVal = std::stoi(argv[i]);
            }
            else if (argv[i] == "--from")
            {
                char d;
                if ((!last) && (argv[++i].size() > 1) && (argv[i].front() == '!'))
                    d = argv[i].at(1);
                else
                    d = argv[i].front();
                if ((last) || (!std::isdigit(d)))
                {
                    consoleOut("Error: '--from' requires a value: '--from <ID>'");
                    return PLUGIN_HANDLED;
                }
                data.from = argv[i];
            }
            else if (argv[i] == "--guild")
            {
                char d;
                if ((!last) && (argv[++i].size() > 1) && (argv[i].front() == '!'))
                    d = argv[i].at(1);
                else
                    d = argv[i].front();
                if ((last) || (!std::isdigit(d)))
                {
                    consoleOut("Error: '--guild' requires a value: '--guild <ID>'");
                    return PLUGIN_HANDLED;
                }
                data.guild = argv[i];
            }
            else if (argv[i] == "--channel")
            {
                char d;
                if ((!last) && (argv[++i].size() > 1) && (argv[i].front() == '!'))
                    d = argv[i].at(1);
                else
                    d = argv[i].front();
                if ((last) || (!std::isdigit(d)))
                {
                    consoleOut("Error: '--channel' requires a value: '--channel <ID>'");
                    return PLUGIN_HANDLED;
                }
                data.channel = argv[i];
            }
            else if (argv[i] == "--")
            {
                ++i;
                break;
            }
        }
        else
            break;
    }
    if (argc-i < 2)
        consoleOut("Usage: " + argv[0] + " [options] <name> <command> [pattern]\n  options:\n    --chat - Trigger a chat command (default)\n    --console - Trigger a console command\n    --flags <int> - Set hook flags.\n    --return <int> - Set hook return value.\n    --from <ID> - Restrict trigger to a specific person\n    --guild <ID> - Restrict trigger to a specific guild\n    --channel <ID> - Restrict trigger to a specific channel. (--guild can be ignored)");
    else
    {
        name = argv[i++];
        data.command = argv[i++];
        for (;i < argc;++i)
            ptrn += argv[i] + ' ';
        if (ptrn.size() > 0)
            ptrn.erase(ptrn.size()-1);
        bool replaced = false;
        auto c = Manager::hookDatas.find(name);
        if (c != Manager::hookDatas.end())
        {
            c->second = data;
            handle.unhookChatPattern(name);
            replaced = true;
        }
        else
            Manager::hookDatas.emplace(name,data);
        handle.hookChatPattern(name,ptrn,&chatHookCB);
        if (replaced)
            consoleOut("Successfully replaced hook '" + name + "', '" + ptrn + "', from '" + data.from + "', in '" + data.guild + ":" + data.channel + "', triggering " + (data.chat ? "(chat) " : "(console) ") + data.command);
        else
            consoleOut("Successfully created hook '" + name + "', '" + ptrn + "', from '" + data.from + "', in '" + data.guild + ":" + data.channel + "', triggering " + (data.chat ? "(chat) " : "(console) ") + data.command);
    }
    return PLUGIN_HANDLED;
}

/*int chatHookReaction(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    
    return PLUGIN_HANDLED;
}

int chatHookEdit(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    
    return PLUGIN_HANDLED;
}

int chatHookDelete(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    
    return PLUGIN_HANDLED;
}*/


















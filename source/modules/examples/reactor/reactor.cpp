#include "api/farage.h"
using namespace Farage;

extern "C" Info Module
{
    "Reaction Tester",
    "nigel",
    "Reaction Tester",
    "0.0.7",
    "http://you.justca.me/",
    FARAGE_API_VERSION
};

int chatHookReaction(Handle&,int,const std::string[],const Message&);
int hookMan(Handle&,ReactHook*,const ServerMember&,const Channel&,const std::string&,const std::string&,const Emoji&);
int chatHookDelete(Handle&,int,const std::string[],const Message&);
int hookDel(Handle&,DeleteHook*,const std::string&,const std::string&);

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.hookReaction("react",nullptr,HOOK_PRINT);
    handle.hookDeleteMessage("delete",nullptr,HOOK_PRINT|HOOK_CALL_ONCE);
    handle.regChatCmd("hookreaction",&chatHookReaction,GENERIC,"Testing some stuff...");
    handle.regChatCmd("hookdelete",&chatHookDelete,GENERIC,"Testing some stuff...");
    return 0;
}

/*extern "C" int onReaction(Handle &handle, Event event, void *user, void *channel, void *message, void *moji)
{
    Global *global = recallGlobal();
    Channel *channel = (Channel*)channel;
    ServerMember *member = (ServerMember*)user;
    std::string *messageID = (std::string*)message;
    Emoji *emoji = (Emoji*)moji;
    consoleOut("[react][" + channel->name + "][" + member->user.username + "][" + *messageID + "]  id = " + emoji->id + " :: name = " + emoji->name);
    return PLUGIN_CONTINUE;
}*/

int chatHookReaction(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    std::string id = message.id;
    std::string user;
    Emoji emoji;
    if (argc > 1)
        id = argv[1];
    if ((argc > 2) && (argv[2] != "null"))
        user = argv[2];
    if (argc > 3)
    {
        static rens::regex emojiptrn ("<:([^:]+):(\\d+)>");
        rens::smatch ml;
        if (rens::regex_match(argv[3],ml,emojiptrn))
        {
            emoji.name = ml[1].str();
            emoji.id = ml[2].str();
        }
        else
            emoji.name = argv[3];
    }
    size_t reactors = handle.reactHooks.size();
    handle.hookReactionMessage("manhook" + std::to_string(reactors),&hookMan,0,id,emoji,user);
    return PLUGIN_HANDLED;
}

int hookMan(Handle &handle, ReactHook *hook, const ServerMember &member, const Channel &channel, const std::string &messsageID, const std::string &guildID, const Emoji &emoji)
{
    sendMessage(channel.id,"[" + hook->name + "] <" + member.nick + "> reacted with " + emoji.display());
    return PLUGIN_CONTINUE;
}

int chatHookDelete(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    std::string id, chan = message.channel_id;
    HookType type = msg;
    if (argc > 1)
    {
        if (argv[1] == "chan")
        {
            id = message.guild_id;
            type = guild;
            if (argc > 2)
                id = argv[2];
        }
        else if (argv[1] == "guild")
        {
            id = message.guild_id;
            chan.clear();
            type = guild;
            if (argc > 2)
                id = argv[2];
        }
        else if (argv[1] == "any")
        {
            chan.clear();
            type = any;
        }
        else
            id = argv[1];
    }
    else
        id = message.id;
    DeleteHook *hook = new DeleteHook{message.channel_id,chan,id,type,&hookDel,0};
    handle.deleteHooks.push_back(hook);
    return PLUGIN_HANDLED;
}

int hookDel(Handle &handle, DeleteHook *hook, const std::string &channel, const std::string &message)
{
    sendMessage(hook->name,"<" + message + "> has been deleted from <#" + channel + '>');
    return PLUGIN_CONTINUE;
}



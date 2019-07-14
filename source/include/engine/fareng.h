#ifndef _FARAGE_ENGINE_
#define _FARAGE_ENGINE_

#include "sleepy_discord/websocketpp_websocket.h"
#include <iostream>
#include <thread>
#include <fstream>
#include <ratio>
#include <ctime>
#include <atomic>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <dirent.h>
#include <dlfcn.h>
#include "conf/.farageEngineBuild.h"
#include "conf/sleepyversion.h"
#include "api/farage.h"
#include "engine/convert.h"
#include "shared/libini.h"

#ifndef FARAGE_CONNECT_DELAY
#define FARAGE_CONNECT_DELAY        1       // Seconds to wait between connection attempts
#endif
#ifndef FARAGE_CONNECT_MAX_RETRIES
#define FARAGE_CONNECT_MAX_RETRIES  99      // Max number of consequtive attempts to reconnect before exiting
#endif
#ifndef FARAGE_TIMEOUT
#define FARAGE_TIMEOUT              30      // Max seconds between checking connection is alive
#endif

namespace Farage
{
    class BotClass;
    
    namespace Internal
    {
        namespace Console
        {
            struct Command
            {
                int (*cb)(Farage::BotClass*,Farage::Global&,int,const std::string[]);
                std::string desc;
            };
            int version(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int modules(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int setprefix(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int reloadadmins(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int gvar(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int ignoreuser(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int ignorechannel(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int setroleflags(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int sendmsg(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int execute(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int nick(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int leave(Farage::BotClass*,Farage::Global&,int,const std::string[]);
            int help(Farage::BotClass*,Farage::Global&,int,const std::string[]);
        };
        
        namespace Chat
        {
            struct Command
            {
                int (*cb)(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
                int flag;
                std::string desc;
            };
            int version(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int setprefix(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int reloadadmins(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int gvar(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int rcon(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int ignoreuser(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int ignorechannel(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int setroleflags(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int execute(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int cmdhelp(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
            int gvarhelp(Farage::BotClass*,Farage::Global&,int,const std::string[],const SleepyDiscord::Message&);
        };
    };
    
    std::string nospace(std::string text)
    {
        bool s = true;
        for (auto it = text.begin();it != text.end();)
        {
            switch (*it)
            {
                case ' ':
                case '\t':
                {
                    if (s)
                        it = text.erase(it);
                    else
                    {
                        ++it;
                        s = true;
                    }
                    break;
                }
                default:
                {
                    ++it;
                    s = false;
                }
            }
        }
        if (s)
        {
            switch (text.at(text.size()-1))
            {
                case ' ':
                case '\t':
                {
                    text.erase(text.size()-1);
                }
            }
        }
        return text;
    }
    
    std::string remove(std::string str, const std::string &substr)
    {
        for (size_t pos = 0, sublen = substr.size();(pos = str.find(substr,pos)) != std::string::npos;str.erase(pos,sublen));
        return str;
    }
    
    std::string removeall(std::string str, const std::string &chars)
    {
        for (size_t pos = 0;(pos = str.find_first_of(chars,pos)) != std::string::npos;str.erase(pos,1));
        return str;
    }
    
    bool str2bool(std::string str)
    {
        str = strlower(nospace(str));
        if ((str.size() < 1) || (str == "false") || (str == "0"))
            return false;
        return true;
    }
    
    std::string *splitString(const std::string &src, const std::string &delim, int &count)
    {
        if (src.size() == 0)
            return nullptr;
        std::vector<std::string> list;
        bool open = false;
        for (size_t x = 0, slen = src.size(), dlen = delim.size(), last = 0; x < slen;)
        {
            if (src.at(x) == '"')
            {
                open = !open;
                if ((open) && (src.find('"',x+1) == std::string::npos))
                    open = false;
            }
            if ((!open) && ((src.compare(x,dlen,delim) == 0) || (x+1 >= slen)))
            {
                std::string arg = nospace(src.substr(last,(x+=dlen)-last-((x+1 < slen) ? 1 : 0)));
                if ((arg.front() == '"') && (arg.back() == '"'))
                    arg = arg.substr(1,arg.size()-2);
                list.push_back(arg);
                last = x;
            }
            else
                x++;
        }
        std::string *split = new std::string[list.size()];
        count = 0;
        for (auto it = list.begin(), ite = list.end();it != ite;++it)
            split[count++] = *it;
        return split;
    }
    
    std::string *splitStringAny(const std::string &src, const std::string &delim, int &count)
    {
        if (src.size() == 0)
            return nullptr;
        std::vector<std::string> list;
        bool open = false;
        char c;
        for (size_t x = 0, slen = src.size(), last = 0;x < slen;)
        {
            if ((c = src.at(x)) == '"')
            {
                open = !open;
                if ((open) && (src.find('"',x+1) == std::string::npos))
                    open = false;
            }
            if ((!open) && ((delim.find(c) != std::string::npos) || (x+1 >= slen)))
            {
                std::string arg = nospace(src.substr(last,(++x)-last-((x+1 < slen) ? 1 : 0)));
                if ((arg.front() == '"') && (arg.back() == '"'))
                    arg = arg.substr(1,arg.size()-2);
                list.push_back(arg);
                last = x;
            }
            else
                x++;
        }
        std::string *split = new std::string[list.size()];
        count = 0;
        for (auto it = list.begin(), ite = list.end();it != ite;++it)
            split[count++] = *it;
        return split;
    }
    
    std::string randomNegativeEmoji()
    {
        static std::string emojis[] = {
                                        "%F0%9F%9A%B7",
                                        "%F0%9F%A4%B9",
                                        "%F0%9F%A4%94",
                                        "%F0%9F%98%92",
                                        "%F0%9F%98%A0",
                                        "%F0%9F%A4%90",
                                        "%F0%9F%91%8E",
                                        "%F0%9F%92%83",
                                        "%F0%9F%A4%92",
                                        "%F0%9F%98%B4",
                                        "%F0%9F%98%AA",
                                        "%F0%9F%94%9E",
                                        "%E2%9B%94",
                                        "%F0%9F%9A%AB",
                                        "%E2%9D%8C"
                                      };
        return emojis[mtrand(0,14)];
    }
    
    class InternalObject
    {
        public:
            InternalObject()
            {
                add("version",{&Internal::Console::version,"FarageBot version information."});
                add("modules",{&Internal::Console::modules,"Module management and information."});
                add("setprefix",{&Internal::Console::setprefix,"Change the command prefix."});
                add("reloadadmins",{&Internal::Console::reloadadmins,"Reload the admin config file."});
                add("gvar",{&Internal::Console::gvar,"Change or view GlobVar values."});
                add("ignoreuser",{&Internal::Console::ignoreuser,"Ignore or unignore a user."});
                add("ignorechannel",{&Internal::Console::ignorechannel,"Ignore or unignore a channel."});
                add("setroleflags",{&Internal::Console::setroleflags,"Set the admin flags for a role."});
                add("sendmsg",{&Internal::Console::sendmsg,"Send a message to a channel."});
                add("execute",{&Internal::Console::execute,"Execute a config script."});
                add("exec",{&Internal::Console::execute,"Execute a config script."});
                add("nick",{&Internal::Console::nick,"Change my nickname for a server."});
                add("leave",{&Internal::Console::leave,"Leave a server."});
                add("help",{&Internal::Console::help,"View information about registered commands and gvars."});
                add("version",{&Internal::Chat::version,NOFLAG,"FarageBot version information."});
                add("setprefix",{&Internal::Chat::setprefix,STATUS,"Change the command prefix."});
                add("reloadadmins",{&Internal::Chat::reloadadmins,GENERIC,"Reload the admin config file."});
                add("gvar",{&Internal::Chat::gvar,GLOBVAR,"Change or view GlobVar values."});
                add("rcon",{&Internal::Chat::rcon,RCON,"Execute commands directly through the console."});
                add("ignoreuser",{&Internal::Chat::ignoreuser,GAG,"Ignore or unignore a user."});
                add("ignorechannel",{&Internal::Chat::ignorechannel,GAG,"Ignore or unignore a channel."});
                add("setroleflags",{&Internal::Chat::setroleflags,ROLE,"Set the admin flags for a role."});
                add("execute",{&Internal::Chat::execute,CONFIG,"Execute a config script."});
                add("exec",{&Internal::Chat::execute,CONFIG,"Execute a config script."});
                add("cmdhelp",{&Internal::Chat::cmdhelp,NOFLAG,"View information about commands."});
                add("gvarhelp",{&Internal::Chat::gvarhelp,GLOBVAR,"View information about gvars."});
            };
            void add(const std::string &cmd, const Internal::Console::Command &command)
            {
                consoleCommands[cmd] = command;
            }
            void add(const std::string &cmd, const Internal::Chat::Command &command)
            {
                chatCommands[cmd] = command;
            }
            int call(BotClass *bot, Global &global, int argc, const std::string argv[])
            {
                int ret = PLUGIN_CONTINUE;
                auto it = consoleCommands.find(argv[0]);
                if (it != consoleCommands.end())
                    ret = (*it->second.cb)(bot,global,argc,argv);
                return ret;
            }
            int call(BotClass *bot, Global &global, AdminFlag flags, int argc, const std::string argv[], const SleepyDiscord::Message &message);
            Internal::Console::Command& getConsole(const std::string &cmd)
            {
                return consoleCommands[cmd];
            }
            Internal::Chat::Command& getChat(const std::string &cmd)
            {
                return chatCommands[cmd];
            }
            std::unordered_map<std::string,Internal::Console::Command>::iterator consoleBegin()
            {
                return consoleCommands.begin();
            }
            std::unordered_map<std::string,Internal::Console::Command>::iterator consoleEnd()
            {
                return consoleCommands.end();
            }
            std::unordered_map<std::string,Internal::Chat::Command>::iterator chatBegin()
            {
                return chatCommands.begin();
            }
            std::unordered_map<std::string,Internal::Chat::Command>::iterator chatEnd()
            {
                return chatCommands.end();
            }
        private:
            std::unordered_map<std::string,Internal::Console::Command> consoleCommands;
            std::unordered_map<std::string,Internal::Chat::Command> chatCommands;
    } internals;
    
    int processLaunchArgs(Global &global, int argc, char *argv[], std::string *token = nullptr, std::vector<std::string> *exec = nullptr)
    {
        int pos = 1;
        if (argc > 1)
        {
            for (;pos < argc;pos++)
            {
                std::string arg = argv[pos];
                if (arg.find("--") == 0)
                {
                    if (arg.size() == 2)
                    {
                        pos++;
                        break;
                    }
                    else if (arg == "--help")
                    {
                        consoleOut("FarageBot " + std::string(FARAGE_ENGINE) + " written by nigel.\n\thttps://github.com/nigelSaysHesHappy/FarageBot/\n  Farage API " + std::string(FARAGE_API_VERSION) + "\n  Powered by " + std::string(SLEEPY_VERSION) + ".\n\thttps://github.com/yourWaifu/sleepy-discord/\n\n\
DESCRIPTION\n\tCan't Barrage Bot the Modular Discord Farage Bot!\n\n\
USAGE\n\
\t" + argv[0] + " [OPTIONS]\n\n\
OPTIONS\n\
\t--\t\t\tMark the end of switch input. Not actually used.\n\
\t--help\t\t\tDisplay this help message and exit.\n\
\t--version\t\tDisplay version info and exit.\n\
\t--verbose\t\tEnable verbose console output.\n\
\t--quiet\t\t\tDisable verbose console output.\n\
\t--debug\t\t\tEnable debug console output.\n\
\t--suppress\t\tDisable debug console output.\n\
\t--token <TOKEN_ID>\tSet the bot token.\n\
\t--token=<TOKEN_ID>\tAlias format to set the token.\n\
\t--prefix <PREFIX>\tSet the default command prefix.\n\
\t--prefix=<PREFIX>\tAlias format to set the command prefix.\n\
\t--execute <PATH>\tAdd a script to be auto executed at startup.\n\t\t\t\tUnlike the 'execute' command, this requires the\n\t\t\t\tfull path to the script.\n\
\t--exec <PATH>\t\tAlias for '--execute'.\n");
                        return 0;
                    }
                    else if (arg == "--version")
                    {
                        consoleOut("FarageBot " + std::string(FARAGE_ENGINE) + " written by nigel.\n\thttps://github.com/nigelSaysHesHappy/FarageBot/\n  Farage API " + std::string(FARAGE_API_VERSION) + "\n  Powered by " + std::string(SLEEPY_VERSION) + ".\n\thttps://github.com/yourWaifu/sleepy-discord/\n");
                        return 0;
                    }
                    else if (arg == "--verbose")
                        global.verbose = true;
                    else if (arg == "--quiet")
                        global.verbose = false;
                    else if (arg == "--debug")
                        global.debug = true;
                    else if (arg == "--suppress")
                        global.debug = false;
                    else if ((arg.find("--token") == 0) && (token != nullptr))
                    {
                        if ((arg == "--token") && (++pos < argc))
                            token->assign(argv[pos]);
                        else if ((arg.size() > 8) && (arg.find("--token=") == 0))
                            token->assign(arg.substr(8));
                        else
                        {
                            errorOut("Error: Missing bot token in '--token' switch. Usage: '--token=<TOKEN_ID> | --token <TOKEN_ID>'");
                            return 1;
                        }
                    }
                    else if (((arg == "--exec") || (arg == "--execute")) && (exec != nullptr))
                    {
                        if (++pos < argc)
                            exec->push_back(argv[pos]);
                        else
                        {
                            errorOut("Error: Missing script path in '--exec <PATH>' switch.");
                            return 1;
                        }
                    }
                    else if (arg.find("--prefix") == 0)
                    {
                        if ((arg == "--prefix") && (++pos < argc))
                            global.prefixes["default"] = nospace(argv[pos]);
                        else if ((arg.size() > 9) && (arg.find("--prefix=") == 0))
                            global.prefixes["default"] = nospace(arg.substr(9));
                        else
                        {
                            errorOut("Error: Missing prefix string in '--prefix' switch. Usage: '--prefix=<PREFIX> | --prefix <PREFIX>'");
                            return 1;
                        }
                    }
                    else
                        consoleOut("Ignoring unknown command switch: " + arg);
                }
                else
                    break;
            }
            // process remaining args
        }
        return 42; // normal exit code
    }
    
    /*void processCinput(BotClass *bot, Global &global, const std::string &input)
    {
        int argc, ret = PLUGIN_CONTINUE;
        std::string *argv = splitString(nospace(input)," ",argc);
        auto plug = global.plugins.begin(), pluge = global.plugins.end();
        bool done = false;
        for (;plug != pluge;++plug)
        {
            if ((*plug)->getLoadPriority() != 0)
                break;
            if ((ret = (*plug)->callConsoleCmd(argv[0],argc,argv)) == PLUGIN_HANDLED)
            {
                done = true;
                break;
            }
        }
        if ((!done) && ((ret = internals.call(bot,global,argc,argv)) != PLUGIN_HANDLED))
            for (;plug != pluge;++plug)
                if ((ret = (*plug)->callConsoleCmd(argv[0],argc,argv)) == PLUGIN_HANDLED)
                    break;
        if (ret == PLUGIN_CONTINUE)
            consoleOut("Unknown command: \"" + argv[0] + "\"");
        delete[] argv;
    }*/
    
    int processCinput(BotClass *bot, Global &global, int argc, const std::string argv[])
    {
        int ret = PLUGIN_CONTINUE;
        auto plug = global.plugins.begin(), pluge = global.plugins.end();
        for (;plug != pluge;++plug)
        {
            if ((*plug)->getLoadPriority() != 0)
                break;
            if ((ret = (*plug)->callConsoleCmd(argv[0],argc,argv)) == PLUGIN_HANDLED)
                return ret;
        }
        if ((ret = internals.call(bot,global,argc,argv)) != PLUGIN_HANDLED)
            for (;plug != pluge;++plug)
                if ((ret = (*plug)->callConsoleCmd(argv[0],argc,argv)) == PLUGIN_HANDLED)
                    break;
        if (ret == PLUGIN_CONTINUE)
            consoleOut("Unknown command: \"" + argv[0] + "\"");
        return ret;
    }
    
    void processCinput(BotClass *bot, Global &global, const std::string &input)
    {
        int argc;
        std::string *argv = splitStringAny(nospace(input)," \t\n",argc);
        processCinput(bot,global,argc,argv);
        delete[] argv;
    }
    
    int processCscript(BotClass *bot, Global &global, const std::string &filepath)
    {
        global.tryGetBuffer().clear();
        std::ifstream file(filepath);
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file,line))
                processCinput(bot,global,line);
            file.close();
            return 0;
        }
        return 1;
    }
    
    std::string runServerCommand(BotClass *discord, Global &global, int argc, const std::string argv[], bool deallo = false)
    {
        global.tryGetBuffer().clear();
        processCinput(discord,global,argc,argv);
        if (deallo)
            delete[] argv;
        std::string output;
        auto buffer = global.tryGetBuffer();
        if (buffer.owns_lock())
        {
            for (auto it = buffer->begin(), ite = buffer->end();it != ite;++it)
                output = output + *it + '\n';
        }
        return output;
    }
    
    std::string runServerCommand(BotClass *discord, Global &global, const std::string &command)
    {
        int argc;
        std::string *argv = splitStringAny(nospace(command)," \t\n",argc);
        return runServerCommand(discord,global,argc,argv,true);
    }
    
    class BotClass : public SleepyDiscord::DiscordClient
    {
        public:
            using SleepyDiscord::DiscordClient::DiscordClient;
            
            void onReady(SleepyDiscord::Ready readyData)
            {
                Farage::Global *global = Farage::recallGlobal();
                createServerCache();
                Ready fready = convertReady(std::move(readyData));
                global->lastReady = fready;
                global->self = fready.user;
                for (auto it = global->globVars.begin(), ite = global->globVars.end();it != ite;++it)
                    if ((*it)->flags & GVAR_DUPLICATE)
                        for (auto g = fready.guilds.begin(), ge = fready.guilds.end();g != ge;++g)
                            (*it)->guildValues.emplace(*g,(*it)->value);
                void *arg0 = (void*)(&fready);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONREADY,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onResumed(const SleepyDiscord::json::Value& jsonMessage)
            {
                Farage::Global *global = Farage::recallGlobal();
                void *arg0 = (void*)(&jsonMessage);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONRESUMED,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onDeleteServer(SleepyDiscord::UnavailableServer server)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::string sserver = server.ID;
                void *arg0 = (void*)(&sserver);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONDELETESERVER,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onEditServer(SleepyDiscord::Server server)
            {
                Farage::Global *global = Farage::recallGlobal();
                Server fserver = convertServer(std::move(server));
                void *arg0 = (void*)(&fserver);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITSERVER,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onBan(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::User user)
            {
                Farage::Global *global = Farage::recallGlobal();
                User fuser = convertUser(std::move(user));
                std::string ID = serverID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&fuser);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONBAN,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                       break;
            }
            
            void onUnban(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::User user)
            {
                Farage::Global *global = Farage::recallGlobal();
                User fuser = convertUser(std::move(user));
                std::string ID = serverID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&fuser);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONUNBAN,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::ServerMember member)
            {
                Farage::Global *global = Farage::recallGlobal();
                ServerMember fmember = convertServerMember(std::move(member));
                for (auto it = global->ignoredUsers.begin(), ite = global->ignoredUsers.end();it != ite;++it)
                    if (*it == fmember.user.id)
                        return;
                std::string ID = serverID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&fmember);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONMEMBER,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                       break;
            }
            
            void onRemoveMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::User user)
            {
                Farage::Global *global = Farage::recallGlobal();
                User fuser = convertUser(std::move(user));
                std::string ID = serverID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&fuser);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONREMOVEMEMBER,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                       break;
            }
            
            void onEditMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::User user, std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Role>> roles, std::string nick)
            {
                Farage::Global *global = Farage::recallGlobal();
                User fuser = convertUser(std::move(user));
                std::string ID = serverID;
                std::vector<std::string> froles;
                froles.reserve(roles.size());
                for (auto it = roles.begin(), ite = roles.end();it != ite;++it)
                    froles.push_back(*it);
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&fuser);
                void *arg2 = (void*)(&froles);
                void *arg3 = (void*)(&nick);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITMEMBER,arg0,arg1,arg2,arg3) == PLUGIN_HANDLED)
                       break;
            }
            
            void onRole(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::Role role)
            {
                Farage::Global *global = Farage::recallGlobal();
                Role frole = convertRole(std::move(role));
                std::string ID = serverID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&frole);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONROLE,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                       break;
            }
            
            void onDeleteRole(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::Snowflake<SleepyDiscord::Role> roleID)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::string ID = serverID, srole = roleID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&srole);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONDELETEROLE,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                       break;
            }
            
            void onEditRole(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::Role role)
            {
                Farage::Global *global = Farage::recallGlobal();
                Role frole = convertRole(std::move(role));
                std::string ID = serverID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&frole);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITROLE,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                       break;
            }
            
            void onEditEmojis(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, std::vector<SleepyDiscord::Emoji> emojis)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::vector<Emoji> femojis;
                femojis.reserve(emojis.size());
                for (auto it = emojis.begin(), ite = emojis.end();it != ite;++it)
                    femojis.push_back(convertEmoji(*it));
                std::string ID = serverID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&femojis);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITEMOJIS,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                       break;
            }
            
            void onMemberChunk(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, std::vector<SleepyDiscord::ServerMember> members)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::vector<ServerMember> fmembers;
                fmembers.reserve(members.size());
                for (auto it = members.begin(), ite = members.end();it != ite;++it)
                    fmembers.push_back(convertServerMember(*it));
                std::string ID = serverID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&fmembers);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONMEMBERCHUNK,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                       break;
            }
            
            void onDeleteChannel(SleepyDiscord::Channel channel)
            {
                Farage::Global *global = Farage::recallGlobal();
                Channel fchannel = convertChannel(std::move(channel));
                void *arg0 = (void*)(&fchannel);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONDELETECHANNEL,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onEditChannel(SleepyDiscord::Channel channel)
            {
                Farage::Global *global = Farage::recallGlobal();
                Channel fchannel = convertChannel(std::move(channel));
                void *arg0 = (void*)(&fchannel);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITCHANNEL,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onPinMessage(SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, std::string lastPinTimestamp)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::string ID = channelID;
                void *arg0 = (void*)(&ID);
                void *arg1 = (void*)(&lastPinTimestamp);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONPINMESSAGE,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onPresenceUpdate(SleepyDiscord::PresenceUpdate presenceUpdate)
            {
                Farage::Global *global = Farage::recallGlobal();
                PresenceUpdate presence = convertPresenceUpdate(std::move(presenceUpdate));
                void *arg0 = (void*)(&presence);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONPRESENCEUPDATE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onEditUser(SleepyDiscord::User user)
            {
                Farage::Global *global = Farage::recallGlobal();
                User fuser = convertUser(std::move(user));
                void *arg0 = (void*)(&fuser);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITUSER,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onEditUserNote(const SleepyDiscord::json::Value& jsonMessage)
            {
                Farage::Global *global = Farage::recallGlobal();
                void *arg0 = (void*)(&jsonMessage);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITUSERNOTE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onEditUserSettings(const SleepyDiscord::json::Value& jsonMessage)
            {
                Farage::Global *global = Farage::recallGlobal();
                void *arg0 = (void*)(&jsonMessage);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITUSERSETTINGS,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onEditVoiceState(SleepyDiscord::VoiceState& state)
            {
                Farage::Global *global = Farage::recallGlobal();
                VoiceState fstate = convertVoiceState(std::move(state));
                void *arg0 = (void*)(&fstate);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITVOICESTATE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onTyping(SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, SleepyDiscord::Snowflake<SleepyDiscord::User> userID, time_t timestamp)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::string channel = channelID, user = userID;
                for (auto it = global->ignoredUsers.begin(), ite = global->ignoredUsers.end();it != ite;++it)
                    if (*it == user)
                        return;
                void *arg0 = (void*)(&channel);
                void *arg1 = (void*)(&user);
                void *arg2 = (void*)(&timestamp);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONTYPING,arg0,arg1,arg2,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onDeleteMessages(SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Message>> messages)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::string channel = channelID;
                std::vector<std::string> smessages;
                smessages.reserve(messages.size());
                for (auto it = messages.begin(), ite = messages.end();it != ite;++it)
                    smessages.push_back(*it);
                void *arg0 = (void*)(&channel);
                void *arg1 = (void*)(&smessages);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONDELETEMESSAGES,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onEditMessage(const SleepyDiscord::json::Value& jsonMessage)
            {
                Farage::Global *global = Farage::recallGlobal();
                Message message = convertMessage(Farage::EditMessage(jsonMessage));
                bool blockEvent = false;
                std::string guildName, chanName;
                bool guildSet = false;
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                {
                    for (auto pit = (*it)->editHooks.begin();pit != (*it)->editHooks.end();)
                    {
                        if ((*pit)->matches(message.author.id,message.channel_id,message.id,message.guild_id))
                        {
                            int flags = (*pit)->flags;
                            if ((flags & HOOK_PRINT) == HOOK_PRINT)
                            {
                                if (!guildSet)
                                {
                                    auto cache = ((BotClass*)(global->discord))->getServerCache();
                                    for (auto it = cache->begin(), ite = cache->end();it != ite;++it)
                                    {
                                        auto chanIt = it->findChannel(message.channel_id);
                                        if (chanIt != it->channels.end())
                                        {
                                            guildName = it->name;
                                            chanName = chanIt->name;
                                            break;
                                        }
                                    }
                                    guildSet = true;
                                }
                                consoleOut("[" + (*pit)->name + "][" + guildName + "](#" + chanName + ")(" + message.id + ")<" + message.author.username + "> Edit: " + message.content);
                            }
                            int ret = PLUGIN_CONTINUE;
                            if ((*pit)->func != nullptr)
                                ret = (*(*pit)->func)(**it,*pit,message);
                            if (ret & PLUGIN_ERASE)
                            {
                                delete *pit;
                                pit = (*it)->editHooks.erase(pit);
                            }
                            else
                                pit++;
                            if (ret & PLUGIN_HANDLED)
                                return;
                            if ((flags & HOOK_BLOCK_EVENT) == HOOK_BLOCK_EVENT)
                                blockEvent = true;
                            if ((flags & HOOK_BLOCK_HOOK) == HOOK_BLOCK_HOOK)
                                break;
                        }
                        else
                            pit++;
                    }
                }
                if (!blockEvent)
                {
                    void *arg0 = (void*)(&message);
                    for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                        if ((*it)->callEvent(Event::ONEDITMESSAGE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                            break;
                }
            }
            
            void onEditVoiceServer(SleepyDiscord::VoiceServerUpdate& update)
            {
                Farage::Global *global = Farage::recallGlobal();
                VoiceServerUpdate fupdate = convertVoiceServerUpdate(std::move(update));
                void *arg0 = (void*)(&fupdate);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONEDITVOICESERVER,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onServerSync(const SleepyDiscord::json::Value& jsonMessage)
            {
                Farage::Global *global = Farage::recallGlobal();
                void *arg0 = (void*)(&jsonMessage);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONSERVERSYNC,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onRelationship(const SleepyDiscord::json::Value& jsonMessage)
            {
                Farage::Global *global = Farage::recallGlobal();
                void *arg0 = (void*)(&jsonMessage);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONRELATIONSHIP,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onDeleteRelationship(const SleepyDiscord::json::Value& jsonMessage)
            {
                Farage::Global *global = Farage::recallGlobal();
                void *arg0 = (void*)(&jsonMessage);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONDELETERELATIONSHIP,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::string user = userID, message = messageID;
                for (auto it = global->ignoredUsers.begin(), ite = global->ignoredUsers.end();it != ite;++it)
                    if (*it == user)
                        return;
                Emoji femoji = convertEmoji(std::move(emoji));
                auto cache = ((BotClass*)(global->discord))->getServerCache();
                std::string guild, guildName;
                ServerMember member;
                Channel channel;
                for (auto it = cache->begin(), ite = cache->end();it != ite;++it)
                {
                    auto chanIt = it->findChannel(channelID);
                    if (chanIt != it->channels.end())
                    {
                        guild = it->ID;
                        guildName = it->name;
                        member = convertServerMember(*it->findMember(userID));
                        channel = convertChannel(*chanIt);
                        break;
                    }
                }
                bool blockEvent = false;
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                {
                    for (auto pit = (*it)->reactHooks.begin();pit != (*it)->reactHooks.end();)
                    {
                        if ((*pit)->matches(member.user.id,channel.id,message,guild,convertEmoji(emoji)))
                        {
                            int flags = (*pit)->flags;
                            if ((flags & HOOK_PRINT) == HOOK_PRINT)
                                consoleOut("[" + (*pit)->name + "][" + guildName + "](#" + channel.name + ")(" + message + ")<" + member.user.username + "> Reaction: " + femoji.display());
                            int ret = PLUGIN_CONTINUE;
                            if ((*pit)->func != nullptr)
                                ret = (*(*pit)->func)(**it,*pit,member,channel,message,guild,femoji);
                            if (ret & PLUGIN_ERASE)
                            {
                                delete *pit;
                                pit = (*it)->reactHooks.erase(pit);
                            }
                            else
                                pit++;
                            if (ret & PLUGIN_HANDLED)
                                return;
                            if ((flags & HOOK_BLOCK_EVENT) == HOOK_BLOCK_EVENT)
                                blockEvent = true;
                            if ((flags & HOOK_BLOCK_HOOK) == HOOK_BLOCK_HOOK)
                                break;
                        }
                        else
                            pit++;
                    }
                }
                if (!blockEvent)
                {
                    void *arg0 = (void*)(&member);
                    void *arg1 = (void*)(&channel);
                    void *arg2 = (void*)(&message);
                    void *arg3 = (void*)(&femoji);
                    for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                        if ((*it)->callEvent(Event::ONREACTION,arg0,arg1,arg2,arg3) == PLUGIN_HANDLED)
                            break;
                }
            }
            
            void onDeleteReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::string user = userID, channel = channelID, message = messageID;
                for (auto it = global->ignoredUsers.begin(), ite = global->ignoredUsers.end();it != ite;++it)
                    if (*it == user)
                        return;
                Emoji femoji = convertEmoji(std::move(emoji));
                void *arg0 = (void*)(&user);
                void *arg1 = (void*)(&channel);
                void *arg2 = (void*)(&message);
                void *arg3 = (void*)(&femoji);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONDELETEREACTION,arg0,arg1,arg2,arg3) == PLUGIN_HANDLED)
                        break;
            }
            
            void onDeleteAllReaction(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::string server = serverID, channel = channelID, message = messageID;
                void *arg0 = (void*)(&server);
                void *arg1 = (void*)(&channel);
                void *arg2 = (void*)(&message);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONDELETEALLREACTION,arg0,arg1,arg2,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onMessage(SleepyDiscord::Message message)
            {
                Farage::Global *global = Farage::recallGlobal();
                std::string ID = message.channelID;
                for (auto it = global->ignoredChannels.begin(), ite = global->ignoredChannels.end();it != ite;++it)
                    if (*it == ID)
                        return;
                ID = message.author.ID;
                for (auto it = global->ignoredUsers.begin(), ite = global->ignoredUsers.end();it != ite;++it)
                    if (*it == ID)
                        return;
                Message fmessage = convertMessage(message);
                rens::smatch ml;
                bool blockEvent = false;
                bool blockCmd = false;
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                {
                    for (auto pit = (*it)->chatHooks.begin();pit != (*it)->chatHooks.end();)
                    {
                        if (rens::regex_search(fmessage.content,ml,(*pit)->pattern))
                        {
                            int flags = (*pit)->flags;
                            if ((flags & HOOK_PRINT) == HOOK_PRINT)
                            {
                                auto cache = ((BotClass*)(global->discord))->getServerCache();
                                std::string guild = fmessage.guild_id;
                                std::string channel = fmessage.channel_id;
                                auto server = cache->findServer(guild);
                                if (server != cache->end())
                                {
                                    guild = server->name;
                                    auto chan = server->findChannel(channel);
                                    if (chan->name.size() > 0)
                                        channel = chan->name;
                                }
                                consoleOut("[" + (*pit)->name + "][" + guild + "](#" + channel + "): <" + fmessage.author.username + "> " + fmessage.content);
                            }
                            int ret = PLUGIN_CONTINUE;
                            if ((*pit)->func != nullptr)
                                ret = (*(*pit)->func)(**it,*pit,ml,fmessage);
                            if (ret & PLUGIN_ERASE)
                            {
                                delete *pit;
                                pit = (*it)->chatHooks.erase(pit);
                            }
                            else
                                pit++;
                            if (ret & PLUGIN_HANDLED)
                                return;
                            if ((flags & HOOK_BLOCK_EVENT) == HOOK_BLOCK_EVENT)
                                blockEvent = true;
                            if ((flags & HOOK_BLOCK_CMD) == HOOK_BLOCK_CMD)
                                blockCmd = true;
                            if ((flags & HOOK_BLOCK_HOOK) == HOOK_BLOCK_HOOK)
                                break;
                        }
                        else
                            pit++;
                    }
                }
                std::string prefix = global->prefix(fmessage.guild_id);
                bool prefixed = (fmessage.content.find(prefix) == 0);
                void *arg0 = (void*)(&fmessage);
                if (!blockEvent)
                {
                    for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    {
                        if (((*it)->callEvent(Event::ONMESSAGE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                          || ((fmessage.type == GUILD_TEXT)
                              && (((*it)->callEvent(Event::ONCMESSAGE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                              || ((prefixed) && ((*it)->callEvent(Event::ONCMESSAGE_PRE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED))))
                          || ((fmessage.type == DM_TEXT)
                              && (((*it)->callEvent(Event::ONDMESSAGE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                              || ((prefixed) && ((*it)->callEvent(Event::ONDMESSAGE_PRE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED))))
                          || ((fmessage.type == GROUP_DM)
                              && (((*it)->callEvent(Event::ONGMESSAGE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                              || ((prefixed) && ((*it)->callEvent(Event::ONGMESSAGE_PRE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)))))
                            return;
                    }
                }
                if ((!blockCmd) && (prefixed) && (ID != global->self.id))
                {
                    std::string command = fmessage.content;
                    command.erase(0,prefix.size());
                    AdminFlag flags = global->getAdminFlags(fmessage.guild_id,ID);
                    int argc;
                    std::string *argv = splitStringAny(nospace(command)," \t\n",argc);
                    auto plug = global->plugins.begin(), pluge = global->plugins.end();
                    bool done = false;
                    for (;plug != pluge;++plug)
                    {
                        if ((*plug)->getLoadPriority() != 0)
                            break;
                        if ((*plug)->callChatCmd(argv[0],flags,argc,argv,fmessage) == PLUGIN_HANDLED)
                        {
                            done = true;
                            break;
                        }
                    }
                    if ((!done) && (internals.call(this,*global,flags,argc,argv,message) != PLUGIN_HANDLED))
                        for (;plug != pluge;++plug)
                            if ((*plug)->callChatCmd(argv[0],flags,argc,argv,fmessage) == PLUGIN_HANDLED)
                                break;
                    delete[] argv;
                }
            }
            
            void onServer(SleepyDiscord::Server server)
            {
                Farage::Global *global = Farage::recallGlobal();
                Server fserver = convertServer(std::move(server));
                for (auto it = global->globVars.begin(), ite = global->globVars.end();it != ite;++it)
                    if ((*it)->flags & GVAR_DUPLICATE)
                        (*it)->guildValues.emplace(fserver.id,(*it)->value);
                void *arg0 = (void*)(&fserver);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONSERVER,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onChannel(SleepyDiscord::Channel channel)
            {
                Farage::Global *global = Farage::recallGlobal();
                Channel fchannel = convertChannel(std::move(channel));
                void *arg0 = (void*)(&fchannel);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONCHANNEL,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onDispatch(const SleepyDiscord::json::Value& jsonMessage)
            {
                Farage::Global *global = Farage::recallGlobal();
                void *arg0 = (void*)(&jsonMessage);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONDISPATCH,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onHeartbeat()
            {
                Farage::Global *global = Farage::recallGlobal();
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONHEARTBEAT,nullptr,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onHeartbeatAck()
            {
                Farage::Global *global = Farage::recallGlobal();
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONHEARTBEATACK,nullptr,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onInvaldSession()
            {
                Farage::Global *global = Farage::recallGlobal();
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONINVALIDSESSION,nullptr,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onDisconnect()
            {
                Farage::Global *global = Farage::recallGlobal();
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONDISCONNECT,nullptr,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onResume()
            {
                Farage::Global *global = Farage::recallGlobal();
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONRESUME,nullptr,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onQuit()
            {
                Farage::Global *global = Farage::recallGlobal();
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONQUIT,nullptr,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onRestart()
            {
                Farage::Global *global = Farage::recallGlobal();
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONRESTART,nullptr,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onResponse(SleepyDiscord::Response response)
            {
                Farage::Global *global = Farage::recallGlobal();
                Response fresponse = convertResponse(std::move(response));
                void *arg0 = (void*)(&fresponse);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONRESPONSE,arg0,nullptr,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
            
            void onError(SleepyDiscord::ErrorCode errorCode, const std::string errorMessage)
            {
                Farage::Global *global = Farage::recallGlobal();
                int code = errorCode;
                void *arg0 = (void*)(&code);
                void *arg1 = (void*)(&errorMessage);
                for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
                    if ((*it)->callEvent(Event::ONERROR,arg0,arg1,nullptr,nullptr) == PLUGIN_HANDLED)
                        break;
            }
    };
    
    namespace Engine
    {
        ObjectResponse<Message> sendMessage(const std::string &chan, const std::string &message, const std::string &json, bool tts)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Message>();
            try
            {
                if (json.size() < 1)
                {
                    SleepyDiscord::ObjectResponse<SleepyDiscord::Message> response = ((BotClass*)(bot))->sendMessage(chan,message,/*SleepyDiscord::Embed::Flag::INVALID_EMBED*/SleepyDiscord::Embed(),tts);
                    //return ObjectResponse<Message>(std::move(convertResponse(response)),std::move(convertMessage(std::move(response.cast()))));
                    return ObjectResponse<Message>(std::move(convertResponse(response)),std::move(convertMessage(std::move(response.cast()))));//Response(0),Message());
                }
                else
                {
                    SleepyDiscord::ObjectResponse<SleepyDiscord::Message> response = ((BotClass*)(bot))->sendMessage(chan,message,SleepyDiscord::Embed(json),tts);
                    //return ObjectResponse<Message>(std::move(convertResponse(response)),std::move(convertMessage(std::move(response.cast()))));
                    return ObjectResponse<Message>(std::move(convertResponse(response)),std::move(convertMessage(std::move(response.cast()))));//Response(0),Message());
                }
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("sendMessage: Error code " + std::to_string(int(err)));
                if (err == SleepyDiscord::ErrorCode::FORBIDDEN)
                    errorOut("sendMessage: FORBIDDEN Cannot send message to " + chan);
                return ObjectResponse<Message>(Response(err),Message());
            }
            return ObjectResponse<Message>(Response(4000),Message());
        }
        
        //ObjectResponse<Message> messageChannelID(const std::string &chan, const std::string &message)
        //{
            /*if ((global->isRateLimited) || (global->buffer.size() > 0))
            {
                debugOut("Not limited, sending now.");
                Farage::ObjectBuffer buff;
                buff.type = FARAGE_BUFFER_MESSAGE;
                buff.channel.id = chan;
                buff.message = message;
                global->buffer.push_back(buff);
            }
            else
            {*/
            //SleepyDiscord::ObjectResponse<SleepyDiscord::Message> response = ((BotClass*)(recallGlobal()->discord))->sendMessage(chan,message);
                /*if (response.statusCode >= SleepyDiscord::TOO_MANY_REQUESTS)
                {
                    debugOut("Rate limited, buffering for later.");
                    Farage::ObjectBuffer buff;
                    buff.type = FARAGE_BUFFER_MESSAGE;
                    buff.channel.id = chan;
                    buff.message = message;
                    global->buffer.push_back(buff);
                }*/
            /*if (response.header["X-RateLimit-Remaining"] == "0")
                global->isRateLimited = true;
            else
                global->isRateLimited = false;*/
            //}
            /*std::cout<<"creation."<<std::endl;
            SleepyDiscord::Message casted = response.cast();
            std::cout<<"created."<<std::endl;
            Response fresponse = convertResponse(response);
            Message fmessage;// = convertMessage(casted);
            ObjectResponse<Message> ret(std::move(fresponse),std::move(fmessage));
            Farage::verboseOut("[messageChannelID>>" + chan + "] " + message);
            return ret;
        }*/
        
        BoolResponse reactToID(const std::string &channel, const std::string &messageID, const std::string &emoji)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            std::string remoji = emoji;
            if (remoji == "randomNegativeEmoji")
                remoji = randomNegativeEmoji();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->addReaction(channel,messageID,remoji);
                //Farage::verboseOut("[reactToID>>" + channel + "," + messageID + "] " + emoji);
                return BoolResponse(std::move(convertResponse(std::move(response))),response.cast());
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("addReaction: Error code " + std::to_string(int(err)));
                if (err == SleepyDiscord::ErrorCode::FORBIDDEN)
                    errorOut("addReaction: FORBIDDEN Cannot react to message " + messageID + " in channel " + channel);
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        ObjectResponse<Channel> getChannel(const std::string &ID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Channel>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Channel> response = ((BotClass*)(bot))->getChannel(ID);
                return ObjectResponse<Channel>(std::move(convertResponse(response)),std::move(convertChannel(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getChannel: Error code " + std::to_string(int(err)));
                return ObjectResponse<Channel>(Response(err),Channel());
            }
            return ObjectResponse<Channel>(Response(4000),Channel());
        }
        
        ObjectResponse<Channel> getDirectMessageChannel(const std::string &userID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Channel>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Channel> response = ((BotClass*)(bot))->createDirectMessageChannel(userID);
                return ObjectResponse<Channel>(std::move(convertResponse(response)),std::move(convertChannel(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getDirectMessageChannel: Error code " + std::to_string(int(err)));
                return ObjectResponse<Channel>(Response(err),Channel());
            }
            return ObjectResponse<Channel>(Response(4000),Channel());
        }
        
        ObjectResponse<User> getUser(const std::string &ID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<User>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::User> response = ((BotClass*)(bot))->getUser(ID);
                return ObjectResponse<User>(std::move(convertResponse(response)),std::move(convertUser(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getUser: Error code " + std::to_string(int(err)));
                return ObjectResponse<User>(Response(err),User());
            }
            return ObjectResponse<User>(Response(4000),User());
        }
        
        ObjectResponse<User> getSelf()
        {
            Global *global = recallGlobal();
            void *bot = global->discord;
            if (bot == nullptr)
                return ObjectResponse<User>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::User> response = ((BotClass*)(bot))->getCurrentUser();
                global->self = std::move(convertUser(std::move(response.cast())));
                return ObjectResponse<User>(std::move(convertResponse(response)),global->self);
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getSelf: Error code " + std::to_string(int(err)));
                return ObjectResponse<User>(Response(err),User());
            }
            return ObjectResponse<User>(Response(4000),User());
        }
        
        BoolResponse sendTyping(const std::string &channel)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->sendTyping(channel);
                return BoolResponse(std::move(convertResponse(std::move(response))),response.cast());
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("sendTyping: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        /*ObjectResponse<Message> sendEmbed(const std::string &chan, const std::string &message, const std::string &json)
        {
            verboseOut("[sendEmbed>>" + chan + "] " + message + " || " + json);
            SleepyDiscord::ObjectResponse<SleepyDiscord::Message> response = ((BotClass*)(recallGlobal()->discord))->sendMessage(chan,message,SleepyDiscord::Embed(json));
            return ObjectResponse<Message>();//(std::move(convertResponse(response)),std::move(convertMessage(std::move(response.cast()))));
        }*/
        
        ObjectResponse<Message> sendFile(const std::string &chan, const std::string &filepath, const std::string &message)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Message>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Message> response = ((BotClass*)(bot))->uploadFile(chan,filepath,message);
                return ObjectResponse<Message>(std::move(convertResponse(response)),std::move(convertMessage(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("sendFile: Error code " + std::to_string(int(err)));
                return ObjectResponse<Message>(Response(err),Message());
            }
            //verboseOut("[sendFile>>" + chan + "] " + message + " || " + filepath);
            return ObjectResponse<Message>(Response(4000),Message());
        }
        
        Server getGuildCache(const std::string &guildID)
        {
            Server guild;
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return guild;
            auto cache = ((BotClass*)(bot))->getServerCache();
            auto server = cache->findServer(guildID);
            if (server != cache->end())
                guild = convertServer(*server);
            return std::move(guild);
        }
        
        ServerMember getServerMember(const std::string &guildID, const std::string &userID)
        {
            ServerMember user;
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return user;
            auto cache = ((BotClass*)(bot))->getServerCache();
            auto server = cache->findServer(guildID);
            if (server != cache->end())
            {
                auto member = server->findMember(userID);
                if (member != server->members.end())
                    user = convertServerMember(*member);
            }
            return std::move(user);
        }
        
        Channel getChannelCache(const std::string &guildID, const std::string &channelID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return Channel{};
            //Channel chan;
            const auto cache = ((BotClass*)(bot))->getServerCache();
            SleepyDiscord::Channel channel;
            auto server = cache->findServer(guildID);
            if (server != cache->end())
                channel = *(server->findChannel(channelID));
            /*for (auto it = cache->begin(), ite = cache->end();it != ite;++it)
            {
                if (it->ID == guildID)
                {
                    channel = *it->findChannel(flake);
                    break;
                }
            }*/
            //SleepyDiscord::Cache<SleepyDiscord::Server>::const_iterator nonsense = cache->findSeverWith(flake);
            
            //if (server != cache->end())
            //{
            //    auto channel = server->findChannel(flake);
            //    if (channel != server->channels.end())
            //        chan = convertChannel(*channel);
            //}
            
            return std::move(convertChannel(std::move(channel)));//*((BotClass*)(recallGlobal()->discord))->getServerCache()->findSeverWith(flake)->findChannel(flake));
        }
        
        ObjectResponse<Channel> editChannel(const std::string &channelID, const std::string &name, const std::string &topic)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Channel>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Channel> response = ((BotClass*)(bot))->editChannel(channelID,name,topic);
                return ObjectResponse<Channel>(std::move(convertResponse(response)),std::move(convertChannel(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editChannel: Error code " + std::to_string(int(err)));
                return ObjectResponse<Channel>(Response(err),Channel());
            }
            return ObjectResponse<Channel>(Response(4000),Channel());
        }
        
        ObjectResponse<Channel> editChannelName(const std::string &channelID, const std::string &name)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Channel>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Channel> response = ((BotClass*)(bot))->editChannelName(channelID,name);
                return ObjectResponse<Channel>(std::move(convertResponse(response)),std::move(convertChannel(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editChannelName: Error code " + std::to_string(int(err)));
                return ObjectResponse<Channel>(Response(err),Channel());
            }
            return ObjectResponse<Channel>(Response(4000),Channel());
        }
        
        ObjectResponse<Channel> editChannelTopic(const std::string &channelID, const std::string &topic)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Channel>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Channel> response = ((BotClass*)(bot))->editChannelTopic(channelID,topic);
                return ObjectResponse<Channel>(std::move(convertResponse(response)),std::move(convertChannel(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editChannelTopic: Error code " + std::to_string(int(err)));
                return ObjectResponse<Channel>(Response(err),Channel());
            }
            return ObjectResponse<Channel>(Response(4000),Channel());
        }
        
        ObjectResponse<Channel> deleteChannel(const std::string &channelID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Channel>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Channel> response = ((BotClass*)(bot))->deleteChannel(channelID);
                return ObjectResponse<Channel>(std::move(convertResponse(response)),std::move(convertChannel(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("deleteChannel: Error code " + std::to_string(int(err)));
                return ObjectResponse<Channel>(Response(err),Channel());
            }
            return ObjectResponse<Channel>(Response(4000),Channel());
        }
        
        ArrayResponse<Message> getMessages(const std::string &channelID, GetMessagesKey when, const std::string &messageID, uint8_t limit)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<Message>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::Message> response = ((BotClass*)(bot))->getMessages(channelID,(SleepyDiscord::BaseDiscordClient::GetMessagesKey)when,messageID,limit);
                return convertArrayResponse<SleepyDiscord::Message,Message>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getMessages: Error code " + std::to_string(int(err)));
                return ArrayResponse<Message>(Response(err));
            }
            return ArrayResponse<Message>(Response(4000));
        }
        
        ObjectResponse<Message> getMessage(const std::string &channelID, const std::string &messageID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Message>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Message> response = ((BotClass*)(bot))->getMessage(channelID,messageID);
                return ObjectResponse<Message>(std::move(convertResponse(response)),std::move(convertMessage(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getMessage: Error code " + std::to_string(int(err)));
                return ObjectResponse<Message>(Response(err),Message());
            }
            return ObjectResponse<Message>(Response(4000),Message());
        }
        
        BoolResponse removeReaction(const std::string &channelID, const std::string &messageID, const std::string &emoji, const std::string &userID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->removeReaction(channelID,messageID,emoji,userID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("removeReaction: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        ArrayResponse<Reaction> getReactions(const std::string &channelID, const std::string &messageID, const std::string &emoji)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<Reaction>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::Reaction> response = ((BotClass*)(bot))->getReactions(channelID,messageID,emoji);
                return convertArrayResponse<SleepyDiscord::Reaction,Reaction>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getReactions: Error code " + std::to_string(int(err)));
                return ArrayResponse<Reaction>(Response(err));
            }
            return ArrayResponse<Reaction>(Response(4000));
        }
        
        Response removeAllReactions(const std::string &channelID, const std::string &messageID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return Response();
            try
            {
                SleepyDiscord::StandardResponse response = ((BotClass*)(bot))->removeAllReactions(channelID,messageID);
                return convertResponse(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("removeAllReactions: Error code " + std::to_string(int(err)));
                return Response(err);
            }
            return Response(4000);
        }
        
        ObjectResponse<Message> editMessage(const std::string &channelID, const std::string &messageID, const std::string &newMessage)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Message>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Message> response = ((BotClass*)(bot))->editMessage(channelID,messageID,newMessage);
                return ObjectResponse<Message>(std::move(convertResponse(response)),std::move(convertMessage(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editMessage: Error code " + std::to_string(int(err)));
                return ObjectResponse<Message>(Response(err),Message());
            }
            return ObjectResponse<Message>(Response(4000),Message());
        }
        
        BoolResponse deleteMessage(const std::string &channelID, const std::string &messageID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->deleteMessage(channelID,messageID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("deleteMessage: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse bulkDeleteMessages(const std::string &channelID, const std::vector<std::string> &messageIDs)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Message>> ids(messageIDs.begin(),messageIDs.end());
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->bulkDeleteMessages(channelID,ids);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("bulkDeleteMessages: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse editChannelPermissions(const std::string &channelID, const std::string &overwriteID, int allow, int deny, const std::string &type)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->editChannelPermissions(channelID,overwriteID,allow,deny,type);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editChannelPermissions: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        ArrayResponse<Invite> getChannelInvites(const std::string &channelID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<Invite>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::Invite> response = ((BotClass*)(bot))->getChannelInvites(channelID);
                return convertArrayResponse<SleepyDiscord::Invite,Invite>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getChannelInvites: Error code " + std::to_string(int(err)));
                return ArrayResponse<Invite>(Response(err));
            }
            return ArrayResponse<Invite>(Response(4000));
        }
        
        ObjectResponse<Invite> createChannelInvite(const std::string &channelID, const uint64_t maxAge, const uint64_t maxUses, const bool temporary, const bool unique)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Invite>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Invite> response = ((BotClass*)(bot))->createChannelInvite(channelID,maxAge,maxUses,temporary,unique);
                return ObjectResponse<Invite>(std::move(convertResponse(response)),std::move(convertInvite(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("createChannelInvite: Error code " + std::to_string(int(err)));
                return ObjectResponse<Invite>(Response(err),Invite());
            }
            return ObjectResponse<Invite>(Response(4000),Invite());
        }
        
        BoolResponse removeChannelPermission(const std::string &channelID,const std::string &ID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->removeChannelPermission(channelID,ID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("removeChannelPermission: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        ArrayResponse<Message> getPinnedMessages(const std::string &channelID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<Message>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::Message> response = ((BotClass*)(bot))->getPinnedMessages(channelID);
                return convertArrayResponse<SleepyDiscord::Message,Message>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getPinnedMessages: Error code " + std::to_string(int(err)));
                return ArrayResponse<Message>(Response(err));
            }
            return ArrayResponse<Message>(Response(4000));
        }
        
        BoolResponse pinMessage(const std::string &channelID, const std::string &messageID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->pinMessage(channelID,messageID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("pinMessage: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse unpinMessage(const std::string &channelID, const std::string &messageID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->unpinMessage(channelID,messageID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("unpinMessage: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        Response addRecipient(const std::string &channelID, const std::string &userID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return Response();
            try
            {
                SleepyDiscord::StandardResponse response = ((BotClass*)(bot))->addRecipient(channelID,userID);
                return convertResponse(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("addRecipient: Error code " + std::to_string(int(err)));
                return Response(err);
            }
            return Response(4000);
        }
        
        Response removeRecipient(const std::string &channelID, const std::string &userID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return Response();
            try
            {
                SleepyDiscord::StandardResponse response = ((BotClass*)(bot))->removeRecipient(channelID,userID);
                return convertResponse(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("removeRecipient: Error code " + std::to_string(int(err)));
                return Response(err);
            }
            return Response(4000);
        }
        
        /*std::string serverCommand(const std::string &command)
        {
            Global *global = recallGlobal();
            bool locked = global->bufferIsLocked();
            if (!locked)
                global->clearBuffer();
            processCinput((BotClass*)(global->discord),*global,command);
            std::string output;
            if (!locked)
            {
                auto buffer = global->getBuffer();
                for (auto it = buffer->begin(), ite = buffer->end();it != ite;++it)
                    output = output + *it + '\n';
                global->returnBuffer();
            }
            return output;
        }*/
        
        std::string serverCommand(const std::string &command)
        {
            Global *global = recallGlobal();
            return runServerCommand((BotClass*)(global->discord),*global,command);
        }
        //
        ObjectResponse<Server> getServer(const std::string &serverID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Server>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Server> response = ((BotClass*)(bot))->getServer(serverID);
                return ObjectResponse<Server>(std::move(convertResponse(response)),std::move(convertServer(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getServer: Error code " + std::to_string(int(err)));
                return ObjectResponse<Server>(Response(err),Server());
            }
            return ObjectResponse<Server>(Response(4000),Server());
        }
        
        ObjectResponse<Server> deleteServer(const std::string &serverID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Server>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Server> response = ((BotClass*)(bot))->deleteServer(serverID);
                return ObjectResponse<Server>(std::move(convertResponse(response)),std::move(convertServer(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("deleteServer: Error code " + std::to_string(int(err)));
                return ObjectResponse<Server>(Response(err),Server());
            }
            return ObjectResponse<Server>(Response(4000),Server());
        }
        
        ArrayResponse<Channel> getServerChannels(const std::string &serverID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<Channel>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::Channel> response = ((BotClass*)(bot))->getServer(serverID);
                return convertArrayResponse<SleepyDiscord::Channel,Channel>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getServerChannels: Error code " + std::to_string(int(err)));
                return ArrayResponse<Channel>(Response(err));
            }
            return ArrayResponse<Channel>(Response(4000));
        }
        
        ObjectResponse<Channel> createTextChannel(const std::string &serverID, const std::string &name)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Channel>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Channel> response = ((BotClass*)(bot))->createTextChannel(serverID,name);
                return ObjectResponse<Channel>(std::move(convertResponse(response)),std::move(convertChannel(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("createTextChannel: Error code " + std::to_string(int(err)));
                return ObjectResponse<Channel>(Response(err),Channel());
            }
            return ObjectResponse<Channel>(Response(4000),Channel());
        }
        
        ArrayResponse<Channel> editChannelPositions(const std::string &serverID, std::vector<std::pair<std::string,uint64_t>> positions)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<Channel>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::Channel> response = ((BotClass*)(bot))->editChannelPositions(serverID,positions);
                return convertArrayResponse<SleepyDiscord::Channel,Channel>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editChannelPositions: Error code " + std::to_string(int(err)));
                return ArrayResponse<Channel>(Response(err));
            }
            return ArrayResponse<Channel>(Response(4000));
        }
        
        ObjectResponse<ServerMember> getMember(const std::string &serverID, const std::string &userID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<ServerMember>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::ServerMember> response = ((BotClass*)(bot))->getMember(serverID,userID);
                return ObjectResponse<ServerMember>(std::move(convertResponse(response)),std::move(convertServerMember(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getMember: Error code " + std::to_string(int(err)));
                return ObjectResponse<ServerMember>(Response(err),ServerMember());
            }
            return ObjectResponse<ServerMember>(Response(4000),ServerMember());
        }
        
        ArrayResponse<ServerMember> listMembers(const std::string &serverID, uint16_t limit, const std::string &after)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<ServerMember>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::ServerMember> response = ((BotClass*)(bot))->listMembers(serverID,limit,after);
                return convertArrayResponse<SleepyDiscord::ServerMember,ServerMember>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("listMembers: Error code " + std::to_string(int(err)));
                return ArrayResponse<ServerMember>(Response(err));
            }
            return ArrayResponse<ServerMember>(Response(4000));
        }
        
        ObjectResponse<ServerMember> addMember(const std::string &serverID, const std::string &userID, const std::string &accessToken, const std::string &nick, std::vector<Role> roles, bool mute, bool deaf)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<ServerMember>();
            std::vector<SleepyDiscord::Role> sleepyRoles;
            sleepyRoles.reserve(roles.size());
            for (auto it = roles.begin(), ite = roles.end();it != ite;++it)
            {
                SleepyDiscord::Role role;
                role.ID = std::move(SleepyDiscord::Snowflake<SleepyDiscord::Role>(it->id));
                role.name = std::move(it->name);
                role.color = std::move(it->color);
                role.hoist = std::move(it->hoist);
                role.position = std::move(it->position);
                role.permissions = std::move(SleepyDiscord::Permission(it->permissions));
                role.managed = std::move(it->managed);
                role.mentionable = std::move(it->mentionable);
                sleepyRoles.push_back(std::move(role));
            }
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::ServerMember> response = ((BotClass*)(bot))->addMember(serverID,userID,accessToken,nick,std::move(sleepyRoles),mute,deaf);
                return ObjectResponse<ServerMember>(std::move(convertResponse(response)),std::move(convertServerMember(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("addMember: Error code " + std::to_string(int(err)));
                return ObjectResponse<ServerMember>(Response(err),ServerMember());
            }
            return ObjectResponse<ServerMember>(Response(4000),ServerMember());
        }
        
        BoolResponse editMember(const std::string &serverID, const std::string &userID, const std::string &nickname, std::vector<std::string> roles, int8_t mute, int8_t deaf, const std::string &channelID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Role>> sleepyRoles;
            sleepyRoles.reserve(roles.size());
            for (auto it = roles.begin(), ite = roles.end();it != ite;++it)
                sleepyRoles.emplace_back(*it);
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->editMember(serverID,userID,nickname,std::move(sleepyRoles),mute,deaf,channelID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editMember: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse muteServerMember(const std::string &serverID, const std::string &userID, bool mute)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->muteServerMember(serverID,userID,mute);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("muteServerMember: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse editNickname(const std::string &serverID, const std::string &newNickname)
        {
            void *bot = recallGlobal()->discord;
            if ((bot == nullptr) || (newNickname.size() < 1))
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->editNickname(serverID,newNickname);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editNickname: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse addRole(const std::string &serverID, const std::string &userID, const std::string &roleID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->addRole(serverID,userID,roleID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("addRole: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse removeRole(const std::string &serverID, const std::string &userID, const std::string &roleID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->removeRole(serverID,userID,roleID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("removeRole: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse kickMember(const std::string &serverID, const std::string &userID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->kickMember(serverID,userID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("kickMember: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        ArrayResponse<User> getBans(const std::string &serverID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<User>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::User> response = ((BotClass*)(bot))->getBans(serverID);
                return convertArrayResponse<SleepyDiscord::User,User>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getBans: Error code " + std::to_string(int(err)));
                return ArrayResponse<User>(Response(err));
            }
            return ArrayResponse<User>(Response(4000));
        }
        
        BoolResponse banMember(const std::string &serverID, const std::string &userID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->banMember(serverID,userID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("banMember: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse unbanMember(const std::string &serverID, const std::string &userID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->unbanMember(serverID,userID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("unbanMember: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        ArrayResponse<Role> getRoles(const std::string &serverID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<Role>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::Role> response = ((BotClass*)(bot))->getRoles(serverID);
                return convertArrayResponse<SleepyDiscord::Role,Role>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getRoles: Error code " + std::to_string(int(err)));
                return ArrayResponse<Role>(Response(err));
            }
            return ArrayResponse<Role>(Response(4000));
        }
        
        ObjectResponse<Role> createRole(const std::string &serverID, const std::string &name, Permission permissions, unsigned int color, bool hoist, bool mentionable)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<Role>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::Role> response = ((BotClass*)(bot))->createRole(serverID,name,SleepyDiscord::Permission(permissions),color,hoist,mentionable);
                return ObjectResponse<Role>(std::move(convertResponse(response)),std::move(convertRole(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("createRole: Error code " + std::to_string(int(err)));
                return ObjectResponse<Role>(Response(err),Role());
            }
            return ObjectResponse<Role>(Response(4000),Role());
        }
        
        ArrayResponse<Role> editRolePosition(const std::string &serverID, const std::vector<std::pair<std::string,uint64_t>> &positions)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<Role>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::Role> response = ((BotClass*)(bot))->editRolePosition(serverID,positions);
                return convertArrayResponse<SleepyDiscord::Role,Role>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editRolePosition: Error code " + std::to_string(int(err)));
                return ArrayResponse<Role>(Response(err));
            }
            return ArrayResponse<Role>(Response(4000));
        }
        
        //StringResponse editRole(const std::string &serverID, const std::string &roleID, const std::string &name, Permission permissions, uint32_t color, int8_t hoist, int8_t mentionable);
        
        BoolResponse deleteRole(const std::string &serverID, const std::string &roleID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->deleteRole(serverID,roleID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("deleteRole: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        Response pruneMembers(const std::string &serverID, const unsigned int numOfDays)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return Response();
            try
            {
                SleepyDiscord::StandardResponse response = ((BotClass*)(bot))->pruneMembers(serverID,numOfDays);
                return convertResponse(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("pruneMembers: Error code " + std::to_string(int(err)));
                return Response(err);
            }
            return Response(4000);
        }
        
        ArrayResponse<VoiceRegion> getVoiceRegions()
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<VoiceRegion>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::VoiceRegion> response = ((BotClass*)(bot))->getVoiceRegions();
                return convertArrayResponse<SleepyDiscord::VoiceRegion,VoiceRegion>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getVoiceRegions: Error code " + std::to_string(int(err)));
                return ArrayResponse<VoiceRegion>(Response(err));
            }
            return ArrayResponse<VoiceRegion>(Response(4000));
        }
        
        ArrayResponse<Invite> getServerInvites(const std::string &serverID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ArrayResponse<Invite>();
            try
            {
                SleepyDiscord::ArrayResponse<SleepyDiscord::Invite> response = ((BotClass*)(bot))->getServerInvites(serverID);
                return convertArrayResponse<SleepyDiscord::Invite,Invite>(std::move(response));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getServerInvites: Error code " + std::to_string(int(err)));
                return ArrayResponse<Invite>(Response(err));
            }
            return ArrayResponse<Invite>(Response(4000));
        }
        
        //StringResponse getIntegrations(const std::string &serverID);
        
        BoolResponse createIntegration(const std::string &serverID, const std::string &type, const std::string &integrationID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->createIntegration(serverID,type,integrationID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("createIntegration: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse editIntegration(const std::string &serverID, const std::string &integrationID, int expireBehavior, int expireGracePeriod, bool enableEmoticons)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->editIntergration(serverID,integrationID,expireBehavior,expireGracePeriod,enableEmoticons);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("editIntegration: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        BoolResponse deleteIntegration(const std::string &serverID, const std::string &integrationID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->deleteIntegration(serverID,integrationID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("deleteIntegration: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);//std::move(convertResponse(response)),std::move(response.cast()));
        }
        
        BoolResponse syncIntegration(const std::string &serverID, const std::string &integrationID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return BoolResponse();
            try
            {
                SleepyDiscord::BoolResponse response = ((BotClass*)(bot))->syncIntegration(serverID,integrationID);
                return BoolResponse(std::move(convertResponse(response)),std::move(response.cast()));
            }
                catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("syncIntegration: Error code " + std::to_string(int(err)));
                return BoolResponse(Response(err),false);
            }
            return BoolResponse(Response(4000),false);
        }
        
        ObjectResponse<ServerEmbed> getServerEmbed(const std::string &serverID)
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return ObjectResponse<ServerEmbed>();
            try
            {
                SleepyDiscord::ObjectResponse<SleepyDiscord::ServerEmbed> response = ((BotClass*)(bot))->getServerEmbed(serverID);
                return ObjectResponse<ServerEmbed>(std::move(convertResponse(response)),std::move(convertServerEmbed(std::move(response.cast()))));
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                errorOut("getServerEmbed: Error code " + std::to_string(int(err)));
                return ObjectResponse<ServerEmbed>(Response(err),ServerEmbed());
            }
            return ObjectResponse<ServerEmbed>(Response(4000),ServerEmbed());
        }
        
        bool isReady()
        {
            void *bot = recallGlobal()->discord;
            if (bot == nullptr)
                return false;
            return ((BotClass*)(bot))->isReady();
        }
    };
    
    int loadAdminRoles(Global &global)
    {
        INIObject conf("config/adminroles.ini");
        for (auto guild = conf.begin(), guilde = conf.end();guild != guilde;++guild)
        {
            std::unordered_map<std::string,AdminFlag> set;
            for (auto role = guild->begin(), rolee = guild->end();role != rolee;++role)
                set.emplace(role->item,getAdminFlagBits(role->value));
            if (set.size() > 0)
                global.adminRoles.emplace(guild->topic(),set);
        }
        return 0;
    }
    
    int loadAdmins(Global &global)
    {
        loadAdminRoles(global);
        std::ifstream file ("config/admins.conf");
        std::string line, flagstr;
        AdminFlag flags;
        size_t pos;
        if (file.is_open())
        {
            global.admins.clear();
            while (std::getline(file,line))
            {
                line = removeall(line," \t");   // remove all spaces and tabs
                if ((pos = line.find('#')) != std::string::npos)
                    line.erase(pos);
                if ((line.size() < 1)           // no data
                ||  (line.front() == '#')       // comment
                ||  ((pos = (line = strlower(line)).find('=')) == std::string::npos)     // invalid data, also removes trailing comments and converts to lowercase
                ||  ((flagstr = line.substr(pos+1)).size() < 1)     // set flagstr to the right half of the '=' and ensure it isn't empty
                ||  ((line = line.substr(0,pos)).size() < 1))       // set line to the left half of the '=' and ensure it isn't empty
                    continue;                   // if we make it past this line, all data passed tests and is ready to be used
                flags = getAdminFlagBits(flagstr);
                //debugOut("Admin: " + line + " Flags: " + flagstr + " Flagbits: " + std::to_string(flags));
                global.admins.emplace(line,flags);
            }
            file.close();
            //debugOut("Admin container:");
            //for (auto it = global.admins.begin(), ite = global.admins.end();it != ite;++it)
            //    debugOut("  " + it->first + " = " + std::to_string(it->second));
            return 0;
        }
        return 1;
    }
    
    int loadPrefixes(Global &global)
    {
        std::string guild, prefix;
        prefix = global.prefixes["default"];
        if (prefix.size() < 1)
            prefix = FARAGE_DEFAULT_PREFIX;
        global.prefixes.clear();
        global.prefixes["default"] = prefix;
        std::ifstream file ("config/prefixes.conf");
        if (file.is_open())
        {
            size_t pos;
            while (std::getline(file,guild))
            {
                guild = removeall(guild," \t");   // remove all spaces and tabs
                if ((pos = guild.find('#')) != std::string::npos)
                    guild.erase(pos);
                if ((guild.size() < 1)           // no data
                ||  (guild.front() == '#')       // comment
                ||  ((pos = guild.find('=')) == std::string::npos)    // invalid data, also removes trailing comments
                ||  ((prefix = guild.substr(pos+1)).size() < 1)       // set guild to the right half of the '=' and ensure it isn't empty
                ||  ((guild = guild.substr(0,pos)).size() < 1))       // set line to the left half of the '=' and ensure it isn't empty
                    continue;                   // if we make it past this line, all data passed tests and is ready to be used
                global.prefixes[guild] = prefix;
            }
            file.close();
            return 0;
        }
        return 1;
    }
    
    int savePrefixes(Global &global)
    {
        std::ofstream file ("config/prefixes.conf",std::ofstream::out|std::ofstream::trunc);
        if (file.is_open())
        {
            for (auto it = global.prefixes.begin(), ite = global.prefixes.end();it != ite;++it)
                file<<it->first<<'='<<it->second<<std::endl;
            file.close();
            return 0;
        }
        return 1;
    }
    
    std::string loadConfig(Global &global, std::string &token)
    {
        token.clear();
        INIObject cfg ("config/farage.conf");
        auto topic = cfg.topic_it("farage");
        for (auto it = topic->begin(), ite = topic->end();it != ite;++it)
        {
            if (it->item == "verbose")
                global.verbose = str2bool(it->value);
            else if (it->item == "debug")
                global.debug = str2bool(it->value);
            else if (it->item == "token")
                token = nospace(it->value);
            else if (it->item == "prefix")
                global.prefixes["default"] = nospace(it->value);
        }
        loadPrefixes(global);
        //if (token.size() < 1)
        //    return "Error: Discord Bot Token missing from \"config/farage.conf\".";
        return "";
    }
    
    int loadModule(Global &global, const std::string &path, size_t priority)
    {
        verboseOut("Loading module \"" + path + "\" . . .");
        for (auto it = global.plugins.begin(), ite = global.plugins.end();it != ite;++it)
        {
            if ((*it)->getPath() == path)
            {
                errorOut("Error: Module \"" + path + "\" is already loaded.");
                return 2;
            }
        }
        Handle *mod = new Handle(path,&global,priority);
        if (!mod->isLoaded())
            //global.plugins.push_back(mod);
        //else
        {
            delete mod;
            errorOut("Error: Module \"" + path + "\" could not be loaded . . .");
            return 1;
        }
        return 0;
    }
    
    std::string getFilename(std::string path)
    {
        size_t pos = 0;
        size_t temp;
        while ((temp = path.find_first_of("/\\",pos)) != std::string::npos)
            pos = temp+1;
        path.erase(0,pos);
        if ((pos = path.find('.')) != std::string::npos)
            path.erase(pos);
        return path;
    }
    
    void loadModules(Global &global)
    {
        global.plugins.clear();
        std::vector<std::pair<size_t,std::string>> priority, sorted;
        std::string dir = "./modules/";
        DIR *wd;
        struct dirent *entry;
        if ((wd = opendir(dir.c_str())))
        {
            std::string file;
            while ((entry = readdir(wd)))
            {
                file = entry->d_name;
                if ((file.size() > 4) && (file.substr(file.size()-4) == ".fso"))
                    priority.push_back({DEFAULT_PRIORITY,file});
            }
            closedir(wd);
        }
        std::string line;
        std::ifstream file;
        for (auto it = priority.begin(), ite = priority.end();it != ite;++it)
        {
            //debugOut("Loading prio file: \"./config/priority/" + getFilename(it->second) + ".prio\" . . .");
            file.open("./config/priority/" + getFilename(it->second) + ".prio");
            if (file.is_open())
            {
                std::getline(file,line);
                line = removeall(line," \t");
                size_t len = line.size();
                if ((len > 0) && ((std::isdigit(line.front())) || ((line.front() == '-') && (len > 1) && (std::isdigit(line.at(1))))))
                    it->first = std::stoull(line);
                file.close();
            }
        }
        sorted.reserve(priority.size());
        for (auto pit = priority.begin(), pite = priority.end();pit != pite;++pit)
        {
            bool emplaced = false;
            for (auto sit = sorted.begin(), site = sorted.end();sit != site;++sit)
            {
                if (pit->first < sit->first)
                {
                    sorted.insert(sit,*pit);
                    emplaced = true;
                    break;
                }
            }
            if (!emplaced)
                sorted.push_back(*pit);
        }
        verboseOut("Loading " + std::to_string(sorted.size()) + " module(s) . . .");
        for (auto it = sorted.begin(), ite = sorted.end();it != ite;++it)
            loadModule(global,"./modules/" + it->second,it->first);
        verboseOut("Loaded " + std::to_string(global.plugins.size()) + " module(s) . . .");
        /*int i = 0;
        for (auto it = global.plugins.begin(), ite = global.plugins.end();it != ite;++it)
        {
            const Farage::Info *info = (*it)->getInfo();
            verboseOut("[" + std::to_string(i++) + "] Name: " + info->name);
            verboseOut("    Author: " + info->author);
            verboseOut("    Desc: " + info->description);
            verboseOut("    Version: " + info->version);
            verboseOut("    URL: " + info->url);
            verboseOut("    API: " + info->API_VER);
        }*/
        size_t pos = 0;
        for (auto it = global.plugins.begin();it != global.plugins.end();++it)
        {
            auto mod = *it;
            if (mod->callEvent(Event::ONLOADED,(void*)(&it),(void*)(&pos),nullptr,nullptr) == PLUGIN_HANDLED)
                break;
            pos++;
        }
    }
    
    int loadIgnoredChannels(Global &global)
    {
        global.ignoredChannels.clear();
        std::ifstream file ("./config/channels.ignore");
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file,line))
                if (line.size() > 0)
                    global.ignoredChannels.push_back(line);
            file.close();
        }
        return global.ignoredChannels.size();
    }

    int loadIgnoredUsers(Global &global)
    {
        global.ignoredUsers.clear();
        std::ifstream file ("./config/users.ignore");
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file,line))
                if (line.size() > 0)
                    global.ignoredUsers.push_back(line);
            file.close();
        }
        return global.ignoredUsers.size();
    }
    
    void loadAssets(Global &global)
    {
        //createDirs();
        debugOut("Loading admins...");
        loadAdmins(global);
        debugOut("Loading modules...");
        loadModules(global);
        debugOut("Loading ignored channels...");
        loadIgnoredChannels(global);
        debugOut("Loading ignored users...");
        loadIgnoredUsers(global);
        verboseOut("Assets loaded . . .");
    }
    
    int cleanUp(BotClass *bot, Global &global)
    {
        delete bot;
        return 404;
    }
    
    BotClass *botConnect(std::atomic<bool> &online, const std::string &token)
    {
        online = true;
        BotClass *bot = new Farage::BotClass(token,1);
        std::thread([bot, &online]
        {
            //std::this_thread::sleep_for(std::chrono::seconds(1));
            bot->run();
            //std::cerr<<"Discord disconnect."<<std::endl;
            online = false;
            errorOut("Discord disconnected.");
        }).detach();
        //std::this_thread::sleep_for(std::chrono::seconds(2));
        return bot;
    }
    
    // handles discord request queue and timers
    // returns the amount of time until either the next discord request can be made, the next timer needs to fire, or the MAX timeout time
    //   whichever one needs to happen first
#ifdef _WIN32
    DWORD processTimers(BotClass *bot, Global &global)
#else
    timeval processTimers(BotClass *bot, Global &global)
#endif
    {
        global.tryGetBuffer().clear();
        std::chrono::high_resolution_clock::time_point curTime = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
        DWORD ret = FARAGE_TIMEOUT*1000;
#else
        timeval ret;
        ret.tv_sec = FARAGE_TIMEOUT;
        ret.tv_usec = 0;
#endif
        int n;
        size_t cap;
        for (auto ita = global.plugins.begin(), ite = global.plugins.end();ita != ite;++ita)
        {
            n = 0;
            cap = (*ita)->timers.capacity();
            for (auto it = (*ita)->timers.begin();it != (*ita)->timers.end();)
            {
                if ((*ita)->invalidTimers)
                    (*ita)->invalidTimers = false;
                if ((curTime >= (*it)->when()) && ((*it)->trigger(**ita)))
                {
                    if (((*ita)->invalidTimers) && (cap != (*ita)->timers.capacity()))
                        it = (*ita)->timers.erase((*ita)->timers.begin()+n);
                    else
                        it = (*ita)->timers.erase(it);
                }
                else
                {
#ifdef _WIN32
                    DWORD t = (*it)->remaining<std::chrono::milliseconds>().count();
                    if (t < ret)
                        ret = t;
#else
                    timeval tv;
                    tv.tv_sec = (*it)->remaining<std::chrono::seconds>().count();
                    tv.tv_usec = (*it)->remaining<std::chrono::microseconds>().count() % 1000000;
                    if ((tv.tv_sec < ret.tv_sec) || ((tv.tv_sec == ret.tv_sec) && (tv.tv_usec < ret.tv_usec)))
                        ret = tv;
#endif
                    ++it;
                    n++;
                }
            }
        }
        return ret;
    }
    
    namespace Internal::Console
    {
        int version(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            consoleOut("FarageBot " + std::string(FARAGE_ENGINE) + " written by nigel.\n\thttps://github.com/nigelSaysHesHappy/FarageBot/\n  Farage API " + std::string(FARAGE_API_VERSION) + "\n  Powered by " + std::string(SLEEPY_VERSION) + ".\n\thttps://github.com/yourWaifu/sleepy-discord/");
            return PLUGIN_HANDLED;
        }
        int modules(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 2)
                consoleOut("Usage: " + argv[0] + " list\n       " + argv[0] + " info <module>\n       " + argv[0] + " load <module>\n       " + argv[0] + " reload <module>\n       " + argv[0] + " unload <module>\n       " + argv[0] + " priority <module> [value|up|down|top|bottom]");
            else
            {
                bool reload = (argv[1] == "reload");
                bool priority = (argv[1] == "priority");
                if (argv[1] == "list")
                {
                    size_t i = 0;
                    for (auto it = global.plugins.begin(), ite = global.plugins.end();it != ite;++it)
                    {
                        const Farage::Info *info = (*it)->getInfo();
                        consoleOut("[" + std::to_string(i++) + "] " + (*it)->getModule() + " (" + info->version + ") by " + info->author);
                    }
                }
                else if (argv[1] == "info") // add an onInfo event that will be called here
                {
                    if (argc < 3)
                        consoleOut("Usage: " + argv[0] + " info <module> - Display info about a loaded module.");
                    else
                    {
                        bool found = false;
                        size_t i = 0;
                        int digit = -1;
                        if (isdigit(argv[2].at(0)))
                            digit = std::stoi(argv[2]);
                        std::string input = strlower(argv[2]);
                        for (auto it = global.plugins.begin(), ite = global.plugins.end();it != ite;++it)
                        {
                            if ((i == digit) || (input == strlower((*it)->getModule())) || (input == strlower((*it)->getPath())))
                            {
                                const Farage::Info *info = (*it)->getInfo();
                                consoleOut("[" + std::to_string(i) + "] Module " + (*it)->getModule() + " (" + (*it)->getPath() + "):" +
                                           "\n    " + info->name + " (" + info->description + ") by " + info->author +
                                           "\n    Version:\t" + info->version +
                                           "\n    URL:\t" + info->url +
                                           "\n    API:\t" + info->API_VER +
                                           "\n    Priority:\t" + std::to_string((*it)->getLoadPriority()));
                                if ((*it)->consoleCommands.size() > 0)
                                {
                                    consoleOut("    Console Commands:");
                                    size_t c = 0;
                                    for (auto com = (*it)->consoleCommands.begin(), come = (*it)->consoleCommands.end();com != come;++com)
                                        consoleOut("        [" + std::to_string(c++) + "]\t" + com->cmd + "\n        \t" + com->desc);
                                }
                                if ((*it)->chatCommands.size() > 0)
                                {
                                    consoleOut("    Chat Commands:");
                                    size_t c = 0;
                                    for (auto com = (*it)->chatCommands.begin(), come = (*it)->chatCommands.end();com != come;++com)
                                        consoleOut("        [" + std::to_string(c++) + "]\t" + com->cmd + "\t\tAdmin Flags: " + std::to_string(com->flag) + "\n        \t" + com->desc);
                                }
                                if ((*it)->events.size() > 0)
                                {
                                    consoleOut("    Registered Events:");
                                    size_t c = 0;
                                    for (auto com = (*it)->events.begin(), come = (*it)->events.end();com != come;++com)
                                        consoleOut("        [" + std::to_string(c++) + "]\t" + eventName(com->first) + "\t(" + std::to_string(com->first) + ")");
                                }
                                if ((*it)->chatHooks.size() > 0)
                                {
                                    consoleOut("    Registered Chat Hooks:");
                                    size_t c = 0;
                                    for (auto com = (*it)->chatHooks.begin(), come = (*it)->chatHooks.end();com != come;++com)
                                        consoleOut("        [" + std::to_string(c++) + "]\t" + (*com)->name + "\t(" + std::to_string((*com)->flags) + ")");
                                }
                                if ((*it)->reactHooks.size() > 0)
                                {
                                    consoleOut("    Registered React Hooks:");
                                    size_t c = 0;
                                    for (auto com = (*it)->reactHooks.begin(), come = (*it)->reactHooks.end();com != come;++com)
                                        consoleOut("        [" + std::to_string(c++) + "]\t" + (*com)->name + "\t(" + std::to_string((*com)->flags) + ")\t Type: " + std::to_string((*com)->type));
                                }
                                if ((*it)->editHooks.size() > 0)
                                {
                                    consoleOut("    Registered Edit Hooks:");
                                    size_t c = 0;
                                    for (auto com = (*it)->editHooks.begin(), come = (*it)->editHooks.end();com != come;++com)
                                        consoleOut("        [" + std::to_string(c++) + "]\t" + (*com)->name + "\t(" + std::to_string((*com)->flags) + ")\t Type: " + std::to_string((*com)->type));
                                }
                                if ((*it)->timers.size() > 0)
                                {
                                    consoleOut("    Running Timers:");
                                    size_t c = 0;
                                    for (auto com = (*it)->timers.begin(), come = (*it)->timers.end();com != come;++com)
                                    {
                                        long remain = (*com)->remainingInterval();
                                        std::string remainstr = std::to_string(remain) + " " + (*com)->intervalString();
                                        if (remain != 1)
                                            remainstr += 's';
                                        consoleOut("        [" + std::to_string(c++) + "]\t" + (*com)->name + "\t" + remainstr + " remaining.");
                                    }
                                }
                                if ((*it)->globVars.size() > 0)
                                {
                                    consoleOut("    Registered GlobVars:");
                                    size_t c = 0;
                                    for (auto com = (*it)->globVars.begin(), come = (*it)->globVars.end();com != come;++com)
                                        consoleOut("        [" + std::to_string(c++) + "]\t" + (*com)->getName() + "\t(" + (*com)->getDesc() + ")\n        \tValue: " + (*com)->getAsString() + "\t(" + (*com)->getDefault() + ')');
                                }
                                found = true;
                                break;
                            }
                            i++;
                        }
                        if (!found)
                            consoleOut("No matching module found. You can input either the number, base filename, or path.");
                    }
                }
                else if (argv[1] == "load")
                {
                    if (argc < 3)
                        consoleOut("Usage: " + argv[0] + " load <module> - Load a module. The module must be located in './modules/', do not include the file extension.");
                    else
                    {
                        size_t prio = -1;
                        if ((argc > 3) && ((isdigit(argv[3].front())) || ((argv[3].front() == '-') && (isdigit(argv[3].at(1))))))
                            prio = std::stoull(argv[3]);
                        if (!loadModule(global,"./modules/" + argv[2] + ".fso",prio))
                            consoleOut("Successfully loaded '" + argv[2] + "'.");
                    }
                }
                else if ((reload) || (priority) || (argv[1] == "unload"))
                {
                    if (argc < 3)
                    {
                        if (reload)
                            consoleOut("Usage: " + argv[0] + " reload <module> - Reload a loaded module.");
                        else if (priority)
                            consoleOut("Usage: " + argv[0] + " priority <module> [value|up|down|top|bottom] - Set or view the priority for a loaded module.");
                        else
                            consoleOut("Usage: " + argv[0] + " unload <module> - Unload a loaded module.");
                    }
                    else
                    {
                        bool found = false;
                        size_t i = 0;
                        int digit = -1;
                        if (isdigit(argv[2].front()))
                            digit = std::stoi(argv[2]);
                        std::string input = strlower(argv[2]);
                        for (auto it = global.plugins.begin(), ite = global.plugins.end();it != ite;++it)
                        {
                            if ((i++ == digit) || (input == strlower((*it)->getModule())) || (input == strlower((*it)->getPath())))
                            {
                                Farage::Handle *mod = *it;
                                if (reload)
                                    mod->load(mod->getPath(),&global,true);
                                else if (priority)
                                {
                                    size_t prio = mod->getLoadPriority();
                                    short shift = 0;
                                    if (argc < 4)
                                        consoleOut("Module '" + mod->getModule() + "' has load priority of " + std::to_string(prio));
                                    else
                                    {
                                        if ((isdigit(argv[3].front())) || ((argv[3].front() == '-') && (isdigit(argv[3].at(1)))))
                                            prio = std::stoull(argv[3]);
                                        else
                                        {
                                            std::string val = strlower(argv[3]);
                                            if (val == "up")
                                            {
                                                shift = SHIFT_UP;
                                                prio = 1;
                                            }
                                            else if (val == "down")
                                            {
                                                shift = SHIFT_DOWN;
                                                prio = 1;
                                            }
                                            else if (val == "top")
                                                prio = 0;
                                            else if (val == "bottom")
                                                prio = -1;
                                        }
                                        if (mod->setLoadPriority(prio,true,shift))
                                            consoleOut("An unknown error occurred while trying to write the prio file . . .");
                                        else
                                            consoleOut("Load priority for module '" + mod->getModule() + "' has been updated to: " + std::to_string(mod->getLoadPriority()));
                                    }
                                }
                                else
                                {
                                    std::string name = mod->getModule();
                                    global.plugins.erase(it);
                                    delete mod;
                                    consoleOut("Module '" + name + "' has been unloaded.");
                                }
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                            consoleOut("No matching module found. You can input either the number, base filename, or path.");
                    }
                }
                else
                    consoleOut("Usage: " + argv[0] + " list\n       " + argv[0] + " info <module>\n       " + argv[0] + " load <module>\n       " + argv[0] + " reload <module>\n       " + argv[0] + " unload <module>\n       " + argv[0] + " priority <module> [value|up|down|top|bottom]");
            }
            return PLUGIN_HANDLED;
        }
        int setprefix(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 2)
                consoleOut("Current default command prefix: \"" + global.prefix() + "\"");
            else if (argc == 2)
            {
                global.prefixes["default"] = argv[1];
                consoleOut("Default command prefix changed to: \"" + global.prefix() + "\"");
                if (savePrefixes(global))
                    errorOut("Error: Unable to save prefixes.conf!");
            }
            else
            {
                global.prefixes[argv[1]] = argv[2];
                consoleOut("Command prefix for guild '" + argv[1] + "' has changed to: \"" + global.prefix(argv[1]) + "\"");
                if (savePrefixes(global))
                    errorOut("Error: Unable to save prefixes.conf!");
            }
            return PLUGIN_HANDLED;
        }
        int reloadadmins(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (loadAdmins(global))
                consoleOut("Error reloading admin config . . .");
            else
                consoleOut("Admin cache has been reloaded.");
            return PLUGIN_HANDLED;
        }
        int gvar(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 2)
                consoleOut("Usage: " + argv[0] + " <gvar> [value] [guild_id]");
            else
            {
                bool change;
                GlobVar *gvar = nullptr;
                if (argc > 2)
                    change = true;
                std::string guild;
                if (argc > 3)
                    guild = argv[3];
                for (auto it = global.globVars.begin(), ite = global.globVars.end();it != ite;++it)
                {
                    if ((*it)->getName() == argv[1])
                    {
                        gvar = *it;
                        break;
                    }
                }
                if (gvar == nullptr)
                    consoleOut("Error: Unknown GlobVar \"" + argv[1] + "\"");
                else
                {
                    if (change)
                        gvar->setString(argv[2],guild);
                    consoleOut(" \"" + argv[1] + "\" == \"" + gvar->getAsString(guild) + "\"");
                }
            }
            return PLUGIN_HANDLED;
        }
        int ignoreuser(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 2)
                consoleOut("Usage: " + argv[0] + " <user_id>");
            else
            {
                int result = ignoreUser(argv[1],true);
                if (result)
                    consoleOut("No longer ignoring all messages from user: " + argv[1]);
                else
                    consoleOut("Now ignoring all messages from user: " + argv[1]);
                if (saveIgnoredUsers())
                    consoleOut("Error: Unable to save './config/users.ignore' . . .");
            }
            return PLUGIN_HANDLED;
        }
        int ignorechannel(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 2)
                consoleOut("Usage: " + argv[0] + " <channel_id>");
            else
            {
                int result = ignoreChannel(argv[1],true);
                if (result)
                    consoleOut("No longer ignoring all messages within channel: " + argv[1]);
                else
                    consoleOut("Now ignoring all messages within channel: " + argv[1]);
                if (saveIgnoredChannels())
                    consoleOut("Error: Unable to save './config/channels.ignore' . . .");
            }
            return PLUGIN_HANDLED;
        }
        int setroleflags(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 2)
                consoleOut("Usage: " + argv[0] + " <guild_id> [role_id] [flagstring|flagbits]");
            else
            {
                auto guild = global.adminRoles.find(argv[1]);
                if ((argc < 4) && (guild == global.adminRoles.end()))
                    consoleOut("No roles are set for guild '" + argv[1] + '\'');
                else if (argc < 3)
                {
                    auto role = guild->second.begin(), rolee = guild->second.end();
                    if (role == rolee)
                        consoleOut("No roles are set for guild '" + guild->first + '\'');
                    else
                    {
                        consoleOut("Listing admin roles for guild: " + guild->first);
                        for (int i = 0;role != rolee;++role,i++)
                           consoleOut("[" + std::to_string(i) + "] '" + role->first + "' has flags: " + getAdminFlagString(role->second) + " (" + std::to_string(role->second) + ')');
                   }
                }
                else if (argc < 4)
                {
                    auto role = guild->second.find(argv[2]);
                    if (role == guild->second.end())
                        consoleOut("No flags set for role '" + argv[2] + "' on guild '" + guild->first + '\'');
                    else
                        consoleOut("Admin flags for role '" + role->first + "' on guild '" + guild->first + "': " + getAdminFlagString(role->second) + " (" + std::to_string(role->second) + ')');
                }
                else
                {
                    Farage::AdminFlag flags = NOFLAG;
                    if (std::isdigit(argv[3].front()))
                        flags = AdminFlag(std::stoi(argv[3]));
                    else
                        flags = getAdminFlagBits(argv[3]);
                    global.adminRoles[argv[1]][argv[2]] = flags;
                    consoleOut("Set admin flags for role '" + argv[2] + "' on guild '" + argv[1] + "' to: " + getAdminFlagString(flags) + " (" + std::to_string(flags) + ')');
                    if (saveAdminRoles())
                        errorOut("Error: Unable to save './config/adminroles.ini' . . .");
                }
            }
            return PLUGIN_HANDLED;
        }
        int sendmsg(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 3)
                consoleOut("Usage: " + argv[0] + " <channel_id> <message ...>");
            else
            {
                std::string msg = argv[2];
                for (int i = 3;i < argc;i++)
                    msg = msg + " " + argv[i];
                bot->sendMessage(argv[1],msg);
            }
            return PLUGIN_HANDLED;
        }
        int execute(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 2)
                consoleOut("Usage: " + argv[0] + " <script.cfg>");
            else
            {
                std::string file = "./config/script/" + argv[1];
                if ((processCscript(bot,global,file) != 0) && (processCscript(bot,global,file += ".cfg") != 0))
                    consoleOut(argv[0] + ": Cannot open file '" + file + '\'');
            }
            return PLUGIN_HANDLED;
        }
        int nick(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 3)
                consoleOut("Usage: " + argv[0] + " <server_id> <nickname>");
            else
                bot->editNickname(argv[1],argv[2]);
            return PLUGIN_HANDLED;
        }
        int leave(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if (argc < 2)
                consoleOut("Usage: " + argv[0] + " <server_id>");
            else
                bot->leaveServer(argv[1]);
            return PLUGIN_HANDLED;
        }
        int help(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[])
        {
            if ((argc < 2) || ((argv[1] != "chat") && (argv[1] != "console") && (argv[1] != "gvar")))
                consoleOut("FarageBot Command and GlobVar Help System\n View information on registered commands and gvars.\n Usage: " + argv[0] + " <chat|console|gvar> [criteria] [page]\n criteria is a regex pattern that must match anywhere within an entry's name or description.");
            else
            {
                short type = 0;
                if (argv[1] == "console")
                    type = 1;
                if (argv[1] == "gvar")
                    type = 2;
                std::string criteriastr = ".*";
                rens::regex criteria;
                std::vector<std::string> output;
                size_t page = 1, cmd = 0;
                if (argc > 2)
                {
                    if (argc > 3)
                    {
                        criteriastr = argv[2];
                        if (std::isdigit(argv[3].front()))
                            page = std::stoi(argv[3]);
                    }
                    else if (std::isdigit(argv[2].front()))
                        page = std::stoi(argv[2]);
                    else
                        criteriastr = argv[2];
                }
                criteria = criteriastr;
                switch (type)
                {
                    case 0:
                    {
                        for (auto it = internals.chatBegin(), ite = internals.chatEnd();it != ite;++it)
                            if ((rens::regex_search(it->first,criteria)) || (rens::regex_search(it->second.desc,criteria)))
                                output.push_back(" [" + std::to_string(cmd++) + "] " + it->first + " - " + it->second.desc);
                        break;
                    }
                    case 1:
                    {
                        for (auto it = internals.consoleBegin(), ite = internals.consoleEnd();it != ite;++it)
                            if ((rens::regex_search(it->first,criteria)) || (rens::regex_search(it->second.desc,criteria)))
                                output.push_back(" [" + std::to_string(cmd++) + "] " + it->first + " - " + it->second.desc);
                   }
                }
                for (auto it = global.plugins.begin(), ite = global.plugins.end();it != ite;++it)
                {
                    switch (type)
                    {
                        case 0:
                        {
                            for (auto c = (*it)->chatCommands.begin(), ce = (*it)->chatCommands.end();c != ce;++c)
                                if ((rens::regex_search(c->cmd,criteria)) || (rens::regex_search(c->desc,criteria)))
                                    output.push_back(" [" + std::to_string(cmd++) + "] " + c->cmd + " - " + c->desc);
                            break;
                        }
                        case 1:
                        {
                            for (auto c = (*it)->consoleCommands.begin(), ce = (*it)->consoleCommands.end();c != ce;++c)
                                if ((rens::regex_search(c->cmd,criteria)) || (rens::regex_search(c->desc,criteria)))
                                    output.push_back(" [" + std::to_string(cmd++) + "] " + c->cmd + " - " + c->desc);
                            break;
                        }
                        case 2:
                        {
                            for (auto c = (*it)->globVars.begin(), ce = (*it)->globVars.end();c != ce;++c)
                                if ((rens::regex_search((*c)->getName(),criteria)) || (rens::regex_search((*c)->getDesc(),criteria)))
                                    output.push_back(" [" + std::to_string(cmd++) + "] " + (*c)->getName() + " - " + (*c)->getDesc() + " = " + (*c)->getAsString() + " Default: " + (*c)->getDefault());
                        }
                    }
                }
                size_t pages = output.size()/10;
                if ((output.size()%10) > 0)
                    pages++;
                if (page > pages)
                    page = pages;
                if (page > 0)
                {
                    std::string typestr;
                    switch (type)
                    {
                        case 0:
                        {
                            typestr = "Chat Commands";
                            break;
                        }
                        case 1:
                        {
                            typestr = "Console Commands";
                            break;
                        }
                        case 2:
                            typestr = "GlobVars";
                    }
                    consoleOut("Displaying Page " + std::to_string(page) + '/' + std::to_string(pages) + " of " + typestr + " matching: '" + criteriastr + '\'');
                    int i = (page-1)*10, j = i+10;
                    for (auto it = output.begin()+i, ite = output.end();((i < j) && (it != ite));++it,++i)
                        consoleOut(*it);
                }
                else
                    consoleOut("No entries found matching: '" + criteriastr + '\'');
            }
            return PLUGIN_HANDLED;
        }
    };
    
    namespace Internal::Chat
    {
        int version(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            bot->sendMessage(message.channelID,"`FarageBot " + std::string(FARAGE_ENGINE) + "` written by nigel <https://github.com/nigelSaysHesHappy/FarageBot/>\n - Farage API `" + std::string(FARAGE_API_VERSION) + "`\n Powered by `" + std::string(SLEEPY_VERSION) + "` <https://github.com/yourWaifu/sleepy-discord/>");
            return PLUGIN_HANDLED;
        }
        int setprefix(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            if (argc < 2)
                bot->sendMessage(message.channelID,"Current command prefix: `" + global.prefix(message.serverID) + "`");
            else
            {
                global.prefixes[message.serverID] = argv[1];
                bot->sendMessage(message.channelID,"Command prefix changed to: `" + global.prefix(message.serverID) + "`");
                if (savePrefixes(global))
                    errorOut("Error: Unable to save prefixes.conf!");
            }
            return PLUGIN_HANDLED;
        }
        int reloadadmins(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            if (loadAdmins(global))
                bot->sendMessage(message.channelID,"Error reloading admin config . . .");
            else
                bot->sendMessage(message.channelID,"Admin cache has been reloaded.");
            return PLUGIN_HANDLED;
        }
        int gvar(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            if (argc < 2)
                bot->sendMessage(message.channelID,"Usage: `" + global.prefix(message.serverID) + argv[0] + " <gvar> [value]`");
            else
            {
                bool change;
                GlobVar *gvar = nullptr;
                if (argc > 2)
                    change = true;
                for (auto it = global.globVars.begin(), ite = global.globVars.end();it != ite;++it)
                {
                    if ((*it)->getName() == argv[1])
                    {
                        gvar = *it;
                        break;
                    }
                }
                if (gvar == nullptr)
                    bot->sendMessage(message.channelID,"Unknown GlobVar \"" + argv[1] + "\"");
                else
                {
                    if (change)
                        gvar->setString(argv[2],message.serverID);
                    bot->sendMessage(message.channelID," \"" + argv[1] + "\" == `" + gvar->getAsString(message.serverID) + "`");
                }
            }
            return PLUGIN_HANDLED;
        }
        /*int rcon(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            bool locked = global.bufferIsLocked();
            if (!locked)
                global.clearBuffer();
            if (argc < 2)
                bot->sendMessage(message.channelID,"Usage: `" + global.prefix(message.serverID) + argv[0] + " <command ...>`");
            else
            {
                processCinput(bot,global,argc-1,argv+1);
                std::string output;
                if (!locked)
                {
                    auto buffer = global.getBuffer();
                    for (auto it = buffer->begin(), ite = buffer->end();it != ite;++it)
                        output = output + *it + '\n';
                    global.returnBuffer();
                }
                if (output.size() > 0)
                    bot->sendMessage(message.channelID,"```\n" + output + "```");
                else
                    bot->addReaction(message.channelID,message.ID,"%E2%9C%85");
            }
            return PLUGIN_HANDLED;
        }*/
        int rcon(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            if (argc < 2)
                bot->sendMessage(message.channelID,"Usage: `" + global.prefix(message.serverID) + argv[0] + " <command ...>`");
            else
            {
                std::string output = runServerCommand(bot,global,argc-1,argv+1);
                if (output.size() > 0)
                    bot->sendMessage(message.channelID,"```\n" + output + "```");
                else
                    bot->addReaction(message.channelID,message.ID,"%E2%9C%85");
            }
            return PLUGIN_HANDLED;
        }
        int ignoreuser(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            if (argc < 2)
                bot->sendMessage(message.channelID,"Usage: `" + global.prefix(message.serverID) + argv[0] + " <user_id|@user>` - Toggle ignoring of a user.");
            else
            {
                std::string ID = argv[1];
                if ((ID.substr(0,2) == "<@") && (ID.back() == '>'))
                {
                    ID.erase(0,2);
                    ID.erase(ID.size()-1);
                    if (ID.front() == '!')
                        ID.erase(0,1);
                }
                int result = ignoreUser(ID,true);
                if (result)
                    bot->sendMessage(message.channelID,"No longer ignoring all messages from user: `" + ID + '`');
                else
                    bot->sendMessage(message.channelID,"Now ignoring all messages from user: `" + ID + '`');
                if (saveIgnoredUsers())
                    errorOut("Error: Unable to save './config/users.ignore' . . .");
            }
            return PLUGIN_HANDLED;
        }
        int ignorechannel(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            if (argc < 2)
                bot->sendMessage(message.channelID,"Usage: `" + global.prefix(message.serverID) + argv[0] + " <channel_id|#channel>` - Toggle ignoring of a channel.");
            else
            {
                std::string ID = argv[1];
                if ((ID.substr(0,2) == "<#") && (ID.back() == '>'))
                {
                    ID.erase(0,2);
                    ID.erase(ID.size()-1);
                }
                int result = ignoreChannel(ID,true);
                if (result)
                    bot->sendMessage(message.channelID,"No longer ignoring all messages within channel: `" + ID + '`');
                else
                    bot->sendMessage(message.channelID,"Now ignoring all messages within channel: `" + ID + '`');
                if (saveIgnoredChannels())
                    errorOut("Error: Unable to save './config/channels.ignore' . . .");
            }
            return PLUGIN_HANDLED;
        }
        int setroleflags(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            if (argc < 2)
                bot->sendMessage(message.channelID,"Usage: `" + global.prefix(message.serverID) + argv[0] + " <role_id|@role|role> [flagstring|flagbits]`");
            else
            {
                std::string role;
                if (std::isdigit(argv[1].front()))
                    role = argv[1];
                else if ((argv[1].substr(0,3) == "<@&") && (argv[1].back() == '>'))
                    role = argv[1].substr(3,argv[1].size()-4);
                else
                {
                    std::string name = strlower(argv[1]);
                    if ((name.front() == '"') && (name.back() == '"'))
                        name = name.substr(1,name.size()-2);
                    auto server = bot->getServerCache()->findServer(message.serverID);
                    for (auto it = server->roles.begin(), ite = server->roles.end();it != ite;++it)
                    {
                        if (strlower(it->name) == name)
                        {
                            role = it->ID;
                            break;
                        }
                    }
                    if (role.size() < 1)
                    {
                        bot->sendMessage(message.channelID,"Unknown role: `" + name + "`!");
                        return PLUGIN_HANDLED;
                    }
                }
                auto guild = global.adminRoles.find(message.serverID);
                if ((argc < 3) && (guild == global.adminRoles.end()))
                    bot->sendMessage(message.channelID,"No admin flags are set for role `" + role + '`');
                else if (argc < 3)
                {
                    auto it = guild->second.find(role);
                    if (it == guild->second.end())
                        bot->sendMessage(message.channelID,"No admin flags set for role `" + role + '`');
                    else
                        bot->sendMessage(message.channelID,"Admin flags for role `" + it->first + "`: `" + getAdminFlagString(it->second) + "` (`" + std::to_string(it->second) + "`)");
                }
                else
                {
                    Farage::AdminFlag flags = NOFLAG;
                    if (std::isdigit(argv[2].front()))
                        flags = AdminFlag(std::stoi(argv[2]));
                    else
                        flags = getAdminFlagBits(argv[2]);
                    global.adminRoles[message.serverID][role] = flags;
                    if (saveAdminRoles())
                        errorOut("Error: Unable to save './config/adminroles.ini' . . .");
                    bot->sendMessage(message.channelID,"Set admin flags for role `" + role + "` to: `" + getAdminFlagString(flags) + "` (`" + std::to_string(flags) + "`)");
                }
            }
            return PLUGIN_HANDLED;
        }
        int execute(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            if (argc < 2)
                bot->sendMessage(message.channelID,"Usage: `" + global.prefix(message.serverID) + argv[0] + " <script.cfg>`");
            else
            {
                std::string file = "./config/script/" + argv[1];
                if ((processCscript(bot,global,file) != 0) && (processCscript(bot,global,file += ".cfg") != 0))
                    bot->sendMessage(message.channelID,"Cannot open file `" + file + '`');
                else
                    bot->addReaction(message.channelID,message.ID,"%E2%9C%85");
            }
            return PLUGIN_HANDLED;
        }
        int cmdhelp(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            //Usage: cmdhelp [criteria] [page]
            AdminFlag flags = NOFLAG;
            std::string warning, embed, channel = message.channelID;
            bool guildMsg = true;
            if (std::string(message.serverID).size() < 1)
            {
                guildMsg = false;
                if ((flags = global.getAdminFlags(message.author.ID)) != ROOT)
                    warning = "**By using this command outside of a guild channel, you will not see any commands that are granted via roles.**";
            }
            else
                flags = global.getAdminFlags(message.serverID,message.author.ID);
            std::string criteriastr = ".*";
            rens::regex criteria;
            std::vector<std::string> output;
            size_t page = 1;
            if (argc > 1)
            {
                if (argc > 2)
                {
                    criteriastr = argv[1];
                    if (std::isdigit(argv[2].front()))
                        page = std::stoi(argv[2]);
                }
                else if (std::isdigit(argv[1].front()))
                    page = std::stoi(argv[1]);
                else
                    criteriastr = argv[1];
            }
            criteria = criteriastr;
            for (auto it = internals.chatBegin(), ite = internals.chatEnd();it != ite;++it)
                if (((flags & it->second.flag) == it->second.flag) && ((rens::regex_search(it->first,criteria)) || (rens::regex_search(it->second.desc,criteria))))
                    output.push_back("`" + it->first + "` - " + it->second.desc);
            for (auto it = global.plugins.begin(), ite = global.plugins.end();it != ite;++it)
                for (auto c = (*it)->chatCommands.begin(), ce = (*it)->chatCommands.end();c != ce;++c)
                    if (((flags & c->flag) == c->flag) && ((rens::regex_search(c->cmd,criteria)) || (rens::regex_search(c->desc,criteria))))
                        output.push_back("`" + c->cmd + "` - " + c->desc);
            size_t pages = output.size()/10;
            if ((output.size()%10) > 0)
                pages++;
            if (page > pages)
                page = pages;
            embed = "{ \"color\": 10664674, \"title\": \"Displaying Page " + std::to_string(page) + '/' + std::to_string(pages) + " matching: `" + criteriastr + "`\", \"description\": \"";
            if (warning.size() > 0)
                embed = embed + warning + "\\n";
            if (page > 0)
            {
                int i = (page-1)*10, j = i+10;
                for (auto it = output.begin()+i, ite = output.end();((i < j) && (it != ite));++it,++i)
                    embed = embed + *it + "\\n";
                embed = embed.substr(0,embed.size()-2) + "\" }";
            }
            else
                embed = embed + "No entries found.\" }";
            std::string dm;
            if (guildMsg)
                dm = bot->createDirectMessageChannel(message.author.ID).cast().ID;
            else
                dm = channel;
            try
            {
                bot->sendMessage(dm,"",SleepyDiscord::Embed(embed));
                if (dm != channel)
                    bot->addReaction(message.channelID,message.ID,"%E2%9C%85");
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                if (err == SleepyDiscord::ErrorCode::FORBIDDEN)
                    bot->sendMessage(message.channelID,"Please enable DM's with me.");
            }
            return PLUGIN_HANDLED;
        }
        int gvarhelp(Farage::BotClass *bot,Farage::Global &global,int argc,const std::string argv[],const SleepyDiscord::Message &message)
        {
            //Usage: cmdhelp [criteria] [page]
            std::string embed, channel = message.channelID;
            bool guildMsg = true;
            if (std::string(message.serverID).size() < 1)
                guildMsg = false;
            std::string criteriastr = ".*";
            rens::regex criteria;
            std::vector<std::string> output;
            size_t page = 1;
            if (argc > 1)
            {
                if (argc > 2)
                {
                    criteriastr = argv[1];
                    if (std::isdigit(argv[2].front()))
                        page = std::stoi(argv[2]);
                }
                else if (std::isdigit(argv[1].front()))
                    page = std::stoi(argv[1]);
                else
                    criteriastr = argv[1];
            }
            criteria = criteriastr;
            for (auto it = global.globVars.begin(), ite = global.globVars.end();it != ite;++it)
                if ((rens::regex_search((*it)->getName(),criteria)) || (rens::regex_search((*it)->getDesc(),criteria)))
                    output.push_back("`" + (*it)->getName() + "` - " + (*it)->getDesc() + " = `" + (*it)->getAsString(message.serverID) + "` Default: `" + (*it)->getDefault() + '`');
            size_t pages = output.size()/10;
            if ((output.size()%10) > 0)
                pages++;
            if (page > pages)
                page = pages;
            embed = "{ \"color\": 10664674, \"title\": \"Displaying Page " + std::to_string(page) + '/' + std::to_string(pages) + " matching: `" + criteriastr + "`\", \"description\": \"";
            if (page > 0)
            {
                int i = (page-1)*10, j = i+10;
                for (auto it = output.begin()+i, ite = output.end();((i < j) && (it != ite));++it,++i)
                    embed = embed + *it + "\\n";
                embed = embed.substr(0,embed.size()-2) + "\" }";
            }
            else
                embed = embed + "No entries found.\" }";
            std::string dm;
            if (guildMsg)
                dm = bot->createDirectMessageChannel(message.author.ID).cast().ID;
            else
                dm = channel;
            try
            {
                bot->sendMessage(dm,"",SleepyDiscord::Embed(embed));
                if (dm != channel)
                    bot->addReaction(message.channelID,message.ID,"%E2%9C%85");
            }
            catch (SleepyDiscord::ErrorCode err)
            {
                if (err == SleepyDiscord::ErrorCode::FORBIDDEN)
                    bot->sendMessage(message.channelID,"Please enable DM's with me.");
            }
            return PLUGIN_HANDLED;
        }
    };
    
    int InternalObject::call(BotClass *bot, Global &global, AdminFlag flags, int argc, const std::string argv[], const SleepyDiscord::Message &message)
    {
        int ret = PLUGIN_CONTINUE;
        auto it = chatCommands.find(argv[0]);
        if (it != chatCommands.end())
        {
            if ((it->second.flag == NOFLAG) || ((flags & it->second.flag) == it->second.flag))
                ret = (*it->second.cb)(bot,global,argc,argv,message);
            else
                bot->addReaction(message.channelID,message.ID,randomNegativeEmoji());
        }
        return ret;
    }
};

#endif


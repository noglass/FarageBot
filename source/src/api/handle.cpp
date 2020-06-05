#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <ratio>
#include <dlfcn.h>
#include "api/handle.h"
#include "api/link.h"

Farage::Handle::Handle()
{
    Farage::recallGlobal()->plugins.push_back(this);
}

Farage::Handle::Handle(const std::string &pluginPath, Farage::Global *global/*, size_t loadPriority*/)
{
    load(pluginPath,global,false/*,loadPriority*/);
}

Farage::Handle::~Handle()
{
    //Farage::debugOut("Farage::Handle::~Handle() called.");
    unload();
    //delete this;
}

void Farage::Handle::unload(Farage::Global *global)
{
    //Farage::debugOut("Farage::Handle::unload() called.");
    if (loaded)
    {
        if (global == nullptr)
            global = Farage::recallGlobal();
        if (global != nullptr)
        {
            chatCommands.clear();
            consoleCommands.clear();
            auto cb = events.find(ONSTOP);
            if (cb != events.end())
                (*cb->second.func)(*this,ONSTOP,nullptr,nullptr,nullptr,nullptr);
            events.clear();
            for (auto it = globVars.begin(), ite = globVars.end();it != ite;++it)
            {
                for (auto it1 = global->globVars.begin(), ite1 = global->globVars.end();it1 != ite1;++it1)
                {
                    if (*it == *it1)
                    {
                        delete *it;
                        *it = nullptr;
                        global->globVars.erase(it1);
                        break;
                    }
                }
            }
            globVars.clear();
            for (auto it = timers.begin(), ite = timers.end();it != ite;++it)
            {
                delete *it;
                *it = nullptr;
            }
            timers.clear();
            for (auto it = chatHooks.begin(), ite = chatHooks.end();it != ite;++it)
            {
                delete *it;
                *it = nullptr;
            }
            chatHooks.clear();
            for (auto it = reactHooks.begin(), ite = reactHooks.end();it != ite;++it)
            {
                delete *it;
                *it = nullptr;
            }
            reactHooks.clear();
            for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
            {
                if (*it == this)
                {
                    global->plugins.erase(it);
                    break;
                }
            }
        }
        dlclose(module);
        module = nullptr;
        loaded = false;
        info = nullptr;
        //priority = DEFAULT_PRIORITY;
        modulePath.clear();
        moduleName.clear();
        dlerror(); // clear any errors
    }
}

bool Farage::Handle::load(const std::string &pluginPath, Farage::Global *global, bool reload/*, size_t loadPriority*/)
{
    if ((loaded) && (reload))
    {
        //loadPriority = priority;
        unload(global);
    }
    if (!loaded)
    {
        if (!(module = dlopen(pluginPath.c_str(),RTLD_LAZY|RTLD_LOCAL)))
        {
            Farage::errorOut("Error: Loading module \"" + pluginPath + "\": " + dlerror());
            module = nullptr;
        }
        else
        {
            dlerror();
            modulePath = pluginPath;
            info = reinterpret_cast<Info*>(dlsym(module,"Module"));
            char *error;
            if (((error = dlerror()) != NULL) || (info->API_VER != FARAGE_API_VERSION))
            {
                if (error == NULL)
                    Farage::errorOut("Error: Incompatible module \"" + modulePath + "\". Recompile with FarageAPI version " + std::string(FARAGE_API_VERSION));
                else
                    Farage::errorOut("Error: Invalid module \"" + modulePath + "\": " + error);
                dlclose(module);
                module = nullptr;
                loaded = false;
                //unload(global);
            }
            else
            {
                int (*start)(Farage::Handle&,Farage::Global*) = nullptr;
                *(void **) (&start) = dlsym(module,FARAGE_ONSTART_FUNC);
                if ((error = dlerror()) != NULL)
                {
                    Farage::errorOut("Error: Unable to start module \"" + modulePath + "\": " + error);
                    dlclose(module);
                    module = nullptr;
                    loaded = false;
                }
                else
                {
                    if ((*start)(*this,global))
                        unload(global);
                    else
                    {//./modules/mock_plugin.fso
                        size_t pos[3] = { 0, 0, 0 };
                        while ((pos[2] = modulePath.find('/',pos[2]+1)) != std::string::npos)
                            pos[0] = pos[2];
                        pos[2] = pos[0];
                        while ((pos[2] = modulePath.find('.',pos[2]+1)) != std::string::npos)
                            pos[1] = pos[2];
                        moduleName = modulePath.substr(pos[0]+1,pos[1]-pos[0]-1);
                        loaded = true;
                        hookEvent(ONSTOP);
                        hookEvent(ONSHUTDOWN);
                        hookEvent(ONLOADED);
                        hookEvent(ONREADY);
                        hookEvent(ONRESUMED);
                        hookEvent(ONDELETESERVER);
                        hookEvent(ONEDITSERVER);
                        hookEvent(ONBAN);
                        hookEvent(ONUNBAN);
                        hookEvent(ONMEMBER);
                        hookEvent(ONREMOVEMEMBER);
                        hookEvent(ONEDITMEMBER);
                        hookEvent(ONROLE);
                        hookEvent(ONDELETEROLE);
                        hookEvent(ONEDITROLE);
                        hookEvent(ONEDITEMOJIS);
                        hookEvent(ONMEMBERCHUNK);
                        hookEvent(ONDELETECHANNEL);
                        hookEvent(ONEDITCHANNEL);
                        hookEvent(ONPINMESSAGE);
                        hookEvent(ONPRESENCEUPDATE);
                        hookEvent(ONEDITUSER);
                        hookEvent(ONEDITUSERNOTE);
                        hookEvent(ONEDITUSERSETTINGS);
                        hookEvent(ONEDITVOICESTATE);
                        hookEvent(ONTYPING);
                        hookEvent(ONDELETEMESSAGES);
                        hookEvent(ONEDITMESSAGE);
                        hookEvent(ONEDITVOICESERVER);
                        hookEvent(ONSERVERSYNC);
                        hookEvent(ONRELATIONSHIP);
                        hookEvent(ONDELETERELATIONSHIP);
                        hookEvent(ONREACTION);
                        hookEvent(ONDELETEREACTION);
                        hookEvent(ONDELETEALLREACTION);
                        hookEvent(ONMESSAGE);
                        hookEvent(ONMESSAGE_PRE);
                        hookEvent(ONDMESSAGE);
                        hookEvent(ONDMESSAGE_PRE);
                        hookEvent(ONCMESSAGE);
                        hookEvent(ONCMESSAGE_PRE);
                        hookEvent(ONGMESSAGE);
                        hookEvent(ONGMESSAGE_PRE);
                        hookEvent(ONSERVER);
                        hookEvent(ONCHANNEL);
                        hookEvent(ONDISPATCH);
                        hookEvent(ONHEARTBEAT);
                        hookEvent(ONHEARTBEATACK);
                        hookEvent(ONINVALIDSESSION);
                        hookEvent(ONDISCONNECT);
                        hookEvent(ONRESUME);
                        hookEvent(ONQUIT);
                        hookEvent(ONRESTART);
                        hookEvent(ONRESPONSE);
                        hookEvent(ONERROR);
                        //priority = loadPriority;
                        //setLoadPriority(loadPriority,false,SHIFT_NEW);
                    }
                }
            }
        }
    }
    dlerror(); // clear any errors
    return loaded;
}

std::string Farage::Handle::getAPI()
{
    return info->API_VER;
}

bool Farage::Handle::isLoaded()
{
    return loaded;
}

/*void Farage::Handle::pluginInfo(const std::string &name, const std::string &author, const std::string &description, const std::string &version, const std::string &url)
{
    API_VER = FARAGE_API_VERSION;
    info.name = name;
    info.author = author;
    info.description = description;
    info.version = version;
    info.url = url;
}*/

const Farage::Info* Farage::Handle::getInfo()
{
    return info;
}

size_t Farage::Handle::hookEvent(Event event, Farage::EventCallback func)
{
    if (func == nullptr)
    {
        std::string function = Farage::eventName(event);
        if (function.size() > 0)
            *(void **) (&func) = dlsym(module,function.c_str());
        else
            return 0;
        if (dlerror() != NULL)
            func = nullptr;
    }
    if (func != nullptr)
    {
        Farage::EventWrap evnt;
        evnt.event = event;
        evnt.func = func;
        events[event] = evnt;
    }
    return events.size();
}


size_t Farage::Handle::regChatCmd(const std::string &command, Farage::ChatCmdCallback func, AdminFlag flags, const std::string &description)
{
    Farage::ChatCommand cmd;
    cmd.cmd = command;
    cmd.func = func;
    cmd.flag = flags;
    cmd.desc = description;
    chatCommands.push_back(cmd);
    return chatCommands.size();
}

size_t Farage::Handle::regConsoleCmd(const std::string &command, Farage::ConCmdCallback func, const std::string &description)
{
    Farage::ConsoleCommand cmd;
    cmd.cmd = command;
    cmd.func = func;
    cmd.desc = description;
    consoleCommands.push_back(cmd);
    return consoleCommands.size();
}

size_t Farage::Handle::unregChatCmd(const std::string &command)
{
    for (auto it = chatCommands.begin();it != chatCommands.end();)
    {
        if (it->cmd == command)
            it = chatCommands.erase(it);
        else
            ++it;
    }
    return chatCommands.size();
}

size_t Farage::Handle::unregConsoleCmd(const std::string &command)
{
    for (auto it = consoleCommands.begin();it != consoleCommands.end();)
    {
        if (it->cmd == command)
            it = consoleCommands.erase(it);
        else
            ++it;
    }
    return consoleCommands.size();
}

bool Farage::Handle::findChatCmd(const std::string &command, Farage::ChatCommand &cmd)
{
    for (auto it = chatCommands.begin(), ite = chatCommands.end();it != ite;++it)
    {
        if (it->cmd == command)
        {
            cmd = *it;
            return true;
        }
    }
    return false;
}

bool Farage::Handle::findConsoleCmd(const std::string &command, Farage::ConsoleCommand &cmd)
{
    for (auto it = consoleCommands.begin(), ite = consoleCommands.end();it != ite;++it)
    {
        if (it->cmd == command)
        {
            cmd = *it;
            return true;
        }
    }
    return false;
}

int Farage::Handle::callChatCmd(const std::string &command, AdminFlag flags, int argc, const std::string argv[], const Farage::Message &msg)
{
    int rval = PLUGIN_CONTINUE;
    for (auto it = chatCommands.begin(), ite = chatCommands.end();it != ite;++it)
    {
        if (it->cmd == command)
        {
            if ((it->flag == NOFLAG) || ((flags & it->flag) == it->flag))
            {
                if ((rval = (*it->func)(*this,argc,argv,msg)) == PLUGIN_HANDLED)
                    return rval;
            }
            else
            {
                // respond lack of permissions?
                //recallGlobal()->callbacks.reaction(msg,"randomNegativeEmoji");
                Farage::reaction(msg,"randomNegativeEmoji");
            }
        }
    }
    return rval;
}

int Farage::Handle::callConsoleCmd(const std::string &command, int argc, const std::string argv[])
{
    //Farage::verboseOut("Farage::Handle::callConsoleCmd() called.");
    int rval = PLUGIN_CONTINUE;
    for (auto it = consoleCommands.begin(), ite = consoleCommands.end();it != ite;++it)
    {
        if (it->cmd == command)
        {
            if ((rval = (*it->func)(*this,argc,argv)) == PLUGIN_HANDLED)
                return rval;
        }
    }
    return rval;
}


int Farage::Handle::callEvent(Event event, void *first, void *second, void *third, void *fourth)
{
    //Farage::verboseOut("Farage::Handle::callEvent() called.");
    //(*events.find(event)->second)(*this,event,nullptr,nullptr);
    auto it = events.find(event);
    if (it != events.end())
    {
        //std::cout<<"On call: "<<&it->second.func<<std::endl;
        //(*it->second)(*this,event,first,second);
        return (*it->second.func)(*this,event,first,second,third,fourth);
    }
    return PLUGIN_CONTINUE;
}

//template<class T>
Farage::Timer* Farage::Handle::createTimer(const std::string &name, long interval, Farage::TimerCallback func, void *args, Farage::TimeScale type)
{
    Farage::Timer *timer = new Farage::Timer(name,interval,func,args,type);
    timers.push_back(timer);
    recallGlobal()->processTimersEarly();
    return timer;
}

//template<class T>
size_t Farage::Handle::killTimer(Farage::Timer *timer)
{
    for (auto it = timers.begin();it != timers.end();)
    {
        if (*it == timer)
        {
            it = timers.erase(it);
            invalidTimers = true;
        }
        else
            ++it;
    }
    return timers.size();
}

size_t Farage::Handle::killTimer(const std::string &name)
{
    for (auto it = timers.begin();it != timers.end();)
    {
        if ((*it)->name == name)
        {
            it = timers.erase(it);
            invalidTimers = true;
        }
        else
            ++it;
    }
    return timers.size();
}

/*size_t Farage::Handle::triggerTimer(Farage::Timer *timer)
{
    size_t count = 0;
    std::chrono::nanoseconds interval;
    for (auto it = timers.begin(), ite = timers.end();it != ite;++it)
    {
        if (*it == timer)
        {
            switch ((*it)->type)
            {
                case Farage::TimeScale::MILLISECONDS:
                {
                    interval = (std::chrono::nanoseconds)((*it)->interval * 1000000);
                    break;
                }
                case Farage::TimeScale::MICROSECONDS:
                {
                    interval = (std::chrono::nanoseconds)((*it)->interval * 1000);
                    break;
                }
                case Farage::TimeScale::NANOSECONDS:
                {
                    interval = (std::chrono::nanoseconds)((*it)->interval);
                    break;
                }
                default:
                {
                    interval = (std::chrono::nanoseconds)((*it)->interval * 1000000000);
                }
            }
            (*it)->last -= interval;
            count++;
        }
    }
    return count;
}*/

size_t Farage::Handle::triggerTimer(const std::string &name)
{
    size_t count = 0;
    //std::chrono::nanoseconds interval;
    for (auto it = timers.begin(), ite = timers.end();it != ite;++it)
    {
        if ((*it)->name == name)
        {
            /*switch ((*it)->type)
            {
                case Farage::TimeScale::MILLISECONDS:
                {
                    interval = (std::chrono::nanoseconds)((*it)->interval * 1000000);
                    break;
                }
                case Farage::TimeScale::MICROSECONDS:
                {
                    interval = (std::chrono::nanoseconds)((*it)->interval * 1000);
                    break;
                }
                case Farage::TimeScale::NANOSECONDS:
                {
                    interval = (std::chrono::nanoseconds)((*it)->interval);
                    break;
                }
                default:
                {
                    interval = (std::chrono::nanoseconds)((*it)->interval * 1000000000);
                }
            }
            (*it)->last -= interval;*/
            (*it)->trigger(*this);
            count++;
        }
    }
    return count;
}

//template<class T>
Farage::Timer* Farage::Handle::findTimer(const std::string &name)
{
    for (auto it = timers.begin(), ite = timers.end();it != ite;++it)
        if ((*it)->name == name)
            return (*it);
    return nullptr;
}

Farage::GlobVar* Farage::Handle::createGlobVar(const std::string &name, const std::string &defaultValue, const std::string &description, short flags, bool hasMin, float min, bool hasMax, float max)
{
    Farage::Global *global = Farage::recallGlobal();
    Farage::GlobVar *exists = Farage::findGlobVar(name);
    if (exists == nullptr)
    {
        Farage::GlobVar *temp = new Farage::GlobVar(this,name,defaultValue,description,flags,hasMin,min,hasMax,max);
        global->globVars.push_back(temp);
        globVars.push_back(temp);
        return temp;
    }
    return exists;
}

/*Farage::ChatHook* Farage::Handle::hookChatPattern(const std::string &name, const std::string &pattern, ChatHookCallback func, int flag)
{
    Farage::ChatHook *hook = new Farage::ChatHook;
    hook->name = name;
    hook->pattern = pattern;
    hook->func = func;
    hook->flags = flag;
    chatHooks.push_back(hook);
    return hook;
}*/

Farage::ChatHook* Farage::Handle::hookChatPattern(const std::string &name, const rens::regex &pattern, ChatHookCallback func, int flag)
{
    Farage::ChatHook *hook = new Farage::ChatHook;
    hook->name = name;
    hook->pattern = pattern;
    hook->func = func;
    hook->flags = flag;
    chatHooks.push_back(hook);
    return hook;
}

size_t Farage::Handle::unhookChatPattern(const std::string &name)
{
    size_t count = 0;
    for (auto it = chatHooks.begin();it != chatHooks.end();)
    {
        if ((*it)->name == name)
        {
            delete *it;
            it = chatHooks.erase(it);
            count++;
        }
        else
            ++it;
    }
    return count;
}

size_t Farage::Handle::unhookChatPattern(Farage::ChatHook *hook)
{
    size_t count = 0;
    for (auto it = chatHooks.begin();it != chatHooks.end();)
    {
        if (*it == hook)
        {
            delete *it;
            it = chatHooks.erase(it);
            count++;
        }
        else
            ++it;
    }
    return count;
}

Farage::ChatHook* Farage::Handle::findChatHook(const std::string &name)
{
    Farage::ChatHook *hook = nullptr;
    for (auto it = chatHooks.begin(), ite = chatHooks.end();it != ite;++it)
    {
        if ((*it)->name == name)
        {
            hook = *it;
            break;
        }
    }
    return hook;
}

/*int Farage::Handle::setLoadPriority(size_t loadPriority, bool write, short shift)
{
    short change = 0;
    if ((shift == SHIFT_NEW) || (shift == SHIFT_DOWN))
        change = 3;
    else if (shift == SHIFT_UP)
        change = 2;
    else if (loadPriority == 0)
        change = 1;
    else if (loadPriority < priority)
        change = 2;
    else if (loadPriority > priority)
        change = 3;
    if (shift <= SHIFT_NEW)
        priority = loadPriority;
    if (change)
    {
        Farage::Global *global = Farage::recallGlobal();
        std::vector<Farage::Handle*>::iterator orig = global->plugins.begin();
        if (shift != SHIFT_NEW) for (auto it = global->plugins.begin(), ite = global->plugins.end();it != ite;++it)
        {
            if (*it == this)
            {
                //debugOut("Farage::Handle::setLoadPriority() found plugin.");
                orig = global->plugins.erase(it);
                break;
            }
        }
        switch (change)
        {
            case SHIFT_NEW:
            {
                orig = global->plugins.begin();
                break;
            }
            case SHIFT_UP:
            {
                bool found = false;
                if (orig == global->plugins.end())
                    orig--;
                for (auto ite = global->plugins.begin();orig != ite;--orig)
                {
                    if ((!found) && (shift == SHIFT_UP))
                    {
                        found = true;
                        priority = (*orig)->priority;
                    }
                    else if ((*orig)->priority < priority)
                    {
                        ++orig;
                        break;
                    }
                }
                if ((shift > SHIFT_NEW) && (orig == global->plugins.begin()) && (orig != global->plugins.end()))
                    priority = (*orig)->priority;
                break;
            }
            default: // SHIFT_DOWN
            {
                bool found = false;
                for (auto ite = global->plugins.end();orig != ite;++orig)
                {
                    if ((!found) && (shift == SHIFT_DOWN))
                    {
                        found = true;
                        priority = (*orig)->priority;
                    }
                    else if ((*orig)->priority > priority)
                        break;
                }
            }
        }
        switch (shift)
        {
            case SHIFT_UP:
            {
                if (priority - loadPriority > priority)
                    priority = 0;
                else
                    priority -= loadPriority;
                break;
            }
            case SHIFT_DOWN:
            {
                if (priority + loadPriority < priority)
                    priority = -1;
                else
                    priority += loadPriority;
                break;
            }
        }
        global->plugins.insert(orig,this);
    }
    if (write)
    {
        std::ofstream file ("./config/priority/" + moduleName + ".prio",std::ofstream::trunc);
        if (file.is_open())
        {
            file<<priority<<std::endl;
            file.close();
            return 0;
        }
        return 1;
    }
    return 0;
}*/

Farage::ReactHook* Farage::Handle::hookReactionMessage(const std::string &name, Farage::ReactHookCallback func, int flags, const std::string &messageID, const std::string &emoji, const std::string &userID)
{
    Farage::ReactHook *hook = new Farage::ReactHook{name,userID,messageID,Farage::HookType::msg,Farage::Emoji(emoji),func,flags};
    reactHooks.push_back(hook);
    return hook;
}

Farage::ReactHook* Farage::Handle::hookReactionMessage(const std::string &name, Farage::ReactHookCallback func, int flags, const std::string &messageID, const Farage::Emoji &emoji, const std::string &userID)
{
    Farage::ReactHook *hook = new Farage::ReactHook{name,userID,messageID,Farage::HookType::msg,emoji,func,flags};
    reactHooks.push_back(hook);
    return hook;
}

Farage::ReactHook* Farage::Handle::hookReactionGuild(const std::string &name, Farage::ReactHookCallback func, int flags, const std::string &guildID, const std::string &emoji, const std::string &userID)
{
    Farage::ReactHook *hook = new Farage::ReactHook{name,userID,guildID,Farage::HookType::guild,Farage::Emoji(emoji),func,flags};
    reactHooks.push_back(hook);
    return hook;
}

Farage::ReactHook* Farage::Handle::hookReactionGuild(const std::string &name, Farage::ReactHookCallback func, int flags, const std::string &guildID, const Farage::Emoji &emoji, const std::string &userID)
{
    Farage::ReactHook *hook = new Farage::ReactHook{name,userID,guildID,Farage::HookType::guild,emoji,func,flags};
    reactHooks.push_back(hook);
    return hook;
}

Farage::ReactHook* Farage::Handle::hookReactionChannel(const std::string &name, Farage::ReactHookCallback func, int flags, const std::string &channelID, const std::string &emoji, const std::string &userID)
{
    Farage::ReactHook *hook = new Farage::ReactHook{name,userID,channelID,Farage::HookType::chan,Farage::Emoji(emoji),func,flags};
    reactHooks.push_back(hook);
    return hook;
}

Farage::ReactHook* Farage::Handle::hookReactionChannel(const std::string &name, Farage::ReactHookCallback func, int flags, const std::string &channelID, const Farage::Emoji &emoji, const std::string &userID)
{
    Farage::ReactHook *hook = new Farage::ReactHook{name,userID,channelID,Farage::HookType::chan,emoji,func,flags};
    reactHooks.push_back(hook);
    return hook;
}

Farage::ReactHook* Farage::Handle::hookReaction(const std::string &name, Farage::ReactHookCallback func, int flags, const std::string &emoji, const std::string &userID)
{
    Farage::ReactHook *hook = new Farage::ReactHook{name,userID,"",Farage::HookType::any,Farage::Emoji(emoji),func,flags};
    reactHooks.push_back(hook);
    return hook;
}

Farage::ReactHook* Farage::Handle::hookReaction(const std::string &name, Farage::ReactHookCallback func, int flags, const Farage::Emoji &emoji, const std::string &userID)
{
    Farage::ReactHook *hook = new Farage::ReactHook{name,userID,"",Farage::HookType::any,emoji,func,flags};
    reactHooks.push_back(hook);
    return hook;
}

size_t Farage::Handle::unhookReaction(const std::string &name)
{
    size_t count = 0;
    for (auto it = reactHooks.begin();it != reactHooks.end();)
    {
        if ((*it)->name == name)
        {
            delete *it;
            it = reactHooks.erase(it);
            count++;
        }
        else
            ++it;
    }
    return count;
}

size_t Farage::Handle::unhookReaction(Farage::ReactHook* hook)
{
    size_t count = 0;
    for (auto it = reactHooks.begin();it != reactHooks.end();)
    {
        if (*it == hook)
        {
            delete *it;
            it = reactHooks.erase(it);
            count++;
        }
        else
            ++it;
    }
    return count;
}

Farage::EditHook* Farage::Handle::hookEditMessage(const std::string &name, Farage::EditHookCallback func, int flags, const Farage::Message &message)
{
    Farage::EditHook *hook = new Farage::EditHook{name,"",message.id,Farage::HookType::msg,func,flags};
    editHooks.push_back(hook);
    return hook;
}

Farage::EditHook* Farage::Handle::hookEditMessage(const std::string &name, Farage::EditHookCallback func, int flags, const std::string &messageID)
{
    Farage::EditHook *hook = new Farage::EditHook{name,"",messageID,Farage::HookType::msg,func,flags};
    editHooks.push_back(hook);
    return hook;
}

Farage::EditHook* Farage::Handle::hookEditMessageUser(const std::string &name, Farage::EditHookCallback func, int flags, const std::string &userID)
{
    Farage::EditHook *hook = new Farage::EditHook{name,userID,"",Farage::HookType::any,func,flags};
    editHooks.push_back(hook);
    return hook;
}

Farage::EditHook* Farage::Handle::hookEditMessageChannel(const std::string &name, Farage::EditHookCallback func, int flags, const std::string &channelID, const std::string &userID)
{
    Farage::EditHook *hook = new Farage::EditHook{name,userID,channelID,Farage::HookType::chan,func,flags};
    editHooks.push_back(hook);
    return hook;
}

Farage::EditHook* Farage::Handle::hookEditMessageGuild(const std::string &name, Farage::EditHookCallback func, int flags, const std::string &guildID, const std::string &userID)
{
    Farage::EditHook *hook = new Farage::EditHook{name,userID,guildID,Farage::HookType::guild,func,flags};
    editHooks.push_back(hook);
    return hook;
}

size_t Farage::Handle::unhookEditMessage(const std::string &name)
{
    size_t count = 0;
    for (auto it = editHooks.begin();it != editHooks.end();)
    {
        if ((*it)->name == name)
        {
            delete *it;
            it = editHooks.erase(it);
            count++;
        }
        else
            ++it;
    }
    return count;
}

size_t Farage::Handle::unhookEditMessage(Farage::EditHook* hook)
{
    size_t count = 0;
    for (auto it = editHooks.begin();it != editHooks.end();)
    {
        if (*it == hook)
        {
            delete *it;
            it = editHooks.erase(it);
            count++;
        }
        else
            ++it;
    }
    return count;
}

Farage::DeleteHook* Farage::Handle::hookDeleteMessage(const std::string &name, Farage::DeleteHookCallback func, int flags, const Farage::Message &message)
{
    Farage::DeleteHook *hook = new Farage::DeleteHook{name,message.channel_id,message.id,Farage::HookType::msg,func,flags};
    deleteHooks.push_back(hook);
    return hook;
}

Farage::DeleteHook* Farage::Handle::hookDeleteMessage(const std::string &name, Farage::DeleteHookCallback func, int flags, const std::string &channelID, const std::string &messageID, const std::string &guildID)
{
    Farage::HookType type = any;
    std::string id;
    if (guildID.size() > 0)
    {
        type = guild;
        id = guildID;
    }
    else if (messageID.size() > 0)
    {
        type = msg;
        id = messageID;
    }
    Farage::DeleteHook *hook = new Farage::DeleteHook{name,channelID,id,type,func,flags};
    deleteHooks.push_back(hook);
    return hook;
}

Farage::DeleteHook* Farage::Handle::hookDeleteMessageChannel(const std::string &name, Farage::DeleteHookCallback func, int flags, const std::string &channelID, const std::string &guildID)
{
    Farage::HookType type = chan;
    if (guildID.size() > 0)
        type = guild;
    Farage::DeleteHook *hook = new Farage::DeleteHook{name,channelID,guildID,type,func,flags};
    deleteHooks.push_back(hook);
    return hook;
}

Farage::DeleteHook* Farage::Handle::hookDeleteMessageGuild(const std::string &name, Farage::DeleteHookCallback func, int flags, const std::string &guildID)
{
    Farage::DeleteHook *hook = new Farage::DeleteHook{name,"",guildID,Farage::HookType::guild,func,flags};
    deleteHooks.push_back(hook);
    return hook;
}

size_t Farage::Handle::unhookDeleteMessage(const std::string &name)
{
    size_t count = 0;
    for (auto it = deleteHooks.begin();it != deleteHooks.end();)
    {
        if ((*it)->name == name)
        {
            delete *it;
            it = deleteHooks.erase(it);
            count++;
        }
        else
            ++it;
    }
    return count;
}

size_t Farage::Handle::unhookDeleteMessage(Farage::DeleteHook* hook)
{
    size_t count = 0;
    for (auto it = deleteHooks.begin();it != deleteHooks.end();)
    {
        if (*it == hook)
        {
            delete *it;
            it = deleteHooks.erase(it);
            count++;
        }
        else
            ++it;
    }
    return count;
}

//Farage::ReactHook* Farage::Handle::hookReaction(const std::string &name, Farage::ReactHookCallback func, const std::string &emoji);
//Farage::ReactHook* Farage::Handle::hookReaction(const std::string &name, Farage::ReactHookCallback func, const Farage::Emoji &emoji);


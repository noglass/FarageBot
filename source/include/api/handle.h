#ifndef _FARAGE_HANDLE_
#define _FARAGE_HANDLE_

#define SHIFT_NEW           1
#define SHIFT_UP            2
#define SHIFT_DOWN          3
#define DEFAULT_PRIORITY    327687

#include "conf/.farageAPIbuild.h"
#include "api/events.h"
#include "api/timers.h"
#include "api/commands.h"
#include "api/hooks.h"
#include "api/globvar.h"

namespace Farage
{
    class Global;
    
    class Handle
    {
        public:
            // create a new object
            Handle();
            
            // create a new object and call load(pluginPath)
            Handle(const std::string &pluginPath, Global *global, size_t loadedPriority = -1);
            
            ~Handle();
            
            std::vector<ChatCommand> chatCommands;
            std::vector<ConsoleCommand> consoleCommands;
            std::unordered_map<Event,EventWrap> events;
            std::vector<ChatHook*> chatHooks;
            std::vector<Timer*> timers;
            std::vector<GlobVar*> globVars;
            std::vector<ReactHook*> reactHooks;
            std::vector<EditHook*> editHooks;
            
            void unload(Global *global = nullptr);
            
            // load plugin, returns success
            bool load(const std::string &pluginPath, Global *global, bool reload = false, size_t loadedPriority = -1);
            
            // return the API version used to compile the plugin
            std::string getAPI();
            
            // return loaded
            bool isLoaded();
            
            // fill info struct with plugin info
            //void pluginInfo(const std::string &name, const std::string &author, const std::string &description, const std::string &version, const std::string &url);
            
            // returns a const pointer to the Info struct
            const Info* getInfo();
            
            // returns the path to the plugin
            std::string getPath() { return modulePath; }
            
            // returns the internal name of the plugin
            std::string getModule() { return moduleName; }
            
            inline size_t getLoadPriority() { return priority; }
            int setLoadPriority(size_t loadPriority, bool write = false, short shift = 0);
            
            // register an event hook
            // returns total number of events hooked, 0 on error
            size_t hookEvent(Event event, EventCallback func = nullptr);
            
            // register an chat command
            // returns total number of commands registered by plugin, 0 on error
            size_t regChatCmd(const std::string &command, ChatCmdCallback func, AdminFlag flags, const std::string &description = "");
            
            // register a console command
            // returns total number of commands registered by plugin, 0 on error
            size_t regConsoleCmd(const std::string &command, ConCmdCallback func, const std::string &description = "");
            
            // remove all chat commands that are 'command'
            // returns total number of commands registered by plugin, 0 is not an error, but lack of any commands.
            size_t unregChatCmd(const std::string &command);
            size_t unregConsoleCmd(const std::string &command);
            
            // fills a copy of the Command struct containing 'command'
            // returns false if no command was found
            bool findChatCmd(const std::string &command, ChatCommand &cmd);
            bool findConsoleCmd(const std::string &command, ConsoleCommand &cmd);
            
            int callChatCmd(const std::string &command, AdminFlag flags, int argc, const std::string argv[], const Message &msg);
            int callConsoleCmd(const std::string &command, int argc, const std::string argv[]);
            
            
            // assigns evnt to the function callback for the given event
            // returns false if no event was found
            //bool findEvent(int event, EventCallback &evnt);
            int callEvent(Event event, void *first, void *second, void *third, void *fourth);
            
            // starts a timer
            // returns a pointer to the timer
            //Timer* createTimer(const std::string &name, long interval, const std::string &function, const std::string &args = "", short type = SECONDS);
            //Timer* createTimer(const std::string &name, long interval, TimerCallback func, const std::string &args = "", short type = FARAGE_SECONDS);
            //template<class T>
            //Timer<T>* createTimer(const std::string &name, T interval, TimerCallback<T> func, void *args = nullptr);
            Timer* createTimer(const std::string &name, long interval, TimerCallback func, void *args = nullptr, TimeScale type = SECONDS);
            
            // kills a timer by pointer
            // returns total number of timers registered by the plugin
            //template<class T>
            size_t killTimer(Timer *timer);
            
            // kills a timer by name
            // returns total number of timers registered by the plugin
            size_t killTimer(const std::string &name);
            
            // set a timer to be triggered on the following tick
            // returns total number of triggered timers
            //size_t triggerTimer(Timer *timer);
            
            // set all timers with 'name' to be triggered on the following tick
            // returns total number of triggered timers
            size_t triggerTimer(const std::string &name);
            
            // returns the first timer found matching name
            // returns nullptr if no timer was found
            //template<class T>
            Timer* findTimer(const std::string &name);
            
            bool invalidTimers = false;
            
            // registers a GlobVar
            // returns a pointer to the GlobVar
            GlobVar* createGlobVar(const std::string &name, const std::string &defaultValue, const std::string &description = "", short flags = 0, bool hasMin = false, float min = 0.0, bool hasMax = false, float max = 0.0);
            
            // not necessary..
            //ChatHook* hookChatPattern(const std::string &name, const std::string &pattern, ChatHookCallback func, int flag = 0);
            
            // hooks a callback to chat messages that match the regex pattern
            // returns the stored pointer to the hook
            ChatHook* hookChatPattern(const std::string &name, const rens::regex &pattern, ChatHookCallback func = nullptr, int flag = 0);
            
            // removes hooks by name
            // returns the number of hooks removed
            size_t unhookChatPattern(const std::string &name);
            
            // removes hooks by pointer.
            // WARNING: The pointer will be deleted and invalidated. Referencing it after this will result in a crash
            // returns the number of hooks removed
            size_t unhookChatPattern(ChatHook *hook);
            
            ChatHook* findChatHook(const std::string &name);
            
            ReactHook* hookReactionMessage(const std::string &name, ReactHookCallback func, int flags, const std::string &messageID, const std::string &emoji = "", const std::string &userID = "");
            ReactHook* hookReactionMessage(const std::string &name, ReactHookCallback func, int flags, const std::string &messageID, const Emoji &emoji, const std::string &userID = "");
            ReactHook* hookReactionGuild(const std::string &name, ReactHookCallback func, int flags, const std::string &guildID, const std::string &emoji = "", const std::string &userID = "");
            ReactHook* hookReactionGuild(const std::string &name, ReactHookCallback func, int flags, const std::string &guildID, const Emoji &emoji, const std::string &userID = "");
            ReactHook* hookReactionChannel(const std::string &name, ReactHookCallback func, int flags, const std::string &channelID, const std::string &emoji = "", const std::string &userID = "");
            ReactHook* hookReactionChannel(const std::string &name, ReactHookCallback func, int flags, const std::string &channelID, const Emoji &emoji, const std::string &userID = "");
            ReactHook* hookReaction(const std::string &name, ReactHookCallback func, int flags = 0, const std::string &emoji = "", const std::string &userID = "");
            ReactHook* hookReaction(const std::string &name, ReactHookCallback func, int flags, const Emoji &emoji, const std::string &userID = "");
            size_t unhookReaction(const std::string &name);
            size_t unhookReaction(Farage::ReactHook* hook);
            //ReactHook* hookReaction(const std::string &name, ReactHookCallback func, const std::string &emoji);
            //ReactHook* hookReaction(const std::string &name, ReactHookCallback func, const Emoji &emoji);
            
            EditHook* hookEditMessage(const std::string &name, EditHookCallback func, int flags, const Message &message);
            EditHook* hookEditMessage(const std::string &name, EditHookCallback func, int flags = 0, const std::string &messageID = "");
            EditHook* hookEditMessageUser(const std::string &name, EditHookCallback func, int flags, const std::string &userID);
            EditHook* hookEditMessageChannel(const std::string &name, EditHookCallback func, int flags, const std::string &channelID, const std::string &userID = "");
            EditHook* hookEditMessageGuild(const std::string &name, EditHookCallback func, int flags, const std::string &guildID, const std::string &userID = "");
            size_t unhookEditMessage(const std::string &name);
            size_t unhookEditMessage(EditHook* hook);
            
            //size_t totalChatCmds() { return chatCommands.size(); }
            //size_t totalConsoleCmds() { return consoleCommands.size(); }
            //size_t totalEvents() { return events.size(); }
            //size_t totalGvars() { return globVars.size(); }
            //size_t totalHooks() { return chatHooks.size(); }
        
        private:
            bool loaded = false;
            Info *info = nullptr;
            void *module = nullptr;
            std::string modulePath;
            std::string moduleName;
            size_t priority = DEFAULT_PRIORITY;
    };
};

#endif


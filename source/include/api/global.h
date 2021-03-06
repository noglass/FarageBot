#ifndef _FARAGE_GLOBAL_
#define _FARAGE_GLOBAL_

#define FARAGE_DEFAULT_PREFIX               "!"

#include "api/internal.h"
#include "api/admins.h"
#include <mutex>
#include <atomic>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
    #ifdef _USE_GTCINTERFACE_
#include "shared/gtci.h"
    #endif
#endif

namespace Farage
{
    class Handle;
    class GlobVar;
    
    template<typename T>
    struct safe_ptr
    {
        std::unique_lock<std::mutex> lock;
        T* ptr;
        safe_ptr(std::unique_lock<std::mutex>&& l, T* data) : lock(std::move(l)), ptr(data) {}
        safe_ptr() : ptr(nullptr) {}
        inline T* operator->() { return ptr; }
        inline bool owns_lock() { return !(ptr == nullptr); }
        void clear()
        {
            if (owns_lock())
                ptr->clear();
        }
    };
    
    struct Alias
    {
        std::string cmd;
        bool perm;
    };
    
    struct AliasSet
    {
        std::unordered_map<std::string,Alias> aliases;
        std::unordered_map<std::string,Alias>::iterator find(const std::string &input)
        {
            auto it = aliases.begin(), ite = aliases.end();
            for (;it != ite;++it)
                if (input.find(it->first) == 0)
                    return it;
            return ite;
        }
        bool get(std::string &input, bool &permission)
        {
            auto it = find(input);
            if (it == aliases.end())
                return false;
            input = it->second.cmd + input.substr(it->first.size());
            permission = it->second.perm;
            return true;
        }
    };
    
    class Global
    {
        public:
#ifdef _WIN32
            Global(std::string version, HANDLE timerTrigger, Internals cbs) : engineVer(version), triggerFD(timerTrigger), callbacks(cbs) {}
#else
    #ifdef _GTCINTERFACE_
            Global(std::string version, int timerTrigger, Internals cbs, gtci::TabCallback tab = nullptr) : engineVer(version), triggerFD(timerTrigger), callbacks(cbs)
            {
                if (tab != nullptr)
                    io.setCallback(tab);
            }
    #else
            Global(std::string version, int timerTrigger, Internals cbs) : engineVer(version), triggerFD(timerTrigger), callbacks(cbs) {}
    #endif
#endif
            std::unordered_map<std::string,AdminFlag> admins;
            std::unordered_map<std::string,std::unordered_map<std::string,AdminFlag>> adminRoles;
            std::vector<Handle*> plugins;
            std::vector<GlobVar*> globVars;
            std::unordered_map<std::string,std::string> prefixes;
            AliasSet aliases;
            AliasSet prefixedAliases;
            std::string prefix(const std::string &guild_id = "default");
            bool verbose;
            bool debug;
            std::vector<std::string> ignoredChannels;
            std::vector<std::string> ignoredUsers;
            User self;
            void *discord = nullptr;
            inline std::string engineVersion() { return engineVer; }
            AdminFlag getAdminFlags(const std::string &userID);
            AdminFlag getAdminFlags(const std::string &guildID, const std::string &userID);
            AdminFlag getAdminRoleFlags(const std::string &guildID, const std::string &roleID);
            Ready lastReady;
            Internals callbacks;
            safe_ptr<std::vector<std::string>> getBuffer()
            {
                std::unique_lock<std::mutex> lock(mut);
                if (buffer.size() > 64)
                    buffer.clear();
                safe_ptr<std::vector<std::string>> ptr(std::move(lock),&buffer);
                return ptr;
            }
            safe_ptr<std::vector<std::string>> tryGetBuffer()
            {
                std::unique_lock<std::mutex> lock(mut,std::try_to_lock);
                if (lock.owns_lock())
                {
                    safe_ptr<std::vector<std::string>> ptr(std::move(lock),&buffer);
                    return ptr;
                }
                return safe_ptr<std::vector<std::string>>();
            }
#ifdef _WIN32
            inline void processTimersEarly() { DWORD annoyingWindows; WriteFile(triggerFD,"\0",1,&annoyingWindows,NULL); }
#else
            inline void processTimersEarly() { write(triggerFD,"\0",1); }
#endif
#ifdef _GTCINTERFACE_
            gtci::interface* getInterface()
            {
                return &io;
            }
#endif
            
        private:
            std::mutex mut;
            std::vector<std::string> buffer;
            std::string engineVer;
#ifdef _WIN32
            HANDLE triggerFD;
#else
            int triggerFD;
#endif
#ifdef _GTCINTERFACE_
            gtci::interface io;
#endif
    };
};

#endif


#ifndef _FARAGE_GLOBAL_
#define _FARAGE_GLOBAL_

#define FARAGE_DEFAULT_PREFIX               "!"

#include "api/internal.h"
#include "api/admins.h"
#include <mutex>
#include <atomic>

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
    
    class Global
    {
        public:
            Global(std::string version, Internals cbs) : engineVer(version), callbacks(cbs) {}
            std::unordered_map<std::string,AdminFlag> admins;
            std::unordered_map<std::string,std::unordered_map<std::string,AdminFlag>> adminRoles;
            std::vector<Handle*> plugins;
            std::vector<GlobVar*> globVars;
            std::unordered_map<std::string,std::string> prefixes;
            std::string prefix(const std::string &guild_id = "default");
            bool verbose;
            bool debug;
            std::vector<std::string> ignoredChannels;
            std::vector<std::string> ignoredUsers;
            std::string selfID;
            void *discord;
            inline std::string engineVersion() { return engineVer; }
            AdminFlag getAdminFlags(const std::string &userID);
            AdminFlag getAdminFlags(const std::string &guildID, const std::string &userID);
            AdminFlag getAdminRoleFlags(const std::string &guildID, const std::string &roleID);
            Internals callbacks;
            safe_ptr<std::vector<std::string>> getBuffer()
            {
                std::unique_lock<std::mutex> lock(mut);
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
            
        private:
            std::mutex mut;
            std::vector<std::string> buffer;
            std::string engineVer;
    };
};

#endif


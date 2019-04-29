#ifndef _FARAGE_GLOBAL_
#define _FARAGE_GLOBAL_

#define FARAGE_DEFAULT_PREFIX               "!"

#include "api/internal.h"
#include "api/admins.h"

namespace Farage
{
    class Handle;
    class GlobVar;
    
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
            inline void clearBuffer() { consoleBuffer.clear(); }
            inline std::vector<std::string>* getBuffer() { return &consoleBuffer; }
        private:
            std::string engineVer;
            std::vector<std::string> consoleBuffer;
    };
};

#endif


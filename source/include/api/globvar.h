#ifndef _FARAGE_GLOBVAR_
#define _FARAGE_GLOBVAR_

#define GVAR_CONSTANT                       1
#define GVAR_DUPLICATE                      2

#include "api/link.h"

namespace Farage
{
    class Handle;
    class GlobVar;
    
    typedef int (*GlobVarHook)(Handle&,GlobVar*,const std::string&,const std::string&,const std::string&);
    
    class GlobVar
    {
        public:
            GlobVar() {};
            GlobVar(Handle *plugin, const std::string &cname, const std::string &defaultValue, const std::string &description = "", short flag = 0, bool hasMinimum = false, float minimum = 0.0, bool hasMaximum = false, float maximum = 0.0);
            short flags;
            bool hasMin;
            bool hasMax;
            float min;
            float max;
            std::string getName();
            std::string getDesc();
            std::string getDefault();
            void reset(const std::string &guild = "");
            std::string getAsString(const std::string &guild = "");
            bool getAsBool(const std::string &guild = "");
            int getAsInt(const std::string &guild = "");
            float getAsFloat(const std::string &guild = "");
            void setString(const std::string &val, const std::string &guild = "");
            void setBool(bool val, const std::string &guild = "");
            void setInt(int val, const std::string &guild = "");
            void setFloat(float val, const std::string &guild = "");
            void hookChange(GlobVarHook func);
            std::vector<GlobVarHook> hooks;
        private:
            std::string name;
            std::string desc;
            std::string defVal;
            std::string value;
            std::unordered_map<std::string,std::string> guildValues;
            Handle *handle;
    };
};

/*namespace Farage
{
    class Handle;
    class GlobVar;
    
    typedef int (*GlobVarHook)(Handle&,GlobVar*,const std::string&,const std::string&);
    
    class GlobVar
    {
        public:
            GlobVar() {};
            GlobVar(Handle *plugin, const std::string &cname, const std::string &defaultValue, const std::string &description = "", short flag = 0, bool hasMinimum = false, float minimum = 0.0, bool hasMaximum = false, float maximum = 0.0);
            short flags;
            bool hasMin;
            bool hasMax;
            float min;
            float max;
            std::string getName();
            std::string getDesc();
            std::string getDefault();
            void reset();
            std::string getAsString();
            bool getAsBool();
            int getAsInt();
            float getAsFloat();
            void setString(const std::string &val);
            void setBool(bool val);
            void setInt(int val);
            void setFloat(float val);
            void hookChange(GlobVarHook func);
            std::vector<GlobVarHook> hooks;
        private:
            std::string name;
            std::string desc;
            std::string defVal;
            std::string value;
            Handle *handle;
    };
};*/

#endif


#ifndef _FARAGE_GLOBVAR_
#define _FARAGE_GLOBVAR_

#define GVAR_CONSTANT                       1

#include "api/basics.h"

namespace Farage
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
};

#endif


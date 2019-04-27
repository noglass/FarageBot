#include <string>
#include <vector>
#include "api/globvar.h"

Farage::GlobVar::GlobVar(Farage::Handle *plugin, const std::string &cname, const std::string &defaultValue, const std::string &description, short flag, bool hasMinimum, float minimum, bool hasMaximum, float maximum)
{
    name = cname;
    desc = description;
    defVal = value = defaultValue;
    flags = flag;
    hasMin = hasMinimum;
    min = minimum;
    hasMax = hasMaximum;
    max = maximum;
    handle = plugin;
}

std::string Farage::GlobVar::getName()
{
    return name;
}

std::string Farage::GlobVar::getDesc()
{
    return desc;
}

std::string Farage::GlobVar::getDefault()
{
    return defVal;
}

void Farage::GlobVar::reset()
{
    setString(defVal);
}

std::string Farage::GlobVar::getAsString()
{
    return value;
}

bool Farage::GlobVar::getAsBool()
{
    std::string val = strlower(value);
    if ((val == "false") || (val == "0") || (val.size() < 1))
        return false;
    return true;
}

int Farage::GlobVar::getAsInt()
{
    return std::stoi(value);
}

float Farage::GlobVar::getAsFloat()
{
    return std::stof(value);
}

void Farage::GlobVar::setString(const std::string &val)
{
    if (!(flags & GVAR_CONSTANT))
    {
        float fstr = ((hasMin) ? (min-1.0) : ((hasMax) ? (max+1.0) : (0.0)));
        if (val == "true")
            fstr = 1.0;
        else if (val == "false")
            fstr = 0.0;
        else
        {
            char temp = val.at(0);
            if ((std::isdigit(temp)) || ((temp == '-') && (std::isdigit(val.at(1)))))
                fstr = std::stof(val);
        }
        if (((hasMin) && (fstr < min)) || ((hasMax) && (fstr > max)))
            return;
        std::string old = value;
        value = val;
        if (old != value)
            for (auto it = hooks.begin(), end = hooks.end();it != end;++it)
                if ((*it)(*handle,this,val,old) == PLUGIN_HANDLED)
                    break;
    }
}

void Farage::GlobVar::setBool(bool val)
{
    if (val)
        setString("true");
    else
        setString("false");
}

void Farage::GlobVar::setInt(int val)
{
    setString(std::to_string(val));
}

void Farage::GlobVar::setFloat(float val)
{
    setString(std::to_string(val));
}

void Farage::GlobVar::hookChange(Farage::GlobVarHook func)
{
    if (func != nullptr)
        hooks.push_back(func);
}


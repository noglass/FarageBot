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
    if ((flag & GVAR_DUPLICATE) && (Farage::isReady()))
    {
        Farage::Global *global = Farage::recallGlobal();
        for (auto it = global->lastReady.guilds.begin(), ite = global->lastReady.guilds.end();it != ite;++it)
            guildValues[*it] = value;
    }
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

void Farage::GlobVar::reset(const std::string &guild)
{
    setString(defVal,guild);
}

std::string Farage::GlobVar::getAsString(const std::string &guild)
{
    if ((flags & GVAR_DUPLICATE) && (guild.size() > 0))
        return guildValues[guild];
    return value;
}

bool Farage::GlobVar::getAsBool(const std::string &guild)
{
    std::string val;
    if ((flags & GVAR_DUPLICATE) && (guild.size() > 0))
        val = strlower(guildValues[guild]);
    else
        val = strlower(value);
    if ((val == "false") || (val == "0") || (val.size() < 1))
        return false;
    return true;
}

int Farage::GlobVar::getAsInt(const std::string &guild)
{
    std::string val;
    if ((flags & GVAR_DUPLICATE) && (guild.size() > 0))
        val = guildValues[guild];
    else
        val = value;
    char dig = val.front();
    if ((std::isdigit(dig)) || ((dig == '-') && (val.size() > 1) && (std::isdigit(val[1]))))
        return std::stoi(val);
    else
        return 0;
}

float Farage::GlobVar::getAsFloat(const std::string &guild)
{
    std::string val;
    if ((flags & GVAR_DUPLICATE) && (guild.size() > 0))
        val = guildValues[guild];
    else
        val = value;
    char dig = val.front();
    size_t pos = 0, size = val.size();
    if ((dig == '-') && (size > 1))
        dig = val[++pos];
    if ((std::isdigit(dig)) || ((dig == '.') && (size-pos > 1) && (std::isdigit(val[pos+1]))))
        return std::stof(val);
    else
        return 0.0;
}

void Farage::GlobVar::setString(const std::string &val, const std::string &guild)
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
            size_t pos = 0, size = val.size();
            if ((temp == '-') && (size > 1))
                temp = val[++pos];
            if ((std::isdigit(temp)) || ((temp == '.') && (std::isdigit(val.at(pos+1)))))
                fstr = std::stof(val);
        }
        if (((hasMin) && (fstr < min)) || ((hasMax) && (fstr > max)))
            return;
        std::string old, cur;
        bool changed = false;
        if (flags & GVAR_DUPLICATE)
        {
            if (guild.size() < 1)
            {
                for (auto it = guildValues.begin(), ite = guildValues.end();it != ite;++it)
                {
                    old = it->second;
                    cur = it->second = val;
                    if (old != cur)
                        changed = true;
                }
            }
            else
            {
                old = guildValues[guild];
                cur = guildValues[guild] = val;
                if (old != cur)
                    changed = true;
            }
        }
        else
        {
            old = value;
            cur = value = val;
            if (old != cur)
                changed = true;
        }
        if (changed)
            for (auto it = hooks.begin(), end = hooks.end();it != end;++it)
                if ((*it)(*handle,this,val,old,guild) == PLUGIN_HANDLED)
                    break;
    }
}

void Farage::GlobVar::setBool(bool val, const std::string &guild)
{
    if (val)
        setString("true",guild);
    else
        setString("false",guild);
}

void Farage::GlobVar::setInt(int val, const std::string &guild)
{
    setString(std::to_string(val),guild);
}

void Farage::GlobVar::setFloat(float val, const std::string &guild)
{
    setString(std::to_string(val),guild);
}

void Farage::GlobVar::hookChange(Farage::GlobVarHook func)
{
    if (func != nullptr)
        hooks.push_back(func);
}


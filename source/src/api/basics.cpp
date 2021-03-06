#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <random>
#include "api/basics.h"
#include "api/globvar.h"

std::string strlower(std::string text)
{
    if (text.size() > 0)
        for (std::string::iterator it = text.begin(), ite = text.end();it != ite;++it)
            *it = tolower(*it);
    return std::move(text);
}

uint32_t mtrand(uint32_t lo, uint32_t hi)
{
    static std::mt19937 mt;// (std::chrono::system_clock::now().time_since_epoch().count());
    static bool seeded = false;
    if (!seeded)
    {
        std::random_device rd;
        std::vector<uint32_t> seed;
        for (int i = 64;i;--i)
            seed.push_back(rd());
        std::seed_seq seeq (seed.begin(),seed.end());
        mt.seed(seeq);
        seeded = true;
    }
    uint32_t r;
    if (lo >= hi)
        r = lo;
    else
        r = (std::uniform_int_distribution<uint32_t>(lo,hi))(mt);
    return r;
}

Farage::Global* Farage::recallGlobal(Farage::Global *global)
{
    static Farage::Global *globe = global;
    if (global != nullptr)
        globe = global;
    return globe;
}

Farage::GlobVar* Farage::findGlobVar(const std::string &name)
{
    Farage::GlobVar *timer = nullptr;
    Farage::Global *global = Farage::recallGlobal();
    for (auto it = global->globVars.begin(), ite = global->globVars.end();it != ite;++it)
    {
        if ((*it)->getName() == name)
        {
            timer = *it;
            break;
        }
    }
    return timer;
}

void Farage::consoleOut(const std::string &msg, bool notry)
{
    Farage::Global *global = Farage::recallGlobal();
    #ifndef _GTCINTERFACE_
        std::cout<<msg<<std::endl;
    #else
        global->getInterface()->printLn(msg);
    #endif
    if (notry)
        global->getBuffer()->push_back(msg);
    else
    {
        auto b = global->tryGetBuffer();
        if (b.owns_lock())
            b->push_back(msg);
    }
}

void Farage::errorOut(const std::string &msg, bool notry)
{
    Farage::Global *global = Farage::recallGlobal();
    #ifndef _GTCINTERFACE_
        std::cerr<<msg<<std::endl;
    #else
        global->getInterface()->printLn(msg);
    #endif
    if (notry)
        global->getBuffer()->push_back(msg);
    else
    {
        auto b = global->tryGetBuffer();
        if (b.owns_lock())
            b->push_back(msg);
    }
}

void Farage::debugOut(const std::string &msg, bool notry)
{
    Farage::Global *global = Farage::recallGlobal();
    if (global->debug)
    {
        #ifndef _GTCINTERFACE_
            std::cerr<<msg<<std::endl;
        #else
            global->getInterface()->printLn(msg);
        #endif
        if (notry)
            global->getBuffer()->push_back(msg);
        else
        {
            auto b = global->tryGetBuffer();
            if (b.owns_lock())
                b->push_back(msg);
        }
    }
}

void Farage::verboseOut(const std::string &msg, bool notry)
{
    Farage::Global *global = Farage::recallGlobal();
    if (global->verbose)
    {
        #ifndef _GTCINTERFACE_
            std::cout<<msg<<std::endl;
        #else
            global->getInterface()->printLn(msg);
        #endif
        if (notry)
            global->getBuffer()->push_back(msg);
        else
        {
            auto b = global->tryGetBuffer();
            if (b.owns_lock())
                b->push_back(msg);
        }
    }
}

int Farage::ignoreChannel(const std::string &ID, bool toggle)
{
    int ret = 0;
    Farage::Global *global = Farage::recallGlobal();
    for (auto it = global->ignoredChannels.begin(), ite = global->ignoredChannels.end();it != ite;++it)
    {
        if (*it == ID)
        {
            if (toggle)
            {
                global->ignoredChannels.erase(it);
                ret = 2;
            }
            else
                ret = 1;
            break;
        }
    }
    if (!ret)
        global->ignoredChannels.push_back(ID);
    return ret;
}

int Farage::ignoreUser(const std::string &ID, bool toggle)
{
    int ret = 0;
    Farage::Global *global = Farage::recallGlobal();
    for (auto it = global->ignoredUsers.begin(), ite = global->ignoredUsers.end();it != ite;++it)
    {
        if (*it == ID)
        {
            if (toggle)
            {
                global->ignoredUsers.erase(it);
                ret = 2;
            }
            else
                ret = 1;
            break;
        }
    }
    if (!ret)
        global->ignoredUsers.push_back(ID);
    return ret;
}

int Farage::saveIgnoredChannels()
{
    Farage::Global *global = Farage::recallGlobal();
    std::fstream file ("./config/channels.ignore",std::ios_base::out|std::ios_base::trunc);
    if (file.is_open())
    {
        for (auto it = global->ignoredChannels.begin(), ite = global->ignoredChannels.end();it != ite;++it)
            file<<*it<<std::endl;
        file.close();
        return 0;
    }
    return 1;
}

int Farage::saveIgnoredUsers()
{
    Farage::Global *global = Farage::recallGlobal();
    std::fstream file ("./config/users.ignore",std::ios_base::out|std::ios_base::trunc);
    if (file.is_open())
    {
        for (auto it = global->ignoredUsers.begin(), ite = global->ignoredUsers.end();it != ite;++it)
            file<<*it<<std::endl;
        file.close();
        return 0;
    }
    return 1;
}

int Farage::saveAdminRoles()
{
    Farage::Global *global = Farage::recallGlobal();
    std::fstream file ("./config/adminroles.ini",std::ios_base::out|std::ios_base::trunc);
    if (file.is_open())
    {
        file<<"# This file is automatically generated, manually editing this file is not advised!"<<std::endl;
        for (auto guild = global->adminRoles.begin(), guilde = global->adminRoles.end();guild != guilde;++guild)
        {
            file<<'['<<guild->first<<']'<<std::endl;
            for (auto role = guild->second.begin(), rolee = guild->second.end();role != rolee;++role)
                file<<role->first<<'='<<Farage::getAdminFlagString(role->second)<<std::endl;
        }
        file.close();
        return 0;
    }
    return 1;
}

/*void Farage::getEventData(void *data, std::string &to)
{
    if (data != nullptr)
        to = std::move(*(std::string*)data);
}

void Farage::getEventData(void *data, Server &to)
{
    if (data != nullptr)
        to = std::move(*(Server*)data);
}

void Farage::getEventData(void *data, Message &to)
{
    if (data != nullptr)
        to = std::move(*(Message*)data);
}*/


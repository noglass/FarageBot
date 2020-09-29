#include "api/farage.h"
#include <iostream>
#include <cstdio>
#include <cstring>
using namespace Farage;

#define VERSION "v0.0.9"

extern "C" Info Module
{
    "Calc",
    "nigel",
    "Calculator",
    VERSION,
    "http://calculations.justca.me/",
    FARAGE_API_VERSION
};

namespace calc
{
    rens::regex rem ("[\\\\\"\\$`';\n]|\\\\n");
    std::string exec(const std::string& command, const int size = 128, bool getAll = false)
    {
        std::string output;
        char buffer[size];
        FILE* outstream = popen(command.c_str(),"r");
        if (outstream)
        {
            if (fgets(buffer,size,outstream) != NULL)
                output = buffer;
            if (getAll)
                while (!feof(outstream))
                    if (fgets(buffer,size,outstream) != NULL)
                        output.append(buffer);
            pclose(outstream);
        }
        for (auto it = output.end(), ite = output.begin();(it-- != ite) && (*it == '\n');it = output.erase(it));
        return output;
    }
    /*int calcCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
    {
        if (argc < 2)
            sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <expression>`");
        else
        {
            std::string expression = argv[1], output;
            for (int i = 2;i < argc;expression += argv[i++]);
            if ((output = calc::exec("exec bash -c 'let \"foo=" + (expression = rens::regex_replace(expression,calc::rem,"")) + "\";echo $foo'")).size() < 1)
                reaction(message,"%E2%9D%97");
            else
                sendMessage(message.channel_id,"`" + expression + "` = `" + output + '`');
        }
        return PLUGIN_HANDLED;
    }*/
    int calcCmd(Handle& handle, int argc, const std::string* argv, const Message& message)
    {
        if (argc < 2)
            sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + *argv + " <expression>`");
        else
        {
            std::string expression = *(argv+1), output;
            for (int i = 2;i < argc;expression += *(argv+i++));
            if ((output = calc::exec("exec bash -c 'let \"foo=" + (expression = rens::regex_replace(expression,calc::rem,"")) + "\";echo $foo'")).size() < 1)
                reaction(message,"%E2%9D%97");
            else
                sendMessage(message.channel_id,"`" + expression + "` = `" + output + '`');
        }
        return PLUGIN_HANDLED;
    }
    int calcFCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
    {
        if (argc < 2)
            sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <expression>`");
        else
        {
            std::string expression = argv[1], output;
            for (int i = 2;i < argc;expression += " " + argv[i++]);
            if ((output = calc::exec("exec bash -c 'awk \"BEGIN { ; print " + (expression = rens::regex_replace(expression,calc::rem,"")) + "}\"'")).size() < 1)
                reaction(message,"%E2%9D%97");
            else
                sendMessage(message.channel_id,"`" + expression + "` = `" + output + '`');
        }
        return 1;
    }
};

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("calc_version",VERSION,"Calc Version",GVAR_CONSTANT);
    handle.regChatCmd("calc",&calc::calcCmd,NOFLAG,"Perform integer math.");
    handle.regChatCmd("calcf",&calc::calcFCmd,NOFLAG,"Perform floating point math.");
    return 0;
}


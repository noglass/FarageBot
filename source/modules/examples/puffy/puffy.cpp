#include "api/farage.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include "shared/regex.h"
using namespace Farage;

#define VERSION "v0.1.5"

extern "C" Info Module
{
    "Puffy",
    "Madison",
    "Puffy Studio",
    VERSION,
    "http://puffy.justca.me/",
    FARAGE_API_VERSION
};

namespace puffy
{
    std::string exec(const std::string& command, const int size = 128, bool getAll = true)
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
        //for (auto it = output.end(), ite = output.begin();(it-- != ite) && (*it == '\n');it = output.erase(it));
        return output;
    }
    static rens::regex emoji ("<?(a:|:)?([^:]+):([0-9]+)>?");
    static rens::regex image ("(?i)^https?://.*\\.(jpe?g|png|gif|svg)(\\?.*)?$");
    int puffyCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
    {
        std::string arg;
        std::string masked, svg;
        if (argc > 1)
        {
            if (argv[1] == "nomask")
            {
                masked = "--nomask ";
                if (argc > 2)
                    arg = argv[2];
            }
            else
                arg = argv[1];
        }
        rens::smatch ml;
        std::string outfile = message.timestamp + "puffy.";
        std::string url;
        if (message.attachments.size() > 0)
        {
            for (auto& a : message.attachments)
            {
                if (rens::regex_search(a.url,ml,image))
                {
                    outfile += ml[1].str();
                    url = a.url;
                    if (strlower(ml[1].str()) == "svg")
                        svg = "--svg ";
                    else if ((strlower(ml[1].str()) == "jpg") || (strlower(ml[1].str()) == "jpeg"))
                        svg = "--jpg ";
                    break;
                }
            }
        }
        if ((url.size() == 0) && (arg.size() > 1))
        {
            if (rens::regex_search(arg,ml,emoji))
            {
                url = "https://cdn.discordapp.com/emojis/" + ml[3].str();
                if (ml[1].str().size() > 1)
                {
                    outfile += "gif";
                    url += ".gif";
                }
                else
                {
                    outfile += "png";
                    url += ".png";
                }
            }
            else if (rens::regex_search(arg,ml,image))
            {
                outfile += ml[1].str();
                url = arg;
                if (strlower(ml[1].str()) == "svg")
                    svg = "--svg ";
                else if ((strlower(ml[1].str()) == "jpg") || (strlower(ml[1].str()) == "jpeg"))
                    svg = "--jpg ";
            }
        }
        if ((url.size() == 0) && (arg.size() == 0))
        {
            url = "https://cdn.discordapp.com/avatars/" + message.author.id + '/' + message.author.avatar + ".png?size=1024";
            outfile += "png";
        }
        if (url.size() > 0)
        {
            //std::string result = puffy::exec("curl -s -o " + outfile + ' ' + url + "2>&1 1>/dev/null",1024);
            system(("curl -s -o " + outfile + ' ' + url).c_str());
            std::string puff = "puffy/" + outfile + ".png";
            //result = puffy::exec("convert puffy/puff_template.png \\( '" + outfile + "' -resize 128x128\\> \\) -composite \\( puffy/puffermask.png -alpha Off -compose CopyOpacity \\) -composite '" + puff + "' 2>&1 1>/dev/null",1024);
            std::string result;
            result = puffy::exec("./puffy.sh " + masked + svg + '\'' + outfile + "' '" + puff + '\'',1024);
            if (result.size() > 0)
                sendMessage(message.channel_id,"Error!\n```\n" + result + "\n```");
            else
                sendFile(message.channel_id,puff,"Puffitized!");
            puffy::exec("rm " + outfile);
        }
        else
            sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " options [image url|custom emoji]`\nOptions can be `nomask` to create a puffy without applying the mask.\nYou can also upload an image.");
        return PLUGIN_HANDLED;
    }
    
    int puffy3dCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
    {
        std::string arg;
        std::string svg;//masked, svg;
        /*if (argc > 1)
        {
            if (argv[1] == "nomask")
            {
                masked = "--nomask ";
                if (argc > 2)
                    arg = argv[2];
            }
            else
                arg = argv[1];
        }*/
        if (argc > 1)
            arg = argv[1];
        rens::smatch ml;
        std::string outfile = message.timestamp + "puffy3d.";
        std::string url;
        if (message.attachments.size() > 0)
        {
            for (auto& a : message.attachments)
            {
                if (rens::regex_search(a.url,ml,image))
                {
                    outfile += ml[1].str();
                    url = a.url;
                    if (strlower(ml[1].str()) == "svg")
                        svg = "--svg ";
                    else if ((strlower(ml[1].str()) == "jpg") || (strlower(ml[1].str()) == "jpeg"))
                        svg = "--jpg ";
                    break;
                }
            }
        }
        if ((url.size() == 0) && (arg.size() > 1))
        {
            if (rens::regex_search(arg,ml,emoji))
            {
                url = "https://cdn.discordapp.com/emojis/" + ml[3].str();
                if (ml[1].str().size() > 1)
                {
                    outfile += "gif";
                    url += ".gif";
                }
                else
                {
                    outfile += "png";
                    url += ".png";
                }
            }
            else if (rens::regex_search(arg,ml,image))
            {
                outfile += ml[1].str();
                url = arg;
                if (strlower(ml[1].str()) == "svg")
                    svg = "--svg ";
                else if ((strlower(ml[1].str()) == "jpg") || (strlower(ml[1].str()) == "jpeg"))
                    svg = "--jpg ";
            }
        }
        if ((url.size() == 0) && (argc == 1))
        {
            url = "https://cdn.discordapp.com/avatars/" + message.author.id + '/' + message.author.avatar + ".png?size=1024";
            outfile += "png";
        }
        if (url.size() > 0)
        {
            //std::string result = puffy::exec("curl -s -o " + outfile + ' ' + url + "2>&1 1>/dev/null",1024);
            system(("curl -s -o " + outfile + ' ' + url).c_str());
            std::string puff = "puffy/" + outfile + ".png";
            //result = puffy::exec("convert puffy/puff_template.png \\( '" + outfile + "' -resize 128x128\\> \\) -composite \\( puffy/puffermask.png -alpha Off -compose CopyOpacity \\) -composite '" + puff + "' 2>&1 1>/dev/null",1024);
            std::string result;
            result = puffy::exec("./puffy3d.sh " + svg + '\'' + outfile + "' '" + puff + '\'',1024);
            if (result.size() > 0)
                sendMessage(message.channel_id,"Error!\n```\n" + result + "\n```");
            else
                sendFile(message.channel_id,puff,"Puffitized!");
            puffy::exec("rm " + outfile);
        }
        else
            sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " [image url|custom emoji]`\nYou can also upload an image.");
        return PLUGIN_HANDLED;
    }
}

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("puffy_version",VERSION,"Puffy Version",GVAR_CONSTANT);
    handle.regChatCmd("puffy",&puffy::puffyCmd,NOFLAG,"Make a puff.");
    handle.regChatCmd("puff",&puffy::puffy3dCmd,NOFLAG,"Make a 3d puff.");
    puffy::exec("rm puffy/*puffy.png 2>/dev/null");
    return 0;
}


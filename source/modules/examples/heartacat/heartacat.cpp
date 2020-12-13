#include "api/farage.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include "shared/regex.h"
using namespace Farage;

#define VERSION "v0.4.3"

extern "C" Info Module
{
    "Heartacat",
    "Madison",
    "Heartacat Studio",
    VERSION,
    "http://heartcat.justca.me/",
    FARAGE_API_VERSION
};

namespace puffy
{
    rens::regex rem ("[\\\\\"\\$`';\n]|\\\\n");
    std::string escapePound(std::string str)
    {
        for (size_t pos = 0;(pos = str.find('#',pos)) != std::string::npos;pos += 2)
            str.insert(pos,1,'\\');
        return str;
    }
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
    static rens::regex image ("(?i)^<?(https?://[^;\\s&$`?]*\\.(jpe?g|png|gif|svg))(\\?.*)?>?$");
    static rens::regex content ("(?i)content-length:\\s*(\\d+)");
    int puffyCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
    {
        std::string arg;
        std::string masked;
        for (int i = 1;i < argc;++i)
        {
            if (argv[i] == "nomask")
                masked += "--nomask ";
            else if (argv[i] == "aspect")
                masked += "--aspect ";
            else if (argv[i] == "trans")
            {
                if (++i >= argc)
                {
                    sendMessage(message.channel_id,"Error: `trans <color[,fuzz%]>` expects a color!");
                    return PLUGIN_HANDLED;
                }
                size_t pos = argv[i].find(',');
                masked += "--trans " + escapePound(rens::regex_replace(argv[i].substr(0,pos),puffy::rem,"")) + ' ';
                if (pos != std::string::npos)
                {
                    int p = 0;
                    if (std::isdigit(argv[i].at(pos+1)))
                        p = std::stoi(argv[i].substr(pos+1));
                    masked += "--fuzz " + std::to_string(p) + "% ";
                }
            }
            else if (argv[i] == "fancy")
                masked += "--fancy ";
            else if (argv[i] == "festive")
                masked += "--festive ";
            else if (argv[i] == "noheart")
                masked += "--nohrt ";
            else if (argv[i] == "color")
            {
                if (++i >= argc)
                {
                    sendMessage(message.channel_id,"Error: `color <color>` expects a color!");
                    return PLUGIN_HANDLED;
                }
                masked += "--bg " + escapePound(rens::regex_replace(argv[i],puffy::rem,"")) + ' ';
            }
            else if (argv[i] == "heart")
            {
                if (++i >= argc)
                {
                    sendMessage(message.channel_id,"Error: `heart <color>` expects a color!");
                    return PLUGIN_HANDLED;
                }
                masked += "--heart " + escapePound(rens::regex_replace(argv[i],puffy::rem,"")) + ' ';
            }
            else if (argv[i] == "stripe")
            {
                if (++i >= argc)
                {
                    sendMessage(message.channel_id,"Error: `stripe <color>` expects a color!");
                    return PLUGIN_HANDLED;
                }
                masked += "--stripe " + escapePound(rens::regex_replace(argv[i],puffy::rem,"")) + ' ';
            }
            else if (argv[i] == "alt")
            {
                if (++i >= argc)
                {
                    sendMessage(message.channel_id,"Error: `alt <color>` expects a color!");
                    return PLUGIN_HANDLED;
                }
                masked += "--alt " + escapePound(rens::regex_replace(argv[i],puffy::rem,"")) + ' ';
            }
            else if (argv[i] == "nude")
            {
                if (++i >= argc)
                {
                    sendMessage(message.channel_id,"Error: `nude <color>` expects a color!");
                    return PLUGIN_HANDLED;
                }
                masked += "--nude " + escapePound(rens::regex_replace(argv[i],puffy::rem,"")) + ' ';
            }
            else if (argv[i] == "nonude")
                masked += "--nonude ";
            else if (argv[i] == "outline")
                masked += "--outline ";
            else
            {
                arg = argv[i];
                break;
            }
        }
        rens::smatch ml;
        std::string outfile = message.timestamp + "heartacat.";
        std::string url;
        bool valid = false;
        if (arg != "null")
        {
            if (message.attachments.size() > 0)
            {
                for (auto& a : message.attachments)
                {
                    if (rens::regex_search(a.url,ml,image))
                    {
                        outfile += ml[2].str();
                        url = ml[1].str();
                        if (strlower(ml[2].str()) == "svg")
                            masked += "--svg ";
                        else if ((strlower(ml[2].str()) == "jpg") || (strlower(ml[2].str()) == "jpeg"))
                            masked += "--jpg ";
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
                    outfile += ml[2].str();
                    url = ml[1].str();
                    if (strlower(ml[2].str()) == "svg")
                        masked += "--svg ";
                    else if ((strlower(ml[2].str()) == "jpg") || (strlower(ml[2].str()) == "jpeg"))
                        masked += "--jpg ";
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
                if (rens::regex_search(puffy::exec("curl -I " + url),ml,content))
                {
                    size_t length = std::stoull(ml[1].str());
                    if (length > 6291456)
                    {
                        sendMessage(message.channel_id,"Error! Image is too large!");
                        return PLUGIN_HANDLED;
                    }
                }
                system(("curl -s -o " + outfile + ' ' + url).c_str());
                valid = true;
            }
        }
        else
        {
            outfile = "null";
            valid = true;
        }
        if (valid)
        {
            std::string puff = "heartacat/" + message.timestamp + "heartacat.png";
            //result = puffy::exec("convert puffy/puff_template.png \\( '" + outfile + "' -resize 128x128\\> \\) -composite \\( puffy/puffermask.png -alpha Off -compose CopyOpacity \\) -composite '" + puff + "' 2>&1 1>/dev/null",1024);
            std::string result;
            result = puffy::exec("./heartacat.sh " + masked + '\'' + outfile + "' '" + puff + '\'',1024);
            if (result.size() > 0)
                sendMessage(message.channel_id,"Error!\n```\n" + result + "\n```");
            else
                sendFile(message.channel_id,puff,"uwu!");
            puffy::exec("rm " + outfile);
        }
        else
            //sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " [options] [image url|custom emoji|null]`\nOptions (any combination of the following):\n  `nomask` to create a heartacat without applying the mask.\n  `aspect` (default) setting this will use poor resizing methods.\n  `noheart` do not render the heart.\n  `outline` add a black outline around the image.\n  `fancy` make a fancy cat!\n  `nonude` do not remove skin tone from svg images.\n  `nude <color>` set the skin tone color to remove from svg images.\n  `trans <color[,fuzz%]>` set a color for transparency.\n  `color <color>` set the main color of the cat.\n  `heart <color>` set the color of the hearts.\n  `alt <color>` set the cat's alternate color.\n  `stripe <color>` set the cat's stripe color.\n__You can also upload an image or enter `null` for no image__.");
            sendEmbed(message.channel_id,"{ \"color\": 11474015, \"title\": \"How to Fill a Cat's Heart!\", \"fields\": [{ \"name\": \"`" + recallGlobal()->prefix(message.guild_id) + argv[0] + " [options] [image url|custom emoji|null]`\", \"value\": \"**Options:**\" }, { \"name\": \"`aspect`\", \"value\": \"disable optimized resizing techniques.\", \"inline\": true }, { \"name\": \"`noheart`\", \"value\": \"do not render the heart.\", \"inline\": true }, { \"name\": \"`nomask`\", \"value\": \"do not apply the heart mask.\", \"inline\": true }, { \"name\": \"`outline`\", \"value\": \"add a black outline around the image.\", \"inline\": true }, { \"name\": \"`fancy`\", \"value\": \"make a fancy cat!\", \"inline\": true }, { \"name\": \"`festive`\", \"value\": \"make a festive cat!\", \"inline\": true }, { \"name\": \"`nonude`\", \"value\": \"do not remove skin tone from svg images.\", \"inline\": true }, { \"name\": \"`nude <color>`\", \"value\": \"set the skin tone color to remove from svg images.\", \"inline\": true }, { \"name\": \"`color <color>`\", \"value\": \"set the main color of the cat.\", \"inline\": true }, { \"name\": \"`alt <color>`\", \"value\": \"set the cat's alternate color.\", \"inline\": true }, { \"name\": \"`heart <color>`\", \"value\": \"set the color of the hearts.\", \"inline\": true }, { \"name\": \"`stripe <color>`\", \"value\": \"set the cat's stripe color.\", \"inline\": true }, { \"name\": \"`trans <color[,fuzz%]>`\", \"value\": \"set a color for transparency, fuzz allows a threshold of similar colors.\", \"inline\": true }, { \"name\": \"By default, your profile picture will be used as the image.\", \"value\": \"__You can also upload an image or enter `null` for no image__\" }]}");
        return PLUGIN_HANDLED;
    }
}

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("heartacat_version",VERSION,"Heartacat Version",GVAR_CONSTANT);
    handle.regChatCmd("heartacat",&puffy::puffyCmd,NOFLAG,"Fill a cat's heart.");
    puffy::exec("rm heartacat/* 2>/dev/null");
    return 0;
}


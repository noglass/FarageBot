#include "api/farage.h"
#include <iostream>
using namespace Farage;

#define REGSUBEX
#include "common_func.h"

#define VERSION "v0.0.6"

extern "C" Info Module
{
    "You Just Came!",
    "Madison",
    "Upload images to justca.me",
    VERSION,
    "http://farage.justca.me/",
    FARAGE_API_VERSION
};

std::string dec2hex(unsigned char n)
{
    std::string res;
    do
    {
        res += "0123456789ABCDEF"[n & 15];
        n >>= 4;
    } while (n);
    return std::string(res.rbegin(),res.rend());
}

std::string encode(const std::string& replace)
{
    return dec2hex(replace.c_str()[0]);
}

int justcameCmd(Handle&,int,const std::string[],const Message&);

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("justcame_version",VERSION,"Justca.me Version",GVAR_CONSTANT);
    handle.regChatCmd("justcame",&justcameCmd,NOFLAG,"Uplaod an image to justca.me.");
    return 0;
}

int justcameCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    static rens::regex imageptrn    ("(?i)\\.(jpe?g|png|gif)$");
    static rens::regex urlptrn      ("(?i)^(https?://[a-z0-9.-]+[^\\s]*?\\.(jpe?g|png|gif))$");
    static rens::regex nonspace     ("[^\\s\\n]");
    static rens::regex success      ("<li><a href=\"(https://i\\.justca\\.me/(\\?perma=[a-zA-Z0-9]{5}\\.[a-z]{3,4}))\">i\\.justca\\.me</a>");
    static rens::regex error        ("<p>([^<]*)</p>");
    Global* global = recallGlobal();
    std::string url;
    if (message.attachments.size() > 0)
    {
        for (auto it = message.attachments.begin(), ite = message.attachments.end();it != ite;++it)
        {
            if (rens::regex_search(it->url,imageptrn))
            {
                url = it->url;
                break;
            }
        }
        if (url.size() == 0)
        {
            sendMessage(message.channel_id,"Error: Filetype not supported!");
            return PLUGIN_HANDLED;
        }
    }
    int arg = 1;
    if (url.size() == 0)
    {
        if (argc < 2)
        {
            sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " [image_url] [custom.title.justca.me/custom/path]` You may optionally include the image as an attachment.");
            return PLUGIN_HANDLED;
        }
        if (!rens::regex_search(argv[1],urlptrn))
        {
            sendMessage(message.channel_id,"Error: You may only provide a valid url to a jpg, png, or gif! You may optionally include the image as an attachment.");
            return PLUGIN_HANDLED;
        }
        url = argv[1];
        arg = 2;
    }
    url = "curl -s -b justcame/cookies -d \"url=" + regsubex(url,"[^a-zA-Z0-9-_.~]","$0",&encode,"%") + "\" -L -X POST https://justca.me/upload.php";
    sendTyping(message.channel_id);
    FILE* outstream;
    outstream = popen("curl -s -c justcame/cookies -d \"email=insert_email&password=insert_password&submit=1\" -L -X POST https://justca.me/login.php","r");
    const int size = 1024;
    char buffer[size];
    if (outstream)
    {
        while (!feof(outstream))
            fgets(buffer,size,outstream);
        pclose(outstream);
    }
    outstream = popen(url.c_str(),"r");
    if (outstream)
    {
        std::string out, output;
        size_t pos;
        bool headend = false;
        while (!feof(outstream))
        {
            if (fgets(buffer,size,outstream) != NULL)
            {
                if (!headend)
                {
                    out.append(buffer);
                    if ((pos = out.find("<div id=\"main\">")) != std::string::npos)
                    {
                        headend = true;
                        out.erase(0,pos);
                    }
                }
                else
                    out = buffer;
                if ((headend) && (out.size() > 0) && (rens::regex_search(out,nonspace)))
                    output.append(out);
            }
        }
        pclose(outstream);
        rens::smatch ml;
        if (rens::regex_search(output,ml,success))
        {
            if (argc > arg)
            {
                out = argv[arg];
                if (out.back() != '/')
                    out += '/';
                out += ml[2].str();
                if (out.find("http") != 0)
                {
                    std::string s = out.substr(0,out.find('/'));
                    int i = 0;
                    for (auto& c : s)
                        if (c == '.')
                            ++i;
                    if (i < 3)
                    {
                        if (i < 2)
                            out = "https://i." + out;
                        else
                            out = "https://" + out;
                    }
                    else
                        out = "http://" + out;
                }
            }
            else
                out = ml[1].str();
            sendMessage(message.channel_id,"Here is your fresh meme!\n" + out);
        }
        else if (rens::regex_search(output,ml,error))
            sendMessage(message.channel_id,"Error: " + ml[1].str());
        else
            sendMessage(message.channel_id,"Error: Unknown response!");
    }
    else
        sendMessage(message.channel_id,"Error: An unknown connection error has occurred!");
    return PLUGIN_HANDLED;
}

/*
curl -b justcame/cookies -c justcame/cookies --data-urlencode "url=https://cdn.discordapp.com/attachments/566212992820051972/62308797480/unknown.png" -L -X POST https://justca.me/upload.php

curl -b justcame/cookies -c justcame/cookies --data-urlencode "url=https%3A%2F%2Fcdn.discordapp.com%2Fattachments%2F566212992820051972%2F697159632308797480%2Funknown.png" -L -X POST https://justca.me/upload.php


<div id="main">

<li><a href="https://i.justca.me/?perma=NeWYq.png">i.justca.me</a> link to this image</li>


<div class="box">

	<p class="title">Message</p>

	<p>Sorry, this extension is not allowed.</p>
*/



















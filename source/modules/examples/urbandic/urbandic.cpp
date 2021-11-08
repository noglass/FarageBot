#include "api/farage.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <regex>
#include <fstream>
using namespace Farage;

#define REGSUBEX_STD
#include "common_func.h"

#define VERSION "v0.9.1"

#define RETRIES 10

extern "C" Info Module
{
    "Urban Dictionary",
    "Madison",
    "Lookup Definitions on Urban Dictionary",
    VERSION,
    "http://farage.justca.me/",
    FARAGE_API_VERSION
};

int urbanCmd(Handle&,int,const std::string[],const Message&);

namespace urbandic
{
    std::string year;
}

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("urbandic_version",VERSION,"Urban Dictionary Version",GVAR_CONSTANT);
    handle.regChatCmd("whats",&urbanCmd,NOFLAG,"Lookup a definition on urbandictionary.com.");
    handle.regChatCmd("what's",&urbanCmd,NOFLAG,"Lookup a definition on urbandictionary.com.");
    time_t t;
    time(&t);
    char year[5];
    strftime(year,5,"%Y",localtime(&t));
    urbandic::year = year;
    return 0;
}

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

std::string resolve_symbol(const std::string& symbol)
{
    static std::unordered_map<std::string,unsigned char> symbols
    ({
        {"apos",'\''},{"quot",'"'},{"amp",'&'},{"lt",'<'},{"gt",'>'},{"nbsp",' '},{"iexcl",161},
        {"cent",162},{"pound",163},{"curren",164},{"yen",165},{"brvbar",166},{"sect",167},
        {"uml",168},{"copy",169},{"ordf",170},{"laquo",171},{"not",172},{"shy",173},{"reg",174},
        {"macr",175},{"deg",176},{"plusmn",177},{"sup2",178},{"sup3",179},{"acute",180},
        {"micro",181},{"para",182},{"middot",183},{"cedil",184},{"sup1",185},{"ordm",186},
        {"raquo",187},{"frac14",188},{"frac12",189},{"frac34",190},{"iquest",191},{"Agrave",192},
        {"Aacute",193},{"Acirc",194},{"Atilde",195},{"Auml",196},{"Aring",197},{"AElig",198},
        {"Ccedil",199},{"Egrave",200},{"Eacute",201},{"Ecirc",202},{"Euml",203},{"Igrave",204},
        {"Iacute",205},{"Icirc",206},{"Iuml",207},{"ETH",208},{"Ntilde",209},{"Ograve",210},
        {"Oacute",211},{"Ocirc",212},{"Otilde",213},{"Ouml",214},{"times",215},{"Oslash",216},
        {"Ugrave",217},{"Uacute",218},{"Ucirc",219},{"Uuml",220},{"Yacute",221},{"THORN",222},
        {"szlig",223},{"agrave",224},{"aacute",225},{"acirc",226},{"atilde",227},{"auml",228},
        {"aring",229},{"aelig",230},{"ccedil",231},{"egrave",232},{"eacute",233},{"ecirc",234},
        {"euml",235},{"igrave",236},{"iacute",237},{"icirc",238},{"iuml",239},{"eth",240},
        {"ntilde",241},{"ograve",242},{"oacute",243},{"ocirc",244},{"otilde",245},{"ouml",246},
        {"divide",247},{"oslash",248},{"ugrave",249},{"uacute",250},{"ucirc",251},{"uuml",252},
        {"yacute",253},{"thorn",254},{"yuml",255},
    });
    std::string c;
    if (symbol.at(0) == '#')
    {
        if ((symbol.size() > 1) && (std::isdigit(symbol.at(1))))
            c = std::string(1,char(std::stoi(symbol.substr(1))));
    }
    else
    {
        auto sym = symbols.find(symbol);
        if (sym != symbols.end())
            c = std::string(1,sym->second);
    }
    if (c == "\"")
        c = "\\\"";
    return std::move(c);
}

namespace urban
{
    static std::regex nonptrn ("<div class=\"shrug space\">[^<]*</div><div class=\"term space\">([^<]*)</div>");
    static std::regex tryptrn ("<li><a href=\"([^\"]*)\">([^<]*)</a></li>");
    static std::regex nameptrn ("<div class=\"def-header\"><a class=\"word\" href=\"(/define\\.php\\?term=[^\"]*)\" name=\"[^\"]*\">([^<]+)</a>"); // good
    //static std::regex catptrn ("<a href=\"(/category\\.php\\?category=[^\"]*)\"><span style=\"[^\"]*\">([^<]*)</span></a></span>");
    static std::regex defptrn ("<div class=\"meaning\">(.+?)</div>"); // good
    static std::regex examptrn ("<div class=\"example\">(.+?)</div>"); // good
    static std::regex tagptrn ("<div class=\"tags\">((<a href=\"/tags\\.php\\?tag=[^\"]*\">[^<]*</a>)*)</div>");
    static std::regex tagsptrn ("<a href=\"(/tags\\.php\\?tag=[^\"]*)\">([^<]*)</a>");
    static std::regex authptrn ("<div class=\"contributor\">by <a href=\"(/author\\.php\\?author=[^\"]*)\">([^<]*)</a>([^<]+)</div>"); // good
    static std::regex footptrn ("<div class=\"def-footer\"><div class=\"[^\"]*\"><div class=\"[^\"]*\"><div class=\"[^\"]*\"><div class=\"[^\"]*\"><a class=\"up\"><i class=\"[^\"]*\"><svg xmlns=\"[^\"]*\" viewBox=\"[^\"]*\"><path d=\"[^\"]*\"/></svg></i><span class=\"count\">(\\d+)</span></a><a class=\"down\"><i class=\"[^\"]*\"><svg xmlns=\"[^\"]*\" viewBox=\"[^\"]*\"><path d=\"[^\"]*\"/></svg></i><span class=\"count\">(\\d+)</span>"); // this is not good!!
    static std::regex brptrn ("<[bB][rR]/?>"); // good
    static std::regex hrefptrn ("<a class=\"autolink\" href=\"([^\"]*)\" on[cC]lick=\"[^\"]*\">([^<]*)</a>"); // good
    static rens::regex imgptrn ("(?i)\"([^\\s]*?\\.(jpe?g|png|gif|webp))\"");
    static std::regex urlptrn ("<a href=\"(/define\\.php\\?term=.+?)&amp;page=(\\d+)\">\\2</a>"); // good
    static std::regex symptrn ("&(#?\\w+);"); // good
    static std::regex aposptrn ("'(.*)'"); // good
    static std::regex bsptrn ("(\\\\)"); // good
    //static std::regex ribbonptrn ("<div class=\"ribbon\">(\\d+|Top definition)</div>"); // not good!
    struct object
    {
        std::string name;
        std::string link;
        std::string build()
        {
            return "[" + regsubex(name,symptrn,"$1",&resolve_symbol) + "](" + link + ')';
        }
    };
    struct entry : public object
    {
        entry(object&& ob) : object(ob) {}
        object category;
        std::string definition;
        std::string example;
        std::vector<object> tags;
        object author;
        std::string date;
        std::string footer;
        std::string image;
        void setImage(const std::string& def)
        {
            rens::smatch pml;
            if (rens::regex_search(def,pml,imgptrn))
            {
                std::string im = pml[1].str();
                if (im.find("cloudfront.net/assets/mug-ad") != std::string::npos)
                    return;
                if (image.size() == 0)
                    image = im;
                else if (example.size() > 0)
                    example += "\\n[image](" + im + ')';
                else if (definition.size() > 0)
                    definition += "\\n[image](" + im + ')';
            }
        }
        void setDefinition(const std::string& def)
        {
            definition = def;
            size_t pos = def.find("</div>");
            if (pos != std::string::npos)
            {
                definition.erase(pos);
                setImage(def.substr(pos));
            }
        }
        void setExample(const std::string& def)
        {
            example = def;
            size_t pos = def.find("</div>");
            if (pos != std::string::npos)
            {
                example.erase(pos);
                setImage(def.substr(pos));
            }
        }
        std::string build(const std::string& criteria, int result = 1)
        {
            std::string out = "{ \"color\": 13809920, \"author\": { \"name\": \"Urban Dictionary: " + criteria + "\", \"icon_url\": \"https://cdn.discordapp.com/attachments/577096847030485001/600249354263199767/udicon.png\" }, \"thumbnail\": { \"url\": \"https://cdn.discordapp.com/attachments/577096847030485001/669769835344953393/udlogo.png\" }, \"title\": \"" + std::to_string(result) + ' ' + regsubex(this->name,symptrn,"$1",&resolve_symbol) + "\", \"url\": \"" + this->link + "\", \"description\": \"";
            //https://urbandictionary.github.io/error.urbandictionary.com/logo.png
            std::string str = regsubex(definition,symptrn,"$1",&resolve_symbol);
            if (str.size() > 2045)
            {
                int i = 2040;
                if (str.at(i) < 1)
                    for (;(i > -1) && (str.at(i) < 1);--i);
                if ((str.find("]",i) < str.find("[",i)) || (str.find(")",i) < str.find("(")))
                    for (int j = 0, k = i;(j = str.find("[",j)) < k;)
                        i = j-1;
                str.erase(i+1);
                str += "...";
            }
            out += str + "\", \"fields\": [";
            str = regsubex(example,symptrn,"$1",&resolve_symbol);
            if (str.size() > 1020)
            {
                int i = 1015;
                if (str.at(i) < 1)
                    for (;(i > -1) && (str.at(i) < 1);--i);
                if ((str.find("]",i) < str.find("[",i)) || (str.find(")",i) < str.find("(")))
                    for (int j = 0, k = i;(j = str.find("[",j)) < k;)
                        i = j-1;
                str.erase(i+1);
                str += "...";
            }
            if (str.size() > 0)
                out += "{ \"name\": \"Examples:\", \"value\": \"" + str + "\" }, ";
            if (tags.size() > 0)
            {
                out += "{ \"name\": \"Tags:\", \"value\": \"";
                std::string tagstr;
                for (auto it = tags.begin(), ite = tags.end();it != ite;++it)
                {
                    std::string tag = it->build();
                    if (tagstr.size() + tag.size() > 1020)
                    {
                        out += tagstr + "\" }, { \"name\": \"** **\", \"value\": \"";
                        tagstr.clear();
                    }
                    tagstr += tag + ' ';
                }
                if (tagstr.size() > 0)
                    out += tagstr;
                out += "\" }, ";
            }
            if (footer.size() < 1)
                footer = "** **";
            out += "{ \"name\": \"" + footer + "\", \"value\": \"Submitted by " +  author.build() + ' ' + regsubex(date,symptrn,"$1",&resolve_symbol) + "\" }], ";
            if (image.size() > 0)
                out += "\"image\": { \"url\": \"" + image + "\" }, ";
            out += "\"footer\": { \"icon_url\": \"https://cdn.discordapp.com/attachments/577096847030485001/600249354263199767/udicon.png\", \"text\": \"© 1999-" + urbandic::year + " Urban Dictionary ®\" } }";
            return std::move(out);
        }
    };
};

int urbanCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global* global = recallGlobal();
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <lookup> [result=1]`");
    else
    {
        size_t pos;
        std::string criteria;
        std::string pageNum;
        int page = 1;
        int result = 1;
        bool rando = false;
        std::string url = "https://www.urbandictionary.com/";
        if (argc == 3)
        {
            if (std::isdigit(argv[2].at(0)))
            {
                result = std::stoi(argv[2]);
                if (result < 1)
                    result = 1;
                page = result / 8 + 1;
                pageNum = "&page=" + std::to_string(page);
                criteria = argv[1];
            }
            else
                criteria = argv[1] + ' ' + argv[2];
        }
        else
        {
            if (argc > 2)
            {
                pos = message.content.find(argv[0]) + argv[0].size() + 1;
                criteria = message.content.substr(pos);
            }
            else
                criteria = argv[1];
        }
        if ((criteria == "this") && (argc == 2))
        {
            page = mtrand(1,2000);
            criteria = "Random Page " + std::to_string(page);
            url = url + "random.php?page=" + std::to_string(page);
            page = 1;
            result = mtrand(1,7);
            rando = true;
        }
        else
        {
            for (;(pos = criteria.find("\"")) != std::string::npos;criteria.erase(pos,1));
            url = url + "define.php?term=" + regsubex(criteria,std::regex("."),"$0",&encode,"%");
        }
        std::string cmd = "curl -L '" + url + "' 2>/dev/null";
        std::string out, output;
        const int size = 1024;
        char buffer[size];
        //sendTyping(message.channel_id);
        static std::regex nonspace ("[^\\s\\n]");
        FILE* outstream;
        for (int tries = RETRIES;(tries) && (output.size() == 0);--tries)
        {
            out.clear();
            outstream = popen(cmd.c_str(),"r");
            if (outstream)
            {
                bool headend = true;//false;
                while (!feof(outstream))
                {
                    if (fgets(buffer,size,outstream) != NULL)
                    {
                        if (!headend)
                        {
                            out.append(buffer);
                            if ((pos = out.find("document.head.insertBefore(btScript, document.head.firstElementChild);")) != std::string::npos)
                            {
                                headend = true;
                                out.erase(0,pos);
                            }
                        }
                        else
                            out = buffer;
                        if ((headend) && (out.size() > 0) && (regex_search(out,nonspace)))
                            output.append(out);
                    }
                }
                pclose(outstream);
            }
        }
        //std::cout<<"urbandic: "<<output<<std::endl;
        while ((pos = output.find_first_of("\r\n")) != std::string::npos)
            output.erase(pos,1);
        std::smatch ml;
        if (std::regex_search(output,ml,urban::nonptrn))
        {
            out = "{ \"color\": 13809920, \"author\": { \"name\": \"Urban Dictionary: " + std::regex_replace(criteria,urban::bsptrn,"\\\\") + "\", \"icon_url\": \"https://cdn.discordapp.com/attachments/577096847030485001/600249354263199767/udicon.png\" }, \"thumbnail\": { \"url\": \"https://cdn.discordapp.com/attachments/577096847030485001/669769835344953393/udlogo.png\" }, \"title\": \"¯\\\\_(ツ)_/¯ " + regsubex(std::regex_replace(ml[1].str(),urban::bsptrn,"\\\\"),urban::symptrn,"$1",&resolve_symbol) + "\", \"description\": \"";
            std::string desc = "How about one of these instead?\\n\\n";
            if ((pos = output.find("<div class=\"try-these\">Or try one of these:<ul>")) != std::string::npos)
            {
                output = std::regex_replace(output.substr(pos,out.find("</div>",pos)),urban::bsptrn,"\\\\");
                bool first = true;
                while (std::regex_search(output,ml,urban::tryptrn))
                {
                    std::string t = ", [";
                    if (first)
                    {
                        t = "[";
                        first = false;
                    }
                    t = t + regsubex(ml[2].str(),urban::symptrn,"$1",&resolve_symbol) + "](https://www.urbandictionary.com" + ml[1].str() + ")";
                    if (desc.size() + t.size() <= 2048)
                        desc += t;
                    else
                        break;
                    output = ml.suffix();
                }
                out += desc;
            }
            else
                out += "Error!";
            out += "\", \"footer\": { \"icon_url\": \"https://cdn.discordapp.com/attachments/577096847030485001/600249354263199767/udicon.png\", \"text\": \"© 1999-" + urbandic::year + " Urban Dictionary ®\" } }";
            std::cout<<out<<std::endl;
            sendEmbed(message.channel_id,out);
            return PLUGIN_HANDLED;
        }
        size_t pages = 1;
        if (!rando)
            pages = output.find("<div class=\"pagination-centered\">");
        if (page > 1)
        {
            if (pages != std::string::npos)
            {
                std::string footer = output.substr(pages);
                if (std::regex_search(footer,ml,urban::urlptrn))
                    url = "https://www.urbandictionary.com" + ml[1].str();
                if ((pages = output.find("Last »",pages)) != std::string::npos)
                {
                    for (pages -= 3;std::isdigit(output.at(pages-1));--pages);
                    pages = std::stoi(output.substr(pages));
                }
                else
                    pages = 1;
            }
            else
                pages = 1;
            if (page > pages)
            {
                page = pages;
                pageNum = "&page=" + std::to_string(page);
                result = 0;
            }
            if (page > 1)
            {
                cmd = "curl '" + url + pageNum + "' 2>/dev/null";
                output.clear();
                for (int tries = RETRIES;(tries) && (output.size() == 0);--tries)
                {
                    out.clear();
                    outstream = popen(cmd.c_str(),"r");
                    if (outstream)
                    {
                        bool headend = true;//false;
                        while (!feof(outstream))
                        {
                            if (fgets(buffer,size,outstream) != NULL)
                            {
                                if (!headend)
                                {
                                    out.append(buffer);
                                    if ((pos = out.find("document.head.insertBefore(btScript, document.head.firstElementChild);")) != std::string::npos)
                                    {
                                        headend = true;
                                        out.erase(0,pos);
                                    }
                                }
                                else
                                    out = buffer;
                                if ((headend) && (out.size() > 0) && (regex_search(out,nonspace)))
                                    output.append(out);
                            }
                        }
                        pclose(outstream);
                    }
                }
                while ((pos = output.find_first_of("\r\n")) != std::string::npos)
                    output.erase(pos,1);
                if ((pos = output.find("<div class=\"pagination-centered\">")) != std::string::npos)
                {
                    int p;
                    if ((p = std::stoi(output.substr(output.find("<li class=\"current\"><a href=\"#\">",pos) + 32,5))) != page)
                    {
                        std::cout<<"Error! "<<p<<" != "<<page<<std::endl;
                        reaction(message,"%E2%9D%8C");
                        return PLUGIN_HANDLED;
                    }
                }
            }
        }
        std::vector<urban::entry> definitions;
        //"<div class=\"def-header\"><a class=\"word\" href=\"(/define\\.php\\?term=[^\"]*)\" name=\"[^\"]*\">([^<]+)</a>"
        while (std::regex_search(output,ml,urban::nameptrn))
        {
            //output = ml.suffix();
            //std::regex_search(output,ml,urban::nameptrn);
            urban::entry def ({std::regex_replace(ml[2].str(),urban::bsptrn,"\\\\"),"https://www.urbandictionary.com" + std::regex_replace(ml[1].str(),urban::bsptrn,"\\\\")});
            output = ml.suffix();
            //"<a href=\"(/category\\.php\\?category=[^\"]*)\"><span style=\"[^\"]*\">([^<]*)</span></a></span>"
            /*if (std::regex_search(output,ml,urban::catptrn))
            {
                def.category = {std::regex_replace(ml[2].str(),urban::bsptrn,"\\\\"),"https://www.urbandictionary.com" + std::regex_replace(ml[1].str(),urban::bsptrn,"\\\\")};
                output = ml.suffix();*/
                //"<div class=\"meaning\">(.+?)</div>"
                if (std::regex_search(output,ml,urban::defptrn))
                {
                    def.setDefinition(std::regex_replace(std::regex_replace(std::regex_replace(ml[1].str(),urban::bsptrn,"\\\\"),urban::hrefptrn,"[$2](https://www.urbandictionary.com$1)"),urban::brptrn,"\\n"));
                    out = ml.suffix();
                    if (std::regex_search(out,ml,urban::footptrn))
                    {
                        def.footer = "👍 " + std::regex_replace(ml[1].str(),urban::bsptrn,"\\\\") + " 👎 " + std::regex_replace(ml[2].str(),urban::bsptrn,"\\\\");
                        out = std::regex_replace(out.erase(ml.position()),urban::bsptrn,"\\\\");
                        output = ml.suffix();
                    }
                    if (std::regex_search(out,ml,urban::examptrn))
                    {
                        pos = ml.position();
                        def.setExample(std::regex_replace(std::regex_replace(ml[1].str(),urban::hrefptrn,"[$2](https://www.urbandictionary.com$1)"),urban::brptrn,"\\n"));
                        if (pos > 0)
                            def.setImage(out.substr(0,pos));
                        out = ml.suffix();
                    }
                    if (std::regex_search(out,ml,urban::tagptrn))
                    {
                        pos = ml.position();
                        std::smatch tml;
                        std::string tags = ml[1].str();
                        if (pos > 0)
                            def.setImage(out.substr(0,pos));
                        while (std::regex_search(tags,tml,urban::tagsptrn))
                        {
                            def.tags.push_back({tml[2].str(),"https://www.urbandictionary.com" + tml[1].str()});
                            tags = tml.suffix();
                        }
                        //out = ml.suffix();
                    }
                    if (std::regex_search(out,ml,urban::authptrn))
                    {
                        def.author = {ml[2].str(),"https://www.urbandictionary.com" + ml[1].str()};
                        def.date = ml[3].str();
                    }
                    definitions.push_back(std::move(def));
                }
            //}
        }
        if (definitions.size() > 0)
        {
            if (result == 0)
                result = (page-1) * 7 + definitions.size();
            int r = result % 7;
            if (r == 0)
                r = 6;
            else
                --r;
            if (r >= definitions.size())
            {
                result = (page-1) * 7 + definitions.size();
                r = definitions.size()-1;
            }
            auto def = definitions.begin() + r;
            std::string embed = def->build(std::regex_replace(criteria,urban::bsptrn,"\\\\"),((rando) ? 0 : result));
            std::cout<<"[Urbandic] ("<<url<<") Pages: "<<pages<<": "<<embed<<std::endl;
            sendEmbed(message.channel_id,embed);
        }
        else
        {
            std::regex_search(cmd,ml,urban::aposptrn);
            std::string link;
            if (ml.size() > 0)
                link = ml[1].str();
            if (link.size() > 0)
                sendMessage(message.channel_id,"An error occurred fetching the definition :sob:\n*Don't be mad at me, you can have the link!*\n" + link);
            else
                reaction(message,"%E2%9D%97");
            if (output.size() > 0)
            {
                reaction(message,"%E2%9D%93");
                std::ofstream file ("urbandic_" + criteria + std::to_string(result) + ".html",std::ios::trunc);
                file<<'['<<url<<':'<<pages<<']'<<std::endl;
                file<<output;
                file.close();
            }
        }
    }
    return PLUGIN_HANDLED;
}


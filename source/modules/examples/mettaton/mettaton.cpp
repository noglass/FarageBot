#include "api/farage.h"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <locale>
#include <codecvt>
#include <chrono>
#include "shared/libini.h"
using namespace Farage;

#define MAKEMENTION
#define HEXIFY
#include "common_func.h"

#define BASEVERSION "v1.7.0"
#ifdef METTA_MINI
    #define VERSION std::string(BASEVERSION) + "-minimal"
#else
    #define VERSION BASEVERSION
#endif

#define SOUNDEXTHRESHOLD    75
#define SOUNDEXTHRESHOLD2   25

extern "C" Info Module
{
    "Mettaton Base Module",
    "Madison",
    "Base commands for Mettaton",
    VERSION,
    "http://farage.justca.me/",
    FARAGE_API_VERSION
};

namespace Mettaton
{
    #ifndef METTA_MINI
    struct koolAidMan
    {
        std::string id;
        int count;
    };
    #endif
    struct faqBlocker
    {
        bool isName;
        rens::regex ptrn;
    };
    struct soundex
    {
        char code = -1;
        int coder;
        int coded;
        std::string full;
    };
    soundex phonetize(std::string word)
    {
                                    //a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z
        static const int values[] = {'0','1','2','3','0','1','2','0','0','2','2','4','5','5','0','1','2','6','2','3','0','1','0','2','0','2'};
        soundex code;
        word = strlower(word);
        auto it = word.begin(), ite = word.end();
        for (;(ite != it) && (!std::isalpha(*it));++it);
        if (it == ite)
        {
            code.code = 0;
            return code;
        }
        code.code = *it;
        code.coder = values[*it-'a'];
        code.full = code.code;
        char previous = ' ';
        for (++it;(it != ite) && (code.full.size() < 4);++it)
        {
            if (!std::isalpha(*it))
                continue;
            char c = values[*it-'a'];
            if ((c != '0') && (c != previous))
            {
                code.full += c;
                previous = c;
            }
        }
        if (code.full.size() < 4)
            code.full.append(4-code.full.size(),'0');
        code.coded = std::stoi(code.full.substr(1));
        return std::move(code);
    }
    struct faqEntry
    {
        faqEntry() {}
        faqEntry(std::string i, std::string v, int category = 0) : item(std::move(i)), value(std::move(v)), categorized(category)
        {
            phonetic = phonetize(item);
        }
        faqEntry& operator=(std::string v)
        {
            value = std::move(v);
            return *this;
        }
        std::string item;
        std::string value;
        soundex phonetic;
        int categorized;
    };
    struct faqCategory
    {
        faqCategory(std::string catName = "", std::string catDesc = "") :
            name(std::move(catName)), desc(std::move(catDesc)) {}
        std::string name;
        std::string desc;
        std::vector<faqEntry*> entries;
        std::string list()
        {
            auto it = entries.begin(), ite = entries.end();
            std::string out = "None", ox;
            switch (entries.size())
            {
                case 0:
                case 1:
                case 2:
                    ox = " and ";
                    break;
                default:
                    ox = ", and ";
            }
            if (it != ite)
            {
                out = "`" + (*it)->item + '`';
                size_t current = out.size();
                std::string temp;
                for (++it;it != ite;++it)
                {
                    if (it != ite-1)
                        temp = ", ";
                    else
                        temp = ox;
                    if (current + temp.size() > 42)
                    {
                        out += temp + '\n';
                        temp.clear();
                        current = 0;
                    }
                    out.append(temp.append(1,'`').append((*it)->item + '`'));
                    current += temp.size();
                }
            }
            return out;
        }
    };
    struct faqDex
    {
        faqDex() {}
        faqDex(INIObject ini) : database(std::move(ini))
        {
            load();
        }
        void load()
        {
            db.clear();
            auto faq = database.topic_it("faq");
            for (auto it = faq->begin(), ite = faq->end();it != ite;++it)
                db.emplace_back(it->item,it->value);
            auto cats = database.topic_it("categories");
            if (cats != database.end())
            {
                for (auto it = cats->begin(), ite = cats->end();it != ite;++it)
                {
                    faqCategory c (it->item,it->value);
                    auto cat = database.topic_it(it->item);
                    if (cat != database.end())
                    {
                        for (auto itt = cat->begin(), itte = cat->end();itt != itte;++itt)
                        {
                            auto i = find(itt->item);
                            if (i != db.end())
                            {
                                c.entries.push_back(&*i);
                                i->categorized++;
                            }
                        }
                    }
                    category.push_back(std::move(c));
                }
            }
        }
        inline int save() { return database.write("database.ini"); }
        int open(const std::string& file)
        {
            int err = database.open(file);
            if (!err)
                load();
            return err;
        }
        int set(const std::string& item, const std::string& value)
        {
            add(item,value);
            return database.write("database.ini");
        }
        int erase(const std::string& item)
        {
            if (!remove(item))
                return database.write("database.ini");
            return 2;
        }
        inline int abs(int n)
        {
            if (n < 0)
                return n*-1;
            return n;
        }
        std::string lookup(const std::string& item, std::string& similar)
        {
            auto it = find(item);
            if (it != db.end())
                return it->value;
            std::string categ = item;
            if (categ.find("📁") == 0)
                categ = categ.substr(std::string("📁").size());
            for (auto& cat : category)
            {
                if (cat.name == categ)
                {
                    similar = std::string(1,std::toupper(cat.name.front())) + cat.name.substr(1) + " FAQs";
                    return cat.list();
                }
            }
            similar.clear();
            soundex code = phonetize(item);
            std::vector<std::pair<int,std::vector<faqEntry>::iterator>> options;
            for (auto it = db.begin(), ite = db.end();it != ite;++it)
            {
                if (it->phonetic.full == code.full)
                    options.emplace_back(abs(item.size() - it->item.size()),it);
                else
                {
                    int difference = abs(code.coded-it->phonetic.coded);
                    if ((it->phonetic.code == code.code) && (difference < SOUNDEXTHRESHOLD))
                        options.emplace_back(difference,it);
                    else if ((it->phonetic.coder == code.coder) && (difference < SOUNDEXTHRESHOLD2))
                        options.emplace_back(difference,it);
                }
            }
            if (options.size() > 1)
            {
                std::vector<std::pair<int,std::vector<faqEntry>::iterator>> sorted;
                for (auto it = options.begin(), ite = options.end();it != ite;++it)
                {
                    bool emplaced = false;
                    for (auto sit = sorted.begin(), site = sorted.end();sit != site;++sit)
                    {
                        if (it->first < sit->first)
                        {
                            sorted.insert(sit,*it);
                            emplaced = true;
                            break;
                        }
                    }
                    if (!emplaced)
                        sorted.push_back(*it);
                }
                options = sorted;
            }
            int count = 5;
            bool oxford = (options.size() > 2);
            for (auto it = options.begin(), ite = options.end();((it != ite) && (count));++it)
            {
                if (--count < 4)
                {
                    if ((it == ite-1) || (count == 0))
                        similar += (oxford ? "**,** or " : " or ");
                    else
                        similar += "**,** ";
                }
                similar = similar + '`' + it->second->item + '`';
            }
            return "";
        }
        int addCategory(const std::string& name, const std::string& desc = "")
        {
            int r = 0;
            if (database.exists("categories",name))
            {
                if (desc.size() < 1)
                    return 1;
                r = -1;
                for (auto& c : category)
                {
                    if (c.name == name)
                    {
                        c.desc = desc;
                        break;
                    }
                }
            }
            else
                category.emplace_back(name,desc);
            database("categories",name) = desc;
            return r;//database.write("database.ini");
        }
        int delCategory(const std::string& name)
        {
            auto it = database.topic_it(name);
            if (it != database.end())
                database.erase(it);
            database.erase("categories",name);
            for (auto cat = category.begin(), cate = category.end();cat != cate;++cat)
            {
                if (cat->name == name)
                {
                    for (auto c = cat->entries.begin(), ce = cat->entries.end();c != ce;++c)
                        (*c)->categorized--;
                    category.erase(cat);
                    return 0;
                }
            }
            return 1;//database.write("database.ini");
        }
        int setCategory(const std::string& name, const std::string& item)
        {
            auto entry = find(item);
            if (entry == db.end())
                return -2;
            auto it = database.topic_it(name);
            if (it == database.end())
            {
                if (database.exists("categories",name))
                    database(name,item) = "1";
                else
                    return 1;
            }
            else
            {
                if (it->exists(item))
                    return -1;
                (*it)(item) = "1";
            }
            for (auto cat = category.begin(), cate = category.end();cat != cate;++cat)
            {
                if (cat->name == name)
                {
                    entry->categorized++;
                    cat->entries.push_back(&*entry);
                    break;
                }
            }
            return 0;//database.write("database.ini");
        }
        int remCategory(const std::string& item, const std::string& name = "")
        {
            if (name.size() < 1)
            {
                auto entry = find(item);
                if (entry == db.end())
                    return 2;
                entry->categorized = 0;
                for (auto cat = category.begin(), cate = category.end();cat != cate;++cat)
                {
                    auto dcat = database.topic_it(cat->name);
                    if (dcat != database.end())
                        dcat->erase(item);
                    for (auto c = cat->entries.begin(), ce = cat->entries.end();c != ce;++c)
                    {
                        if ((*c)->item == item)
                        {
                            cat->entries.erase(c);
                            break;
                        }
                    }
                }
                return 0;
            }
            else
            {
                auto it = database.topic_it(name);
                if (it != database.end())
                {
                    if (!it->erase(item))
                    {
                        bool categ = false;
                        for (auto cat = category.begin(), cate = category.end();cat != cate;++cat)
                        {
                            if (cat->name == name)
                            {
                                for (auto c = cat->entries.begin(), ce = cat->entries.end();c != ce;++c)
                                {
                                    if ((*c)->item == item)
                                    {
                                        (*c)->categorized--;
                                        cat->entries.erase(c);
                                        break;
                                    }
                                }
                            }
                        }
                        return 0;//database.write("database.ini");
                    }
                }
            }
            return 1;
        }
        
        INIObject database;
        std::vector<faqEntry> db;
        std::vector<faqCategory> category;
        void add(const std::string& item, const std::string& value)
        {
            database("faq",item) = value;
            auto it = find(item);
            if (it == db.end())
                db.emplace_back(item,value);
            else
                *it = value;
        }
        private:
            int remove(const std::string& item)
            {
                auto it = find(item);
                if (it != db.end())
                {
                    for (auto& i : category)
                    {
                        for (auto c = i.entries.begin(), ce = i.entries.end();c != ce;++c)
                        {
                            if ((*c)->item == item)
                            {
                                i.entries.erase(c);
                                database.erase(i.name,item);
                                break;
                            }
                        }
                    }
                    db.erase(it);
                }
                return database.erase("faq",item);
            }
            std::vector<faqEntry>::iterator find(const std::string& item)
            {
                auto ite = db.end();
                for (auto it = db.begin();it != ite;++it)
                {
                    if (it->item == item)
                        return it;
                }
                return ite;
            }
    } database;
    std::unordered_map<std::string,faqBlocker> faqBlock;
    #ifndef METTA_MINI
    std::unordered_map<std::string,koolAidMan> koolaid;
    //int koolaidRefill = 3;
    GlobVar *koolaidRefill = nullptr;
    //std::string botID;
    #endif
    GlobVar *inviteLink = nullptr;
    std::string lastChannel;
    //ChatHook *chatHook = nullptr;
    std::chrono::high_resolution_clock::time_point uptime;
};

//int chatHookChange(Handle&,GlobVar*,const std::string&,const std::string&,const std::string&);
int typingCmd(Handle&,int,const std::string[]);
int addfaqCmd(Handle&,int,const std::string[],const Message&);
int delfaqCmd(Handle&,int,const std::string[],const Message&);
int addfaqcatCmd(Handle&,int,const std::string[],const Message&);
int faquncatCmd(Handle&,int,const std::string[],const Message&);
int delfaqcatCmd(Handle&,int,const std::string[],const Message&);
int faqLogCmd(Handle&,int,const std::string[],const Message&);
int faqBlockCmd(Handle&,int,const std::string[],const Message&);
int faqCmd(Handle&,int,const std::string[],const Message&);
int voteCmd(Handle&,int,const std::string[],const Message&);
int inviteCmd(Handle&,int,const std::string[],const Message&);
int replyCmd(Handle&,int,const std::string[]);
#ifndef METTA_MINI
int redirectReact(Handle&,ReactHook*,const ServerMember&,const Channel&,const std::string&,const std::string&,const Emoji&);
#endif
int phonetizeCmd(Handle&,int,const std::string[],const Message&);
int uptimeCmd(Handle&,int,const std::string[],const Message&);
int whoamiCmd(Handle&,int,const std::string[],const Message&);
int hexifyCmd(Handle&,int,const std::string[],const Message&);

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("mettaton_version",VERSION,"Mettaton Version",GVAR_CONSTANT);
    #ifndef METTA_MINI
    Mettaton::koolaidRefill = handle.createGlobVar("ohyeah","3","How many `oh no`'s does it take to break a hole in the wall?",GVAR_DUPLICATE|GVAR_STORE,true,1.0);//->hookChange(&ohyeahChange);
    #endif
    Mettaton::inviteLink = handle.createGlobVar("invite","https://discord.gg/wavfwcu","Set an invite link for the invite command.",GVAR_DUPLICATE|GVAR_STORE);
    handle.regConsoleCmd("typing",&typingCmd,"Make me type in a channel.");
    handle.regChatCmd("addfaq",&addfaqCmd,NOFLAG,"Add or modify a faq.");
    handle.regChatCmd("delfaq",&delfaqCmd,CUSTOM2,"Delete a faq.");
    handle.regChatCmd("faq",&faqCmd,NOFLAG,"View a faq.");
    handle.regChatCmd("addfaqcat",&addfaqcatCmd,CUSTOM2,"Create a faq category and/or add faqs to one.");
    handle.regChatCmd("faquncat",&faquncatCmd,CUSTOM2,"Remove a faq from one or all categories.");
    handle.regChatCmd("delfaqcat",&delfaqcatCmd,CUSTOM2,"Delete a faq category.");
    handle.regChatCmd("vote",&voteCmd,NOFLAG,"Start a vote.");
    handle.regChatCmd("invite",&inviteCmd,NOFLAG,"Get a server invite... Maybe.");
    handle.regChatCmd("faqlog",&faqLogCmd,CUSTOM2,"View the Faq Audit Log.");
    handle.regChatCmd("faqblock",&faqBlockCmd,CUSTOM2,"Modify or view the faqblock list.");
    handle.regChatCmd("phonetize",&phonetizeCmd,NOFLAG,"View the phonetization of a string.");
    handle.regChatCmd("uptime",&uptimeCmd,NOFLAG,"How long since my last crash or restart.");
    handle.regChatCmd("whoami",&whoamiCmd,NOFLAG,"Can't quite figure out who you are? Let Mettaton take a stab at it!");
    handle.regChatCmd("hexify",&hexifyCmd,NOFLAG,"Turn input into hexadecimal.");
    //handle.createGlobVar("message_output","true","Whether or not to output messages to STDOUT.",0,true,0.0,true,1.0)->hookChange(&chatHookChange);
    //Mettaton::chatHook = handle.hookChatPattern("chat",".",nullptr,HOOK_PRINT);
    handle.regConsoleCmd("reply",&replyCmd,"Send a reply to the channel which last received a message.");
    #ifndef METTA_MINI
    handle.hookReactionGuild("redirect",&redirectReact,0,"737112449798635610","➡️");
    handle.hookReactionGuild("redirectMobile",&redirectReact,0,"737112449798635610","➡");
    #endif
    Mettaton::database.open("database.ini");
    if (Mettaton::database.database.exists("block"))
    {
        auto block = Mettaton::database.database.topic_it("block");
        for (auto it = block->begin(), ite = block->end();it != ite;++it)
        {
            if (it->item == "name")
                Mettaton::faqBlock.emplace(it->value,Mettaton::faqBlocker{true,rens::regex(it->value)});
            else
                Mettaton::faqBlock.emplace(it->value,Mettaton::faqBlocker{false,rens::regex(it->value)});
        }
    }
    Mettaton::uptime = std::chrono::high_resolution_clock::now();
    return 0;
}

extern "C" int onReaction(Handle &handle, Event event, void *member, void *channel, void *message, void *femoji)
{
    if ((((std::string*)message)->size() < 1) || (((ServerMember*)member)->user.id.size() < 1) || (((Channel*)channel)->id.size() < 1))
        return std::stoi("quit");
    return PLUGIN_CONTINUE;
}

extern "C" int onTyping(Handle &handle, Event event, void *channel, void *user, void *timestamp, void *nil)
{
    if ((((std::string*)channel)->size() < 1) || (((std::string*)user)->size() < 1))
        return std::stoi("quit");
    return PLUGIN_CONTINUE;
}

extern "C" int onInvalidSession(Handle &handle, Event event, void *nil, void *foo, void *bar, void *baz)
{
    return std::stoi("quit");
}

extern "C" int onQuit(Handle &handle, Event event, void *nil, void *foo, void *bar, void *baz)
{
    return std::stoi("quit");
}

extern "C" int onDisconnect(Handle &handle, Event event, void *nil, void *foo, void *bar, void *baz)
{
    return std::stoi("quit");
}

extern "C" int onMessage(Handle &handle, Event event, void *message, void *nil, void *foo, void *bar)
{
    Message *msg = (Message*)message;
    if (msg->channel_id.size() < 1)
        return std::stoi("quit");
    Mettaton::lastChannel = msg->channel_id;
    Global *global = recallGlobal();
    auto server = getGuildCache(msg->guild_id);
    std::string guild = msg->guild_id;
    std::string channel = msg->channel_id;
    if (server.name.size() > 0)
        guild = server.name;
    for (auto& chan : server.channels)
    {
        if (chan.id == channel)
        {
            if (chan.name.size() > 0)
                channel = chan.name;
            break;
        }
    }
    consoleOut("[" + guild + "](#" + channel + "): <" + msg->author.username + "> " + msg->content);
    #ifndef METTA_MINI
    std::string prefix = global->prefix(msg->guild_id);
    //std::string prefix = global->prefix;
    /*if (msg->content.find(prefix) == 0)
    {
        std::string content = msg->content;
        content.erase(0,prefix.size());
        size_t pos = content.find(" ");
        if (pos != std::string::npos)
            content.erase(pos);
        *//*if (content == "groups")
        {
            //std::string nick = msg->author.user.nick;
            //std::vector<std::string> role;
            ServerMember user = getServerMember(msg->guild_id,msg->author.id);
            user.roles.push_back("577923476317536317");
            BoolResponse response = editMember(msg->guild_id,msg->author.id,"",std::move(user.roles));//,role);//,{"577923476317536317"});
            if (response.result)
                reactToID(msg->channel_id,msg->id,"%F0%9F%8E%B7");
            else
                sendMessage(msg->channel_id,"Error: " + std::to_string(response.response.statusCode));
                //reactToID(msg->channel_id,msg->id,"%E2%98%A0");
        }
        else *//*if ((content == "about")
         || (content == "groups")
         || (content == "jira")
         || (content == "mcc12")
         || (content == "mccq")
         || (content == "block")
         || (content == "nbt")
         || (content == "ping")
         //|| (content == "quote")
         || (content == "quotem"))
        {
            reactToID(msg->channel_id,msg->id,"randomNegativeEmoji");
            return PLUGIN_HANDLED;
        }
    }*/
    if (msg->content.find(prefix) != 0)
    {
        int refill = Mettaton::koolaidRefill->getAsInt(msg->guild_id);
        auto channel = Mettaton::koolaid.find(msg->channel_id);
        static rens::regex koolaidptrn("[*_~|`]*[oO][*_~|`]*[hH]?[*_~|`]* ?[*_~|`]*[nN][*_~|`]*[oO][*_~|`?!.,;:'\"<>=+-/\\\\]*");
        if ((msg->channel_id != "541749565540532224") && (rens::regex_match(msg->content,koolaidptrn)))
        {
            if (channel == Mettaton::koolaid.end())
            {
                Mettaton::koolAidMan koolaidman = { msg->author.id, 1 };
                Mettaton::koolaid[msg->channel_id] = koolaidman;
                channel = Mettaton::koolaid.find(msg->channel_id);
            }
            else if (channel->second.id != msg->author.id)
            {
                channel->second.count++;
                channel->second.id = msg->author.id;
            }
            if (channel->second.count >= refill)
            {
                sendFile(msg->channel_id,"ohyeah.jpg","***OH YEAH***");
                Mettaton::koolaid.erase(channel);
            }
        }
        else if (channel != Mettaton::koolaid.end())
            Mettaton::koolaid.erase(channel);
        if ((msg->author.id != global->self.id) && (msg->channel_id != "541749565540532224"))
        {
            std::string balp = strlower(msg->content);
            int balps = 0;
            while (balp.find("balp") == 0)
            {
                balps++;
                balp.erase(0,((balp[4] == ' ') ? 5 : 4));
            }
            if (balps > 0)
            {
                balps = mtrand(1,balps*mtrand(1,5));
                if (balps > 400)
                    balps = 400;
                balp.clear();
                if ((balps % 7) == 0)
                    messageChannelID(msg->channel_id,"*We can **balp** if we want to. We can **balp** your **balps** behind, cause your **balps** don't **balp** and if they don't **balp**-- Well, they're no **balps** of mine.*");
                else
                {
                    for (int i = 0;i < balps;i++)
                        balp += "balp ";
                    messageChannelID(msg->channel_id,balp);
                }
            }
        }
    }
    #endif
    return PLUGIN_CONTINUE;
}

/*int chatHookChange(Handle &handle, GlobVar *gvar, const std::string &newvalue,const std::string &oldvalue, const std::string &guild)
{
    if (Mettaton::chatHook != nullptr)
        handle.unhookChatPattern(Mettaton::chatHook);
    Mettaton::chatHook = nullptr;
    if (gvar->getAsBool())
        Mettaton::chatHook = handle.hookChatPattern("chat",".",nullptr,HOOK_PRINT);
    return PLUGIN_CONTINUE;
}*/

int typingCmd(Handle &handle, int argc, const std::string argv[])
{
    if (argc < 2)
        sendTyping(Mettaton::lastChannel);
    else if (std::isdigit(argv[1].front()))
        sendTyping(argv[1]);
    else if ((argv[1].front() == '<') && (argv[1].back() == '>'))
        sendTyping(argv[1].substr(2,argv[1].size()-3));
    else
        consoleOut("Usage: " + argv[0] + " [channel_id]");
    return PLUGIN_HANDLED;
}

int replyCmd(Handle &handle, int argc, const std::string argv[])
{
    if (argc < 2)
        consoleOut("Usage: " + argv[0] + " <message ...>");
    else if (Mettaton::lastChannel.size() > 0)
    {
        std::string msg = argv[1];
        for (int i = 2;i < argc;i++)
            msg = msg + " " + argv[i];
        sendMessage(Mettaton::lastChannel,msg);
    }
    else
        consoleOut("No message to reply to!");
    return PLUGIN_HANDLED;
}

int addfaqCmd(Handle &handle, int argc, const std::string args[], const Message &message)
{
    static rens::regex escape ("[`\\\\]");
    Global *global = recallGlobal();
    if ((argc < 3) || (args[1].size() == 0) || (args[1] == "\\") || (args[2].size() == 0))
    {
        // error
        messageReply(message,"Usage: `" + global->prefix(message.guild_id) + args[0] + " <name> <data>`");
        return PLUGIN_HANDLED;
    }
    int flags = global->getAdminFlags(message.guild_id,message.author.id);
    if (((flags & CUSTOM9) == CUSTOM9) && (flags != ROOT))
    {
        reactToID(message.channel_id,message.id,"randomNegativeEmoji");
        return PLUGIN_HANDLED;
    }
    bool exists = false;
    if (((exists = Mettaton::database.database.exists("faq",args[1]))) && ((flags & PIN) == 0))
    {
        messageReply(message,"You do not have access to overwrite a faq!");
        return PLUGIN_HANDLED;
    }
    std::string content, cat;
    std::string faq = rens::regex_replace(strlower(args[1]),escape,"");
    size_t p = faq.find(':');
    if (p != std::string::npos)
    {
        cat = faq.substr(p+1);
        faq = faq.substr(0,p);
    }
    for (int i = 2;i < argc;i++)
        content = content + args[i] + " ";
    content.erase(content.size()-1);
    for (size_t pos = content.find('\n');pos != std::string::npos;pos = content.find('\n'))
        content[pos] = ' ';
    for (size_t pos = faq.find('=');pos != std::string::npos;pos = faq.find('='))
        faq.erase(pos,1);
    for (auto it = Mettaton::faqBlock.begin(), ite = Mettaton::faqBlock.end();it != ite;++it)
    {
        if (it->second.isName)
        {
            if (rens::regex_search(faq,it->second.ptrn))
            {
                messageReply(message,"Invalid FAQ! The name must not match: `" + it->first + "`.");
                return PLUGIN_HANDLED;
            }
        }
        else if (rens::regex_search(content,it->second.ptrn))
        {
            messageReply(message,"Invalid FAQ! The content must not match: `" + it->first + "`.");
            return PLUGIN_HANDLED;
        }
    }
    std::string audit = '`' + message.author.username + '#' + message.author.discriminator + '`';
    if (exists)
        audit = audit + " has updated `";
    else
        audit = audit + " has created `";
    audit = audit + faq + "`: `" + content + "`.";
    Mettaton::database.add(faq,content);
    if ((p != std::string::npos) && (Mettaton::database.setCategory(cat,faq) == 1))
        messageReply(message,"Error adding FAQ to nonexistant category `" + cat + "`.");
    reaction(message,"%E2%9C%85");
    if (Mettaton::database.save())
        messageReply(message,"An error occurred saving the database. The entry will be lost when I inevitably crash.");
    else
    {
        std::ofstream log ("./faqlog.audit",std::ofstream::out|std::ofstream::app);
        if (log.is_open())
        {
            log<<audit<<std::endl;
            log.close();
        }
    }
    return PLUGIN_HANDLED;
}

int delfaqCmd(Handle &handle, int argc, const std::string args[], const Message &message)
{
    Global *global = recallGlobal();
    if (argc < 2)
    {
        messageReply(message,"Usage: `" + global->prefix(message.guild_id) + args[0] + " <name>`");
        return PLUGIN_HANDLED;
    }
    std::string faq = strlower(args[1]);
    for (int i = 2;i < argc;i++)
         faq = faq + ' ' + strlower(args[i]);
    int err = Mettaton::database.erase(faq);
    switch (err)
    {
        case 2:
        {
            // lookup similar
            std::string similar;
            Mettaton::database.lookup(faq,similar);
            if (similar.size() > 0)
                messageReply(message,"Did you mean: " + similar + '?');
            else
                reaction(message,"%F0%9F%A4%B9");
            return PLUGIN_HANDLED;
        }
        case 1:
            messageReply(message,"An error occurred saving the database. The entry will come back when I inevitably crash.");
        default:
            reaction(message,"%E2%9C%85");
    }
    if (!err)
    {
        std::ofstream log ("./faqlog.audit",std::ofstream::out|std::ofstream::app);
        if (log.is_open())
        {
            log<<'`' + message.author.username + '#' + message.author.discriminator + "` has deleted `" + faq + "`."<<std::endl;
            log.close();
        }
    }
    return PLUGIN_HANDLED;
}

int faqCmd(Handle &handle, int argc, const std::string args[], const Message &message)
{
    if ((argc < 2) || (strlower(args[1]) == "all"))
    { // list faqs
        std::string avail, title = "All FAQs:";// = "No available FAQs!";
        if (argc < 2)
        {
            auto cat = Mettaton::database.category.begin(), cate = Mettaton::database.category.end();
            if (cat != cate)
            {
                avail = "FAQ Categories:\n`" + cat->name + '`';
                if (cat->desc.size() > 0)
                    avail += ": " + cat->desc;
                for (++cat;cat != cate;++cat)
                {
                    avail += "\n`" + cat->name + '`';
                    if (cat->desc.size() > 0)
                        avail += ": " + cat->desc;
                }
                avail += "\n`all`: Cover yourself in FAQ's like the filthy whore you are\n\n";
            }
            title = "Uncategorized FAQs:";
        }
        //auto faq = Mettaton::database.database.topic_it("faq");
        auto it = Mettaton::database.db.begin(), ite = Mettaton::database.db.end();
        if (argc < 2) for (;(it != ite) && (it->categorized);++it);
        if (it != ite)
        {
            std::string oxford;
            avail += title + "\n`" + it->item + '`';
            size_t current = it->item.size() + 2;
            if (argc < 2) for (++it;(it != ite) && (it->categorized);++it);
            if (it != ite-1)
                oxford = ", and ";
            else
                oxford = " and ";
            size_t soxford = oxford.size();
            for (;it != ite;++it)
            {
                if ((argc < 2) && (it->categorized))
                    continue;
                std::string temp;
                if ((avail.size() + soxford + it->item.size() + 2) > 1650)
                {
                    messageReply(message,avail);
                    avail = title + " (continued):\n";
                    current = 0;
                }
                else
                {
                    if (it == ite-1)
                        temp = oxford;
                    else
                        temp = ", ";
                    if ((current + temp.size()) > 42)
                    {
                        avail += temp + '\n';
                        temp.clear();
                        current = 0;
                    }
                }
                avail += (temp = temp + '`' + it->item + '`');
                current += temp.size();
            }
        }
        if (avail.size() < 1)
            avail = "No available FAQs!";
        messageReply(message,avail);
        return PLUGIN_HANDLED;
    }
    std::string faq = strlower(args[1]);
    for (int i = 2;i < argc;i++)
        faq = faq + ' ' + strlower(args[i]);
    std::string similar;
    std::string value = Mettaton::database.lookup(faq,similar);
    if (!value.size())
    {
        if (similar.size() > 0)
            messageReply(message,"Did you mean: " + similar + '?');
        else
            reaction(message,"%F0%9F%A4%B9");
        return PLUGIN_HANDLED;
    }
    if (similar.size() > 0)
    {
        // category listing
        value = similar + ":\n" + value;
        while (value.size() > 1650)
        {
            size_t pos = 1650;
            for (;value.at(pos) != '\n';--pos);
            messageReply(message,value.substr(0,pos));
            value.erase(0,pos+1);
            if (value.size() > 0)
                value = similar + " (continued):\n" + value;
        }
    }
    if (value.size() > 0)
        messageReply(message,value);
    return PLUGIN_HANDLED;
}

int addfaqcatCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    // Usage: addfaqcat <category[:description]> [faqs ...]
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <category[:description]> [faqs ...]`");
    else
    {
        size_t pos = argv[1].find(':');
        std::string name = strlower(argv[1].substr(0,pos)), desc;
        if (pos != std::string::npos)
            desc = argv[1].substr(pos+1);
        int err = Mettaton::database.addCategory(name,desc);
        if (argc > 2)
        {
            std::string out;
            int count = 0;
            for (int i = 2;i < argc;++i)
            {
                if ((err = Mettaton::database.setCategory(name,strlower(argv[i]))) == 0)
                    ++count;
                else if (err == -2)
                    out += "Error: `" + argv[i] + "` is not a valid FAQ!\n";
            }
            if (count == 0)
                out += "Nothing to be done.";
            else
            {
                Mettaton::database.save();
                out += "Successfully added " + std::to_string(count) + " FAQ(s) to category `" + name + "`.";
            }
            sendMessage(message.channel_id,out);
        }
        else if (err == 1)
            sendMessage(message.channel_id,"FAQ Category `" + name + "` already exists!");
        else
        {
            Mettaton::database.save();
            sendMessage(message.channel_id,std::string("Successfully ") + (err < 0 ? "updated" : "created") + " FAQ category `" + name + '`');
        }
    }
    return PLUGIN_HANDLED;
}

int faquncatCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    // Usage: faquncat <faq> [category] - remove a FAQ from a category or all.
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <faq> [category]`");
    else
    {
        std::string cat;
        if (argc > 2)
            cat = argv[2];
        int err = Mettaton::database.remCategory(argv[1],cat);
        if (cat.size() < 1)
            cat = "all categories.";
        else
            cat = "`" + cat + "` category.";
        if (err == 1)
            sendMessage(message.channel_id,"Error: No such " + cat);
        else if (err == 2)
            sendMessage(message.channel_id,"Error: No such FAQ `" + argv[1] + "`.");
        else
        {
            Mettaton::database.save();
            sendMessage(message.channel_id,"Successfully removed `" + argv[1] + "` from " + cat);
        }
    }
    return PLUGIN_HANDLED;
}

int delfaqcatCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <category>`");
    if (Mettaton::database.delCategory(argv[1]))
        sendMessage(message.channel_id,"Error: No such `" + argv[1] + "` category.");
    else
    {
        Mettaton::database.save();
        sendMessage(message.channel_id,"Successfully deleted `" + argv[1] + "` category.");
    }
    return PLUGIN_HANDLED;
}

std::string escapeChar(std::string in, std::string escapes)
{
    static const std::string special = "^[]";
    std::string out;
    rens::smatch ml;
    for (size_t pos = 0;(pos = escapes.find_first_of(special,pos)) != std::string::npos;++pos)
        escapes.insert(pos++,1,'\\');
    rens::regex escape ("(\\\\*)([" + escapes + "])");
    while (rens::regex_search(in,ml,escape))
    {
        out += ml.prefix().str() + ml[1].str();
        if (ml[1].length() % 2 == 0)
            out.append(1,'\\').append(ml[2].str());
        else
            out += ml[2].str();
        in = ml.suffix().str();
    }
    if (out.size() < 1)
        out = in;
    return std::move(out);
}

int faqLogCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global *global = recallGlobal();
    if (argc > 1)
    {
        if (argv[1] == "help")
        {
            sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " [regex_pattern] [page]`");
            return PLUGIN_HANDLED;
        }
        if (argv[1] == "--purge")
        {
            if ((global->getAdminFlags(message.guild_id,message.author.id) & RCON) != RCON)
            {
                sendMessage(message.channel_id,"You do not have access to purge the faq audit log!");
                return PLUGIN_HANDLED;
            }
            std::vector<std::string> keep;
            size_t purged = 0;
            if (argc > 2)
            {
                rens::regex ptrn (argv[2]);
                std::ifstream log ("./faqlog.audit");
                if (log.is_open())
                {
                    std::string line;
                    for (;std::getline(log,line);purged++)
                        if (!rens::regex_search(line,ptrn))
                            keep.push_back(line);
                    log.close();
                }
                purged -= keep.size();
            }
            else
                purged = std::string::npos;
            std::ofstream log ("./faqlog.audit",std::ofstream::out|std::ofstream::trunc);
            if (log.is_open())
            {
                for (auto it = keep.begin(), ite = keep.end();it != ite;++it)
                    log<<*it<<std::endl;
                log.close();
            }
            if (purged != std::string::npos)
                sendMessage(message.channel_id,"Purged `" + std::to_string(purged) + "` matching audits from the faq log.");
            else
                sendMessage(message.channel_id,"Purged every audit from the faq log.");
            return PLUGIN_HANDLED;
        }
    }
    std::vector<std::string> log;
    std::ifstream audit ("./faqlog.audit");
    if (audit.is_open())
    {
        std::vector<std::string> logreverse;
        std::string line;
        while (std::getline(audit,line))
            logreverse.push_back(line);
        log.assign(logreverse.rbegin(),logreverse.rend());
        audit.close();
    }
    if (log.size() < 1)
    {
        sendMessage(message.channel_id,"No audit log file found!");
        return PLUGIN_HANDLED;
    }
    std::string embed = "{ \"color\": 801647, \"title\": \"Faq Audit Log ";
    int page = 1;
    rens::regex ptrn;
    bool useRegex = false;
    if (argc > 1)
    {
        int parg = 1;
        if (argc > 2)
            parg = 2;
        if (std::isdigit(argv[parg].front()))
            page = std::stoi(argv[parg]);
        if (std::to_string(page) != argv[parg])
        {
            page = 1;
            parg = 2;
        }
        if (parg == 2)
        {
            embed = embed + "(" + argv[1] + ") ";
            ptrn = argv[1];
            useRegex = true;
        }
    }
    std::vector<std::string> content;
    for (auto it = log.begin(), ite = log.end();it != ite;++it)
        if (((useRegex) && (rens::regex_search(*it,ptrn))) || (!useRegex))
            content.push_back(escapeChar(std::move(*it),"\""));
    int pages = content.size()/10;
    if ((content.size()%10) > 0)
        pages++;
    if (page > pages)
        page = pages;
    embed = embed + "- Page " + std::to_string(page) + '/' + std::to_string(pages) + "\", \"description\": \"";
    if (page > 0)
    {
        int i = (page-1)*10, j = i+10;
        for (auto it = content.begin()+i, ite = content.end();((i < j) && (it != ite));++it,++i)
        {
            if ((content.size() > 5) && (it->size() > 99))
            {
                int i = 99;
                if ((it->size() > i+1) && (it->at(i+1) < 1))
                    for (;(i > -1) && (it->at(i) < 1);i--);
                embed = embed + it->substr(0,i+1) + "...`\\n\\n";
            }
            else if ((content.size() > 1) && (it->size() > 299))
            {
                int i = 299;
                if ((it->size() > i+1) && (it->at(i+1) < 1))
                    for (;(i > -1) && (it->at(i) < 1);i--);
                embed = embed + it->substr(0,i+1) + "...`\\n\\n";
            }
            else if (it->size() > 2000)
            {
                int i = 1999;
                if ((it->size() > i+1) && (it->at(i+1) < 1))
                    for (;(i > -1) && (it->at(i) < 1);i--);
                embed = embed + it->substr(0,i+1) + "...`\\n\\n";
            }
            else
            {
                embed = embed + *it + "\\n";
                if (it->size() > 59)
                    embed += "\\n";
            }
        }
    }
    if (content.size() < 1)
        embed += "No matching content.";
    embed += "\" }";
    debugOut(embed);
    sendEmbed(message.channel_id,embed);
    return PLUGIN_HANDLED;
}

int faqBlockCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    bool usage = false;
    if (argc < 2)
        usage = true;
    else if (argv[1] == "list")
    {
        std::string title, list;
        short type = 2;
        if (argc == 2); // list both
        else if (argv[2] == "name") // list names
        {
            title = "Name ";
            type = 1;
        }
        else if (argv[2] == "content") // list content
        {
            title = "Content ";
            type = 0;
        }
        else
            usage = true;
        if (!usage)
        {
            for (auto it = Mettaton::faqBlock.begin(), ite = Mettaton::faqBlock.end();it != ite;++it)
            {
                if ((type == 2) || (it->second.isName == type))
                {
                    if (type == 2)
                    {
                        if (it->second.isName)
                            list.append("Name Block: ");
                        else
                            list.append("Content Block: ");
                    }
                    list.append("`" + it->first + "`\\n");
                }
            }
            if (list.size() == 0)
                list = "None!";
            sendEmbed(message.channel_id,"{ \"color\": 16711680, \"title\": \"Blocked FAQ " + title + "Patterns\", \"description\": \"" + list + "\" }");
        }
    }
    else if (argc < 4)
        usage = true;
    else if (argv[1] == "add")
    {
        bool isName = true;
        if (argv[2] == "name");
        else if (argv[2] == "content")
            isName = false;
        else
            usage = true;
        if (!usage)
        {
            Mettaton::faqBlock.emplace(argv[3],Mettaton::faqBlocker{isName,rens::regex(argv[3])});
            reaction(message,"%E2%9C%85");
            std::ofstream db ("database.ini",std::ofstream::out|std::ofstream::app);
            if (db.is_open())
            {
                if (Mettaton::faqBlock.size() == 1)
                    db<<"[block]\n";
                if (isName)
                    db<<"name=";
                else
                    db<<"content=";
                db<<argv[3]<<std::endl;
                db.close();
            }
            else
                messageReply(message,"An error occurred saving the database. The entry will be lost when I inevitably crash.");
        }
    }
    else if (argv[1] == "remove")
    {
        bool isName = true;
        if (argv[2] == "name");
        else if (argv[2] == "content")
            isName = false;
        else
            usage = true;
        if (!usage)
        {
            auto block = Mettaton::database.database.topic_it("block");
            if (block->items() == 0)
                reaction(message,"%F0%9F%A4%B9");
            else
            {
                for (auto it = block->begin(), ite = block->end();it != ite;++it)
                {
                    if ((((isName) && (it->item == "name")) || ((!isName) && (it->item != "name"))) && (it->value == argv[3]))
                    {
                        block->erase(it);
                        Mettaton::faqBlock.erase(argv[3]);
                        reaction(message,"%E2%9C%85");
                        if (Mettaton::database.database.write("database.ini"))
                            messageReply(message,"An error occurred saving the database. The entry will come back when I inevitably crash.");
                        break;
                    }
                }
            }
        }
    }
    if (usage)
        messageReply(message,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <list|add|remove> [<name|content> [regex_pattern]]`");
    return PLUGIN_HANDLED;
}

struct timerDataStruct
{
    std::vector<std::string> reactions;
    std::string channel;
    std::string message;
};

int voteTimer(Handle& handle, Timer* timer, void* data)
{
    timerDataStruct* reactions = (timerDataStruct*)data;
    if (reactions->reactions.size())
    {
        auto it = reactions->reactions.begin();
        //auto response = reactToID(reactions->channel,reactions->message,*it);
        reactToID(reactions->channel,reactions->message,*it);
        //if (response.result)
        reactions->reactions.erase(it);
    }
    if (reactions->reactions.size())
        return PLUGIN_CONTINUE;
    delete reactions;
    return PLUGIN_HANDLED;
}

int voteCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    timerDataStruct* data = new timerDataStruct;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    rens::smatch ml;
    static rens::regex emojiptrn ("<a?:([-\\w]+:\\d+)>");
    //consoleOut("vote: " + std::to_string(argc));
    for (int i = 1;i < argc;i++)
    {
        std::string foo = argv[i];
        std::wstring test = converter.from_bytes(foo);
        //consoleOut(":: " + std::to_string(test.size()) + " :: " + std::to_string(argv[i].size()));
        if (foo.size() > test.size())
            data->reactions.push_back(hexify(foo));
        else while (rens::regex_search(foo,ml,emojiptrn))
        {
            data->reactions.push_back(ml[1].str());
            foo = ml.suffix().str();
        }
        //else
        //    break;
    }
    if (data->reactions.size() < 1)
    {
        data->reactions.push_back("%F0%9F%8E%B7");
        data->reactions.push_back("%E2%98%A0%EF%B8%8F");
    }
    auto it = data->reactions.begin();
    reactToID(message.channel_id,message.id,*it);
    data->reactions.erase(it);
    if (data->reactions.size() > 0)
    {
        data->channel = message.channel_id;
        data->message = message.id;
        handle.createTimer("vote",1,&voteTimer,(void*)data);
    }
    else
        delete data;
    return PLUGIN_HANDLED;
}

int inviteCmd(Handle &handle, int argc, const std::string args[], const Message &message)
{
    //messageReply(message,"Ask " + makeMention("221981109506801664",message.guild_id) + " for a new invite!");
    messageReply(message,Mettaton::inviteLink->getAsString(message.guild_id));
    return PLUGIN_HANDLED;
}

#ifndef METTA_MINI
int redirectReact(Handle& handle, ReactHook* hook, const ServerMember& member, const Channel& channel, const std::string& messageID, const std::string& guildID, const Emoji& emoji)
{
    if (member.user.id != recallGlobal()->self.id)
    {
        ObjectResponse<Message> response = getMessage(channel.id,messageID);
        if (response.response.error())
            reactToID(channel.id,messageID,"%E2%9D%93");
        else if (!response.object.author.bot)
        {
            for (auto it = response.object.reactions.begin(), ite = response.object.reactions.end();it != ite;++it)
            {
                if (it->emoji == emoji)
                {
                    if (it->count == 1)
                    {
                        sendEmbed(channel.id,"{ \"color\": 7455205, \"description\": \"Hi " + makeMention(response.object.author.id,guildID) + ", please read <#737113941003730994> and post content that makes everyone go \\\"*aww*\\\" in <#737115353792118846>!\" }");
                        reactToID(channel.id,messageID,emoji.encoded());//,member.user.id);
                    }
                    break;
                }
            }
        }
    }
    return PLUGIN_HANDLED;
}
#endif

int phonetizeCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <string>`");
    else
    {
        Mettaton::soundex sound = Mettaton::phonetize(argv[1]);
        sendMessage(message.channel_id,"Phonetics for " + argv[1] + "\nCode: `" + sound.code + "(" + std::to_string(sound.coder) + "):" + std::to_string(sound.coded) + "`, Full: `" + sound.full + '`');
    }
    return PLUGIN_HANDLED;
}

int uptimeCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    auto now = std::chrono::high_resolution_clock::now();
    size_t seconds = std::chrono::duration_cast<std::chrono::seconds>(now - Mettaton::uptime).count(), minutes, hours, days, weeks;
    time_t t = std::chrono::duration_cast<std::chrono::seconds>(Mettaton::uptime.time_since_epoch()).count();
    std::string output = ctime(&t);
    output.erase(output.size()-1);
    output += ", ";
    minutes = (hours = seconds / 60) % 60;
    seconds %= 60;
    hours = (days = hours / 60) % 60;
    days = (weeks = days / 24) % 24;
    weeks /= 7;
    if (weeks)
        output += std::to_string(weeks) + " week" + (weeks > 1 ? "s, " : ", ");
    if (days)
        output += std::to_string(days) + " day" + (days > 1 ? "s, " : ", ");
    if (hours)
        output += std::to_string(hours) + " hour" + (hours > 1 ? "s, " : ", ");
    if (minutes)
        output += std::to_string(minutes) + " minute" + (minutes > 1 ? "s, " : ", ");
    output += std::to_string(seconds) + " second" + ((seconds > 1) || (seconds == 0) ? "s." : ".");
    sendMessage(message.channel_id,output);
    return PLUGIN_HANDLED;
}

int whoamiCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    std::vector<std::string> list;
    std::ifstream file("prefixes.dict");
    std::string line, title, temp, nick, nickless;
    if (file.is_open())
    {
        while (std::getline(file,line))
            list.emplace_back(line);
        file.close();
        title = (nick = list.at(mtrand(0,list.size()-1))) + ' ';
        nickless = nick + ", the ";
        list.clear();
        if ((message.member.nick.size() > 0) && ((message.member.nick.size() < 17) || (message.member.nick.size() < message.author.username.size())))
            title += message.member.nick + ", the ";
        else
            title += message.author.username + ", the ";
        if (title.size()-6 < 33)
            nick = title.substr(0,title.size()-6);
        else if (nick.size() + message.author.username.size() + 1 < 33)
            nick.append(" ").append(message.author.username);
        file.open("titles.dict");
        if (file.is_open())
        {
            while (std::getline(file,line))
                list.emplace_back(line);
            file.close();
            temp = list.at(mtrand(0,list.size()-1));
            list.clear();
            bool adverb;
            line = temp.substr(temp.size()-3);
            if (line == "ing")
                adverb = (mtrand(0,3) == 0);
            else if (line.substr(1) == "ly")
                adverb = (mtrand(0,9) == 0);
            else
                adverb = (mtrand(0,1) == 0);
            if (adverb)
            {
                file.open("adverbs.dict");
                while (std::getline(file,line))
                    list.emplace_back(line);
                file.close();
                std::string a = list.at(mtrand(0,list.size()-1)) + ' ';
                title += a;
                nickless += a;
                list.clear();
            }
            title += temp;
            nickless += temp;
            if ((title.size() > 32) && (nickless.size() < 33))
                nick = nickless;
            else if (title.size() < 33)
                nick = title;
            else if (nick.size() + temp.size() + 6 < 33)
                nick += ", the " + temp;
            else if (nick.size() + temp.size() + 1 < 33)
                nick = temp + ' ' + nick;
            if (nick.size() < 33)
                editMember(message.guild_id,message.author.id,nick);
            if (nick != title)
                nick = " (" + nick + ')';
            else
                nick.clear();
            sendMessage(message.channel_id,"Why you are " + title + nick + ", of course!");
        }
    }
    return PLUGIN_HANDLED;
}

int hexifyCmd(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + recallGlobal()->prefix(message.guild_id) + argv[0] + " <string>`");
    else
        sendMessage(message.channel_id,"**Hex**: `" + hexify(argv[1]) + '`');
    return PLUGIN_HANDLED;
}


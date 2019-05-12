#include <iostream>
#include <ctime>
#include <random>
#include <fstream>
#include "api/farage.h"
#include "str_tok.h"
#include "shared/libini.h"
using namespace Farage;

#define VERSION "0.7.5"

extern "C" Info Module
{
    "Discord Rock Paper Scissors",
    "nigel",
    "Rock Paper Scissors Game",
    VERSION,
    "http://paper.justca.me/all/over/rock/while/scissors/watched",
    FARAGE_API_VERSION
};

struct rpsPlayer
{
    std::string ID;
    std::string DM;
    std::string name;
    std::string choice = "";
    int wins = 0;
    int multiWins = 0;
};

struct rpsGame
{
    std::string gameMode;
    std::string chan;
    std::string guild_id;
    time_t expire;
    std::vector<rpsPlayer> player;
    bool accepted = false;
    bool multi = false;
    int wins = 1;
};

namespace RPS
{
    //int rounds = 2;
    //std::string mod = "Ultimate";
    GlobVar *rounds = nullptr;
    GlobVar *mod = nullptr;
    std::string myBotID;
    std::vector<rpsGame> rpsGames;
    INIObject rpsConf;
    struct color
    {
        std::string info = "4565417";
        std::string result = "13415680";
        std::string conclude = "2464071";
        std::string win = "6607510";
        std::string lose = "5592405";
        std::string draw = "3680312";
        std::string knockout = "13132850";
        std::string notify = "687534";
        std::string error = "11741481";
        
        std::string Info(const std::string &mod = "")
        {
            std::string out = info;
            if ((mod.size() > 0) && (rpsConf.exists(mod,"infocolor")))
                out = rpsConf(mod,"infocolor");
            return out;
        }
        
        std::string Result(const std::string &mod = "")
        {
            std::string out = result;
            if ((mod.size() > 0) && (rpsConf.exists(mod,"resultcolor")))
                out = rpsConf(mod,"resultcolor");
            return out;
        }
        
        std::string Conclude(const std::string &mod = "")
        {
            std::string out = conclude;
            if ((mod.size() > 0) && (rpsConf.exists(mod,"concludecolor")))
                out = rpsConf(mod,"concludecolor");
            return out;
        }
        
        std::string Win(const std::string &mod = "")
        {
            std::string out = win;
            if ((mod.size() > 0) && (rpsConf.exists(mod,"wincolor")))
                out = rpsConf(mod,"wincolor");
            return out;
        }
        
        std::string Lose(const std::string &mod = "")
        {
            std::string out = lose;
            if ((mod.size() > 0) && (rpsConf.exists(mod,"losecolor")))
                out = rpsConf(mod,"losecolor");
            return out;
        }
        
        std::string Draw(const std::string &mod = "")
        {
            std::string out = draw;
            if ((mod.size() > 0) && (rpsConf.exists(mod,"drawcolor")))
                out = rpsConf(mod,"drawcolor");
            return out;
        }
        
        std::string Knockout(const std::string &mod = "")
        {
            std::string out = knockout;
            if ((mod.size() > 0) && (rpsConf.exists(mod,"knockoutcolor")))
                out = rpsConf(mod,"knockoutcolor");
            return out;
        }
        
        std::string Notify(const std::string &mod = "")
        {
            std::string out = notify;
            if ((mod.size() > 0) && (rpsConf.exists(mod,"notifycolor")))
                out = rpsConf(mod,"notifycolor");
            return out;
        }
        
        std::string Error(const std::string &mod = "")
        {
            std::string out = error;
            if ((mod.size() > 0) && (rpsConf.exists(mod,"errorcolor")))
                out = rpsConf(mod,"errorcolor");
            return out;
        }
    } colors;
};

int chatRPS(Handle &handle, int argc, const std::string argv[], const Message &message);
int chatRPSResult(Handle &handle, int argc, const std::string argv[], const Message &message);
int chatRPSStatus(Handle &handle, int argc, const std::string argv[], const Message &message);
int chatRPSReset(Handle &handle, int argc, const std::string argv[], const Message &message);
int chatRPSReload(Handle &handle, int argc, const std::string argv[], const Message &message);
int rpsExpireCheck(Handle &handle, Timer *timer, void *args);
int modChange(Handle&,GlobVar*,const std::string&,const std::string&,const std::string&);
//int roundsChange(Handle&,GlobVar*,const std::string&,const std::string&,const std::string&);

bool isPlaying(const std::string &ID);
bool isChannelActive(const std::string &ID);
std::string rpsModInfo(std::string mod = "", bool options = false);
bool rpsGameLookup(std::vector<rpsGame>::iterator &game, std::string channelID = "", std::string userID = "");
bool rpsIsValidMod(std::string &mod);
void rpsConclude(std::vector<rpsGame>::iterator &game);
std::string rpsParseMention(std::string str);
void rpsStartRound(std::vector<rpsGame>::iterator game);
std::vector<rpsPlayer>::iterator rpsGetPlayer(std::vector<rpsGame>::iterator &game, const std::string &ID);
bool rpsPlayersReady(std::vector<rpsGame>::iterator &game);
void rpsConcludeMulti(std::vector<rpsGame>::iterator &game);
int rpsGetWinner(const std::string &mode, const std::string &v1, const std::string &v2, std::string &result);
std::string randomComeback(const std::string &name);

void loadConfig()
{
    auto conf = RPS::rpsConf.topic_it("config");
    std::string color;
    color = conf->find("infocolor");
    if (color.size() > 0)
        RPS::colors.info = color;
    color = conf->find("resultcolor");
    if (color.size() > 0)
        RPS::colors.result = color;
    color = conf->find("concludecolor");
    if (color.size() > 0)
        RPS::colors.conclude = color;
    color = conf->find("wincolor");
    if (color.size() > 0)
        RPS::colors.win = color;
    color = conf->find("losecolor");
    if (color.size() > 0)
        RPS::colors.lose = color;
    color = conf->find("drawcolor");
    if (color.size() > 0)
        RPS::colors.draw = color;
    color = conf->find("knockoutcolor");
    if (color.size() > 0)
        RPS::colors.knockout = color;
    color = conf->find("notifycolor");
    if (color.size() > 0)
        RPS::colors.notify = color;
    color = conf->find("errorcolor");
    if (color.size() > 0)
        RPS::colors.error = color;
}

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("rps_version",VERSION,"Rock Paper Scissors Version",GVAR_CONSTANT);
    RPS::rounds = handle.createGlobVar("rps_def_rounds","2","Rock Paper Scissors default rounds",GVAR_DUPLICATE,true,1.0);//->hookChange(&roundsChange);
    (RPS::mod = handle.createGlobVar("rps_def_mod","Ulimate","Rock Paper Scissors default mod",GVAR_DUPLICATE))->hookChange(&modChange);
    handle.regChatCmd("rps",&chatRPS,NOFLAG,"Challenge someone to a game of Rock Paper Scissors!");
    handle.regChatCmd("rpsresult",chatRPSResult,NOFLAG,"View the result of two options on a RPS game.");
    handle.regChatCmd("rpsstatus",chatRPSStatus,NOFLAG,"Check the game status of the current channel.");
    handle.regChatCmd("rpsreset",chatRPSReset,CUSTOM1,"Reset the game status of the current channel.");
    handle.regChatCmd("rpsreload",chatRPSReload,RCON,"Reload the rps.ini file.");
    handle.createTimer("rps_expire",60,&rpsExpireCheck);
    if (!RPS::rpsConf.open("rps.ini"))
        loadConfig();
    else
    {
        errorOut("RPS: Error loading rps.ini file!");
        return 1;
    }
    return 0;
}

int modChange(Handle &handle, GlobVar *gvar, const std::string &newvalue, const std::string &oldvalue, const std::string &guild)
{
    //RPS::mod = newvalue;
    std::string mod = newvalue;
    if (!rpsIsValidMod(mod))
    {
        //RPS::mod = oldvalue;
        mod = oldvalue;
        if (rpsIsValidMod(mod))
            gvar->setString(mod,guild);
        else
            gvar->reset(guild);
    }
    else
        gvar->setString(mod,guild);
    return PLUGIN_HANDLED;
}

/*int roundsChange(Handle &handle, GlobVar *gvar, const std::string &newvalue, const std::string &oldvalue, const std::string &guild)
{
    RPS::rounds = gvar->getAsInt(guild);
    return PLUGIN_HANDLED;
}*/

extern "C" int onReady(Handle &handle, Event event, void *data, void *nil, void *foo, void *bar)
{
    //User self;
    //recallGlobal()->callbacks.getSelf(self);
    //RPS::myBotID = self.id;
    //auto readyData = GET_EVENT_ARG1(event,data);
    //auto readyData = ((event == 2) ? ((Farage::Ready*)data) : (nullptr));
    //auto readyData = ((Farage::Ready*)data);
    //auto readyData = reinterpret_cast<GET_EVENT_CAST1(event)>(data);
    //GET_READY(data,readyData);
    //GET_EVENT_ARG1(event,data,readyData);
    RPS::myBotID = ((Ready*)(data))->user.id;
    return PLUGIN_CONTINUE;
}

extern "C" int onMessage(Handle &handle, Event event, void *message, void *nil, void *foo, void *bar)
{
    Global *global = recallGlobal();
    //Message *msg = (Message*)message;
    //auto msg = GET_EVENT_ARG1(event,message);
    //auto readyData = reinterpret_cast<GET_EVENT_CAST1(event)>(message);
    //GET_MESSAGE(message,msg);
    //GET_EVENT_ARG1(event,message,msg);
    Message *msg = (Message*)message;
    Channel channel = getChannel(msg->channel_id).object;
    //Channel channel = getChannelCache(msg->guild_id,msg->channel_id);
    if (msg->author.id == RPS::myBotID)
        return PLUGIN_CONTINUE;
    std::vector<rpsGame>::iterator game;
    if (rpsGameLookup(game,"",msg->author.id))
    {
        auto player = rpsGetPlayer(game,msg->author.id);
        if (channel.id == player->DM)
        {
            if (!game->accepted)
            {
                messageReply(*msg,"The game needs to be accepted first!");
                return 0;
            }
            int p = 0;
            if (player->choice.size() > 0)
            {
                messageReply(*msg,"You have already made your choice!");
                return 0;
            }
            std::string choice = lower(msg->content);
            if (istokcs(RPS::rpsConf.find(game->gameMode,"opts"),choice," "))
            {
                player->choice = choice;
                reaction(*msg,"%E2%9C%85");
                if (rpsPlayersReady(game))
                    rpsConclude(game);
            }
            else
                reaction(*msg,"%E2%9D%97");
        }
    }
    return PLUGIN_CONTINUE;
}

std::string randomtok(std::string text, std::string delim)
{
    if (numtok(text,delim) <= 1) return text;
    return gettok(text,mtrand(1,numtok(text,delim)),delim);
}

int chatRPSReload(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    if (RPS::rpsConf.open("rps.ini"))
    {
        loadConfig();
        reaction(message,"%E2%9D%97");
    }
    else
        reaction(message,"%E2%9C%85");
    return PLUGIN_HANDLED;
}

int chatRPS(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global *global = recallGlobal();
    //Channel channel;
    //global->callbacks.getChannel(message.channel_id,channel);
    std::string prefix = global->prefix(message.guild_id);
    argc--;
    std::string mod = RPS::mod->getAsString(message.guild_id);
    int rounds = RPS::rounds->getAsInt(message.guild_id);
    if ((argc < 1) || (argc > 3))
    {
        //consoleOut("displaying usage...");
        //global->callbacks.messageReply(message,"Usage: `" + prefix + "rps [mod] <@opponent> [multi] [rounds]` to challenge someone\nUsage: `" + prefix + "rps mods` to view loaded mods\nUsage: `" + prefix + "rps accept` to accept a challenge\nUsage: `" + prefix + "rps join` to join a group match\nUsage: `" + prefix + "rps start` to start a group match");
        //std::string out = "{\"color\": " + RPS::colors.Info() + ", \"title\": \"Rock Paper Scissors Usage\",  \"fields\": [{ \"name\": \"`" + prefix + "rps [mod=" + RPS::mod + "] <@opponent|multi|multibot> [rounds=" + std::to_string(RPS::rounds) + "]`\", \"value\": \"To challenge someone to a game.\" }, { \"name\": \"Currently loaded mods:\", \"value\": \"" + rpsModInfo() + "\" }, { \"name\": \"Accepting and joining a game:\", \"value\": \"Use `" + prefix + "rps accept` if you were challenged directly.\\nUse `" + prefix + "rps join` to join a group match.\\nIf you initiated the group match, you can start the game early with `" + prefix + "rps start`\" }]}";
        //consoleOut(out);
        sendEmbed(message.channel_id,"{\"color\": " + RPS::colors.Info() + ", \"title\": \"Rock Paper Scissors Usage\",  \"fields\": [{ \"name\": \"`" + prefix + "rps [mod=" + mod + "] <@opponent|multi|multibot> [rounds=" + std::to_string(rounds) + "]`\", \"value\": \"To challenge someone to a game.\" }, { \"name\": \"Currently loaded mods:\", \"value\": \"" + rpsModInfo() + "\" }, { \"name\": \"Accepting and joining a game:\", \"value\": \"Use `" + prefix + "rps accept` if you were challenged directly.\\nUse `" + prefix + "rps join` to join a group match.\\nIf you initiated the group match, you can start the game early with `" + prefix + "rps start`\" }]}");
        return PLUGIN_HANDLED;
    }
    std::string authorNick = getServerMember(message.guild_id,message.author.id).nick;
    if (authorNick.size() < 1)
        authorNick = message.author.username;
    std::string arg1 = nospace(argv[1]);
    //std::string mod = RPS::mod;
    //int rounds = RPS::rounds;
    if (argc == 3)
    {
        std::string r = nospace(argv[3]);
        if (stringisnum(r,1))
        {
            rounds = std::stoi(r);
            argc--;
        }
        else
        {
            //global->callbacks.messageReply(message,"Rounds must be a number greater than 0.\nUsage: `" + prefix + "rps [mod] <@opponent> [multi] [rounds]` to challenge someone\nUsage: `" + prefix + "rps mods` to view loaded mods\nUsage: `" + prefix + "rps accept` to accept a challenge\nUsage: `" + prefix + "rps join` to join a group match\nUsage: `" + prefix + "rps start` to start a group match");
            sendEmbed(message.channel_id,"{\"color\": " + RPS::colors.Error(mod) + ", \"title\": \"Error: Rounds must be a number greater than 0!\", \"description\": \"Use `" + prefix + "rps` for help.\"}");
            return 1;
        }
    }
    if (argc == 2)
    {
        std::string m = arg1;
        if (rpsIsValidMod(m))
        {
            mod = m;
            arg1 = nospace(argv[2]);
            argc--;
        }
        else
        {
            //global->callbacks.messageReply(message,"Unknown mod: `" + arg1 + "`\nSee `" + prefix + "rps mods`");
            sendEmbed(message.channel_id,"{\"color\": " + RPS::colors.Error(mod) + ", \"title\": \"Error: Unknown mod `" + arg1 + "`!\", \"description\": \"See `" + prefix + "rps mods`\"}");
            return PLUGIN_HANDLED;
        }
    }
    if (argc == 1)
    {
        if ((message.mentions.size() < 1) && (arg1 != "multi") && (arg1 != "multibot") && (arg1 != "@everyone") && (arg1 != "@here"))
        {
            if (arg1 == "mods")
            {
                //global->callbacks.messageReply(message,"Currently loaded RPS mods:\n" + rpsModInfo());
                sendEmbed(message.channel_id,"{\"color\": " + RPS::colors.info + ", \"title\": \"Currently loaded RPS mods:\",  \"fields\": [" + rpsModInfo("",true) + "]}");
                return PLUGIN_HANDLED;
            }
            else if (arg1 == "accept")
            {
                std::vector<rpsGame>::iterator newGame;
                if (!rpsGameLookup(newGame,message.channel_id,message.author.id))
                {
                    messageReply(message,"You were not challenged to a game here!");
                    return PLUGIN_HANDLED;
                }
                if (newGame->accepted)
                {
                    messageReply(message,"Your game is already in progress! Why aren't you playing!?");
                    return PLUGIN_HANDLED;
                }
                if (newGame->multi)
                {
                    messageReply(message,"This is a group match, use `" + prefix + "rps join` to join!");
                    return PLUGIN_HANDLED;
                }
                newGame->accepted = true;
                rpsStartRound(newGame);
                messageReply(message,"The game has begun! Check your DM's!");
                return PLUGIN_HANDLED;
            }
            else if (arg1 == "join")
            {
                std::vector<rpsGame>::iterator newGame;
                if (!rpsGameLookup(newGame,message.channel_id))
                {
                    messageReply(message,"There is no game to join!");
                    return PLUGIN_HANDLED;
                }
                if (!newGame->multi)
                {
                    if (!rpsGameLookup(newGame,message.channel_id,message.author.id))
                        messageReply(message,"This is not a group match and you were not challenged to it!");
                    else
                        messageReply(message,"This is not a group match, use `" + prefix + "rps accept` to accept a challenge!");
                    return PLUGIN_HANDLED;
                }
                if (rpsGetPlayer(newGame,message.author.id) != newGame->player.end())
                {
                    messageReply(message,"You have already joined this match!");
                    return PLUGIN_HANDLED;
                }
                if (isPlaying(message.author.id))
                {
                    messageReply(message,"You are already playing a game!");
                    return PLUGIN_HANDLED;
                }
                if (newGame->accepted)
                {
                    messageReply(message,"You are too late to join this game!");
                    return PLUGIN_HANDLED;
                }
                Channel dmc = getDirectMessageChannel(message.author.id).object;
                rpsPlayer p;
                p.ID = message.author.id;
                p.DM = dmc.id;
                p.name = authorNick;
                newGame->expire = time(NULL)+120;
                newGame->player.push_back(p);
                reaction(message,"%E2%9C%85");
                return PLUGIN_HANDLED;
            }
            else if (arg1 == "start")
            {
                std::vector<rpsGame>::iterator newGame;
                if (!rpsGameLookup(newGame,message.channel_id))
                {
                    messageReply(message,"There isn't a game to start here!");
                    return PLUGIN_HANDLED;
                }
                if (!newGame->multi)
                {
                    messageReply(message,"This is not a group match!");
                    return PLUGIN_HANDLED;
                }
                if (newGame->accepted)
                {
                    messageReply(message,"This game is already in progress!");
                    return PLUGIN_HANDLED;
                }
                if (newGame->player[0].ID != message.author.id)
                {
                    messageReply(message,"Only the person who initiated the game can start it prematurely.");
                    return PLUGIN_HANDLED;
                }
                if (newGame->player.size() > 1)
                {
                    messageChannelID(newGame->chan,"The game has begun, check your DM's!");
                    rpsStartRound(newGame);
                }
                else
                {
                    messageChannelID(newGame->chan,"No one else wanted to play, " + newGame->player[0].name + "! :(");
                    RPS::rpsGames.erase(newGame);
                }
                return PLUGIN_HANDLED;
            }
            else
            {
                //global->callbacks.messageReply(message,"Unknown user `" + arg1 + "`\nUsage: `" + prefix + "rps [mod] <@opponent> [rounds]` to challenge someone\nUsage: `" + prefix + "rps mods` to view loaded mods\nUsage: `" + prefix + "rps accept` to accept a challenge\nUsage: `" + prefix + "rps join` to join a group match\nUsage: `" + prefix + "rps start` to start a group match");
                sendEmbed(message.channel_id,"{\"color\": " + RPS::colors.Error(mod) + ", \"title\": \"Error: Unknown user `" + arg1 + "`!\", \"description\": \"Use `" + prefix + "rps` for help.\"}");
                return PLUGIN_HANDLED;
            }
        }
        if (isChannelActive(message.channel_id))
        {
            messageReply(message,"This channel already has an active game in progress!");
            return PLUGIN_HANDLED;
        }
        if (isPlaying(message.author.id))
        {
            messageReply(message,"You are already playing a game!");
            return PLUGIN_HANDLED;
        }
        std::string oppID, oppName;
        if ((arg1 != "multi") && (arg1 != "multibot") && (arg1 != "@everyone") && (arg1 != "@here"))
        {
            oppID = rpsParseMention(arg1);
            //debugOut("rpsParseMention(" + arg1 + ") == " + oppID);
            //User opp = getUser(oppID).object;
            //oppName = opp.username;
            //oppName = getServerMember(message.guild_id,oppID).nick;
            if (oppID == message.author.id)
            {
                messageReply(message,randomComeback(message.author.username));
                return PLUGIN_HANDLED;
            }
            ServerMember t = getServerMember(message.guild_id,oppID);
            if (t.nick.size() > 0)
                oppName = t.nick;
            else
                oppName = t.user.username;
        }
        if (isPlaying(oppID))
        {
            messageReply(message,oppName + " is already in a game!");
            return PLUGIN_HANDLED;
        }
        rpsGame newGame;
        newGame.guild_id = message.guild_id;
        newGame.gameMode = mod;
        newGame.chan = message.channel_id;
        newGame.expire = time(NULL)+120;
        newGame.wins = rounds;
        Channel dmc = getDirectMessageChannel(message.author.id).object;
        rpsPlayer p;
        p.ID = message.author.id;
        p.DM = dmc.id;
        //p.name = message.author.username;
        p.name = authorNick;
        newGame.player.push_back(p);
        std::string round = "";
        if (rounds > 1)
            round = " first to " + std::to_string(rounds);
        if (oppID == RPS::myBotID)
        {
            //temp = rpsModInfo(mod,true);
            rpsPlayer p;
            p.ID = RPS::myBotID;
            p.DM = "";
            p.name = "I";
            newGame.player.push_back(p);
            newGame.accepted = true;
            //global->callbacks.messageReply(message,message.author.username + " has challenged me to a game of " + rpsModInfo(mod) + round + "!\nCheck your DM's, " + message.author.username);
            std::string out = "{\"color\": " + RPS::colors.Notify(mod) + ", \"title\": \"" + authorNick + " has challenged me to a game of " + rpsModInfo(mod) + round + "!\",  \"description\": \"";
            if (RPS::rpsConf.exists(mod,"desc"))
                out = out + RPS::rpsConf.find(mod,"desc") + "\\n";
            out = out + "Check your DM's, " + message.author.username + ".\"}";
            sendEmbed(message.channel_id,out);
        }
        else if (oppID.size() > 0)
        {
            Channel dmc = getDirectMessageChannel(oppID).object;
            rpsPlayer p;
            p.ID = oppID;
            p.DM = dmc.id;
            p.name = oppName;
            newGame.player.push_back(p);
            //global->callbacks.messageReply(message,message.author.username + " has challenged " + oppName + " to a game of " + rpsModInfo(mod) + round + "!\n" + oppName + " use `" + prefix + "rps accept` to accept the challenge!");
            std::string out = "{\"color\": " + RPS::colors.Notify(mod) + ", \"title\": \"" + authorNick + " has challenged " + oppName + " to a game of " + rpsModInfo(mod) + round + "!\",  \"description\": \"";
            if (RPS::rpsConf.exists(mod,"desc"))
                out = out + RPS::rpsConf.find(mod,"desc") + "\\n";
            out = out + "Use `" + prefix + "rps accept` to accept the challenge!\"}";
            sendEmbed(message.channel_id,out);
        }
        else
        {
            if ((arg1 == "multibot") || (arg1 == "@everyone") || (arg1 == "@here"))
            {
                rpsPlayer p;
                p.ID = RPS::myBotID;
                p.DM = "";
                //User self = getUser(RPS::myBotID).object;
                //p.name = self.username;
                ServerMember t = getServerMember(message.guild_id,RPS::myBotID);
                if (t.nick.size() > 0)
                    p.name = t.nick;
                else
                    p.name = t.user.username;
                newGame.player.push_back(p);
            }
            newGame.multi = true;
            newGame.expire += 30;
            //global->callbacks.messageReply(message,message.author.username + " has initiated a group game of " + rpsModInfo(mod) + round + "!\nUse `" + prefix + "rps join` if you would like to play!");
            std::string out = "{\"color\": " + RPS::colors.Notify(mod) + ", \"title\": \"" + authorNick + " has initiated a group game of " + rpsModInfo(mod) + round + "!\",  \"description\": \"";
            if (RPS::rpsConf.exists(mod,"desc"))
                out = out + RPS::rpsConf.find(mod,"desc") + "\\n";
            out = out + "Use `" + prefix + "rps join` if you would like to play!\"}";
            sendEmbed(message.channel_id,out);
        }
        RPS::rpsGames.push_back(newGame);
        if (oppID == RPS::myBotID)
        {
            std::vector<rpsGame>::iterator game;
            rpsGameLookup(game,message.channel_id,message.author.id);
            rpsStartRound(game);
        }
    }
    return PLUGIN_HANDLED;
}

int chatRPSResult(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global *global = recallGlobal();
    //Channel channel = getChannelCache(message.guild_id,message.channel_id);
    std::string prefix = global->prefix(message.guild_id);
    argc--;
    std::string mod = RPS::mod->getAsString(message.guild_id);
    if ((argc < 2) || (argc > 3))
    {
        //messageReply(message,"Usage: `" + prefix + "rpsresult [mod] <choiceA> <choiceB>`");
        sendEmbed(message.channel_id,"{\"color\": " + RPS::colors.info + ", \"title\": \"Usage: `" + prefix + "rpsresult [mod=" + mod + "] <choice1> <choice2>`\", \"description\": \"View the outcome of `choice1` vs `choice2` from `mod`!\"}");
        return 1;
    }
    std::string arg1 = nospace(argv[1]);
    std::string choices[2];
    if ((argc == 3) && (rpsIsValidMod(arg1)))
    {
        choices[0] = nospace(argv[2]);
        choices[1] = nospace(argv[3]);
        mod = arg1;
    }
    else if (argc == 2)
    {
        choices[0] = arg1;
        choices[1] = nospace(argv[2]);
        //mod = mod;
    }
    else
    {
        //messageReply(message,"Invalid mod: `" + arg1 + "`!");
        sendEmbed(message.channel_id,"{\"color\": " + RPS::colors.error + ", \"title\": \"Invalid mod: `" + arg1 + "`!\", \"fields\":[{ \"name\": \"Valid mods:\", \"value\": \"" + rpsModInfo() + "\"}]}");
        return PLUGIN_HANDLED;
    }
    std::string title, result, color = RPS::colors.Result(mod);
    int r = rpsGetWinner(mod,choices[0],choices[1],result);
    //std::string desc = "Player 1 chose " + choices[0] + " and Player 2 chose " + choices[1] + ".\\n";
    if (r < 0)
    {
        r = (r*-1)-1;
        sendEmbed(message.channel_id,"{\"color\": " + RPS::colors.Error(mod) + ", \"title\": \"Invalid option: `" + choices[r] + "`!\", \"fields\":[{ \"name\": \"Valid options for mod `" + mod + "`:\", \"value\": \"" + rpsModInfo(mod,true) + "\"}]}");
        return PLUGIN_HANDLED;
    }
    if (r == 0)
    {
        title = "The game ends in a draw!";
        color = RPS::colors.Draw(mod);
        //desc = desc.erase(desc.size()-2);
        std::string name = getServerMember(message.guild_id,message.author.id).nick;
        if (name.size() < 1)
            name = message.author.username;
        result = randomComeback(name);
    }
    else if (r == 1)
        title = "Player 1 wins!";
    else if (r == 2)
        title = "Player 2 Wins!";
    //messageReply(message,result);
    sendEmbed(message.channel_id,"{\"color\": " + color + ", \"title\": \"" + title + "\", \"description\": \"" + result + "\"}");
    return PLUGIN_HANDLED;
}

int chatRPSStatus(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global *global = recallGlobal();
    if (isChannelActive(message.channel_id))
    {
        std::vector<rpsGame>::iterator game;
        rpsGameLookup(game,message.channel_id);
        std::string mod = rpsModInfo(game->gameMode);
        std::string title = mod;
        if (game->wins > 1)
            title = title + " first to " + std::to_string(game->wins) + "!";
        if (game->multi)
            title = "group match of " + title;
        else
            title = "game of " + title;
        title = game->player[0].name + " initiated a " + title;
        std::string desc, fieldname, fieldvalue;
        if (!game->accepted)
        {
            if (game->multi)
                desc = "The game will begin in ";
            else
                desc = "The challenge will expire in ";
        }
        else
            desc = "The game will expire in ";
        Timer *timer;
        long ctime = long(time(NULL));
        if ((timer = handle.findTimer("rps_expire")) != nullptr)
        {
            //std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
            //std::chrono::high_resolution_clock::time_point checkTime = timer.last;
            //checkTime += std::chrono::seconds(timer.interval);
            //long check = std::chrono::duration_cast<std::chrono::seconds>(checkTime-now).count();
            long check = timer->remaining<std::chrono::seconds>().count();
            long expire = long(game->expire) - ctime;
            //consoleOut(timer.name + ": " + std::to_string(timer.interval) + ": " + std::to_string(expire) + " > " + std::to_string(check));
            for (;expire > check;check += timer->interval);
            desc = desc + std::to_string(check) + " second";
            if (check != 1)
                desc += 's';
            desc += '.';
        }
        else
        {
            if (long(game->expire) < ctime)
                desc = desc + "less than " + std::to_string(60-(ctime-long(game->expire))) + " seconds.";
            else
                desc = desc + "about " + std::to_string(game->expire-ctime) + " seconds.";
        }
        if (game->accepted)
        {
            fieldname = "Players in this game:";
            for (auto it = game->player.begin(), ite = game->player.end();it != ite;++it)
            {
                fieldvalue = fieldvalue + "**" + it->name + "**";
                if (it->choice.size() > 0)
                {
                    if ((it->ID == RPS::myBotID) && (it->name == "I"))
                        fieldvalue += " have made my selection.";
                    else
                        fieldvalue += " has made thier selection.";
                }
                else
                {
                    if ((it->ID == RPS::myBotID) && (it->name == "I"))
                        fieldvalue += " am still deciding. *Spooky.*";
                    else
                        fieldvalue += " is still deciding.";
                }
                fieldvalue += "\\n";
            }
            fieldvalue.erase(fieldvalue.size()-2);
        }
        std::string out = "{\"color\":" + RPS::colors.Info(mod) + ",\"title\":\"" + title + "\",\"description\":\"" + desc + "\"";
        if (fieldname.size() > 0)
            out = out + ",\"fields\":[{\"name\":\"" + fieldname + "\",\"value\":\"" + fieldvalue + "\"}]";
        out += "}";
        sendEmbed(message.channel_id,out);
    }
    else
        messageReply(message,"There is no active game here!");
    return PLUGIN_HANDLED;
}

int chatRPSReset(Handle &handle, int argc, const std::string argv[], const Message &message)
{
    Global *global = recallGlobal();
    if (isChannelActive(message.channel_id))
    {
        std::vector<rpsGame>::iterator game;
        rpsGameLookup(game,message.channel_id);
        RPS::rpsGames.erase(game);
        messageReply(message,"This channel's game has been reset.");
    }
    else
        messageReply(message,"There is no active game here!");
    return PLUGIN_HANDLED;
}

int rpsExpireCheck(Handle &handle, Timer *timer, void *args)
{
    //consoleOut(timer->name + " triggering.");
    Global *global = recallGlobal();
    if (RPS::rpsGames.size() > 0)
    {
        time_t ctime = time(NULL);
        for (auto it = RPS::rpsGames.begin();it != RPS::rpsGames.end();)
        {
            if (ctime >= it->expire)
            {
                if (it->multi)
                {
                    if (it->accepted)
                    {
                        for (auto player = it->player.begin();player != it->player.end();)
                        {
                            if (player->choice.size() < 1)
                            {
                                messageChannelID(it->chan,player->name + " did not respond, they have been removed from the game.");
                                player = it->player.erase(player);
                            }
                            else
                                ++player;
                        }
                        if (it->player.size() > 1)
                        {
                            rpsConclude(it);
                            it = RPS::rpsGames.begin();
                        }
                        else
                        {
                            if (it->player.size() == 1)
                                messageChannelID(it->chan,it->player[0].name + " wins!");
                            else
                                messageChannelID(it->chan,"Everyone died! :slight_frown:");
                            it = RPS::rpsGames.erase(it);
                        }
                    }
                    else
                    {
                        if (it->player.size() > 1)
                        {
                            messageChannelID(it->chan,"The game has begun, check your DM's!");
                            rpsStartRound(it++);
                        }
                        else
                        {
                            messageChannelID(it->chan,"No one else wanted to play, " + it->player[0].name + "! :slight_frown:");
                            it = RPS::rpsGames.erase(it);
                        }
                    }
                }
                else
                {
                    if (!it->accepted)
                        messageChannelID(it->chan,"Sorry " + it->player[0].name + ", it looks like " + it->player[1].name + " is busy, but you can always play with me :smiley:");
                    else
                        messageChannelID(it->chan,"There wasn't a response within a timely manor. The game between " + it->player[0].name + " and " + it->player[1].name + " ends in a draw!");
                    it = RPS::rpsGames.erase(it);
                }
            }
            else
                ++it;
        }
    }
    return 0;
}

bool isPlaying(const std::string &ID)
{
    for (auto &game: RPS::rpsGames)
        for (auto &player: game.player)
            if (player.ID == ID)
                return true;
    return false;
}

bool isChannelActive(const std::string &ID)
{
    for (auto &game: RPS::rpsGames)
        if (game.chan == ID)
            return true;
    return false;
}

bool rpsGameLookup(std::vector<rpsGame>::iterator &game, std::string channelID, std::string userID)
{
    for (auto it = RPS::rpsGames.begin(), ite = RPS::rpsGames.end();it != ite;++it)
    {
        game = it;
        if ((channelID.size() < 1) || (it->chan == channelID))
        {
            if (userID.size() > 0)
            {
                for (auto player = game->player.begin(), playere = game->player.end();player != playere;++player)
                    if (player->ID == userID)
                        return true;
            }
            else
                return true;
        }
    }
    return false;
}

std::string rpsModInfo(std::string mod, bool options)
{
    std::string ret = "";
    if (mod.size() < 1)
    {
        if (!options)
        {
            for (auto it = RPS::rpsConf.begin(), ite = RPS::rpsConf.end();it != ite;++it)
            {
                if (it->topic() == "config")
                    continue;
                ret = ret + "`" + it->topic() + "`: __" + it->find("info") + "__ written by " + it->find("author") + "\\n";
            }
            ret.erase(ret.size()-2);
        }
        else
        {
            for (auto it = RPS::rpsConf.begin(), ite = RPS::rpsConf.end();it != ite;++it)
            {
                if (it->topic() == "config")
                    continue;
                ret = ret + "{\"name\": \"`" + it->topic() + "`: " + it->find("info") + "\", \"value\": \"";
                if (it->exists("desc"))
                    ret = ret + it->find("desc") + "\\n";
                ret = ret + "Written by __" + it->find("author") + "__\"},";
            }
            ret.erase(ret.size()-1);
        }
    }
    else if (RPS::rpsConf.exists(mod))
    {
        if (!options)
            ret = RPS::rpsConf.find(mod,"info");
        else
            ret = RPS::rpsConf.find(mod,"opts");
    }
    return ret;
}

bool rpsIsValidMod(std::string &mod)
{
    mod = lower(mod);
    for (auto it = RPS::rpsConf.begin(), ite = RPS::rpsConf.end();it != ite;++it)
    {
        if (mod == lower(it->topic()))
        {
            mod = it->topic();
            return true;
        }
    }
    return false;
}

void rpsConclude(std::vector<rpsGame>::iterator &game)
{
    if (game->multi)
        rpsConcludeMulti(game);
    else
    {
        auto mod = RPS::rpsConf.topic_it(game->gameMode);
        std::string opts = mod->find("opts");
        std::string desc = game->player[0].name + " chose " + game->player[0].choice + " and " + game->player[1].name + " chose " + game->player[1].choice + ".";
        std::string title, result, decider, color = RPS::colors.Result(game->gameMode);
        if (game->player[0].choice == game->player[1].choice)
        {
            title = "The game ends in a draw!";
            color = RPS::colors.Draw(game->gameMode);
        }
        else if ((result = mod->find(game->player[0].choice,findtok(opts,game->player[1].choice,1," ")-1)) == "0")
        {
            game->player[0].wins++;
            result = mod->find(game->player[1].choice,findtok(opts,game->player[0].choice,1," ")-1);
            title = game->player[0].name + " wins!";
        }
        else if (game->player[1].ID != RPS::myBotID)
        {
            game->player[1].wins++;
            title = game->player[1].name + " wins!";
        }
        else
        {
            game->player[1].wins++;
            title = "You lose, " + game->player[0].name + ", better luck next time!";
            color = RPS::colors.Lose(game->gameMode);
        }
        decider = game->player[0].name + " has " + std::to_string(game->player[0].wins) + " win(s) and " + game->player[1].name;
        if (game->player[1].ID == RPS::myBotID)
            decider += " have ";
        else
            decider += " has ";
        result = strreplace(result,"\"","\\\"");
        decider = decider + std::to_string(game->player[1].wins) + "!";
        std::string chan = game->chan;
        bool nextRound = true;
        for (int i = 0;i < 2;i++)
        {
            if (game->player[i].wins >= game->wins)
            {
                if (game->wins > 1)
                {
                    if (game->player[i].ID == RPS::myBotID)
                        decider = game->player[i].name + " win the first to " + std::to_string(game->wins) + " wins!";
                    else
                    {
                        decider = game->player[i].name + " wins the first to " + std::to_string(game->wins) + " wins!";
                        color = RPS::colors.Win(game->gameMode);
                    }
                }
                RPS::rpsGames.erase(game);
                nextRound = false;
                break;
            }
        }
        if (nextRound)
            rpsStartRound(game);
        //recallGlobal()->callbacks.messageChannelID(chan,out);
        if (result.size() < 1)
            sendEmbed(chan,"{\"color\": " + color + ", \"title\": \"" + title + "\", \"description\": \"" + decider + "\"}");
        else
            sendEmbed(chan,"{\"color\": " + color + ", \"title\": \"" + title + "\", \"description\": \"" + desc + "\", \"fields\": [{\"name\": \"" + result + "\", \"value\": \"" + decider + "\"}]}");
    }
}

void rpsConcludeMulti(std::vector<rpsGame>::iterator &game)
{
    std::string title, name, results, tailname, tailvalue, temp, channel = game->chan, color = RPS::colors.Result(game->gameMode);
    if (game->player.size() == 2)
        name = game->player[0].name + " chose " + game->player[0].choice + " and " + game->player[1].name + " chose " + game->player[1].choice;
    else
    {
        for (auto player = game->player.begin(), playere = game->player.end();player != playere;++player)
        {
            if (player == playere-1)
                name += ", and ";
            else if (player != game->player.begin())
                name += ", ";
            name = name + player->name + " chose " + player->choice;
        }
    }
    name += ".";
    for (auto playerA = game->player.begin(), playere = game->player.end()-1;playerA != playere;++playerA)
    {
        for (auto playerB = playerA+1, playerBe = game->player.end();playerB != playerBe;++playerB)
        {
            bool noAdd = false;
            switch (rpsGetWinner(game->gameMode,playerA->choice,playerB->choice,temp))
            {
                case 1:
                {
                    playerA->multiWins++;
                    break;
                }
                case 2:
                {
                    playerB->multiWins++;
                    break;
                }
                default:
                    noAdd = true;
            }
            if (!noAdd)
                results = addtok(results,temp,"\\n");
        }
    }
    if (results.size() < 1)
    {
        results = "Looks like we have a mexican standoff!";
        color = RPS::colors.Draw(game->gameMode);
    }
    int w = 0;
    std::vector<std::string> realWinners;
    for (auto player = game->player.begin(), playere = game->player.end();player != playere;++player)
    {
        if (player->multiWins > w)
            w = player->multiWins;
    }
    std::vector<std::string> winners;
    if (w > 0)
    {
        for (auto player = game->player.begin(), playere = game->player.end();player != playere;++player)
        {
            if (player->multiWins == w)
            {
                winners.push_back(player->name);
                if (++player->wins >= game->wins)
                    realWinners.push_back(player->name);
            }
        }
    }
    else
        title = "Everyone has tied!";
    if (winners.size() > 1)
        color = RPS::colors.Draw(game->gameMode);
    if (winners.size() == 1)
        title = (*winners.begin()) + " wins!";
    else if (winners.size() == 2)
        title = winners[0] + " and " + winners[1] + " have tied!";
    else if (winners.size() > 2)
    {
        for (auto it = winners.begin(), ite = winners.end();it != ite;++it)
        {
            if (it == ite-1)
                title += ", and ";
            else if (it != winners.begin())
                title += ", ";
            title += *it;
        }
        title += " have all tied!";
    }
    if ((game->wins > 1) && (realWinners.size() > 0))
    {
        if (realWinners.size() == 1)
        {
            color = RPS::colors.Conclude(game->gameMode);
            tailname = realWinners[0] + " wins the first to " + std::to_string(game->wins) + " wins!";
            tailvalue = "Use `" + recallGlobal()->prefix(game->guild_id) + "rps` to play again!";
            RPS::rpsGames.erase(game);
        }
        else
        {
            color = RPS::colors.Knockout(game->gameMode);
            std::vector<std::string> losers;
            for (auto it = game->player.begin();it != game->player.end();)
            {
                bool dinner = false;
                for (auto chicken = realWinners.begin(), chickene = realWinners.end();chicken != chickene;++chicken)
                {
                    if (it->name == *chicken)
                    {
                        it->wins--;
                        dinner = true;
                        break;
                    }
                }
                if (!dinner)
                {
                    losers.push_back(it->name);
                    it = game->player.erase(it);
                }
                else
                    ++it;
            }
            if (losers.size() > 0)
            {
                tailname = "All losers have been knocked out of the game!";
                tailvalue = "Sorry ";
                if (losers.size() == 1)
                    tailvalue += losers[0];
                else if (losers.size() == 2)
                    tailvalue = tailvalue + losers[0] + " and " + losers[1];
                else
                {
                    for (auto it = losers.begin(), ite = losers.end();it != ite;++it)
                    {
                        if (it == ite-1)
                            tailvalue += ", and ";
                        else if (it != losers.begin())
                            tailvalue += ", ";
                        tailvalue += *it;
                    }
                }
                tailvalue += " :slight_frown:";
            }
            else
            {
                tailname = "No one has been knocked out!";
                tailvalue = "Sudden death!";
            }
            rpsStartRound(game);
        }
    }
    else if (game->wins > 1)
    {
        int top = 0;
        for (auto it = game->player.begin(), ite = game->player.end();it != ite;++it)
            if (it->wins > top)
                top = it->wins;
        if (top > 0)
        {
            for (auto it = game->player.begin(), ite = game->player.end();it != ite;++it)
                if (it->wins >= top)
                    tailvalue = tailvalue + it->name + "\\n";
            tailname = "Current leaderboard with " + std::to_string(top) + " win";
            if (top > 1)
                tailname += "s";
            tailname += ":";
            tailvalue.erase(tailvalue.size()-2);
        }
        rpsStartRound(game);
    }
    //recallGlobal()->callbacks.messageChannelID(channel,out);
    std::string out = "{\"color\": " + color + ",\"title\":\"" + title + "\",\"fields\":[{\"name\":\"" + name + "\",\"value\":\"" + results + "\"}";
    if (tailname.size() > 0)
        out = out + ",{\"name\":\"" + tailname + "\",\"value\":\"" + tailvalue + "\"}]}";
    else
        out += "]}";
    sendEmbed(channel,out);
}

int rpsGetWinner(const std::string &mode, const std::string &v1, const std::string &v2, std::string &result)
{
    auto mod = RPS::rpsConf.topic_it(mode);
    std::string opts = mod->find("opts");
    int ret;
    std::string str;
    if (!istok(opts,v1," "))
        ret = -1;
    else if (!istok(opts,v2," "))
        ret = -2;
    else if (v1 == v2)
    {
        ret = 0;
        result.clear();
    }
    else if ((str = mod->find(v1,findtok(opts,v2,1," ")-1)) == "0")
    {
        ret = 1;
        result = mod->find(v2,findtok(opts,v1,1," ")-1);
    }
    else
    {
        ret = 2;
        result = str;
    }
    result = strreplace(result,"\"","\\\"");
    return ret;
}

std::string rpsParseMention(std::string str)
{
    str.erase(str.size()-1);
    str.erase(0,2);
    if (str.front() == '!')
        str.erase(0,1);
    return str;
}

void rpsStartRound(std::vector<rpsGame>::iterator game)
{
    game->expire = time(NULL)+120;
    game->accepted = true;
    std::string temp = rpsModInfo(game->gameMode,true);
    for (auto player = game->player.begin();player != game->player.end();)
    {
        player->choice.clear();
        player->multiWins = 0;
        if (player->ID != RPS::myBotID)
        {
            auto response = messageChannelID(player->DM,"Respond with one of the following:\n" + temp);
            if (response.response.error())
            {
                if (game->player.size() > 2)
                {
                    messageChannelID(game->chan,player->name + " has been removed from the match, due to their privacy settings!");
                    player = game->player.erase(player);
                }
                else
                {
                    messageChannelID(game->chan,"The game has been cancelled due to " + player->name + "'s privacy settings!");
                    RPS::rpsGames.erase(game);
                    return;
                }
            }
            else
                ++player;
        }
        else
        {
            player->choice = randomtok(temp," ");
            ++player;
        }
    }
}

std::vector<rpsPlayer>::iterator rpsGetPlayer(std::vector<rpsGame>::iterator &game, const std::string &ID)
{
    auto ite = game->player.end();
    for (auto it = game->player.begin();it != ite;++it)
        if (it->ID == ID)
            return it;
    return ite;
}

bool rpsPlayersReady(std::vector<rpsGame>::iterator &game)
{
    int ready = 0;
    for (auto it = game->player.begin(), ite = game->player.end();it != ite;++it)
        if (it->choice.size() > 0)
            ready++;
    if (ready >= game->player.size())
        return true;
    return false;
}

std::string randomComeback(const std::string &name)
{
    static const int MAXSOLO = 9;
    static const std::string SOLO[MAXSOLO] =
    {
        "You may have enough hands, but please don't play with yourself here...",
        "Let's keep it PG please...",
        "[name] has wandered off to play with themself.",
        "_whistles_",
        "Are you expecting me to help you with that?",
        "*Look Ma both hands!*",
        ":unamused:",
        "Now ignoring all messages from [name].",
        "_note to self:_ Ban [name] as soon as nigel gives me privileges."
    };
    return strreplace(SOLO[mtrand(0,MAXSOLO-1)],"[name]",name);
}


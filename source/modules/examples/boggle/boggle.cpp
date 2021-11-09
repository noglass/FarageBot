#include "api/farage.h"
#include <random>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include "shared/regex.h"
using namespace Farage;

#define MAKEMENTION
#include "common_func.h"

#define VERSION "v0.2.2"

extern "C" Info Module
{
    "Boggle",
    "Muddy",
    "Classic Multiplayer Boggle Game",
    VERSION,
    "http://boggle.justca.me/",
    FARAGE_API_VERSION
};

namespace boggle
{
    const char dice[16][7] =
    {
        "ACHSOP",
        "NUSIEE",
        "FAKPSF",
        "SOTSIE",
        "RLNZNH",
        "DIXLER",
        "TYSTID",
        "MUNIHQ",
        "WRVEHT",
        "BOOBAJ",
        "LERYTT",
        "NEWGHE",
        "VEYLDR",
        "AEEGAN",
        "COTUMI",
        "TOOWAT"
    };
    
    const int positions[16][2] =
    {
        { 112, 84 },
        { 178, 88 },
        { 237, 86 },
        { 299, 87 },
        { 112, 147 },
        { 176, 150 },
        { 237, 149 },
        { 300, 150 },
        { 112, 212 },
        { 176, 213 },
        { 236, 209 },
        { 301, 211 },
        { 112, 270 },
        { 180, 272 },
        { 236, 270 },
        { 299, 275 }
    };
    
    const int DIRECTIONS[8][3] =
    {
        {-1,-1,-5},
        {-1,0,-4},
        {-1,1,-3},
        {0,-1,-1},
        {0,1,1},
        {1,-1,3},
        {1,0,4},
        {1,1,5}
    };
    
    struct Player
    {
        Player(User author, std::string guild)
        {
            ID = author.id;
            DM = getDirectMessageChannel(author.id).object.id;
            name = getServerMember(guild,author.id).nick;
            if (name.size() < 1)
                name = author.username;
            consoleOut("DM Channel with " + name + ": " + DM);
        }
        Player() {}
        std::string ID;
        std::string DM;
        std::string name;
        int total;
        std::unordered_map<std::string,int> words;
    };
    
    struct Game
    {
        Game(std::string channel, std::string guild) : chan(channel), guild_id(guild)
        {
            expire = time(NULL)+120;
        }
        Game() {}
        std::string chan;
        std::string guild_id;
        std::string board;
        std::string board_url;
        time_t expire;
        std::vector<Player> player;
        bool running = false;
    };
    
    std::vector<Game> games;
    GlobVar *wobbleVar = nullptr;
    
    std::string upper(std::string text)
    {
        if (text.size() > 0)
            for (std::string::iterator it = text.begin(), ite = text.end();it != ite;++it)
                *it = toupper(*it);
        return std::move(text);
    }
    
    int32_t signedrand(int32_t lo, int32_t hi)
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
        int32_t r;
        if (lo >= hi)
            r = lo;
        else
            r = (std::uniform_int_distribution<int32_t>(lo,hi))(mt);
        return r;
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
        return output;
    }
    
    std::string getBoard()
    {
        std::string b, board;
        for (size_t i = 0;i < 16;++i)
            b += dice[i][signedrand(0,5)];
        for (size_t i = 0, j = signedrand(0,b.size()-1);i < 16;++i,j = signedrand(0,b.size()-1))
        {
            board += b.at(j);
            b.erase(j,1);
        }
        return std::move(board);
    }
    
    std::string insertLetter(const std::string& board, size_t pos, int rotate)
    {
        std::string geo = "+";
        int rot = 0;
        if (!(rotate & 1))
        {
            switch (signedrand(0,6))
            {
                case 0:
                case 1:
                case 2:
                case 3:
                    rot = 0;
                    break;
                case 4:
                    rot = 1;
                    break;
                case 5:
                    rot = 2;
                    break;
                default:
                    rot = 3;
            }
            if (rot & 1)
                geo += std::to_string(positions[pos][0]-5) + '+' + std::to_string(positions[pos][1]+5);
            else
                geo += std::to_string(positions[pos][0]) + '+' + std::to_string(positions[pos][1]);
        }
        else
            geo += std::to_string(positions[pos][0]) + '+' + std::to_string(positions[pos][1]);
        rot *= 90;
        if ((!(rotate & 2)) && ((rot += signedrand(-4,4)) < 0))
            rot += 360;
        std::string out = "\\( source/modules/examples/boggle/assets/letters/";
        out += board.at(pos);
        out += ".png -background None -rotate " + std::to_string(rot) + " -gravity NorthWest -geometry " + geo + " \\) -composite ";
        return std::move(out);
    }
    
    std::string buildCommand(const std::string& board, int rot = 0)
    {
        std::string out = "convert -size 450x400 xc:none ";
        for (size_t i = 0;i < 16;++i)
            out += insertLetter(board,i,rot);
        out += "\\( source/modules/examples/boggle/assets/board.png -compose DstOver \\) -composite gameboard.png 2>&1";
        return std::move(out);
    }
    
    bool inBounds(int pos, int d)
    {
        int x = pos / 4 + DIRECTIONS[d][0];
        int y = pos % 4 + DIRECTIONS[d][1];
        return ((0 <= x) && (x < 4) && (0 <= y) && (y < 4));
    }
    
    int getPoints(size_t i)
    {
        switch (i)
        {
            case 3:
            case 4:
                return 1;
            case 5:
                return 2;
            case 6:
                return 3;
            case 7:
                return 5;
        }
        return 11;
    }
    
    int reValidateWord(const std::string& board, const std::string& word, size_t i, int cpos, std::unordered_set<size_t> places)
    {
        if (i == word.size())
        {
            if (word.find('Q') != std::string::npos)
                ++i;
            return getPoints(i);
        }
        for (size_t d = 0;d < 8;++d)
        {
            int zpos = cpos + DIRECTIONS[d][2];
            if ((inBounds(cpos,d)) && (word.at(i) == board.at(zpos)) && (places.find(zpos) == places.end()))
            {
                places.emplace(zpos);
                int r = reValidateWord(board,word,i+1,zpos,places);
                if (r)
                    return r;
                else
                    places.erase(zpos);
            }
        }
        return 0;
    }
    
    int validateWord(const std::string& board, std::string word)
    {
        for (size_t i = 0;(i = word.find("QU",i)) != std::string::npos;word.erase(++i));
        if ((word.size() > 16) || (word.size() < 3))
            return 0;
        for (size_t pos = 0;(pos = board.find(word.front(),pos)) != std::string::npos;++pos)
        {
            int r = reValidateWord(board,word,1,pos,{pos});
            if (r)
                return r;
        }
        return 0;
    }
    
    bool gameLookup(std::vector<Game>::iterator &game, std::string channelID, std::string userID)
    {
        for (auto it = games.begin(), ite = games.end();it != ite;++it)
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
    
    std::vector<Player>::iterator getPlayer(std::vector<Game>::iterator &game, const std::string &ID)
    {
        auto ite = game->player.end();
        for (auto it = game->player.begin();it != ite;++it)
            if (it->ID == ID)
                return it;
        return ite;
    }
    
    void concludeGame(std::vector<Game>::iterator game)
    {
        std::unordered_set<std::string> words, dupes;
        for (auto& player : game->player)
        {
            for (auto& word : player.words)
                if (words.emplace(word.first).second == false)
                    dupes.emplace(word.first);
        }
        for (auto& player : game->player)
        {
            auto wend = player.words.end();
            for (auto& dupe : dupes)
            {
                auto w = player.words.find(dupe);
                if (w != wend)
                {
                    player.words.emplace("~~" + w->first + "~~",0);
                    player.words.erase(w);
                }
            }
        }
        std::string name;
        int points = 0;
        bool tie = false;
        for (auto& player : game->player)
        {
            std::string fields;
            int valid = 0;
            player.total = 0;
            for (auto& word : player.words)
            {
                fields += "{\"name\": \"" + word.first + "\", \"value\": \"" + std::to_string(word.second) + "\", \"inline\": true},";
                player.total += word.second;
                valid += (word.second != 0);
            }
            if (fields.size() > 0)
                fields.erase(fields.size()-1);
            if (player.total > points)
            {
                name = player.name;
                points = player.total;
                tie = false;
            }
            else if (player.total == points)
            {
                name = name + " & " + player.name;
                tie = true;
            }
            sendEmbed(game->chan,"{\"color\": 44269, \"title\": \"" + player.name + "'s Boggle Results!\", \"description\": Total Valid Words: " + std::to_string(valid) + " - Total Points: " + std::to_string(player.total) + "!\", \"fields\": [" + fields + "]}");
        }
        if (tie)
            sendMessage(game->chan,"Congratulations to " + name + "! You won with " + std::to_string(points) + "!!");
        else
            sendMessage(game->chan,"Congratulations to " + name + "! You have all tied with " + std::to_string(points) + "!!");
        games.erase(game);
    }
    
    int timerEndGame(Handle& handle, Timer* timer, void* nil)
    {
        std::vector<Game>::iterator game;
        if (gameLookup(game,timer->name,""))
            concludeGame(game);
        else
            sendMessage(timer->name,"An unknown Boggle error has occurred . . . :(");
        return 1;
    }
    
    void startGame(Handle& handle, std::vector<Game>::iterator game)
    {
        Global *global = recallGlobal();
        game->running = true;
        int rot = wobbleVar->getAsInt(game->guild_id);
        game->board = getBoard();
        std::string command = buildCommand(game->board,rot);
        //consoleOut("Building Boggle board: '" + board + "': " + command);
        std::string result = exec(command,1024);
        if (result.size() > 0)
        {
            sendMessage(game->chan,"Error!\n```\n" + result + "\n```");
            games.erase(game);
        }
        else
        {
            auto response = sendFile(game->chan,"gameboard.png","","{\"color\": 44269, \"image\": { \"url\": \"attachment://gameboard.png\" }, \"title\": \"Boggle\", \"description\": \"Type `" + global->prefix(game->guild_id) + "boggle` to join!\"}");
            if ((!response.response.error()) && (response.object.attachments.size() > 0))
            {
                game->board_url = response.object.attachments.front().url;
                //consoleOut(game->board_url);
                std::cout<<response.object.attachments.front().url<<std::endl;
            }
            else
                consoleOut("Error!");
            for (auto player = game->player.begin();player != game->player.end();++player)
            {
                /*auto response = */sendEmbed(player->DM,"{\"color\": 44269, \"title\": \"Boggle!\", \"description\": \"Send all your words here. You can send multiple words separated with whitespace.\", \"image\": { \"url\": \"" + game->board_url + "\"}}");
                /*if (response.response.error())
                {
                    //if (game->player.size() > 2)
                    {
                        messageChannelID(game->chan,player->name + " has been removed from the match, due to their privacy settings!");
                        player = game->player.erase(player);
                    }*/
                    /*else
                    {
                        messageChannelID(game->chan,"The game has been cancelled due to " + player->name + "'s privacy settings!");
                        games.erase(game);
                        return;
                    }*/
                /*}
                else
                    ++player;*/
            }
            handle.createTimer(game->chan,180,&timerEndGame);
        }
    }
    
    int timerStartGame(Handle& handle, Timer* timer, void* nil)
    {
        std::vector<Game>::iterator game;
        if (gameLookup(game,timer->name,""))
        {
            if (game->running == false)
                startGame(handle,game);
        }
        else
            sendMessage(timer->name,"An unknown Boggle error has occurred! :(");
        return 1;
    }
};

int boggleCmd(Handle&, int, const std::string[], const Message&);

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("boggle_version",VERSION,"Boggle Version",GVAR_CONSTANT);
    boggle::wobbleVar = handle.createGlobVar("boggle_wobble","0","Control the rotation and wobble of the letters on the board. 1 = no 90 or 180 degree rotation, 2 = no variable wobble, 3 = neither, 0 = both.",GVAR_DUPLICATE|GVAR_STORE,true,0.0,true,3.0);
    handle.regChatCmd("boggle",&boggleCmd,NOFLAG,"Play Boggle.");
    return 0;
}

int boggleCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global *global = recallGlobal();
    std::vector<boggle::Game>::iterator game;
    if (boggle::gameLookup(game,message.channel_id,""))
    {
        auto player = boggle::getPlayer(game,message.author.id);
        if (player == game->player.end()) // player is not in game
        {
            std::vector<boggle::Game>::iterator otherGame;
            if (boggle::gameLookup(otherGame,"",message.author.id)) // player is already in another game
                sendMessage(message.channel_id,"You are already in a game in <#" + otherGame->chan + ">!");
            else // add player to game
            {
                game->player.emplace_back(message.author,message.guild_id);
                sendEmbed(game->player.back().DM,"{\"color\": 44269, \"title\": \"Boggle!\", \"description\": \"Send all your words here. You can send multiple words separated with whitespace.\", \"image\": { \"url\": \"" + game->board_url + "\"}}");
                reaction(message,"%E2%9C%85");
            }
        }
        else // player is already in game
        {
            if ((argc > 1) && (strlower(argv[1]) == "start"))
                boggle::startGame(handle,game);
            else
                sendMessage(message.channel_id,"You are already in this game! Type `" + global->prefix(message.guild_id) + argv[0] + " start` to start the game now.");
        }
    }
    else // make a new game
    {
        boggle::Game newGame(message.channel_id,message.guild_id);
        newGame.player.emplace_back(message.author,message.guild_id);
        boggle::games.push_back(newGame);
        sendMessage(message.channel_id,makeMention(message.author.id,message.guild_id) + " has initiated a game of Boggle! Type `" + global->prefix(message.guild_id) + argv[0] + "` to play!");
        handle.createTimer(message.channel_id,120,&boggle::timerStartGame);
    }
    return PLUGIN_HANDLED;
}


extern "C" int onMessage(Handle& handle, Event event, void* message, void* nil, void* foo, void* bar)
{
    static const rens::regex split ("^\\s*(\\S+)");
    Global *global = recallGlobal();
    Message *msg = (Message*)message;
    //Channel channel = getChannel(msg->channel_id).object;
    if ((msg->author.id == global->self.id) || (msg->guild_id.size() > 0))
        return PLUGIN_CONTINUE;
    std::vector<boggle::Game>::iterator game;
    if (boggle::gameLookup(game,"",msg->author.id))
    {
        auto player = boggle::getPlayer(game,msg->author.id);
        if (msg->channel_id == player->DM)
        {
            if (!game->running)
                return PLUGIN_CONTINUE;
            std::string words = boggle::upper(msg->content);
            rens::smatch ml;
            std::string invalid;
            int w = 0, v = 0;
            for (;rens::regex_search(words,ml,split);words = ml.suffix().str())
            {
                ++w;
                std::string word = ml[1].str();
                if (player->words.find(word) != player->words.end())
                    invalid = invalid + ' ' + word + "(you already found it!)";
                else
                {
                    int points = boggle::validateWord(game->board,word);
                    if (points)
                    {
                        player->words.emplace(word,points);
                        ++v;
                    }
                    else
                        invalid = invalid + ' ' + word;
                }
            }
            if (v)
                reaction(*msg,"%E2%9C%85");
            if (invalid.size() > 0)
                sendMessage(msg->channel_id,"The following words were deemed invalid:" + invalid);
            return PLUGIN_HANDLED;
        }
    }
    return PLUGIN_CONTINUE;
}



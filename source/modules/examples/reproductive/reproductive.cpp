#include "api/farage.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
//#include <locale>
//#include <codecvt>
#include "shared/libini.h"
using namespace Farage;

#define MAKEMENTION
#define SPLITSTRING
#include "common_func.h"

#define VERSION "v0.4.5"

#define SOUNDEXTHRESHOLD    75
#define SOUNDEXTHRESHOLD2   25

extern "C" Info Module
{
    "Reproductive Bot Organs",
    "Madison",
    "Interactive Chat",
    VERSION,
    "http://farage.justca.me/",
    FARAGE_API_VERSION
};

namespace dictionary
{
    enum speech
    {
        unknown,
        noun,
        verb,
        pronoun,
        adjective,
        adverb,
        preposition,
        conjunction,
    };
    enum context
    {
        none,
        possessive,
        plural,
        singular,
        present,
        future,
        past,
    };
    struct soundex
    {
        char code = -1;
        int coder;
        int coded;
        std::string full;
    };
    struct definition
    {
        std::string word;
        speech part = unknown;
        int derogative = 0;
        context clues = none;
        std::string def;
        soundex phonetic;
        std::unordered_map<std::string,definition*> synonyms;
        std::unordered_map<std::string,definition*> antonyms;
    };
    soundex phonetize(std::string word)
    {
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
        //std::string encoded;
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
    
    struct diction
    {
        std::unordered_map<std::string,definition> master;
        std::unordered_map<std::string,definition*> nouns;
        std::unordered_map<std::string,definition*> verbs;
        std::unordered_map<std::string,definition*> pronouns;
        std::unordered_map<std::string,definition*> adjectives;
        std::unordered_map<std::string,definition*> adverbs;
        std::unordered_map<std::string,definition*> prepositions;
        std::unordered_map<std::string,definition*> conjunctions;
    }dict;
    
    size_t loadDic(const std::string& path = "dictionary.ini")
    {
        INIObject ini(path);
        for (auto it = ini.begin(), ite = ini.end();it != ite;++it)
        {
            definition entry;
            entry.word = it->topic();
            if (it->exists("speech"))
                entry.part = speech(std::stoi(it->find("speech")));
            if (it->exists("derogative"))
                entry.derogative = std::stoi(it->find("derogative"));
            if (it->exists("context"))
                entry.clues = context(std::stoi(it->find("context")));
            entry.def = it->find("definition");
            entry.phonetic = std::move(phonetize(entry.word));
            auto where = dict.master.emplace(entry.word,entry);
            switch (entry.part)
            {
                case noun:
                {
                    dict.nouns.emplace(entry.word,&where.first->second);
                    break;
                }
                case verb:
                {
                    dict.verbs.emplace(entry.word,&where.first->second);
                    break;
                }
                case pronoun:
                {
                    dict.pronouns.emplace(entry.word,&where.first->second);
                    break;
                }
                case adjective:
                {
                    dict.adjectives.emplace(entry.word,&where.first->second);
                    break;
                }
                case adverb:
                {
                    dict.adverbs.emplace(entry.word,&where.first->second);
                    break;
                }
                case preposition:
                {
                    dict.prepositions.emplace(entry.word,&where.first->second);
                    break;
                }
                case conjunction:
                    dict.conjunctions.emplace(entry.word,&where.first->second);
            }
        }
        for (auto it = ini.begin(), ite = ini.end();it != ite;++it)
        {
            definition* entry = &dict.master[it->topic()];
            int s;
            std::string* syn = splitString(it->find("synonym")," ",s);
            if (syn != nullptr) for (--s;s >= 0;--s)
            {
                if (syn[s].size() < 1)
                    continue;
                entry->synonyms.emplace(syn[s],&dict.master[syn[s]]);
            }
            delete[] syn;
            syn = splitString(it->find("antonym")," ",s);
            if (syn != nullptr) for (--s;s >= 0;--s)
            {
                if (syn[s].size() < 1)
                    continue;
                entry->antonyms.emplace(syn[s],&dict.master[syn[s]]);
            }
            delete[] syn;
        }
        return dict.master.size();
    }
    int saveDic(const std::string& path = "dictionary.ini")
    {
        std::ofstream file(path,std::ios::out|std::ios::trunc);
        if (file.is_open())
        {
            for (auto it = dict.master.begin(), ite = dict.master.end();it != ite;++it)
            {
                file<<'['<<it->second.word<<']'<<std::endl;
                file<<"speech="<<it->second.part<<std::endl;
                file<<"derogative="<<it->second.derogative<<std::endl;
                file<<"context="<<it->second.clues<<std::endl;
                file<<"definition="<<it->second.def<<std::endl;
                file<<"synonym=";
                for (auto s = it->second.synonyms.begin(), se = it->second.synonyms.end();s != se;++s)
                    file<<s->first<<' ';
                file<<std::endl<<"antonym=";
                for (auto s = it->second.antonyms.begin(), se = it->second.antonyms.end();s != se;++s)
                    file<<s->first<<' ';
                file<<std::endl;
            }
            file.close();
            return 0;
        }
        return 1;
    }
    inline int abs(int n)
    {
        if (n < 0)
            return n*-1;
        return n;
    }
    definition& lookupWord(std::string word)
    {
        //std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        //std::wstring test = converter.from_bytes(word);
        //if (test.size() >= word.size())
        word = strlower(word);
        auto entry = dict.master.find(word);
        if (entry != dict.master.end())
            return entry->second;
        std::vector<std::unordered_map<std::string,definition>::iterator> options;
        soundex code = std::move(phonetize(word));
        bool exact = false;
        for (auto it = dict.master.begin(), ite = dict.master.end();it != ite;++it)
        {
            if (it->second.phonetic.full == code.full)
            {
                exact = true;
                options.push_back(it);
            }
            else if (!exact)
            {
                int difference = abs(code.coded-it->second.phonetic.coded);
                if ((it->second.phonetic.code == code.code) && (difference < SOUNDEXTHRESHOLD))
                    options.push_back(it);
                else if ((it->second.phonetic.coder == code.coder) && (difference < SOUNDEXTHRESHOLD2))
                    options.push_back(it);
            }
        }
        if (options.size() > 0)
        {
            uint8_t closest = -1;
            for (auto it = options.begin(), ite = options.end();it != ite;++it)
            {
                int difference;
                if (exact)
                    difference = abs(word.size()-(*it)->first.size());
                else
                    difference = abs(code.coded-(*it)->second.phonetic.coded);
                if (difference < closest)
                    closest = difference;
            }
            for (auto it = options.begin(), ite = options.end();it != ite;++it)
                if (abs(code.coded-(*it)->second.phonetic.coded) == closest)
                    return (*it)->second;
        }
        definition* def = &dict.master[word];
        def->word = word;
        return *def; // return a new dictionary entry
    }
    void addWord(const definition& word, bool overwrite = true)
    {
        auto entry = dict.master.insert({word.word,word});
        if ((!entry.second) && (overwrite))
        {
            if (word.part != entry.first->second.part)
            {
                switch (entry.first->second.part)
                {
                    case noun:
                    {
                        dict.nouns.erase(word.word);
                        break;
                    }
                    case verb:
                    {
                        dict.verbs.erase(word.word);
                        break;
                    }
                    case pronoun:
                    {
                        dict.pronouns.erase(word.word);
                        break;
                    }
                    case adjective:
                    {
                        dict.adjectives.erase(word.word);
                        break;
                    }
                    case adverb:
                    {
                        dict.adverbs.erase(word.word);
                        break;
                    }
                    case preposition:
                    {
                        dict.prepositions.erase(word.word);
                        break;
                    }
                    case conjunction:
                        dict.conjunctions.erase(word.word);
                }
            }
            entry.first->second = word;
        }
        if ((entry.second) || ((!entry.second) && (overwrite))) switch (word.part)
        {
            case noun:
            {
                dict.nouns.emplace(word.word,&entry.first->second);
                break;
            }
            case verb:
            {
                dict.verbs.emplace(word.word,&entry.first->second);
                break;
            }
            case pronoun:
            {
                dict.pronouns.emplace(word.word,&entry.first->second);
                break;
            }
            case adjective:
            {
                dict.adjectives.emplace(word.word,&entry.first->second);
                break;
            }
            case adverb:
            {
                dict.adverbs.emplace(word.word,&entry.first->second);
                break;
            }
            case preposition:
            {
                dict.prepositions.emplace(word.word,&entry.first->second);
                break;
            }
            case conjunction:
                dict.conjunctions.emplace(word.word,&entry.first->second);
        }
    }
    void removeWord(std::unordered_map<std::string,definition>::iterator entry)
    {
        switch (entry->second.part)
        {
            case noun:
            {
                dict.nouns.erase(entry->first);
                break;
            }
            case verb:
            {
                dict.verbs.erase(entry->first);
                break;
            }
            case pronoun:
            {
                dict.pronouns.erase(entry->first);
                break;
            }
            case adjective:
            {
                dict.adjectives.erase(entry->first);
                break;
            }
            case adverb:
            {
                dict.adverbs.erase(entry->first);
                break;
            }
            case preposition:
            {
                dict.prepositions.erase(entry->first);
                break;
            }
            case conjunction:
                dict.conjunctions.erase(entry->first);
        }
        dict.master.erase(entry);
    }
    definition* getSpecific(const std::unordered_map<std::string,definition*>& dictit, int derogativity, context clue = context(-1))
    {
        int distance;
        uint8_t closest = -1;
        bool withClue = false;//(clue > context(-1));
        for (auto it = dictit.begin(), ite = dictit.end();it != ite;++it)
        {
            if ((withClue) && (it->second->clues != clue))
                continue;
            distance = derogativity-it->second->derogative;
            if (distance < 0)
                distance *= -1;
            if (distance < closest)
                closest = distance;
        }
        std::vector<definition*> options;
        for (auto it = dictit.begin(), ite = dictit.end();it != ite;++it)
        {
            if ((withClue) && (it->second->clues != clue))
                continue;
            distance = derogativity-it->second->derogative;
            if (distance < 0)
                distance *= -1;
            if (distance == closest)
                options.push_back(it->second);
        }
        if (options.size() > 0)
            return options.at(mtrand(0,options.size()-1));
        return nullptr;
    }
    definition* getAny(int derogativity, context clue = context(-1))
    {
        int distance;
        uint8_t closest = -1;
        bool withClue = false;//(clue > context(-1));
        for (auto it = dict.master.begin(), ite = dict.master.end();it != ite;++it)
        {
            if ((withClue) && (it->second.clues != clue))
                continue;
            distance = derogativity-it->second.derogative;
            if (distance < 0)
                distance *= -1;
            if (distance < closest)
                closest = distance;
        }
        std::vector<definition*> options;
        for (auto it = dict.master.begin(), ite = dict.master.end();it != ite;++it)
        {
            if ((withClue) && (it->second.clues != clue))
                continue;
            distance = derogativity-it->second.derogative;
            if (distance < 0)
                distance *= -1;
            if (distance == closest)
                options.push_back(&it->second);
        }
        if (options.size() > 0)
            return options.at(mtrand(0,options.size()-1));
        return nullptr;
    }
    
    speech partOfSpeech(const std::string &str)
    {
        switch (str.front())
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': return speech(std::stoi(str));
            case 'n': return speech::noun;
            case 'v': return speech::verb;
            case 'p':
            {
                if (str.size() > 2)
                {
                    if (str.at(2) == 'o')
                        return speech::pronoun;
                    if (str.at(2) == 'e')
                        return speech::preposition;
                }
                break;
            }
            case 'a':
            {
                if (str.size() > 2)
                {
                    if (str.at(2) == 'j')
                        return speech::adjective;
                    if (str.at(2) == 'v')
                        return speech::adverb;
                }
                break;
            }
            case 'c': return speech::conjunction;
        }
        return speech::unknown;
    }
    
    std::string partOfSpeechStr(speech part)
    {
        switch (part)
        {
            case noun:          return "noun";
            case verb:          return "verb";
            case pronoun:       return "pronoun";
            case adjective:     return "adjective";
            case adverb:        return "adverb";
            case preposition:   return "preposition";
            case conjunction:   return "conjunction";
        }
        return "unknown";
    }
    
    GlobVar* channel = nullptr;
    GlobVar* enable = nullptr;
};

int dictCmd(Handle&,int,const std::string[],const Message&);
int dictAddCmd(Handle&,int,const std::string[],const Message&);
int dictAddMassCmd(Handle&,int,const std::string[],const Message&);
int dictDelCmd(Handle&,int,const std::string[],const Message&);
int dictSaveCmd(Handle&,int,const std::string[],const Message&);
int dictReloadCmd(Handle&,int,const std::string[],const Message&);
int dictTestCmd(Handle&,int,const std::string[],const Message&);
int dictStatsCmd(Handle&,int,const std::string[],const Message&);

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("reproductive_version",VERSION,"Reproductive Bot Organs Version",GVAR_CONSTANT);
    dictionary::enable = handle.createGlobVar("lewd_enable","true","Enable Reproductive Bot Organs",GVAR_DUPLICATE,true,0.0,true,1.0);
    dictionary::channel = handle.createGlobVar("reproductive_channel","573284656192159754","Channel for automatic reproduction",GVAR_DUPLICATE);
    handle.regChatCmd("dictionary",&dictCmd,GENERIC,"View reproductive dictionary entries.");
    handle.regChatCmd("dictionary_add",&dictAddCmd,GENERIC,"Add or modify reproductive dictionary entries.");
    handle.regChatCmd("dictionary_remove",&dictDelCmd,GENERIC,"Remove reproductive dictionary entries.");
    handle.regChatCmd("dictionary_save",&dictSaveCmd,GENERIC,"Save reproductive dictionary entries.");
    handle.regChatCmd("dictionary_reload",&dictReloadCmd,GENERIC,"Reload reproductive dictionary entries.");
    handle.regChatCmd("dict",&dictCmd,GENERIC,"View reproductive dictionary entries.");
    handle.regChatCmd("dict_add",&dictAddCmd,GENERIC,"Add or modify reproductive dictionary entries.");
    handle.regChatCmd("dict_addmass",&dictAddMassCmd,GENERIC,"Mass add or modify reproductive dictionary entries.");
    handle.regChatCmd("dict_remove",&dictDelCmd,GENERIC,"Remove reproductive dictionary entries.");
    handle.regChatCmd("dict_save",&dictSaveCmd,GENERIC,"Save reproductive dictionary entries.");
    handle.regChatCmd("dict_reload",&dictReloadCmd,GENERIC,"Reload reproductive dictionary entries.");
    handle.regChatCmd("dict_gimmie",&dictTestCmd,NOFLAG,"Formulate a sentence.");
    handle.regChatCmd("please",&dictTestCmd,NOFLAG,"Formulate a sentence.");
    handle.regChatCmd("dict_stats",&dictStatsCmd,GENERIC,"Stats.");
    consoleOut("Reproductive Bot Organs: Loaded " + std::to_string(dictionary::loadDic()) + " dictionary entries.");
    return 0;
}

extern "C" int onChannelMessage(Handle& handle, Event event, void* message, void* nil, void* foo, void* bar)
{
    Global* global = recallGlobal();
    Message* msg = (Message*)message;
    if ((msg->content.find(global->prefix(msg->guild_id)) == 0) || (msg->channel_id != dictionary::channel->getAsString(msg->guild_id)))
        return 0;
    
    return 0;
}

int dictCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global* global = recallGlobal();
    if (argc < 2)
    {
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <word>`");
        return PLUGIN_HANDLED;
    }
    dictionary::definition word = dictionary::lookupWord(argv[1]);
    if (word.phonetic.code == -1)
    {
        sendMessage(message.channel_id,"`" + argv[1] + "` is not defined! Can you please tell me what it means?");
        return PLUGIN_HANDLED;
    }
    std::string defin = "`" + word.word + "` is ";
    switch (word.part)
    {
        case dictionary::speech::noun:
        {
            defin += "a noun.";
            break;
        }
        case dictionary::speech::verb:
        {
            defin += "a verb.";
            break;
        }
        case dictionary::speech::pronoun:
        {
            defin += "a pronoun.";
            break;
        }
        case dictionary::speech::adjective:
        {
            defin += "an adjective.";
            break;
        }
        case dictionary::speech::adverb:
        {
            defin += "an adverb.";
            break;
        }
        case dictionary::speech::preposition:
        {
            defin += "a preposition.";
            break;
        }
        case dictionary::speech::conjunction:
        {
            defin += "a conjunction.";
            break;
        }
        default:
            defin += "of unknown part of speech!";
    }
    if (word.clues != dictionary::context::none) switch (word.clues)
    {
        case dictionary::context::possessive:
        {
            defin += "\nThis word seems to be possessive.";
            break;
        }
        case dictionary::context::plural:
        {
            defin += "\nThis word seems to be plural.";
            break;
        }
        case dictionary::context::singular:
        {
            defin += "\nThis word seems to be singular.";
            break;
        }
        default:
            defin += "\nThis word seems to be ... invalid.. :juggling:";
    }
    if (word.def.size() > 0)
        defin += "\nYou might know this word as *" + word.def + "*.";
    defin += "\nI think this word has about " + std::to_string(word.derogative) + " lewds.";
    if (word.synonyms.size() > 0)
    {
        defin += "\nThis word is a lot like these ones: `";
        for (auto it = word.synonyms.begin(), ite = word.synonyms.end();it != ite;++it)
            defin += it->first + ' ';
        defin.back() = '`';
    }
    if (word.antonyms.size() > 0)
    {
        defin += "\nThis word probably means the opposite of these ones: `";
        for (auto it = word.antonyms.begin(), ite = word.antonyms.end();it != ite;++it)
            defin += it->first + ' ';
        defin.back() = '`';
    }
    sendMessage(message.channel_id,defin);
    return PLUGIN_HANDLED;
}

int dictAddCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global* global = recallGlobal();
    if (argc < 3)
    {
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <word> <part_of_speech> [context_clues] [lewdness_level] [definition] [synonyms] [antonyms]`");
        return PLUGIN_HANDLED;
    }
    dictionary::definition def;
    def.word = strlower(argv[1]);
    def.part = dictionary::partOfSpeech(argv[2]);
    if ((def.part <= dictionary::speech::unknown) || (def.part > dictionary::speech::conjunction))
    {
        sendMessage(message.channel_id,"Error: `<part_of_speech>` is invalid!");
        return PLUGIN_HANDLED;
    }
    if (argc > 3)
    {
        if (std::isdigit(argv[3].front()))
            def.clues = dictionary::context(std::stoi(argv[3]));
        else
        {
            sendMessage(message.channel_id,"Error: `[context_clues]` and `[lewdness_level]` must both be integers!");
            return PLUGIN_HANDLED;
        }
        if (argc > 4)
        {
            size_t p = 0;
            if (((p = std::string("-0123456789").find_first_of(argv[4].front())) != std::string::npos) && ((p > 0) || (std::isdigit(argv[4].at(1)))))
                def.derogative = std::stoi(argv[4]);
            else
            {
                sendMessage(message.channel_id,"Error: `[context_clues]` and `[lewdness_level]` must both be integers!");
                return PLUGIN_HANDLED;
            }
            if (argc > 5)
            {
                def.def = argv[5];
                if (argc > 6)
                {
                    int s;
                    std::string* syn = splitString(argv[6]," ",s);
                    if (syn != nullptr) for (--s;s >= 0;--s)
                    {
                        if (syn[s].size() < 1)
                            continue;
                        def.synonyms.emplace(syn[s],&dictionary::lookupWord(syn[s]));
                    }
                    delete[] syn;
                    if (argc > 7)
                    {
                        syn = splitString(argv[7]," ",s);
                        if (syn != nullptr) for (--s;s >= 0;--s)
                        {
                            if (syn[s].size() < 1)
                                continue;
                            def.antonyms.emplace(syn[s],&dictionary::lookupWord(syn[s]));
                        }
                        delete[] syn;
                    }
                }
            }
        }
    }
    def.phonetic = dictionary::phonetize(def.word);
    dictionary::addWord(def,true);
    reaction(message,"%E2%9C%85");
    return PLUGIN_HANDLED;
}

int dictAddMassCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global* global = recallGlobal();
    if (argc < 4)
    {
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <part_of_speech> <lewdness_level> <words ...>`");
        return PLUGIN_HANDLED;
    }
    int derogative;
    dictionary::speech part = dictionary::partOfSpeech(argv[1]);
    if ((part <= dictionary::speech::unknown) || (part > dictionary::speech::conjunction))
    {
        sendMessage(message.channel_id,"Error: `<part_of_speech>` is invalid!");
        return PLUGIN_HANDLED;
    }
    /*dictionary::definition def;
    def.word = strlower(argv[1]);
    def.part = dictionary::partOfSpeech(argv[2]);
    if ((def.part <= dictionary::speech::none) || (def.part > dictionary::speech::conjunction))
    {
        sendMessage(message.channel_id,"Error: `<part_of_speech>` is invalid!");
        return PLUGIN_HANDLED;
    }*/
    size_t p = 0;
    if (((p = std::string("-0123456789").find_first_of(argv[2].front())) != std::string::npos) && ((p > 0) || (std::isdigit(argv[2].at(1)))))
        derogative = std::stoi(argv[2]);
    else
    {
        sendMessage(message.channel_id,"Error: `<lewdness_level>` must be an integer!");
        return PLUGIN_HANDLED;
    }
    p = 0;
    for (int i = 3;i < argc;i++)
    {
        dictionary::definition def;
        def.word = strlower(argv[i]);
        def.part = part;
        def.derogative = derogative;
        def.phonetic = dictionary::phonetize(def.word);
        dictionary::addWord(def,true);
        p++;
    }
    sendMessage(message.channel_id,"Successfully added " + std::to_string(p) + ' ' + partOfSpeechStr(part) + "s!");
    return PLUGIN_HANDLED;
}

int dictDelCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global* global = recallGlobal();
    if (argc < 2)
    {
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + argv[0] + " <word>`");
        return PLUGIN_HANDLED;
    }
    auto entry = dictionary::dict.master.find(argv[1]);
    if (entry != dictionary::dict.master.end())
    {
        dictionary::removeWord(entry);
        reaction(message,"%E2%9C%85");
    }
    else
        sendMessage(message.channel_id,"I do not know that word!");
    return PLUGIN_HANDLED;
}

int dictSaveCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    if (dictionary::saveDic())
        messageReply(message,"An error occurred saving the database. The entry will be lost when I inevitably crash.");
    else
        reaction(message,"%E2%9C%85");
    return PLUGIN_HANDLED;
}

int dictReloadCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    dictionary::dict.master.clear();
    dictionary::dict.nouns.clear();
    dictionary::dict.verbs.clear();
    dictionary::dict.pronouns.clear();
    dictionary::dict.adjectives.clear();
    dictionary::dict.adverbs.clear();
    dictionary::dict.prepositions.clear();
    dictionary::dict.conjunctions.clear();
    messageReply(message,"Reproductive Bot Organs: Loaded " + std::to_string(dictionary::loadDic()) + " dictionary entries.");
    return PLUGIN_HANDLED;
}

extern "C" void makeLewd(std::string& sentence, int args[4])
{
    int min = 1, max = 15, lewdmin = 0, lewdmax = 99;
    if (args[0] > -1)
        max = args[0];
    if (args[1] > -1)
        min = args[1];
    if (min < 1)
        min = 1;
    size_t p;
    if (args[2] > -1)
        lewdmin = args[2];
    if (args[3] > -1)
        lewdmax = args[3];
    lewdmax = mtrand(lewdmin,lewdmax);
    dictionary::definition* last = dictionary::getAny(mtrand(lewdmin,lewdmax));
    if (last == nullptr)
    {
        sentence = "Oh no! An error occurred!";
        return;
    }
    sentence = last->word;
    sentence.front() = std::toupper(sentence.front());
    dictionary::definition* curr = last;
    for (int words = mtrand(min,max);words;--words)
    {
        for (int tries = 15;tries && curr == last;tries--)
        {
            int lewd = mtrand(lewdmin,lewdmax);
            switch (last->part)
            {
                case dictionary::speech::noun:
                {
                    int d = mtrand(0,4);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.adverbs,lewd);
                    if (d < 3)
                        curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                }
                case dictionary::speech::verb:
                {
                    int d = mtrand(0,4);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.adjectives,lewd);
                    else if (d == 1)
                        curr = dictionary::getSpecific(dictionary::dict.adverbs,lewd);
                    else if (d == 2)
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.prepositions,lewd);
                    break;
                }
                case dictionary::speech::pronoun:
                {
                    curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    break;
                }
                case dictionary::speech::adjective:
                {
                    if (mtrand(0,1) == 0)
                        curr = dictionary::getSpecific(dictionary::dict.nouns,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.pronouns,lewd);
                    break;
                }
                case dictionary::speech::adverb:
                {
                    int d = mtrand(0,4);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.nouns,lewd);
                    else if (d == 1)
                        curr = dictionary::getSpecific(dictionary::dict.prepositions,lewd);
                    else if (d == 2)
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    break;
                }
                case dictionary::speech::conjunction:
                {
                    int d = mtrand(0,11);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                    else if (d < 3)
                        curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    else if (d < 5)
                        curr = dictionary::getSpecific(dictionary::dict.nouns,lewd);
                    else if (d < 7)
                        curr = dictionary::getSpecific(dictionary::dict.adjectives,lewd);
                    else if (d < 9)
                        curr = dictionary::getSpecific(dictionary::dict.prepositions,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.pronouns,lewd);
                    break;
                }
                case dictionary::speech::preposition:
                {
                    int d = mtrand(0,13);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.prepositions,lewd);
                    else if (d < 3)
                        curr = dictionary::getSpecific(dictionary::dict.adverbs,lewd);
                    else if (d < 6)
                        curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    else if (d < 9)
                        curr = dictionary::getSpecific(dictionary::dict.pronouns,lewd);
                    else if (d == 9)
                        curr = dictionary::getSpecific(dictionary::dict.adjectives,lewd);
                    else if (d == 10)
                        curr = dictionary::getSpecific(dictionary::dict.nouns,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                }
                default:
                    curr = dictionary::getAny(lewd);
            }
            if (curr == nullptr)
            {
                sentence = "Oh no! An error occurred! " + sentence;
                return;
            }
        }
        while (curr == last)
            curr = dictionary::getAny(1);
        sentence += " " + curr->word;
        last = curr;
    }
}

int dictTestCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    if ((dictionary::enable->getAsBool(message.guild_id) == false) || (message.channel_id == "541749565540532224"))
        return PLUGIN_HANDLED;
    int min = 1, max = 15, lewdmin = 0, lewdmax = 99;
    if ((argc > 1) && (std::isdigit(argv[1].front())))
        max = std::stoi(argv[1]);
    if ((argc > 2) && (std::isdigit(argv[2].front())))
        min = std::stoi(argv[2]);
    if (min < 1)
        min = 1;
    size_t p;
    if ((argc > 3) && (((p = std::string("-0123456789").find_first_of(argv[3].front())) != std::string::npos) && ((p > 0) || (std::isdigit(argv[3].at(1))))))
        lewdmin = std::stoi(argv[3]);
    if ((argc > 4) && (((p = std::string("-0123456789").find_first_of(argv[4].front())) != std::string::npos) && ((p > 0) || (std::isdigit(argv[4].at(1))))))
        lewdmax = std::stoi(argv[4]);
    lewdmax = mtrand(lewdmin,lewdmax);
    /*
    dictionary::definition* last = dictionary::getAny(mtrand(lewdmin,lewdmax));
    if (last == nullptr)
    {
        sendMessage(message.channel_id,"Oh no! An error occurred!");
        return PLUGIN_HANDLED;
    }
    std::string sentence = last->word;
    sentence.front() = std::toupper(sentence.front());
    dictionary::definition* curr = last;
    for (int words = mtrand(min,max);words;--words)
    {
        for (int tries = 15;tries && curr == last;tries--)
        {
            int lewd = mtrand(lewdmin,lewdmax);
            switch (last->part)
            {
                case dictionary::speech::noun:
                {
                    int d = mtrand(0,4);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.adverbs,lewd);
                    if (d < 3)
                        curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                }
                case dictionary::speech::verb:
                {
                    int d = mtrand(0,4);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.adjectives,lewd);
                    else if (d == 1)
                        curr = dictionary::getSpecific(dictionary::dict.adverbs,lewd);
                    else if (d == 2)
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.prepositions,lewd);
                    break;
                }
                case dictionary::speech::pronoun:
                {
                    curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    break;
                }
                case dictionary::speech::adjective:
                {
                    if (mtrand(0,1) == 0)
                        curr = dictionary::getSpecific(dictionary::dict.nouns,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.pronouns,lewd);
                    break;
                }
                case dictionary::speech::adverb:
                {
                    int d = mtrand(0,4);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.nouns,lewd);
                    else if (d == 1)
                        curr = dictionary::getSpecific(dictionary::dict.prepositions,lewd);
                    else if (d == 2)
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    break;
                }
                case dictionary::speech::conjunction:
                {
                    int d = mtrand(0,11);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                    else if (d < 3)
                        curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    else if (d < 5)
                        curr = dictionary::getSpecific(dictionary::dict.nouns,lewd);
                    else if (d < 7)
                        curr = dictionary::getSpecific(dictionary::dict.adjectives,lewd);
                    else if (d < 9)
                        curr = dictionary::getSpecific(dictionary::dict.prepositions,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.pronouns,lewd);
                    break;
                }
                case dictionary::speech::preposition:
                {
                    int d = mtrand(0,13);
                    if (d == 0)
                        curr = dictionary::getSpecific(dictionary::dict.prepositions,lewd);
                    else if (d < 3)
                        curr = dictionary::getSpecific(dictionary::dict.adverbs,lewd);
                    else if (d < 6)
                        curr = dictionary::getSpecific(dictionary::dict.verbs,lewd);
                    else if (d < 9)
                        curr = dictionary::getSpecific(dictionary::dict.pronouns,lewd);
                    else if (d == 9)
                        curr = dictionary::getSpecific(dictionary::dict.adjectives,lewd);
                    else if (d == 10)
                        curr = dictionary::getSpecific(dictionary::dict.nouns,lewd);
                    else
                        curr = dictionary::getSpecific(dictionary::dict.conjunctions,lewd);
                }
                default:
                    curr = dictionary::getAny(lewd);
            }
            if (curr == nullptr)
            {
                sendMessage(message.channel_id,"Oh no! An error occurred! " + sentence);
                return PLUGIN_HANDLED;
            }
        }
        while (curr == last)
            curr = dictionary::getAny(1);
        sentence += " " + curr->word;
        last = curr;
    }*/
    std::string sentence;
    int i[4] = {max,min,lewdmin,lewdmax};
    makeLewd(sentence,i);
    sendMessage(message.channel_id,sentence);
    return PLUGIN_HANDLED;
}

int dictStatsCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    sendMessage(message.channel_id,"Nouns: " + std::to_string(dictionary::dict.nouns.size()) + " " + dictionary::dict.nouns.begin()->second->word +
        "\nVerbs: " + std::to_string(dictionary::dict.verbs.size()) + " " + dictionary::dict.verbs.begin()->second->word +
        "\nPronouns: " + std::to_string(dictionary::dict.pronouns.size()) + " " + dictionary::dict.pronouns.begin()->second->word +
        "\nAdjectives: " + std::to_string(dictionary::dict.adjectives.size()) + " " + dictionary::dict.adjectives.begin()->second->word +
        "\nAdverbs: " + std::to_string(dictionary::dict.adverbs.size()) + " " + dictionary::dict.adverbs.begin()->second->word +
        "\nPrepositions: " + std::to_string(dictionary::dict.prepositions.size()) + " " + dictionary::dict.prepositions.begin()->second->word +
        "\nConjunctions: " + std::to_string(dictionary::dict.conjunctions.size()) + " " + dictionary::dict.conjunctions.begin()->second->word +
        "\nTotal: " + std::to_string(dictionary::dict.master.size()) + " " + dictionary::dict.master.begin()->second.word);
    return PLUGIN_HANDLED;
}





/*
noun: conjunction, verb
verb: conjunction, adverb, preposition
pronoun: verb
adjective: noun, adjective, pronoun
adverb: preposition, conjunction, noun

preposition: conjunction, noun, pronoun, adjective, adverb, preposition, verb
conjunction: conjunction, noun all
*/















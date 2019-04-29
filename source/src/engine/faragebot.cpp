#define SLEEPY_ONE_THREAD

//#define FARAGE_TIMEOUT      1

#include "sleepy_discord/websocketpp_websocket.h"
#include "engine/fareng.h"

int main(int argc, char *argv[])
{
    #ifdef FARAGE_USE_PCRE2
    if (!rens::regex_match("jit test",".*"))
    {
        std::cerr<<"ERROR: RECOMPILE PCRE2 WITH `./configure --enable-jit`"<<std::endl;
        return 101;
    }
    #endif
    Farage::Global global(FARAGE_ENGINE,{&Farage::Engine::sendMessage, &Farage::Engine::reactToID, &Farage::Engine::getChannel, &Farage::Engine::getDirectMessageChannel, &Farage::Engine::getUser, &Farage::Engine::getSelf, &Farage::Engine::sendTyping, &Farage::Engine::sendFile, &Farage::Engine::getGuildCache, &Farage::Engine::getServerMember, &Farage::Engine::getChannelCache, &Farage::Engine::editChannel, &Farage::Engine::editChannelName, &Farage::Engine::editChannelTopic, &Farage::Engine::deleteChannel, &Farage::Engine::getMessages, &Farage::Engine::getMessage, &Farage::Engine::removeReaction, &Farage::Engine::getReactions, &Farage::Engine::removeAllReactions, &Farage::Engine::editMessage, &Farage::Engine::deleteMessage, &Farage::Engine::bulkDeleteMessages, &Farage::Engine::editChannelPermissions, &Farage::Engine::getChannelInvites, &Farage::Engine::createChannelInvite, &Farage::Engine::removeChannelPermission, &Farage::Engine::getPinnedMessages, &Farage::Engine::pinMessage, &Farage::Engine::unpinMessage, &Farage::Engine::addRecipient, &Farage::Engine::removeRecipient});
    Farage::recallGlobal(&global);
    std::string FARAGE_TOKEN;
    std::string error = Farage::loadConfig(global,FARAGE_TOKEN);
    if (error.size() > 0)
    {
        Farage::errorOut(error);
        return 1;
    }
    std::vector<std::string> autoexec(1,"./config/script/autoexec.cfg");
    int clr = Farage::processLaunchArgs(global,argc,argv,&FARAGE_TOKEN,&autoexec);
    if (clr != 42)
        return clr;
    if (FARAGE_TOKEN.size() < 1)
    {
        Farage::errorOut(std::string("Error: Discord Bot Token not defined in \"config/farage.conf\".\nYou can also use the '--token' switch to set this at run time. '") + argv[0] + " --help' for help.");
        return 1;
    }
    Farage::loadAssets(global);
    std::atomic<bool> online;
    Farage::BotClass *farage;
    farage = Farage::botConnect(online,FARAGE_TOKEN);
    global.discord = (void*)farage;
    for (auto it = autoexec.begin(), ite = autoexec.end();it != ite;++it)
        Farage::processCscript(farage,global,*it);
    bool running = true;
    fd_set cinset;
    timeval timeout;
    std::string cinput;
    while (running)
    {
        for (int i = 0;!online;i++)
        {
            if (i)
                std::this_thread::sleep_for(std::chrono::seconds(FARAGE_CONNECT_DELAY));
            if (i > ((FARAGE_CONNECT_MAX_RETRIES < 0) ? i : FARAGE_CONNECT_MAX_RETRIES))
                return Farage::cleanUp(farage,global);
            delete farage;
            farage = Farage::botConnect(online,FARAGE_TOKEN);
        }
        FD_ZERO(&cinset);
        FD_SET(0,&cinset);
        timeout = Farage::processTimers(farage,global);
        if (select(1,&cinset,NULL,NULL,&timeout) > 0)
        {
            if (FD_ISSET(0,&cinset))
            {
                std::getline(std::cin,cinput);
                if (cinput.size() > 0)
                {
                    global.clearBuffer();
                    Farage::processCinput(farage,global,cinput);
                }
                cinput.clear();
            }
        }
    }
    return 0;
}


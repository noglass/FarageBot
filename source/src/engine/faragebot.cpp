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
#ifdef _WIN32
    HANDLE timerTrigger[2];
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = true;
    saAttr.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&timerTrigger[0],&timerTrigger[1],&saAttr,1))
#else
    int timerTrigger[2];
    if (pipe(timerTrigger) < 0)
#endif
    {
        std::cerr<<"Fatal: Error creating timer trigger pipe."<<std::endl;
        return 1;
    }
    Farage::Global global(
        FARAGE_ENGINE,
        timerTrigger[1],
        {
            &Farage::Engine::sendMessage,
            &Farage::Engine::reactToID,
            &Farage::Engine::getChannel,
            &Farage::Engine::getDirectMessageChannel,
            &Farage::Engine::getUser,
            &Farage::Engine::getSelf,
            &Farage::Engine::sendTyping,
            &Farage::Engine::sendFile,
            &Farage::Engine::getGuildCache,
            &Farage::Engine::getServerMember,
            &Farage::Engine::getChannelCache,
            &Farage::Engine::editChannel,
            &Farage::Engine::editChannelName,
            &Farage::Engine::editChannelTopic,
            &Farage::Engine::deleteChannel,
            &Farage::Engine::getMessages,
            &Farage::Engine::getMessage,
            &Farage::Engine::removeReaction,
            &Farage::Engine::getReactions,
            &Farage::Engine::removeAllReactions,
            &Farage::Engine::editMessage,
            &Farage::Engine::deleteMessage,
            &Farage::Engine::bulkDeleteMessages,
            &Farage::Engine::editChannelPermissions,
            &Farage::Engine::getChannelInvites,
            &Farage::Engine::createChannelInvite,
            &Farage::Engine::removeChannelPermission,
            &Farage::Engine::getPinnedMessages,
            &Farage::Engine::pinMessage,
            &Farage::Engine::unpinMessage,
            &Farage::Engine::addRecipient,
            &Farage::Engine::removeRecipient,
            &Farage::Engine::serverCommand,
            &Farage::Engine::getServer,
            &Farage::Engine::deleteServer,
            &Farage::Engine::getServerChannels,
            &Farage::Engine::createTextChannel,
            &Farage::Engine::editChannelPositions,
            &Farage::Engine::getMember,
            &Farage::Engine::listMembers,
            &Farage::Engine::addMember,
            &Farage::Engine::editMember,
            &Farage::Engine::muteServerMember,
            &Farage::Engine::editNickname
        }
    );
    Farage::recallGlobal(&global);
    std::string FARAGE_TOKEN;
    std::string error = Farage::loadConfig(global,FARAGE_TOKEN);
    if (error.size() > 0)
    {
        std::cerr<<error<<std::endl;
        return 1;
    }
    std::vector<std::string> autoexec(1,"./config/script/autoexec.cfg");
    int clr = Farage::processLaunchArgs(global,argc,argv,&FARAGE_TOKEN,&autoexec);
    if (clr != 42)
        return clr;
    if (FARAGE_TOKEN.size() < 1)
    {
        std::cerr<<"Error: Discord Bot Token not defined in \"config/farage.conf\".\nYou can also use the '--token' switch to set this at run time. \n'"<<argv[0]<<" --help' for help."<<std::endl;
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
#ifdef _WIN32
    HANDLE events[] =
    {
        GetStdHandle(STD_INPUT_HANDLE),
        timerTrigger[0]
    };
    DWORD wresult;
    INPUT_RECORD inrecord;
    DWORD recRead;
#else
    fd_set cinset;
    timeval timeout;
    char bufClear;
#endif
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
#ifndef _WIN32
        FD_ZERO(&cinset);
        FD_SET(0,&cinset);
        FD_SET(timerTrigger[0],&cinset);
#endif
        timeout = Farage::processTimers(farage,global);
#ifdef _WIN32
        if ((wresult = WSAWaitForMultipleEvents(sizeof(events)/sizeof(events[0]),&events[0],false,timeout,true)) == WSA_WAIT_EVENT_0)
        {
            if ((ReadConsoleInput(events[0],&inrecord,1,&recRead)) && (inrecord.EventType == KEY_EVENT) && (inrecord.Event.KeyEvent.bKeyDown))
            {
#else
        if (select(timerTrigger[0]+1,&cinset,NULL,NULL,&timeout) > 0)
        {
            if (FD_ISSET(0,&cinset))
            {
#endif
                std::getline(std::cin,cinput);
                if (cinput.size() > 0)
                {
                    global.tryGetBuffer().clear();
                    Farage::processCinput(farage,global,cinput);
                }
                cinput.clear();
            }
#ifdef _WIN32
        }
        else if (wresult == WSA_WAIT_EVENT_0+1)
        {
            DWORD dwRead;
            ReadFile(timerTrigger[0],&bufClear,1,&dwRead,NULL);
#else
            else
                read(timerTrigger[0],&bufClear,1);
#endif
        }
    }
    return 0;
}


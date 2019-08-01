#ifndef _GTCINTERFACE_
#define _GTCINTERFACE_

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <cstdio>
#include <vector>
#include <deque>

#define MOD_NONE    0
#define MOD_SHIFT   1 // '2'
#define MOD_ALT     2 // '3'
#define MOD_CTRL    4 // '5'
#define MOD_UNKNOWN 256

namespace gtci
{
    inline size_t divCeil(size_t x, size_t y)
    {
        return x / y + !!(x % y);
    }
    
    struct interface;
    typedef std::string (*TabCallback)(interface&,const std::string&);
    
    struct interface
    {
        interface(TabCallback cb = nullptr) : callback(cb)
        {
            getWinsize();
            termstart = {0};
            if (tcgetattr(0,&termstart) < 0)
                std::cerr<<"GTCInterface Error: Unable to get original input attributes."<<std::endl;
            termios termnew = termstart;
            termnew.c_lflag &= ~ICANON;
            termnew.c_lflag &= ~ECHO;
            termnew.c_cc[VMIN] = 1;
            termnew.c_cc[VTIME] = 0;
            if (tcsetattr(0,TCSANOW,&termnew) < 0)
                std::cerr<<"GTCInterface Error: Unable to set new input attributes."<<std::endl;
            hpos = history.rbegin();
            running = true;
            stdinput = std::thread([this]
            {
                std::cout<<"\e[7;1m\e7"<<std::string(termsize.ws_col,' ')<<"\e[0m\e8"<<std::flush;
                watchi();
            });
        }
        ~interface()
        {
            running = false;
            if (tcsetattr(0, TCSADRAIN, &termstart) < 0)
                std::cerr<<"GTCInterface Error: Unable to reset input attributes."<<std::endl;
            stdinput.join();
        }
        void getWinsize()
        {
            ioctl(0,TIOCGWINSZ,&termsize);
        }
        void printLn(const std::string& str)
        {
            std::unique_lock<std::mutex> lock (omut);
            unprintCurrent(lock,false);
            std::cout<<"\e[0m"<<str<<'\n';
            printCurrent(lock,false);
        }
        virtual int getModifier(char c)
        {
            switch (c)
            {
                case '2': return MOD_SHIFT;
                case '3': return MOD_ALT;
                case '4': return MOD_SHIFT|MOD_ALT;
                case '5': return MOD_CTRL;
                case '6': return MOD_SHIFT|MOD_CTRL;
                case '7': return MOD_ALT|MOD_CTRL;
                case '8': return MOD_SHIFT|MOD_ALT|MOD_CTRL;
            }
            return MOD_UNKNOWN;
        }
        virtual void kbKeyInput(char c)
        {
            if (currpos == 0)
                current.append(1,c);
            else
                current.insert(current.size()-currpos,1,c);
        }
        virtual void kbCtrlAltV() {}
        virtual void kbUP()
        {
            if (hpos != history.rend())
                current = *hpos++;
            else if (hpos != history.rbegin())
                current = *(--hpos);
            if ((hpos == history.rend()) && (history.size() > 0))
                --hpos;
            currpos = 0;
        }
        virtual void kbDOWN()
        {
            if (hpos != history.rbegin())
                current = *(--hpos);
            else
                current.clear();
            currpos = 0;
        }
        virtual void kbRIGHT()
        {
            if (currpos > 0)
                --currpos;
        }
        virtual void kbLEFT()
        {
            if (currpos < current.size())
                ++currpos;
        }
        virtual void kbModUP(int mod) {}
        virtual void kbModDOWN(int mod) {}
        virtual void kbModRIGHT(int mod)
        {
            if (mod == MOD_CTRL)
            {
                if (currpos > 0)
                {
                    size_t pos = current.substr(current.size()-currpos).find(" "); // need to rewrite this without substr
                    if (pos == std::string::npos)
                        currpos = 0;
                    else
                        currpos -= pos+1;
                }
            }
        }
        virtual void kbModLEFT(int mod)
        {
            if (mod == MOD_CTRL)
            {
                if (currpos < current.size())
                {
                    size_t pos = std::string(current.rbegin(),current.rend()).find(" ",currpos);
                    if (pos == std::string::npos)
                        currpos = current.size();
                    else
                        currpos = pos+1;
                }
            }
        }
        virtual void kbModBACKSPACE(int mod)
        {
            if (mod == MOD_ALT)
            {
                if (currpos < current.size())
                {
                    std::string backwards = std::string(current.rbegin(),current.rend());
                    size_t offset = 0;
                    for (;backwards.at(currpos+offset) == ' ';++offset);
                    size_t pos = backwards.find(" ",currpos+offset);
                    if (pos == std::string::npos)
                    {
                        current.erase(0,current.size()-currpos);
                        currpos = current.size();
                    }
                    else
                    {
                        pos = current.size()-pos;
                        size_t len = current.size()-currpos-pos;
                        current.erase(pos,len);
                    }
                }
            }
        }
        virtual void kbModDELETE(int mod)
        {
            if (mod == MOD_CTRL)
            {
                if (currpos > 0)
                {
                    size_t pos = current.size()-currpos;
                    std::string sub = current.substr(pos);
                    size_t offset = 0;
                    for (;sub.at(offset) == ' ';++offset);
                    size_t len = sub.find(" ",offset);
                    if (len == std::string::npos)
                    {
                        current.erase(pos);
                        currpos = 0;
                    }
                    else
                    {
                        current.erase(pos,len);
                        currpos -= len;
                    }
                }
            }
        }
        bool getline(std::string& out)
        {
            std::unique_lock<std::mutex> lock (rmut);
            cv.wait(lock,[this]{ return !cinput.empty(); });
            out = cinput.front();
            cinput.pop_front();
            return true;
        }
        bool getline_nonblock(std::string& out)
        {
            std::unique_lock<std::mutex> lock (rmut);
            //cv.wait(lock,[this]{ return !cinput.empty(); });
            if (!cinput.empty())
            {
                out = cinput.front();
                cinput.pop_front();
                return true;
            }
            return false;
        }
        private:
            std::thread stdinput;
            std::mutex omut;
            std::mutex rmut;
            std::condition_variable cv;
            std::string current;
            winsize termsize;
            termios termstart;
            TabCallback callback;
            size_t currpos = 0;
            std::atomic<bool> running;
            std::deque<std::string> cinput;
            std::vector<std::string> history;
            std::vector<std::string>::reverse_iterator hpos;
            size_t getWidth()
            {
                size_t width = termsize.ws_col;
                if (current.size() > 0)
                {
                    size_t i = current.size() % termsize.ws_col;
                    if (i)
                        width -= i;
                    else
                        width = i;
                }
                return width;
            }
            void unprintCurrent(std::unique_lock<std::mutex>& lock, bool flush = true)
            {
                size_t height = divCeil(current.size(),termsize.ws_col);
                std::cout<<"\e["<<height<<"B\r\e[2K";
                for (;height > 1;--height)
                    std::cout<<"\e[1A\e[2K";
                if (flush)
                    std::cout<<std::flush;
            }
            void printCurrent(std::unique_lock<std::mutex>& lock, bool unprint = true, bool flush = true)
            {
                if (unprint)
                    unprintCurrent(lock,false);
                std::cout<<"\e[7;1m"<<current<<"\e[K\e7"<<std::string(getWidth(),' ')<<"\e[0m\e8";
                size_t lines, line, pos = currpos;
                if ((current.size() > termsize.ws_col) && ((line = divCeil(current.size()-currpos+1,termsize.ws_col)) != (lines = divCeil(current.size(),termsize.ws_col))))
                {
                    std::cout<<"\r\e["<<lines-line<<"A\e["<<termsize.ws_col<<'C';
                    pos = (current.size()-currpos+1) % termsize.ws_col;
                    if (pos > 0)
                        pos = termsize.ws_col - pos;
                }
                std::cout<<std::string(pos,'\b');
                if (flush)
                    std::cout<<std::flush;
            }
            void printCurrent(std::mutex& mut, bool unprint = true, bool flush = true)
            {
                std::unique_lock<std::mutex> lock (omut);
                printCurrent(lock,unprint,flush);
            }
            void watchi()
            {
                char c;
                while (running)
                {
                    c = getchar();
                    if (!running)
                        break;
                    int mod = 0;
                    switch (c)
                    {
                        case '\033':
                        {
                            if ((c = getchar()) != '[')
                                mod = MOD_ALT;
                            else
                                c = getchar();
                            std::unique_lock<std::mutex> lock (omut);
                            unprintCurrent(lock,false);
                            if (c == '1') // modifier
                            {
                                if ((c = getchar()) == ';')
                                {
                                    mod = getModifier(c = getchar());
                                    c = getchar();
                                }
                            }
                            if (c == 22) // ctrl + alt + v
                                kbCtrlAltV();
                            else if (c == 'A') // up
                            {
                                if (mod)
                                    kbModUP(mod);
                                else
                                    kbUP();
                            }
                            else if (c == 'B') // down
                            {
                                if (mod)
                                    kbModDOWN(mod);
                                else
                                    kbDOWN();
                            }
                            else if (c == 'C') // right
                            {
                                if (mod)
                                    kbModRIGHT(mod);
                                else
                                    kbRIGHT();
                            }
                            else if (c == 'D') // left
                            {
                                if (mod)
                                    kbModLEFT(mod);
                                else
                                    kbLEFT();
                            }
                            else if (c == '3') // used for delete maybe more?
                            {
                                if ((c = getchar()) == ';')
                                {
                                    mod = getModifier(c = getchar());
                                    c = getchar();
                                }
                                if (c == '~') // delete
                                {
                                    if (mod)
                                        kbModDELETE(mod);
                                    else if (currpos > 0)
                                        current.erase(current.size()-currpos--,1);
                                }
                            }
                            else if (c == 'H') // home
                                currpos = current.size();
                            else if (c == 'F') // end
                                currpos = 0;
                            else if (c == 127)
                                kbModBACKSPACE(mod);
                            printCurrent(lock,false);
                            break;
                        }
                        case 8:     // delete, seemingly unused
                        case 127:   // backspace
                        {
                            std::unique_lock<std::mutex> lock (omut);
                            unprintCurrent(lock,false);
                            if (mod)
                                kbModBACKSPACE(mod);
                            else if (current.size() > 0)
                            {
                                if (currpos == 0)
                                    current.erase(current.size()-1);
                                else if (currpos < current.size())
                                    current.erase(current.size()-currpos-1,1);
                            }
                            printCurrent(lock,false);
                            break;
                        }
                        case '\r':
                        case '\n':
                        {
                            currpos = 0;
                            hpos = history.rbegin();
                            if ((current.size() > 0) && (((history.size() > 0) && (current != *hpos)) || (history.size() == 0)))
                            {
                                history.push_back(current);
                                hpos = history.rbegin();
                            }
                            std::unique_lock<std::mutex> lock (omut);
                            std::cout<<"\e[1A";
                            unprintCurrent(lock,false);
                            std::cout<<"\e[0;1m"<<current<<'\n';
                            {
                                std::unique_lock<std::mutex> cvl (rmut);
                                cinput.push_back(current);
                            }
                            cv.notify_one();
                            current.clear();
                            getWinsize();
                            printCurrent(lock,false);
                            break;
                        }
                        case '\t':
                        {
                            if (callback != nullptr)
                            {
                                size_t pos = current.size()-currpos;
                                std::string suffix;
                                if (currpos > 0)
                                {
                                    suffix = current.substr(pos);
                                    current.erase(pos);
                                }
                                current = callback(*this,current) + suffix;
                            }
                            printCurrent(omut);
                            break;
                        }
                        default:
                        {
                            std::unique_lock<std::mutex> lock (omut);
                            unprintCurrent(lock,false);
                            kbKeyInput(c);
                            printCurrent(lock,false);
                        }
                    }
                }
            }
    };
};

#endif


#ifndef _FARAGE_TIMER_
#define _FARAGE_TIMER_

#include <ctime>
#include <chrono>
#include <string>

namespace Farage
{
    enum TimeScale
    {
        SECONDS,
        MILLISECONDS,
        MICROSECONDS,
        NANOSECONDS
    };
    
    class Handle;
    
    //template<class T>
    struct Timer;
    
    //template<class T>
    //using TimerCallback = int (*)(Handle&,Timer<T>*,void*);
    typedef int (*TimerCallback)(Handle&,Timer*,void*);
    
    /*template<class T>
    struct Timer
    {
        Timer()
        {
            last = std::chrono::high_resolution_clock::now();
        }
        Timer(const std::string &Name, T Interval, TimerCallback<T> Callback, void *Args = nullptr) : name(Name), interval(Interval), func(Callback), args(Args)
        {
            last = std::chrono::high_resolution_clock::now();
        }
        std::string name;
        inline std::chrono::high_resolution_clock::time_point when() { return last + interval; }
        inline std::chrono::duration<T> remaining() { return std::chrono::duration_cast<T>(when() - std::chrono::high_resolution_clock::now()); }
        template<class As>
        inline std::chrono::duration<As> remainingAs() { return std::chrono::duration_cast<As>(when() - std::chrono::high_resolution_clock::now()); }
        inline T getInterval() { return interval; }
        inline int trigger(Handle &handle)
        {
            last = std::chrono::high_resolution_clock::now();
            if (func != nullptr)
                return (*func)(handle,this,args);
            return 1;
        }
        
        private:
            TimerCallback<T> func = nullptr;
            void *args = nullptr;
            T interval;
            std::chrono::high_resolution_clock::time_point last;
    };*/
    
    struct Timer
    {
        long interval;
        TimeScale type;
        Timer()
        {
            last = std::chrono::high_resolution_clock::now();
        }
        Timer(const std::string &Name, long Interval, TimerCallback Callback, void *Args = nullptr, TimeScale Type = SECONDS) : name(Name), interval(Interval), func(Callback), args(Args), type(Type)
        {
            last = std::chrono::high_resolution_clock::now();
        }
        std::string name;
        std::chrono::high_resolution_clock::time_point when()
        {
            std::chrono::high_resolution_clock::time_point point = last;
            switch (type)
            {
                case MILLISECONDS:
                {
                    point += std::chrono::milliseconds(interval);
                    break;
                }
                case MICROSECONDS:
                {
                    point += std::chrono::microseconds(interval);
                    break;
                }
                case NANOSECONDS:
                {
                    point += std::chrono::nanoseconds(interval);
                    break;
                }
                default:
                    point += std::chrono::seconds(interval);
            }
            return point;
        }
        template<class T>
        inline auto remaining() { return std::chrono::duration_cast<T>(when() - std::chrono::high_resolution_clock::now()); }
        inline int trigger(Handle &handle)
        {
            last = std::chrono::high_resolution_clock::now();
            if (func != nullptr)
                return (*func)(handle,this,args);
            return 1;
        }
        
        private:
            TimerCallback func = nullptr;
            void *args = nullptr;
            std::chrono::high_resolution_clock::time_point last;
    };
};

#endif


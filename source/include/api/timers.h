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
    struct Timer;
    
    typedef int (*TimerCallback)(Handle&,Timer*,void*);
    
    struct Timer
    {
        std::string name;
        long interval;
        std::chrono::high_resolution_clock::time_point last;
        TimerCallback func;
        void *args;
        TimeScale type;
    };
};

#endif


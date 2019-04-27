#ifndef _FARAGE_REGEX_
#define _FARAGE_REGEX_

#include "conf/regex_style.h"

#ifdef FARAGE_USE_PCRE2
    #include "shared/pcre2_halfwrap.h"
    #define rens pcre2w
#else
    #include <regex>
    #define rens std
#endif

#endif


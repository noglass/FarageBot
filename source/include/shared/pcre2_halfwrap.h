#ifndef PCRE2_HALFWRAP
#define PCRE2_HALFWRAP

#include <string>
#include <cstring>
#include <vector>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

namespace pcre2w
{
    class regex
    {
        public:
            regex(const char *re);
            regex(const unsigned char *re);
            regex(const std::string &re);
            regex(const regex &other);
            regex() {}
            ~regex();
            
            pcre2_code *code = nullptr;
            
            int set(const char *re);
            int set(const unsigned char *re);
            int set(const std::string &re);
            
            regex& operator= (const char *re);
            regex& operator= (const unsigned char *re);
            regex& operator= (std::string re);
            regex& operator= (const regex &other);
            
            inline uint32_t getMatchCount() const { return match_count; }
            
        private:
            int compile(PCRE2_SPTR re);
            uint32_t match_count;
    };
    
    class smatch_data
    {
        public:
            smatch_data() : pos(PCRE2_UNSET) {}
            smatch_data(const std::string &st, size_t p);
            smatch_data(const smatch_data &other);
            smatch_data& operator= (const smatch_data &other);
            std::string s;
            size_t pos;
            inline std::string str() const { return s; }
            inline size_t position() const { return pos; }
            inline size_t length() const { return s.size(); }
            inline void clear() { s.clear(); pos = 0; }
            int compare(const std::string &str);
            int compare(const smatch_data &md);
            int compare(const char *str);
            int compare(const unsigned char *str);
            inline std::string operator() () { return s; }
    };
            
    class smatch
    {
        smatch_data pref, suff;
        size_t pos;
        public:
            smatch() {}
            smatch(const smatch &other);
            smatch& operator= (const smatch &other);
            inline size_t position() const { return pos; }
            inline smatch_data prefix() const { return pref; }
            inline smatch_data suffix() const { return suff; }
            std::vector<smatch_data> capture;
            inline std::vector<smatch_data>::iterator begin() { return capture.begin(); }
            inline std::vector<smatch_data>::iterator end() { return capture.end(); }
            inline std::vector<smatch_data>::reverse_iterator rbegin() { return capture.rbegin(); }
            inline std::vector<smatch_data>::reverse_iterator rend() { return capture.rend(); }
            inline std::vector<smatch_data>::const_iterator cbegin() { return capture.cbegin(); }
            inline std::vector<smatch_data>::const_iterator cend() { return capture.cend(); }
            inline std::vector<smatch_data>::const_reverse_iterator crbegin() { return capture.crbegin(); }
            inline std::vector<smatch_data>::const_reverse_iterator crend() { return capture.crend(); }
            void populate(PCRE2_SPTR subject, pcre2_match_data *ml, int rc, const uint32_t cc);
            smatch_data& operator[] (size_t n);
            inline size_t size() const { return capture.size(); }
            inline void clear() { pref.clear(); suff.clear(); capture.clear(); }
            inline bool empty() { return (bool)capture.size(); }
            inline size_t max_size() const { return capture.max_size(); }
            void swap(smatch &sm);
            std::string format(const std::string &fmt) const;
    };
    
    int regex_search(const unsigned char *subject, smatch &results, const regex &re, bool with = true);
    int regex_search(const std::string &subject, smatch &results, const regex &re);
    int regex_search(const char *subject, smatch &results, const regex &re);
    int regex_search(const unsigned char *subject, const regex &re);
    int regex_search(const std::string &subject, const regex &re);
    int regex_search(const char *subject, const regex &re);
    int regex_match(const unsigned char *subject, smatch &results, const regex &re);
    int regex_match(const std::string &subject, smatch &results, const regex &re);
    int regex_match(const char *subject, smatch &results, const regex &re);
    int regex_match(const unsigned char *subject, const regex &re);
    int regex_match(const std::string &subject, const regex &re);
    int regex_match(const char *subject, const regex &re);
    std::string regex_replace(const std::string &subject, const regex &re, const std::string &format, uint32_t options = PCRE2_SUBSTITUTE_GLOBAL);
    std::string regex_replace(const unsigned char *subject, const regex &re, const unsigned char *format, uint32_t options = PCRE2_SUBSTITUTE_GLOBAL);
    std::string regex_replace(const unsigned char *subject, const regex &re, const std::string &format, uint32_t options = PCRE2_SUBSTITUTE_GLOBAL);
    std::string regex_replace(const std::string &subject, const regex &re, const unsigned char *format, uint32_t options = PCRE2_SUBSTITUTE_GLOBAL);
    std::string regex_replace(const char *subject, const regex &re, const char *format, uint32_t options = PCRE2_SUBSTITUTE_GLOBAL);
    std::string regex_replace(const char *subject, const regex &re, const std::string &format, uint32_t options = PCRE2_SUBSTITUTE_GLOBAL);
    std::string regex_replace(const std::string &subject, const regex &re, const char *format, uint32_t options = PCRE2_SUBSTITUTE_GLOBAL);
    
}

#endif


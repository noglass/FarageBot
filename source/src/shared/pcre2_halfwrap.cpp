#include "pcre2_halfwrap.h"

pcre2w::regex::regex(const char *re) { compile((PCRE2_SPTR)re); }
pcre2w::regex::regex(const unsigned char *re) { compile(re); }
pcre2w::regex::regex(const std::string &re) { compile((PCRE2_SPTR)re.c_str()); }
pcre2w::regex::regex(const regex &other) { code = pcre2_code_copy(other.code); pcre2_jit_compile(code,PCRE2_JIT_COMPLETE); }
pcre2w::regex::~regex() { if (code != nullptr) pcre2_code_free(code); }
int pcre2w::regex::set(const char *re) { return compile((PCRE2_SPTR)re); }
int pcre2w::regex::set(const unsigned char *re) { return compile(re); }
int pcre2w::regex::set(const std::string &re) { return compile((PCRE2_SPTR)re.c_str()); }
pcre2w::regex& pcre2w::regex::operator= (const char *re) { compile((PCRE2_SPTR)re); return *this; }
pcre2w::regex& pcre2w::regex::operator= (const unsigned char *re) { compile(re); return *this; }
pcre2w::regex& pcre2w::regex::operator= (std::string re) { compile((PCRE2_SPTR)re.c_str()); return *this; }
pcre2w::regex& pcre2w::regex::operator= (const regex &other) { code = pcre2_code_copy(other.code); pcre2_jit_compile(code,PCRE2_JIT_COMPLETE); return *this; }
int pcre2w::regex::compile(PCRE2_SPTR re)
{
    if (code != nullptr)
        pcre2_code_free(code);
    int errorcode;
    PCRE2_SIZE erroroffset;
    code = pcre2_compile(re,PCRE2_ZERO_TERMINATED,0,&errorcode,&erroroffset,NULL);
    pcre2_jit_compile(code,PCRE2_JIT_COMPLETE);
    pcre2_pattern_info(code,PCRE2_INFO_CAPTURECOUNT,&match_count);
    return errorcode;
}
pcre2w::smatch_data::smatch_data(const std::string &st, size_t p) { s = st; pos = p; }
pcre2w::smatch_data::smatch_data(const smatch_data &other) { s = other.s; pos = other.pos; }
pcre2w::smatch_data& pcre2w::smatch_data::operator= (const smatch_data &other) { s = other.s; pos = other.pos; return *this; }
int pcre2w::smatch_data::compare(const std::string &str) { return s.compare(str); }
int pcre2w::smatch_data::compare(const pcre2w::smatch_data &md) { return s.compare(md.s); }
int pcre2w::smatch_data::compare(const char *str) { return s.compare(str); }
int pcre2w::smatch_data::compare(const unsigned char *str) { return s.compare((const char*)str); }
pcre2w::smatch::smatch(const smatch &other)
{
    capture.clear();
    capture.reserve(other.capture.size());
    pref = other.pref;
    suff = other.suff;
    for (auto it = other.capture.begin(), ite = other.capture.end();it != ite;++it)
        capture.push_back(*it);
}
pcre2w::smatch& pcre2w::smatch::operator= (const smatch &other)
{
    capture.clear();
    capture.reserve(other.capture.size());
    pref = other.pref;
    suff = other.suff;
    for (auto it = other.capture.begin(), ite = other.capture.end();it != ite;++it)
        capture.push_back(*it);
    return *this;
}
void pcre2w::smatch::populate(PCRE2_SPTR subject, pcre2_match_data *ml, int rc, const uint32_t cc) 
{
    //std::cout<<"pcre2w::smatch::populate("<<(char*)subject<<",ml,"<<rc<<','<<cc<<") begin"<<std::endl;
    clear();
    if (rc > 0)
    {
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(ml);
        //std::cout<<"pcre2_get_ovector_pointer()"<<std::endl;
        size_t offset = ovector[0];
        std::string sub = (char *)subject;
        if (offset)
            pref = { sub.substr(0,offset), offset };
        size_t start;
        size_t length;
        pos = ovector[0];
        //std::cout<<"Begin filling vector..."<<std::endl;
        for (int i = 0;i < rc;++i)
        {
            start = ovector[2*i];
            length = ovector[2*i+1] - ovector[2*i];
            if ((start == PCRE2_UNSET) || (length == PCRE2_UNSET))
                capture.push_back({"",start});
            else
                capture.push_back({sub.substr(start,length),start});
        }
        //std::cout<<"Vector filled size: "<<capture.size()<<std::endl;
        offset = ovector[1];
        if (offset < sub.size())
            suff = { sub.substr(offset,std::string::npos), offset };
        //if (cc < 61) while (capture.size() <= cc)
        //    capture.push_back({"",PCRE2_UNSET});
    }
    //std::cout<<"pcre2w::smatch::populate() end"<<std::endl;
}

pcre2w::smatch_data& pcre2w::smatch::operator[] (size_t n)
{
    while (n >= capture.size())
        capture.emplace_back(pcre2w::smatch_data());
    return capture.at(n);
}

void pcre2w::smatch::swap(pcre2w::smatch &sm)
{
    pcre2w::smatch temp = sm;
    sm = *this;
    *this = temp;
}
std::string pcre2w::smatch::format(const std::string &fmt) const
{
    std::string out;
    size_t pos[2] = {0};
    for (pos[1] = fmt.find('$',pos[0]);pos[1] != std::string::npos;pos[1] = fmt.find('$',pos[0]))
    {
        out += fmt.substr(pos[0],pos[1]-pos[0]);
        if (pos[1]+1 < fmt.size())
        {
            char c = fmt.at(++pos[1]);
            if (c == '$')
                out += '$';
            else if (c == '&')
            {
                if (size() > 0)
                    out += capture.at(0).s;
            }
            else if (c == '`')
                out += pref.s;
            else if (c == '\'')
                out += suff.s;
            else if (isdigit(c))
            {
                int n;
                if ((fmt.size() > pos[1]+1) && (isdigit(fmt.at(pos[1]+1))))
                    n = std::stoi(fmt.substr(pos[1]++,2));
                else
                    n = fmt.at(pos[1])-48;//'0' == 48
                if (n < size())
                    out += capture.at(n).s;
            }
        }
        pos[0] = pos[1]+1;
    }
    out += fmt.substr(pos[0]);
    return out;
}

int pcre2w::regex_search(const unsigned char *subject, pcre2w::smatch &results, const pcre2w::regex &re, bool with)
{
    pcre2_match_data *ml = pcre2_match_data_create_from_pattern(re.code, NULL);
    int rc = pcre2_jit_match(re.code,subject,strlen((char *)subject),0,0,ml,NULL);
    if (with)
        results.populate(subject,ml,rc,re.getMatchCount());
    if (rc < 0)
        rc = 0;
    pcre2_match_data_free(ml);
    return rc;
}
int pcre2w::regex_search(const std::string &subject, pcre2w::smatch &results, const pcre2w::regex &re)
{
    return pcre2w::regex_search((const unsigned char*)subject.c_str(),results,re);
}
int pcre2w::regex_search(const char *subject, pcre2w::smatch &results, const pcre2w::regex &re)
{
    return pcre2w::regex_search((const unsigned char*)subject,results,re);
}

int pcre2w::regex_search(const unsigned char *subject, const pcre2w::regex &re)
{
    pcre2w::smatch results;
    return pcre2w::regex_search(subject,results,re,false);
}
int pcre2w::regex_search(const std::string &subject, const pcre2w::regex &re)
{
    return pcre2w::regex_search((const unsigned char*)subject.c_str(),re);
}
int pcre2w::regex_search(const char *subject, const pcre2w::regex &re)
{
    return pcre2w::regex_search((const unsigned char*)subject,re);
}

int pcre2w::regex_match(const unsigned char *subject, pcre2w::smatch &results, const pcre2w::regex &re)
{
    int rc = pcre2w::regex_search(subject,results,re);
    if ((rc > 0) && (results[0].length() != strlen((const char*)subject)))
    {
        rc = 0;
        results.clear();
    }
    return rc;
}
int pcre2w::regex_match(const std::string &subject, pcre2w::smatch &results, const pcre2w::regex &re)
{
    return pcre2w::regex_match((const unsigned char*)subject.c_str(),results,re);
}
int pcre2w::regex_match(const char *subject, pcre2w::smatch &results, const pcre2w::regex &re)
{
    return pcre2w::regex_match((const unsigned char*)subject,results,re);
}

int pcre2w::regex_match(const unsigned char *subject, const pcre2w::regex &re)
{
    pcre2w::smatch results;
    return pcre2w::regex_match(subject,results,re);
}
int pcre2w::regex_match(const std::string &subject, const pcre2w::regex &re)
{
    return pcre2w::regex_match((const unsigned char*)subject.c_str(),re);
}
int pcre2w::regex_match(const char *subject, const pcre2w::regex &re)
{
    return pcre2w::regex_match((const unsigned char*)subject,re);
}

std::string pcre2w::regex_replace(const std::string &subject, const pcre2w::regex &re, const std::string &format, uint32_t options)
{
    size_t subsize = subject.size(), fsize = format.size(), outsize = subsize*3+fsize;
    if (outsize < subsize+fsize)
        outsize = -1;
    PCRE2_UCHAR *outbuf = new PCRE2_UCHAR[outsize];
    int rc = pcre2_substitute(re.code,(const unsigned char*)subject.c_str(),subsize,0,options,NULL,NULL,(const unsigned char*)format.c_str(),fsize,outbuf,&outsize);
    std::string out;
    if (outsize > 0)
    {
        out = std::string((char*)outbuf,outsize);
        if (out.size() > outsize)
            out.erase(outsize);
    }
    delete[] outbuf;
    return out;
}
std::string pcre2w::regex_replace(const unsigned char *subject, const pcre2w::regex &re, const unsigned char *format, uint32_t options)
{
    return pcre2w::regex_replace(std::string((const char*)subject),re,std::string((const char*)format),options);
}
std::string pcre2w::regex_replace(const unsigned char *subject, const pcre2w::regex &re, const std::string &format, uint32_t options)
{
    return pcre2w::regex_replace(std::string((const char*)subject),re,format,options);
}
std::string pcre2w::regex_replace(const std::string &subject, const pcre2w::regex &re, const unsigned char *format, uint32_t options)
{
    return pcre2w::regex_replace(subject,re,std::string((const char*)format),options);
}
std::string pcre2w::regex_replace(const char *subject, const pcre2w::regex &re, const char *format, uint32_t options)
{
    return pcre2w::regex_replace(std::string(subject),re,std::string(format),options);
}
std::string pcre2w::regex_replace(const char *subject, const pcre2w::regex &re, const std::string &format, uint32_t options)
{
    return pcre2w::regex_replace(std::string(subject),re,format,options);
}
std::string pcre2w::regex_replace(const std::string &subject, const pcre2w::regex &re, const char *format, uint32_t options)
{
    return pcre2w::regex_replace(subject,re,std::string(format),options);
}



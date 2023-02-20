#ifndef EVO_UNICODE
#define EVO_UNICODE

#include <cstdint>
#include <istream>
#include <string>
#include <iostream>

namespace unicode {

    using codepoint = uint32_t;
    using string = std::basic_string<codepoint>;

    string fromPlatform(const std::string str);
    string fromCodePoint(const codepoint value);
    string fromUTF8(const std::string str);
    string fromUTF16LE(const std::string str);
    string fromUTF32LE(const std::string str);
    
    std::string toPlatform(const string str);
    std::string toUTF8(const string str);
    std::string toUTF16LE(const string str);
    std::string toUTF32LE(const string str);


    void readString(std::istream &is,string &str);

    inline string to_string(int value){
        return fromPlatform(std::to_string(value)); 
    }

    inline string to_string(long value){
        return fromPlatform(std::to_string(value)); 
    }

    inline string to_string(long long value){
        return fromPlatform(std::to_string(value)); 
    }

    inline string to_string(unsigned value){
        return fromPlatform(std::to_string(value)); 
    }

    inline string to_string(unsigned long value){
        return fromPlatform(std::to_string(value)); 
    }

    inline string to_string(unsigned long long value){
        return fromPlatform(std::to_string(value)); 
    }

    inline string to_string(float value){
        return fromPlatform(std::to_string(value)); 
    }

    inline string to_string(double value){
        return fromPlatform(std::to_string(value)); 
    }

    inline string to_string(long double value){
        return fromPlatform(std::to_string(value)); 
    }

    codepoint toLower(codepoint value);

    string toLower(const string &str);
}

unicode::codepoint operator""_codepoint(const char *c_str,std::size_t len);

unicode::string operator""_utf32(const char *c_str,std::size_t len);

inline std::ostream &operator<<(std::ostream &os,const unicode::string& str){
    return os<<unicode::toPlatform(str);
}

inline std::istream &operator>>(std::istream &is,unicode::string& str){
    unicode::readString(is,str);
    return is;
}

#endif
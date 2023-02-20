#include "unicode.h"
#include "unicode/utypes.h"   /* Basic ICU data types */
#include "unicode/ucnv.h"     /* C   Converter API    */
#include "unicode/ustring.h"  /* some more string fcns*/
#include "unicode/uchar.h"    /* char names           */
#include "unicode/uloc.h"
#include "unicode/unistr.h"
#include <algorithm>
#include <cstddef>
#include <cstring>


namespace unicode {

    class Raw{
        bool shared;
        int length;
        char *bytes;
    public:
        Raw(const Raw &) = delete;
        Raw(Raw&& r) : shared(r.shared), length(r.length), bytes(r.bytes) {
            r.bytes = nullptr;
        }

        explicit Raw() : bytes(nullptr), length(0), shared(true) {}
        explicit Raw(int length) : bytes((char*)malloc(length)), length(length), shared(false){}
        Raw(char *bytes, int length) : bytes(bytes), length(length), shared(true){}
        int getLength() const { return length; }
        char *getBytes(){ return bytes; }
        ~Raw(){ 
            if(!shared){
                free((void*)bytes); 
                bytes = nullptr;
            }
        }
    };

    constexpr int bufferSize = 100;
    char buffer[bufferSize];
    const char *utf8_encoding = "utf8",
                *utf16le_encoding = "utf16le",
                *utf32le_encoding = "utf32le",
                *platform_encoding = ucnv_getDefaultName();

    Raw convert(const char *dst_name, const char *src_name, const char *str, int length){
        if(length==0)return Raw();

        UErrorCode status = U_ZERO_ERROR;

        auto target_length = ucnv_convert(dst_name, src_name, buffer, bufferSize, str, length, &status);
        if(target_length > bufferSize){
            status = U_ZERO_ERROR;
            Raw raw(target_length);
            ucnv_convert(dst_name, src_name, raw.getBytes(), raw.getLength(), str, length, &status);
            return raw;
        }
        else{
            return Raw(buffer,target_length);
        }
    }

    string fromPlatform(const std::string str){
        auto raw = convert(utf32le_encoding, platform_encoding, str.data(), str.length());
        return string((codepoint*)raw.getBytes(),raw.getLength() / sizeof(codepoint));
    }

    string fromUTF8(const std::string str){
        auto raw = convert(utf32le_encoding, utf8_encoding, str.data(), str.length());
        return string((codepoint*)raw.getBytes(),raw.getLength() / sizeof(codepoint));
    }

    string fromUTF16LE(const std::string str){
        auto raw = convert(utf32le_encoding, utf16le_encoding, str.data(), str.length());
        return string((codepoint*)raw.getBytes(),raw.getLength() / sizeof(codepoint));
    }

    string fromUTF32LE(const std::string str){
        return string((codepoint*)str.data(),str.length() / sizeof(codepoint));
    }
    
    std::string toPlatform(const string str){
        auto raw = convert(platform_encoding, utf32le_encoding, (char*)str.data(), str.length() * sizeof(codepoint));
        return std::string(raw.getBytes(),raw.getLength());
    }

    std::string toUTF8(const string str){
        auto raw = convert(utf8_encoding, utf32le_encoding, (char*)str.data(), str.length() * sizeof(codepoint));
        return std::string(raw.getBytes(),raw.getLength());
    }

    std::string toUTF16LE(const string str){
        auto raw = convert(utf16le_encoding, utf32le_encoding, (char*)str.data(), str.length() * sizeof(codepoint));
        return std::string(raw.getBytes(),raw.getLength());
    }

    std::string toUTF32LE(const string str){
        return std::string((char*)str.data(),str.length() * sizeof(codepoint));
    }

    void readString(std::istream &is,string &str){
        std::string tmp;
        is>>tmp;
        str.clear();
        str+=fromPlatform(tmp);
    }

}

unicode::codepoint operator""_codepoint(const char *c_str,std::size_t len){
    auto raw = unicode::convert(unicode::utf32le_encoding, unicode::platform_encoding, c_str, len);
    return *((unicode::codepoint*)raw.getBytes());
}   

unicode::string operator""_utf32(const char *c_str,std::size_t len){
    return unicode::fromPlatform(std::string(c_str,len));
}

namespace unicode {
    const codepoint letter_A = "A"_codepoint,
                letter_Z = "Z"_codepoint,
                letter_a = "a"_codepoint,
                letter_z = "z"_codepoint;

    codepoint toLower(codepoint value){
        if(value >= letter_A && value <= letter_Z) return value - letter_A + letter_a;
        else return value;
    }

    string toLower(const string &str){
        string ret = str;
        std::transform(str.begin(), str.end(), ret.begin(), [](codepoint value){ return toLower(value); });
        return ret;
    }
}

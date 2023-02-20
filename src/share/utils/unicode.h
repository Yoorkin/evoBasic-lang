//
// Created by yorkin on 2/17/22.
//

#ifndef EVOBASIC_UNICODE_H
#define EVOBASIC_UNICODE_H
#include "data.h"
#include <string>
#include <vector>
#include <list>
#include <ostream>

namespace evoBasic::unicode{

    using Storage = std::string;

    using CodePoint = data::u32;

    class Utf8Char{
        friend std::istream &operator>>(std::istream &lhs,Utf8Char &rhs);
        Storage bytes;

        static constexpr data::u8 first_byte_mask[5] = {
            0,0b01111111,0b11111,0b1111,0b111
        };

        static constexpr int first_byte_valid_count[5] = {
            0,7,5,4,3
        };

        static constexpr data::u8 first_byte_flag[5] = {
            0,0b00000000,0b11000000,0b11100000,0b11110000
        };

        static constexpr data::u8 remain_byte_mask = 0b00111111;
        static constexpr int remain_byte_valid_count = 6;

        static constexpr data::u8 remain_byte_flag = 0b10000000;

        static constexpr CodePoint represent_range[5] = {
                0x0000,0x0080,0x0800,0x10000,0x10FFFF
        };

        static int getRepresentByteCount(CodePoint point);

    public:
        Utf8Char(CodePoint point);
        Utf8Char(){}
        Utf8Char(const char *c_str,std::size_t len);
        Utf8Char(Storage::iterator beg,data::u64 size);
        const Storage &getBytes()const;
        bool operator==(const Utf8Char &rhs);
        auto operator<=>(const Utf8Char &rhs)const;
        auto operator<=>(const char c)const;
        CodePoint getCodePoint()const;
        bool empty()const;
        void clear();
    };

    class Utf8String {
    private:
        Storage bytes;
        constexpr static data::u8 bit_flag[4] = {
                1<<7,1<<5,1<<4,1<<3
        };

        static data::u8 getUnitSize(unsigned char head);

    public:

        class iterator : public std::iterator<
                                std::input_iterator_tag,
                                Utf8Char,
                                int,
                                const Utf8Char*,
                                Utf8Char>{
            friend Utf8String;
            const Storage &bytes;
            Storage::iterator beg;
        public:
            iterator(const Storage &bytes,const Storage::iterator begin);
            iterator &operator++();
            iterator operator++(int);
            reference operator*() const;
            bool operator==(const iterator &rhs);
        };

        Utf8String(const char* c_str);
        Utf8String(){}
        explicit Utf8String(const char* c_str,std::size_t len);
        Utf8String(std::string str);
        Utf8String(iterator begin,iterator end);
        iterator begin();
        iterator end();
        void push_back(Utf8Char c);
        std::size_t size()const;
        const Storage &getBytes()const;
        const char *c_str()const;
        std::strong_ordering operator<=>(const Utf8String &rhs)const;
        bool operator==(const Utf8String &rhs);
        void clear();
        bool empty();
        std::size_t count()const;
        Utf8String operator+(const Utf8String &rhs)const;
        Utf8String operator+(const Utf8Char &rhs)const;
        Utf8String &operator+=(const Utf8String &rhs);
        Utf8String &operator+=(const Utf8Char &rhs);
        Utf8String substr(std::size_t pos, std::size_t len)const;
    };

    std::ostream &operator<<(std::ostream &lhs,const Utf8String &rhs);

    std::ostream &operator<<(std::ostream &lhs,const Utf8Char &rhs);

    std::istream &operator>>(std::istream &lhs,Utf8Char &rhs);

    std::ostream &operator<<(std::ostream &lhs, const Utf8String &rhs);

    std::ostream &operator<<(std::ostream &lhs, const Utf8Char &rhs);

    std::istream &operator>>(std::istream &lhs, Utf8Char &rhs);

    Utf8String operator""_utf8(const char *c_str,std::size_t len);

    Utf8Char operator""_utf8char(const char *c_str,std::size_t len);

    std::istream &getline(std::istream &is, Utf8String &str,Utf8Char delim);

    std::istream &getline(std::istream &is, Utf8String &str);

    bool isDigit(const Utf8Char &c);

    bool isLowerCase(const Utf8Char &c);

    bool isUpperCase(const Utf8Char &c);

    bool isAlpha(const Utf8Char &c);

    bool isBasicHan(const Utf8Char &c);

    Utf8Char toLowerCase(const Utf8Char &c);

    Utf8String to_string(int value);

    int stoi(Utf8String str);

    double stod(Utf8String str);

    std::istream &operator>>(std::istream &lhs,Utf8Char &rhs);

}

#endif //EVOBASIC_UNICODE_H

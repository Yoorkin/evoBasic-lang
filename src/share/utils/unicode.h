//
// Created by yorkin on 2/17/22.
//

#ifndef EVOBASIC_UNICODE_H
#define EVOBASIC_UNICODE_H
#include "data.h"
#include "nullSafe.h"
#include <string>
#include <vector>
#include <list>
#include <ostream>
#include <string.h>

namespace evoBasic::unicode{
    class Utf8String;
    using Storage = std::vector<data::Byte>;

    using CodePoint = data::u32;

    class Utf8Char{
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

        static int getRepresentByteCount(CodePoint point){
            for(int i=0;i<=3;i++){
                if(point>=represent_range[i] && point<represent_range[i+1])
                    return i+1;
            }
            PANIC;
        }


    public:

        Utf8Char(CodePoint point){
            auto count = getRepresentByteCount(point);
            std::stack<data::Byte> tmp;
            data::Byte byte;
            for(int i = count-1;i>0;i--){
                byte = remain_byte_flag + (remain_byte_mask & point);
                point >>= remain_byte_valid_count;
                tmp.push(byte);
            }
            byte = first_byte_flag[count] + (first_byte_mask[count] & point);
            bytes.push_back(byte);
            while(!tmp.empty()){
                bytes.push_back(tmp.top());
                tmp.pop();
            }
        }


        Utf8Char(const char *c_str,std::size_t len) : bytes(len){
            for(int i=0;i<len;i++){
                bytes[i] = c_str[i];
            }
        }
        Utf8Char(Storage::iterator beg,data::u64 size) : bytes(beg,beg+size){}
        const Storage &getBytes()const{return bytes;}
        bool operator==(const Utf8Char &rhs){
            if(bytes.size() != rhs.bytes.size())return false;
            for(int i=0;i<bytes.size();i++){
                if(bytes[i] != rhs.bytes[i])return false;
            }
            return true;
        }

        auto operator<=>(const Utf8Char &rhs)const{
            return getCodePoint()<=>rhs.getCodePoint();
        }

        CodePoint getCodePoint()const{
            auto size = bytes.size();
            if(size==1) {
                return bytes[0];
            }
            else{
                CodePoint point = (CodePoint)(bytes[0] & first_byte_mask[size]);
                for(int i=1;i<size;i++){
                    point <<= remain_byte_valid_count;
                    point |= (bytes[i] & remain_byte_mask);
                }
                return point;
            }
        }

    };

    class Utf8String{
    private:
        Storage bytes;
        constexpr static data::u8 bit_flag[4] = {
                1<<7,1<<5,1<<4,1<<3
        };

        static data::u8 getUnitSize(unsigned char head){
            for(int i=0;i<4;i++){
                if((head & bit_flag[i]) == 0)return i+1;
            }
            PANIC;
        }

    public:

        class iterator : public std::iterator<
                                std::input_iterator_tag,
                                Utf8Char,
                                int,
                                const Utf8Char*,
                                Utf8Char>{
            friend Utf8String;
            Storage &bytes;
            Storage::iterator beg;
        public:
            iterator(Storage &bytes, Storage::iterator begin)
                : bytes(bytes),beg(begin){}

            iterator &operator++(){
                if(beg!=bytes.end())
                    beg += getUnitSize(*beg);
                return *this;
            }

            iterator operator++(int){
                iterator prv = *this;
                if(beg!=bytes.end())
                    beg += getUnitSize(*beg);
                return prv;
            }
            reference operator*() const {
                return Utf8Char(beg, getUnitSize(*beg));
            }

            bool operator==(const iterator &rhs){
                return beg==rhs.beg;
            }
        };

        Utf8String(const char* c_str){
            int len = strlen(c_str);
            bytes.resize(len);
            for(int i=0;i<len;i++)
                bytes[i] = c_str[i];
        }

        explicit Utf8String(const char* c_str,std::size_t len) : bytes(len){
            for(int i=0;i<len;i++)
                bytes[i] = c_str[i];
        }

        Utf8String(iterator begin,iterator end)
            : bytes(begin.beg,end.beg){}

        iterator begin() {return {bytes,bytes.begin()};}
        iterator end() {return {bytes,bytes.end()};}

        void push_back(Utf8Char &c){
            bytes.insert(bytes.end(),c.getBytes().begin(), c.getBytes().end());
        }

        std::size_t size()const{
            return bytes.size();
        }

        const Storage &getBytes()const{
            return bytes;
        }

        bool operator==(const Utf8String &rhs){
            if(size() != rhs.size()) return false;
            for(int i=0;i<size();i++){
                if(bytes[i] != rhs.bytes[i])return false;
            }
            return true;
        }
    };

    std::ostream &operator<<(std::ostream &lhs,const Utf8String &rhs){
        for(auto c : rhs.getBytes()){
            lhs.put(c);
        }
        return lhs;
    }

    std::ostream &operator<<(std::ostream &lhs,const Utf8Char &rhs){
        for(auto c : rhs.getBytes()){
            lhs.put(c);
        }
        return lhs;
    }

    Utf8String operator""_utf8(const char *c_str,std::size_t len){
        return Utf8String(c_str,len);
    }

    Utf8Char operator""_utf8char(const char *c_str,std::size_t len){
        return Utf8Char(c_str,len);
    }


    bool isDigit(const Utf8Char &c){
        return c>="0"_utf8char && c<="9"_utf8char;
    }

    bool isLowerCase(const Utf8Char &c){
        return c>="a"_utf8char && c<="z"_utf8char;
    }

    bool isUpperCase(const Utf8Char &c){
        return c>="A"_utf8char && c<="Z"_utf8char;
    }

    bool isAlpha(const Utf8Char &c){
        return isLowerCase(c) || isUpperCase(c);
    }

    bool isBasicHan(const Utf8Char &c){
        return 0x4E00 <= c.getCodePoint() && 0x9FA5 >= c.getCodePoint();
    }

}

#endif //EVOBASIC_UNICODE_H

//
// Created by yorkin on 2/19/22.
//
#include "unicode.h"
#include <cstring>
#include <stack>
#include "nullSafe.h"

namespace evoBasic::unicode{

    Utf8Char::Utf8Char(CodePoint point) {
        auto count = getRepresentByteCount(point);
        if(count==1){
            bytes.push_back(point);
        }
        else{
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
    }

    int Utf8Char::getRepresentByteCount(CodePoint point) {
        for(int i=0;i<=3;i++){
            if(point>=represent_range[i] && point<represent_range[i+1])
                return i+1;
        }
        PANIC;
    }

    Utf8Char::Utf8Char(const char *c_str, std::size_t len) {
        bytes.resize(len);
        for(int i=0;i<len;i++){
            bytes[i] = c_str[i];
        }
    }

    Utf8Char::Utf8Char(Storage::iterator beg, data::u64 size) : bytes(beg,beg+size){}

    const Storage &Utf8Char::getBytes() const {
        return bytes;
    }

    bool Utf8Char::operator==(const Utf8Char &rhs){
        if(bytes.size() != rhs.bytes.size())return false;
        for(int i=0;i<bytes.size();i++){
            if(bytes[i] != rhs.bytes[i])return false;
        }
        return true;
    }

    auto Utf8Char::operator<=>(const Utf8Char &rhs) const {
        return getCodePoint()<=>rhs.getCodePoint();
    }

    auto Utf8Char::operator<=>(const char c) const {
        return getCodePoint()<=>(data::u32)c;
    }

    CodePoint Utf8Char::getCodePoint() const {
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

    bool Utf8Char::empty() const {
        return bytes.empty();
    }

    void Utf8Char::clear() {
        bytes.clear();
    }


    data::u8 Utf8String::getUnitSize(unsigned char head) {
        for(int i=0;i<4;i++){
            if((head & bit_flag[i]) == 0)return i+1;
        }
        PANIC;
    }

    Utf8String::Utf8String(const char *c_str) {
        int len = strlen(c_str);
        bytes.resize(len);
        for(int i=0;i<len;i++)
            bytes[i] = c_str[i];
    }

    Utf8String::Utf8String(const char *c_str, std::size_t len) {
        bytes.resize(len);
        for(int i=0;i<len;i++)
            bytes[i] = c_str[i];
    }

    Utf8String::Utf8String(Utf8String::iterator begin, Utf8String::iterator end)
        : bytes(begin.beg,end.beg){}

    Utf8String::iterator Utf8String::begin() {
        return iterator(bytes,bytes.begin());
    }

    Utf8String::iterator Utf8String::end() {
        return iterator(bytes,bytes.end());
    }

    void Utf8String::push_back(Utf8Char c) {
        bytes.insert(bytes.end(),c.getBytes().begin(), c.getBytes().end());
    }

    std::size_t Utf8String::size() const {
        return bytes.size();
    }

    const Storage &Utf8String::getBytes() const {
        return bytes;
    }

    std::strong_ordering Utf8String::operator<=>(const Utf8String &rhs)const {
        return bytes <=> rhs.bytes;
    }

    void Utf8String::clear() {
        bytes.clear();
    }

    Utf8String Utf8String::operator+(const Utf8String &rhs) const {
        Utf8String ret = *this;
        for(auto byte : rhs.bytes){
            ret.bytes.push_back(byte);
        }
        return ret;
    }

    Utf8String Utf8String::operator+(const Utf8Char &rhs) const {
        Utf8String ret = *this;
        for(auto byte : rhs.getBytes()){
            ret.bytes.push_back(byte);
        }
        return ret;
    }

    bool Utf8String::empty() {
        return bytes.empty();
    }

    const char *Utf8String::c_str() const {
        return bytes.c_str();
    }

    Utf8String::Utf8String(std::string str) : bytes(str){}

    bool Utf8String::operator==(const Utf8String &rhs) {
        return *this<=>rhs == std::strong_ordering::equal;
    }

    Utf8String &Utf8String::operator+=(const Utf8String &rhs) {
        bytes.insert(bytes.end(),rhs.bytes.begin(),rhs.bytes.end());
        return *this;
    }

    Utf8String &Utf8String::operator+=(const Utf8Char &rhs) {
        bytes.insert(bytes.end(),rhs.getBytes().begin(),rhs.getBytes().end());
        return *this;
    }

    std::size_t Utf8String::count() {
        std::size_t s = 0,ans = 0;
        while(s<bytes.size()){
            s+=getUnitSize(bytes[s]);
            ans++;
        }
        return ans;
    }

    Utf8String Utf8String::substr(std::size_t pos, std::size_t len)const {
        std::size_t beg = 0,end;
        while(pos--){
            beg += getUnitSize(bytes[beg]);
        }
        end = beg;
        while(len--){
            end += getUnitSize(bytes[end]);
        }
        return Utf8String(bytes.substr(beg,beg-end));
    }

    std::ostream &operator<<(std::ostream &lhs, const Utf8String &rhs) {
        lhs << rhs.getBytes();
        return lhs;
    }

    std::ostream &operator<<(std::ostream &lhs, const Utf8Char &rhs) {
        for(auto c : rhs.getBytes()){
            lhs.put(c);
        }
        return lhs;
    }

    std::istream &operator>>(std::istream &lhs, Utf8Char &rhs) {
        rhs.clear();
        data::Byte byte;
        byte = lhs.get();
        rhs.bytes.push_back(byte);
        for(int i=0;i<Utf8Char::getRepresentByteCount(byte)-1;i++){
            byte = lhs.get();
            rhs.bytes.push_back(byte);
        }
        return lhs;
    }


    Utf8String::iterator::iterator(const Storage &bytes,const Storage::iterator begin)
            : bytes(bytes),beg(begin){}

    Utf8String::iterator &Utf8String::iterator::operator++() {
        if(beg!=bytes.end())
            beg += getUnitSize(*beg);
        return *this;
    }

    Utf8String::iterator Utf8String::iterator::operator++(int) {
        iterator prv = *this;
        if(beg!=bytes.end())
            beg += getUnitSize(*beg);
        return prv;
    }

    Utf8Char Utf8String::iterator::operator*() const {
        return Utf8Char(beg, getUnitSize(*beg));
    }

    bool Utf8String::iterator::operator==(const Utf8String::iterator &rhs) {
        return beg==rhs.beg;
    }

    Utf8String operator""_utf8(const char *c_str,std::size_t len){
        return Utf8String(c_str,len);
    }

    Utf8Char operator""_utf8char(const char *c_str,std::size_t len){
        return Utf8Char(c_str,len);
    }

    std::istream &getline(std::istream &is, Utf8String &str,Utf8Char delim){
        str.clear();
        Utf8Char c;
        while(true){
            if(is.eof())break;
            is >> c;
            if(c==delim||c.getCodePoint()==0)break;
            str.push_back(c);
        }
        return is;
    }

    std::istream &getline(std::istream &is, Utf8String &str){
        return getline(is,str,"\n"_utf8char);
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

    Utf8Char toLowerCase(const Utf8Char &c){
        if(isUpperCase(c))
            return c.getCodePoint()+('a'-'A');
        else
            return c;
    }

    Utf8String to_string(int value){
        return {std::to_string(value)};
    }

    int stoi(Utf8String str){
        return std::stoi(str.getBytes());
    }

    double stod(Utf8String str){
        return std::stod(str.getBytes());
    }
}
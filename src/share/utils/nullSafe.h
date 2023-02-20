//
// Created by yorkin on 10/26/21.
//

#ifndef EVOBASIC2_NULLSAFE_H
#define EVOBASIC2_NULLSAFE_H
#include <exception>
#include <utils/unicode.h>
#include "format.h"
#define DEBUG
#ifdef DEBUG

class AssertionException : public std::exception{
     evoBasic::unicode::Utf8String msg;
public:
    AssertionException(std::string file,evoBasic::unicode::Utf8String func,int line,evoBasic::unicode::Utf8String message){
        msg = Format() << "In file '" << file << "' function '" << func << "' line '" << line << "'," << message;
    }
    const char * what() const noexcept override{
        return msg.c_str();
    }
};
#define PANIC throw AssertionException(__FILE__,__func__,__LINE__,"")
#define PANICMSG(Msg) throw AssertionException(__FILE__,__func__,__LINE__,Msg)
#define ASSERT(Exp,Msg) if(Exp) PANICMSG(Msg)
#define ASSERT_TRUE(Exp) if(Exp) PANICMSG(# Exp)
#define NotNull(ptr) ASSERT(ptr==nullptr, #ptr " is null")

#else
define NotNull(ptr)
#endif

#endif //EVOBASIC2_NULLSAFE_H

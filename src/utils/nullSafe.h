//
// Created by yorkin on 10/26/21.
//

#ifndef EVOBASIC2_NULLSAFE_H
#define EVOBASIC2_NULLSAFE_H
#include<exception>
#include<string>
#include"formator.h"
#define DEBUG
#ifdef DEBUG

class AssertionException : public std::exception{
    std::string msg;
public:
    AssertionException(std::string file,std::string func,int line,std::string message){
        msg = format()<<"In file '"<<file<<"' function '"<<func<<"' line '"<<line<<"',"<<message;
    }
    const char * what() const noexcept override{
        return msg.c_str();
    }
};
#define ASSERT(Exp,Msg) if(Exp) throw AssertionException(__FILE__,__func__,__LINE__,Msg)
#define NotNull(ptr) ASSERT(ptr==nullptr, #ptr " is null")

#else
define NotNull(ptr)
#endif

#endif //EVOBASIC2_NULLSAFE_H

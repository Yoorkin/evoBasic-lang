//
// Created by yorkin on 7/15/21.
//

#include "Logger.h"
namespace evoBasic{
    Logger::Logger(ostream &out):out(out){

    }

    void Logger::log(string message){

    }

    void Logger::error(Token* token, string message){
        cout<<"error("<<token->y<<","<<token->x<<","<<Token::TokenToString[(int)token->token]<<"): "<<message<<endl;
    }

    void Logger::warning(Token* token, string message){
        cout<<"warning: "<<message<<endl;
    }
}


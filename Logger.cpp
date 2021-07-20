//
// Created by yorkin on 7/15/21.
//

#include "Logger.h"
namespace evoBasic{
    Logger::Logger(ostream &out):out(out){

    }

    void Logger::log(string message){

    }

    void Logger::error(Token& token, string message){

    }

    void Logger::warning(Token& token, string message){
        cout<<"warning: "<<message<<endl;
    }
}


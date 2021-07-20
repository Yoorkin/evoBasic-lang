//
// Created by yorkin on 7/15/21.
//

#ifndef EVOBASIC_LOGGER_H
#define EVOBASIC_LOGGER_H
#include<iostream>
#include<string>
#include"Lexer.h"
namespace evoBasic{
    using namespace std;
    class Logger {
        ostream& out;
    public:
        Logger(ostream& out);
        void log(string message);
        void error(Token& token,string message);
        void warning(Token& token,string message);
    };
}

#endif //EVOBASIC_LOGGER_H

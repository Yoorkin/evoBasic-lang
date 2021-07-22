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
    class Token;
    class Logger {
        ostream& out;
    public:
        Logger(ostream& out);
        void log(string message);
        static void error(Token* token,string message);
        static void warning(Token* token,string message);
    };
}

#endif //EVOBASIC_LOGGER_H

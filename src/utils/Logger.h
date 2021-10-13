//
// Created by yorkin on 7/15/21.
//

#ifndef EVOBASIC_LOGGER_H
#define EVOBASIC_LOGGER_H
#include<iostream>
#include<string>
#include<initializer_list>
#include<vector>
#include<iomanip>
#include<sstream>
#include<fstream>
#include<list>
#include<memory>
#include<iostream>

namespace evoBasic{
    using namespace std;
    class Position;
    class Token;
    class Logger {
        static std::ostream* out;
    public:
        Logger()=delete;
        static void redirect(std::ostream* stream);
        static bool debugMode;
        static int errorCount,warningCount;

        static void error(string message);
        static void warning(string message);
        static void dev(string message);

        static void code(const Position& pos);
        static void error(const Position& pos,const string& message);
        static void warning(const Position& pos,const string& message);
        static void panic(const std::list<std::pair<std::string,Position>>& callstack,const Position& pos,const string& message);
    };

    class Format{
    public:
        stringstream stream;
        template<typename T>
        Format & operator<<(T t){
            stream<<t;
            return *this;
        }
        operator string(){
            return move(stream.str());
        }
    };
}

#endif //EVOBASIC_LOGGER_H

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

namespace evoBasic{
    using namespace std;
    class Position;
    class Token;
    class Logger {
        vector<string> lines;
        string filePath;
        stringstream out;
    public:
        Logger(string filePath);
        void log(string message);
        static void error(string message);
        static void warning(string message);
        static void dev(string message);
        static bool debugMode;
        static int errorCount,warningCount;
        void error(Position pos,string message);
        void warning(Position pos,string message);
        void error(initializer_list<Position> posList,string message);
        void warning(initializer_list<Position> posList,string message);
        void flush(ostream &stream);
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

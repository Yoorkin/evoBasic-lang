//
// Created by yorkin on 7/15/21.
//

#include "Logger.h"
#include "../frontend/Lexer.h"

namespace evoBasic{

    void Logger::log(string message){
        out<<message;
    }

    void Logger::error(Position pos,string message) {
        Logger::errorCount++;
        out << "error(" << pos.y << "," << pos.x  << "): " << message << endl;
        out << setw(8) << setfill(' ');
        out <<pos.y<<" │ "<< lines[pos.y-1] << endl;
        for(int i=0;i<8;i++)out<<' ';
        out<<" │ ";
        for(int i=0;i<pos.x-1;i++)out<<' ';
        out<<'^';
        for(int i=1;i<pos.w;i++)out<<'~';
        out<<endl;
    }

    void Logger::warning(Position pos,string message) {
        Logger::warningCount++;
        out << "warning(" << pos.y << "," << pos.x  << "): " << message << endl;
        out << setw(8) << setfill(' ');
        out <<pos.y<<" │ "<< lines[pos.y-1] << endl;
        for(int i=0;i<8;i++)out<<' ';
        out<<" │ ";
        for(int i=0;i<pos.x-1;i++)out<<' ';
        out<<'^';
        for(int i=1;i<pos.w;i++)out<<'~';
        out<<endl;
    }

    Logger::Logger(string filePath):filePath(filePath){
        string str;
        ifstream in(filePath);
        while(getline(in,str)){
            lines.push_back(str);
        }
        lines.emplace_back("");
    }

    void Logger::print(ostream &stream) {
        stream<<endl<<"In file '"<<filePath<<"':"<<endl;
        stream<<out.str();
    }

    bool Logger::debugMode = false;

    int Logger::errorCount=0;
    int Logger::warningCount=0;

    void Logger::error(string message) {
        cout << "error: " << message << endl;
    }

    void Logger::warning(string message){
        cout << "warning: " << message << endl;
    }

    void Logger::dev(string message){
        if(debugMode)cout<<message;
    }

    void Logger::error(initializer_list<Position> posList, string message) {
        Logger::errorCount++;
        out << "error: " << message << endl;
        for(auto pos:posList){
            out << setw(8) << setfill(' ');
            out <<pos.y<<" │ "<< lines[pos.y-1] << endl;
            for(int i=0;i<8;i++)out<<' ';
            out<<" │ ";
            for(int i=0;i<pos.x-1;i++)out<<' ';
            out<<'^';
            for(int i=1;i<pos.w;i++)out<<'~';
            out<<endl;
        }
    }

    void Logger::warning(initializer_list<Position> posList, string message) {
        Logger::warningCount++;
        out << "warning: " << message << endl;
        for(auto pos:posList){
            out << setw(8) << setfill(' ');
            out <<pos.y<<" │ "<< lines[pos.y-1] << endl;
            for(int i=0;i<8;i++)out<<' ';
            out<<" │ ";
            for(int i=0;i<pos.x-1;i++)out<<' ';
            out<<'^';
            for(int i=1;i<pos.w;i++)out<<'~';
            out<<endl;
        }
    }
}


//
// Created by yorkin on 7/15/21.
//

#include "Logger.h"
#include "../frontend/Lexer.h"

namespace evoBasic{

    bool Logger::debugMode = false;

    int Logger::errorCount=0;
    int Logger::warningCount=0;

    void Logger::error(string message) {
        errorCount++;
        cout << "error: " << message << endl;
    }

    void Logger::warning(string message){
        warningCount++;
        cout << "warning: " << message << endl;
    }

    void Logger::dev(string message){
        if(debugMode)cout<<message;
    }

    ostream* Logger::out = &std::cout;

    void Logger::redirect(std::ostream *stream) {
        out = stream;
    }

    void Logger::code(const Position &pos) {
        *out << setw(8) << setfill(' ');
        *out << pos.getY()<<" │ "<< pos.getSource()->getLine(pos.getY()-1) << endl;
        for(int i=0;i<8;i++)*out<<' ';
        *out<<" │ ";
        for(int i=0;i<pos.getX()-1;i++)*out<<' ';
        *out<<'^';
        for(int i=1;i<pos.getW();i++)*out<<'~';
        *out<<endl;
    }

    void Logger::error(const Position &pos, const string &message) {
        Logger::errorCount++;
        *out << "error: " << message << endl;
        code(pos);
    }

    void Logger::warning(const Position &pos, const string &message) {
        Logger::warningCount++;
        *out << "warning: " << message << endl;
        code(pos);
    }

    void Logger::panic(const list<std::pair<std::string, Position>> &callstack, const Position &pos, const string &message) {
        for(const auto& call:callstack){
            *out<<"In: "<<call.first<<"  ("<<call.second.getY()<<','<<call.second.getX()<<')';
        }
        *out << "panic: " << message <<endl;
        code(pos);
    }

}


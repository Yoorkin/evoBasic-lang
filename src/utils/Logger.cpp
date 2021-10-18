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
        if(pos.getY() == pos.getEndY()){
            *out << setw(8) << setfill(' ');
            *out << pos.getY()<<" │ "<< pos.getSource()->getLine(pos.getY()-1) << endl;
            for(int i=0;i<8;i++)*out<<' ';
            *out<<" │ ";
            for(int i=0;i<pos.getX()-1;i++)*out<<' ';
            *out<<'^';
            for(int i=1;i<pos.getEndX()-pos.getX();i++)*out<<'~';
            *out<<endl;
        }
        else{
            *out << setw(8) << setfill(' ');
            auto str = pos.getSource()->getLine(pos.getY()-1);
            *out << pos.getY()<<" │ "<< str << endl;
            for(int i=0;i<8;i++)*out<<' ';
            *out<<" │ ";
            for(int i=0;i<pos.getX()-1;i++)*out<<' ';
            *out<<'^';
            for(int i=0;i<str.size()-pos.getX();i++)*out<<'~';
            *out<<endl;
            for(int i=pos.getY()+1;i<pos.getEndY();i++){
                str = pos.getSource()->getLine(i-1);
                *out << setw(8) << setfill(' ');
                *out << i <<" │ "<< str << endl;
                for(int i=0;i<8;i++)*out<<' ';
                *out<<" │ ";
                for(int j=1;j<str.size();j++)
                    *out<<"~";
                *out<<endl;
            }
            str = pos.getSource()->getLine(pos.getEndY()-1);
            *out << setw(8) << setfill(' ');
            *out << pos.getEndY()-1 <<" │ "<< str << endl;
            for(int i=0;i<8;i++)*out<<' ';
            *out<<" │ ";
            for(int j=1;j<pos.getEndX();j++)
                *out<<"~";
            *out<<endl;
        }
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
            if(&call == &callstack.front()){
                *out<<"In: "<<call.first;
            }
            else {
                *out<<"In: "<<call.first<<"  ("<<call.second.getY()<<','<<call.second.getX()<<')';
            }
        }
        *out << "panic: " << message <<endl;
        code(pos);
    }

}


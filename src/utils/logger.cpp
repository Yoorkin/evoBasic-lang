//
// Created by yorkin on 7/15/21.
//

#include "logger.h"
#include "nullSafe.h"
using namespace std;
namespace evoBasic{

    const string RED="\033[0;31m",YELLOW="\033[0;33m",PURPLE = "\033[0;35m",NC="\033[0m";

    bool Logger::debugMode = false;

    int Logger::errorCount=0;
    int Logger::warningCount=0;

    void Logger::error(string message) {
        errorCount++;
        *out << RED << "error: " << message << NC << endl;
    }

    void Logger::warning(string message){
        warningCount++;
        *out << YELLOW << "warning: " << message << NC << endl;
    }

    void Logger::dev(string message){
        if(debugMode)*out<<message;
    }

    ostream* Logger::out = &std::cout;

    void Logger::redirect(std::ostream *stream) {
        out = stream;
    }

    void Logger::code(const Location *pos,bool is_error) {
        NotNull(pos);
        NotNull(pos->getSource());
        if(pos->getBeginY() == pos->getEndY()){
            *out << setw(8) << setfill(' ');
            *out << pos->getBeginY() << " │ " << pos->getSource()->getLine(pos->getBeginY() - 1) << endl;
            for(int i=0;i<8;i++)*out<<' ';
            *out<<" │ ";
            for(int i=0;i<pos->getBeginX()-1;i++)*out<<' ';
            *out<< ( is_error ? RED : PURPLE) <<'^';
            for(int i=1;i<pos->getEndX()-pos->getBeginX();i++)*out<<'~';
            *out<<NC<<endl;
        }
        else{
            *out << setw(8) << setfill(' ');
            auto str = pos->getSource()->getLine(pos->getBeginY()-1);
            *out << pos->getBeginY()<<" │ "<< str << endl;
            for(int i=0;i<8;i++)*out<<' ';
            *out<<" │ ";
            for(int i=0;i<pos->getBeginX()-1;i++)*out<<' ';
            *out<<'^';
            for(int i=0;i<str.size()-pos->getBeginX();i++)*out<<'~';
            *out<<endl;
            for(int i=pos->getBeginY()+1;i<pos->getEndY();i++){
                str = pos->getSource()->getLine(i-1);
                *out << setw(8) << setfill(' ');
                *out << i <<" │ "<< str << endl;
                for(int i=0;i<8;i++)*out<<' ';
                *out<<" │ ";
                for(int j=1;j<str.size();j++)
                    *out<<"~";
                *out<<endl;
            }
            str = pos->getSource()->getLine(pos->getEndY()-1);
            *out << setw(8) << setfill(' ');
            *out << pos->getEndY()-1 <<" │ "<< str << endl;
            for(int i=0;i<8;i++)*out<<' ';
            *out<<" │ ";
            for(int j=1;j<pos->getEndX();j++)
                *out<<"~";
            *out<<endl;
        }
    }

    void Logger::error(const Location *pos, const string &message) {
        Logger::errorCount++;
        *out << RED << "error: " << message << NC << endl;
        code(pos,true);
    }

    void Logger::warning(const Location *pos, const string &message) {
        Logger::warningCount++;
        *out << PURPLE << "warning: " << message << NC << endl;
        code(pos);
    }

    void Logger::panic(const list<std::pair<std::string, Location*>> &callstack, const Location *pos, const string &message) {
        for(const auto& call:callstack){
            if(&call == &callstack.front()){
                *out<<"In: "<<call.first;
            }
            else {
                *out<<"In: "<<call.first<<"  ("<<call.second->getBeginY()<<','<<call.second->getBeginX()<<')';
            }
        }
        *out << "panic: " << message <<endl;
        code(pos);
    }

}


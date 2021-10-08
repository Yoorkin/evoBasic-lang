//
// Created by yorkin on 7/28/21.
//

#ifndef EVOBASIC_EXCEPTION_H
#define EVOBASIC_EXCEPTION_H
#include<exception>
#include<string>
#include "Lexer.h"

namespace evoBasic{
    using namespace std;
    class MemberNotFindException:public exception{
    public:
        string memberName;
        explicit MemberNotFindException(string name):memberName(std::move(name)){}
    };

    class NameConflictException:public exception{
    public:
        string name;
        Position namePos,conflictNamePos;
        explicit NameConflictException(string name):name(std::move(name)){}

        NameConflictException(string name,Position namePos,Position conflictNamePos)
                :name(std::move(name)),namePos(namePos),conflictNamePos(conflictNamePos){}

        const char * what() const noexcept override{
            return name.c_str();
        }
    };

    class OptionalNotFindException:public exception{
    public:
        string optionalName;
        explicit OptionalNotFindException(string optionalName):optionalName(move(optionalName)){}
        const char * what() const noexcept override{
            return optionalName.c_str();
        }
    };

    class SyntaxException:public exception{
    public:
        const Token& token;
        explicit SyntaxException(const Token& token,Token::Enum expected):token(token),expected(expected){};
        Token::Enum expected;
        const char * what() const noexcept override{
            return (Token::reserved[(int)token.kind] + " but expected "+ Token::reserved[(int)expected]).c_str();
        }
    };

}


#endif //EVOBASIC_EXCEPTION_H

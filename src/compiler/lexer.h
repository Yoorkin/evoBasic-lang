//
// Created by yorkin on 10/26/21.
//

#ifndef EVOBASIC2_LEXER_H
#define EVOBASIC2_LEXER_H
#include "token.h"
#include "format.h"
#include <set>
namespace evoBasic{
    class SyntaxException:public std::exception{
        std::string tmp;
    public:
        const Token& token;
        explicit SyntaxException(Token& token,Token::Enum expected):token(token),expected(expected){
            auto l = token.getLocation();
            tmp = Format() << token.getLexeme() << " but expected " << Token::enum_to_string[(int)expected]
                           << " in (" << l->getBeginY() << "," << l->getBeginX() << ")";
        };
        Token::Enum expected;
        const char * what() const noexcept override{
            return tmp.c_str();
        }
    };

    class Lexer {
        int begin_x,begin_y;
        int x=-1,y=1;
        char c = -1,lexeme_char = -1;
        bool resume_char = true;
        std::string lexeme;

        Source *source_;
        Token *LL();
        std::vector<Token*> tokens;
        int next_token = 0;
    public:
        explicit Lexer(Source* source);
        Token* getNextToken();
        Token* getToken();
        void match(Token::Enum kind);
        void match(Token::Enum kind,std::set<Token::Enum> follows,std::string message);
        bool predict(Token::Enum kind);
        void skipUntil(std::set<Token::Enum> follows);
    };

}


#endif //EVOBASIC2_LEXER_H

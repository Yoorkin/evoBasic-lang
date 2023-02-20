//
// Created by yorkin on 10/26/21.
//

#include "lexer.h"
#include <utils/logger.h>
#include <functional>
#include <cctype>
#include <utils/format.h>
#include <utils/nullSafe.h>
#include <utils/unicode.h>
using namespace std;
using namespace evoBasic::unicode;
namespace evoBasic{

    bool isNewLine(Utf8Char &c) {
        return c == '\n' || c == '\r';
    }

    bool isWhiteSpace(Utf8Char &c) {
        return c == '\t' || c == ' ' || isNewLine(c);
    }

    bool isValidCharForID(Utf8Char &c){
        return isAlpha(c) || isBasicHan(c);
    }

    Lexer::Lexer(Source *source) : source_(source), iter(source->getSource().begin()) {

        NotNull(source);
        map<Token::Enum, Token::Enum> merge = {
                {Token::if_,       Token::END_IF},
                {Token::sub_,      Token::END_SUB},
                {Token::function_, Token::END_FUNCTION},
                {Token::module_,   Token::END_MODULE},
                {Token::enum_,     Token::END_ENUM},
                {Token::class_,    Token::END_CLASS},
                {Token::select_,   Token::END_SELECT},
                {Token::type_,     Token::END_TYPE},
                {Token::operator_, Token::END_OPERATOR},
                {Token::interface_,Token::END_INTERFACE},
                {Token::new_,      Token::END_NEW}
        };
        Token *token = nullptr,*next = LL();
        do{
            token = next;
            next = LL();
            if(token->getKind() == Token::end_){
                auto trans = merge.find(next->getKind());
                if(trans != merge.end()){
                    Location location(token->getLocation(),next->getLocation());
                    auto tmp = new Token(location,trans->second, token->getLexeme() + ' ' + next->getLexeme());
                    delete token;
                    delete next;
                    token = tmp;
                    next = LL();
                }
            }
            tokens.push_back(token);
        }while(token->getKind() != Token::EOF_);

        for(auto& token:tokens){
            Logger::print(Channel::Tokens,token->toString());
            Logger::print(Channel::Tokens,"\n");
        }
    }

    Token *Lexer::getToken() {
        ASSERT(next_token==0,"invalid");
        return tokens[next_token-1];
    }

    Token *Lexer::getNextToken(){
        return tokens[next_token];
    }

    void Lexer::match(Token::Enum kind) {
        if(getNextToken()->getKind() == kind){
            Logger::print(Channel::Tokens,Format() << "match: " << getNextToken()->toString() << '\n');
            next_token++;
        }
        else throw SyntaxException(*getNextToken(),kind);
    }

    void Lexer::match(Token::Enum kind,std::set<Token::Enum> follows,unicode::Utf8String message){
        if(getNextToken()->getKind() == kind){
            Logger::print(Channel::Tokens,Format() << "match: " << getNextToken()->toString() << '\n');
            next_token++;
        }
        else{
            Logger::error(getNextToken()->getLocation(),message);
            skipUntil(move(follows));
        }
    }

    void Lexer::skipUntil(std::set<Token::Enum> follows) {
        if(next_token == tokens.size())throw "error";
        while(!follows.contains(getNextToken()->getKind())){
            Logger::print(Channel::Tokens,Format() << "skip : " << getNextToken()->toString() << '\n');
            next_token++;
        }
    }

    Token *Lexer::LL() {

        enum State {
            Err, START, NUM, POINT, E1, E2, ID1, ID2, COM1, COM2, CB1, CB2, STR, CHAR1, CHAR2, L, E, G, END
        } state = START;
        Token::Enum result_kind;

        auto noResumeState = [&](State state_){
            state = state_;
            resume_char = false;
        };

        auto endState = [&](bool resume_,Token::Enum kind_){
            state = END;
            resume_char = resume_;
            result_kind = kind_;
        };

        auto error = [&](){
            state = START;
            Logger::error(new Location(begin_x,x,begin_y,source_),"Token error");
        };


        while(true){

            if(resume_char){
                if(isNewLine(c)){
                    y++;x=1;
                }
                else {
                    x++;
                }

                if(iter==source_->getSource().end()) {
                    lexeme_char.clear();
                    c.clear();
                }
                else{
                    lexeme_char = *iter;
                    c = toLowerCase(lexeme_char);
                    iter++;
                }

            }
            else {
                resume_char = true;
            }

            switch (state) {
                case Err:
                    break;
                case START:
                    lexeme.clear();
                    begin_x = x;
                    begin_y = y;
                    if(isDigit(c))           state = NUM;
                    else if(isValidCharForID(c))     state = ID1;
                    else if(c == '/')        state = COM1;
                    else if(c == '"')        state = STR;
                    else if(c == '\'')       state = CHAR1;
                    else if(c == '<')        state = L;
                    else if(c == '=')        state = E;
                    else if(c == '>')        state = G;
                    else if(isWhiteSpace(c)) state = START;
                    else if(c == '+')        endState(true,Token::ADD);
                    else if(c == '-')        endState(true,Token::MINUS);
                    else if(c == ')')        endState(true,Token::RP);
                    else if(c == '/')        endState(true,Token::FDIV);
                    else if(c == '\\')       endState(true,Token::DIV);
                    else if(c == '*')        endState(true,Token::MUL);
                    else if(c == '(')        endState(true,Token::LP);
                    else if(c == '.')        endState(true,Token::DOT);
                    else if(c == ',')        endState(true,Token::COMMA);
                    else if(c == '[')        endState(true,Token::LB);
                    else if(c == ']')        endState(true,Token::RB);
                    else if(c == ':')        endState(true,Token::COLON);
                    else if(c.empty())       endState(false,Token::EOF_);
                    else error();
                    break;
                case NUM:
                    if(isDigit(c))       state = NUM;
                    else if(c == 'e')    state = E1;
                    else if(c == '.')    state = POINT;
                    else endState(false,Token::DIGIT);
                    break;
                case POINT:
                    if(isDigit(c))       state = POINT;
                    else if(c == 'e')    state = E1;
                    else endState(false,Token::DECIMAL);
                    break;
                case E1:
                    if(c == '-') state = E2;
                    else if(isDigit(c)) noResumeState(E2);
                    else error();
                    break;
                case E2:
                    if(isDigit(c)) state = E2;
                    else endState(false,Token::DECIMAL);
                    break;
                case ID1:
                    if(isValidCharForID(c))     state = ID1;
                    else if(isDigit(c)) state = ID2;
                    else endState(false,Token::ID);
                    break;
                case ID2:
                    if(isDigit(c))      state = ID2;
                    else if(isValidCharForID(c))state = ID1;
                    else endState(false,Token::ID);
                    break;
                case COM1:
                    if(c == '/')      state = COM2;
                    else if(c == '*') state = CB1;
                    else error();
                    break;
                case COM2:
                    if(isNewLine(c)) state = START;
                    else state = COM2;
                    break;
                case CB1:
                    if(c == '*')  state = CB2;
                    else          state = CB1;
                    break;
                case CB2:
                    if(c == '/') state = START;
                    break;
                case STR:
                    if(c == '"') endState(true,Token::STRING);
                    else state = STR;
                    break;
                case CHAR1:
                    state = CHAR2;
                    break;
                case CHAR2:
                    if(c == '\'') endState(true,Token::CHAR);
                    break;
                case L:
                    if(c == '>') endState(true,Token::NE);
                    else if(c == '=') endState(true,Token::LE);
                    else endState(false,Token::LT);
                    break;
                case E:
                    if(c == '<')        endState(true,Token::LE);
                    else if(c == '=')   endState(true,Token::EQ);
                    else                endState(false,Token::ASSIGN);
                    break;
                case G:
                    if(c == '=') endState(true,Token::GE);
                    else         endState(false,Token::GT);
                    break;
                case END:
                    resume_char = false;
                    if(result_kind == Token::ID){
                        Utf8String tmp;
                        for(auto c : lexeme){
                            tmp.push_back(toLowerCase(c));
                        }
                        auto ret = Token::reserved_words.find(tmp);
                        if(ret != Token::reserved_words.end()){
                            result_kind = ret->second;
                        }
                    }
                    auto token = new Token(Location(begin_x,x,y,source_),result_kind,lexeme);
                    return token;
                    break;
            }

            if(resume_char)lexeme.push_back(lexeme_char);
        }


    }

    bool Lexer::predict(Token::Enum kind) {
        return getNextToken()->getKind() == kind;
    }


}

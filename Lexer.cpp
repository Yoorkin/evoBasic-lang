//
// Created by yorkin on 7/11/21.
//

#include "Lexer.h"
#include<iostream>
#define ReturnWithoutForward(token) {stream.putback(c);return Token(lexeme,token,beginX,y);}
#define ReturnWithForward(token) {lexeme.push_back(c);return Token(lexeme,token,beginX,y);}
namespace evoBasic{
    bool isDigit(char c){
        return c<='9' && c>='0';
    }
    bool isLetter(char c) {
        return c>='a' && c<='z';
    }
    bool isNewLine(char c){
        return c=='\n'||c=='\r';
    }
    bool isWhiteSpace(char c){
        return c=='\t'||c==' '||isNewLine(c);
    }
    void error(){

    }
    char ToLower(char c) {
        if(c>='A'&&c<='Z') return c-'A'+'a';
        else return c;
    }
    Token Lexer::LL(){
        string lexeme;
        enum {START,NUM,POINT,E1,E2,ID1,ID2,COM1,COM2,CB1,CB2,STR,CHAR1,CHAR2,L,E,G}state=START;
        beginX=x;
        while(true){
            x++;
            char c = ToLower(stream.get());
            if(stream.eof())c=-1;
            switch(state){
                case START:
                    lexeme="";
                    if(isDigit(c))state=NUM;
                    else if(isLetter(c))state=ID1;
                    else if(c=='/')state=COM1;
                    else if(c=='"')state=STR;
                    else if(c=='\'')state=CHAR1;
                    else if(c=='<')state=L;
                    else if(c=='(')ReturnWithForward(Token::LB)
                    else if(c==')')ReturnWithForward(Token::RB)
                    else if(c=='*')ReturnWithForward(Token::MUL)
                    else if(c=='\\')ReturnWithForward(Token::DIV)
                    else if(c=='+')ReturnWithForward(Token::ADD)
                    else if(c=='-')ReturnWithForward(Token::MINUS)
                    else if(c=='.')ReturnWithForward(Token::DOT)
                    else if(c==',')ReturnWithForward(Token::COMMA)
                    else if(isWhiteSpace(c)){
                        if(isNewLine(c)){
                            beginX=x=0;
                            y++;
                        }
                        state=START;
                    }
                    else if(c=='<')state=L;
                    else if(c=='=')state=E;
                    else if(c=='>')state=G;
                    else if(c==-1)return Token("EOF",Token::EOF_);
                    else error();
                    break;
                case NUM:
                    if(isDigit(c))state=NUM;
                    else if(c=='.')state=POINT;
                    else if(c=='e')state=E1;
                    else ReturnWithoutForward(Token::DIGIT)
                    break;
                case POINT:
                    if(isDigit(c))state=POINT;
                    else if(c=='e')state=E1;
                    else ReturnWithoutForward(Token::DECIMAL)
                    break;
                case E1:
                    if(c=='-'||isDigit(c))state=E2;
                    else error();
                    break;
                case E2:
                    if(isDigit(c))state=E2;
                    else ReturnWithoutForward(Token::DECIMAL)
                    break;
                case ID1:
                    if(isLetter(c))state=ID1;
                    else if(isDigit(c))state=ID2;
                    else {
                        auto res = reserved.find(lexeme);
                        if(res==reserved.end())ReturnWithoutForward(Token::ID)
                        else ReturnWithoutForward(res->second.token)
                    }
                    break;
                case ID2:
                    if(isDigit(c))state=ID2;
                    else if(isLetter(c))state=ID1;
                    else {
                        auto res = reserved.find(lexeme);
                        if(res==reserved.end())ReturnWithoutForward(Token::ID)
                        else ReturnWithoutForward(res->second.token)
                    }
                    break;
                case COM1:
                    if(c=='/')state=COM2;
                    else if(c=='*')state=CB1;
                    else ReturnWithoutForward(Token::FDIV)
                    break;
                case COM2:
                    if(isNewLine(c))state=START;
                    else state=COM2;
                    break;
                case CB1:
                    if(c=='*')state=CB2;
                    else state=CB1;
                    break;
                case CB2:
                    if(c=='/')state=START;
                    else error();
                    break;
                case STR:
                    if(c=='"')ReturnWithForward(Token::STRING)
                    else state=STR;
                    break;
                case CHAR1:
                    state=CHAR2;
                    break;
                case CHAR2:
                    if(c=='\'')ReturnWithForward(Token::CHAR)
                    break;
                case L:
                    if(c=='>')ReturnWithForward(Token::NE)
                    else if(c=='=')ReturnWithForward(Token::LE)
                    else ReturnWithoutForward(Token::LT);
                    break;
                case E:
                    if(c=='<')ReturnWithForward(Token::LE)
                    else if(c=='=')ReturnWithForward(Token::EQ)
                    else ReturnWithoutForward(Token::ASSIGN)
                    break;
                case G:
                    if(c=='=')ReturnWithForward(Token::GE)
                    else ReturnWithoutForward(Token::GT);
                    break;
            }
            lexeme.push_back(c);
        }

    }

    Lexer::Lexer(istream &in):stream(in){
        reserved={
                {"module",Token("module",Token::module_)},
                {"class",Token("class",Token::class_)},
                {"public",Token("public",Token::public_)},
                {"private",Token("private",Token::private_)},
                {"friend",Token("friend",Token::friend_)},
                {"static",Token("static",Token::static_)},
                {"virtual",Token("virtual",Token::virtual_)},
                {"override",Token("override",Token::override_)},
                {"function",Token("function",Token::function_)},
                {"sub",Token("sub",Token::sub_)},
                {"byval",Token("byval",Token::byval_)},
                {"byref",Token("byref",Token::byref_)},
                {"optional",Token("optional",Token::optional_)},
                {"as",Token("as",Token::as_)},
                {"let",Token("let",Token::let_)},
                {"end",Token("end",Token::end_)},
                {"enum",Token("enum",Token::enum_)},
                {"type",Token("type",Token::type_)},
                {"select",Token("select",Token::select_)},
                {"case",Token("case",Token::case_)},
                {"for",Token("for",Token::for_)},
                {"to",Token("to",Token::to_)},
                {"step",Token("step",Token::step_)},
                {"while",Token("while",Token::while_)},
                {"wend",Token("wend",Token::wend_)},
                {"if",Token("if",Token::if_)},
                {"then",Token("then",Token::then_)},
                {"elseif",Token("elseif",Token::elseif_)},
                {"else",Token("else",Token::else_)},
                {"and",Token("and",Token::and_)},
                {"or",Token("or",Token::or_)},
                {"xor",Token("xor",Token::xor_)},
                {"not",Token("not",Token::not_)},
                {"return",Token("return",Token::return_)},
                {"continue",Token("continue",Token::continue_)},
                {"exit",Token("exit",Token::exit_)},
                {"next",Token("next",Token::next_)}
        };
        nextToken=LL();
    }

    Token& Lexer::getNextToken(){
        return nextToken;
    }

    Token& Lexer::getToken(){
        return token;
    }

    bool Lexer::match(string lexeme){
        if(lexeme==nextToken.lexeme){
            this->token=nextToken;
            nextToken=LL();
            return true;
        }
        return false;
    }

    bool Lexer::match(Token::Enum token){
        if(token==nextToken.token){
            this->token=nextToken;
            nextToken=LL();
            cout.width(20);
            cout.setf(ios::left);
            cout.fill(' ');
            std::cout<<nextToken.lexeme<<' '<<nextToken.y<<':'<<nextToken.x<<endl;
            cout.unsetf(ios::left);
            return true;
        }else throw "unimplement";//TODO
        return false;
    }
}

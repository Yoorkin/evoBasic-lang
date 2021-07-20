//
// Created by yorkin on 7/11/21.
//
#include<istream>
#include<string>
#include<map>
#include<vector>
#ifndef EVOBASIC_LEXER_H
#define EVOBASIC_LEXER_H
using namespace std;
namespace evoBasic{
    class Token{
    public:
        enum Enum{ID,DIGIT,DECIMAL,STRING,CHAR,
                DOT,COMMA,LE,GE,EQ,LT,GT,NE,LB,RB,MUL,DIV,FDIV,MINUS,ADD,ASSIGN,EOF_,
                module_,class_,public_,private_,friend_,static_,virtual_,override_,
                function_,sub_,byval_,byref_,optional_,as_,let_,end_,enum_,type_,
                select_,case_,for_,to_,step_,while_,wend_,if_,then_,elseif_,else_,
                and_,or_,xor_,not_,return_,continue_,exit_,next_}token;
        string lexeme;
        int x,y;
        Token(string lexeme,Enum token):lexeme(move(lexeme)),token(token){}
        Token(string lexeme,Enum token,int x,int y):lexeme(move(lexeme)),token(token),x(x),y(y){}
        explicit Token(Enum token){this->token=token;}
        Token()=default;
    };

    class Lexer {
        int x=0,y=1,beginX=0;
        Token nextToken;
        Token token;
        char c;
        istream& stream;
    public:
        Token LL();
        map<string,Token> reserved;
        explicit Lexer(istream &in);
        Token& getNextToken();
        Token& getToken();
        bool match(string lexeme);
        bool match(Token::Enum token);
    };

}



#endif //EVOBASIC_LEXER_H

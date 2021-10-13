//
// Created by yorkin on 7/11/21.
//

#ifndef EVOBASIC_LEXER_H
#define EVOBASIC_LEXER_H
#include<istream>
#include<string>
#include<map>
#include <utility>
#include<vector>
#include<set>
#include<list>
#include<memory>
#include"../utils/Logger.h"
using namespace std;
namespace evoBasic{

    class Source{
        std::ifstream stream;
        std::string file_path;
        std::vector<std::string> lines;
    public:
        explicit Source(const std::string& file_path);
        const std::string& operator[](int idx);
        const std::string& getLine(int idx);
        std::istream& getStream();
    };

    class Position{
        int x,y,w;
        std::shared_ptr<Source> source;
    public:
        Position(int x,int y,int w,std::shared_ptr<Source> source):x(x),y(y),w(w),source(std::move(source)){}
        explicit Position():x(-1),y(-1),w(-1){}
        [[nodiscard]] int getX()const{return x;}
        [[nodiscard]] int getY()const{return y;}
        [[nodiscard]] int getW()const{return w;}
        [[nodiscard]] const std::shared_ptr<Source>& getSource()const{return source;}
        static Position accross(const Position& begin,const Position& end);
    };

#define E(x) x
#define E_(x) x##_
#define LEXEME_LIST E(Error),E(ID),E(DIGIT),E(DECIMAL),E(STRING),E(CHAR),\
    E(DOT),E(COMMA),E(LE),E(GE),E(EQ),E(LT),E(GT),E(NE),E(LB),E(RB),E(MUL),E(DIV),E(FDIV),E(MINUS),E(ADD),E(ASSIGN),E(EOF_),\
    E_(module),E_(class),E_(public),E_(private),E_(friend),E_(static),E_(virtual),E_(override),\
    E_(function),E_(sub),E_(byval),E_(byref),E_(optional),E_(as),E_(let),E_(end),E_(enum),E_(type),\
    E_(select),E_(case),E_(for),E_(to),E_(step),E_(while),E_(wend),E_(if),E_(then),E_(elseif),E_(else),\
    E_(and),E_(or),E_(xor),E_(not),E_(return),E_(continue),E_(exit),E_(next),E_(false),E_(true),\
    E(end_if),E(end_function),E(end_sub),E(end_module),E(end_class),E(end_type),E(end_enum),E(end_select),E(import),E(declare),E(lib),E(alias),E(impl)

    class Token{
    public:
        enum Enum{LEXEME_LIST}kind;
        string lexeme;
        Position pos;
        Token()=default;
        Token(string lexeme,Enum token,Position position)
            : lexeme(move(lexeme)), kind(token), pos(move(position)){}
//        explicit Token(Enum token){this->token=token;}
        static const vector<string> reserved;
        static const map<string,Token::Enum> reservedToEnumMap;
    };

    class Lexer {
        int x=1,y=1,beginX,beginY;
        char c;
        void increaseX(char c);
        //vector<string> lines;
        vector<Token> tokens;
        int current_idx=-1;
        std::shared_ptr<Source> source;
    public:
        Token LL();
        explicit Lexer(std::shared_ptr<Source> source);
        const Token& getNextToken();
        const Token& getToken();
        void match(Token::Enum token);
        void match(initializer_list<Token::Enum> token_seq);
        bool match(Token::Enum token,set<Token::Enum> follows,string errorMessage);
        bool match(initializer_list<Token::Enum> token_seq,set<Token::Enum> follows,string errorMessage);
        void traceBack(int count=1);
        void forward(int count=1);
        void skipUntilFollow(const set<Token::Enum>& follows);
        void pushAndPrint(Token token);
        void printLexeme(Token nextToken, string action);
    };

    void printFollows(set<Token::Enum> follows,string name);

}



#endif //EVOBASIC_LEXER_H

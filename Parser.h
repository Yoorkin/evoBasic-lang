//
// Created by yorkin on 7/13/21.
//

#ifndef EVOBASIC_PARSER_H
#define EVOBASIC_PARSER_H
#include"Lexer.h"
#include"AST.h"
#include"Type.h"
#include"Logger.h"
#include<exception>
#include<set>
namespace evoBasic{

    class Parser {
        Lexer& lexer;
        Domain& global;
    public:
        Parser(Lexer& lexer,Domain& domain);
        Node* parse();

        Node *expr(set<Token::Enum> follows={});

        Node *cmp();

        Node *add();

        Node *term();

        Node *factor();

        Node *unit();

        Node *terminal();

        Node *argsBody();

        Node *locating();

        Node *ifStmt();

        Node *elsePart();

        Node *stmtSet();

        Node *stmt();

        Node *loopStmt();

        Node *forStmt();

        Node *controlStmt();

        Node *selectStmt();

        Node *variableDecl();

        Node *functionDecl();

        Node *parameterList();

        Node *subDecl();

        Node *ID();

        Node *enumDecl();

        Node *DIGIT();

        Node *typeDecl();

        Node *moduleDecl();

        Node *classDecl();

        Node *accessFlag();

        Node *methodFlag();

        Node *classMember();

        Node *moduleMember();
    };

    class SyntaxException:public exception{
    public:
        SyntaxException(Token* token):token(token){};
        Token* token;
        const char *what() const throw ()
        {
            return "test";
        }
    };

}



#endif //EVOBASIC_PARSER_H

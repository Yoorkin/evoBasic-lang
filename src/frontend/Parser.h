//
// Created by yorkin on 7/13/21.
//

#ifndef EVOBASIC_PARSER_H
#define EVOBASIC_PARSER_H
#include"Lexer.h"
#include"AST.h"
#include"Type.h"
#include"../utils/Logger.h"
#include<exception>
#include<set>
namespace evoBasic{
    set<Token::Enum> combine(set<Token::Enum> a,set<Token::Enum> b);

    class Parser {
        Lexer &lexer;
    public:
        explicit Parser(Lexer &lexer);
        AST parse();

        shared_ptr<Node> expr(set<Token::Enum> follows={});

        shared_ptr<Node> logic(set<Token::Enum> follows={});

        shared_ptr<Node> logicNot(set<Token::Enum> follows={});

        shared_ptr<Node> cmp(set<Token::Enum> follows={});

        shared_ptr<Node> add(set<Token::Enum> follows={});

        shared_ptr<Node> term(set<Token::Enum> follows={});

        shared_ptr<Node> factor(set<Token::Enum> follows={});

        shared_ptr<Node> unit(set<Token::Enum> follows={});

        shared_ptr<Node> terminal(set<Token::Enum> follows={});

        shared_ptr<Node> argsBody(set<Token::Enum> follows={});

        shared_ptr<Node> locating(set<Token::Enum> follow={});

        shared_ptr<Node> importDecl(set<Token::Enum> follow);

        shared_ptr<Node> declareDecl(set<Token::Enum> follow);

        shared_ptr<Node> ifStmt(set<Token::Enum> follow);

        shared_ptr<Node> stmtSet(set<Token::Enum> follow={});

        shared_ptr<Node> stmt(set<Token::Enum> follow={});

        shared_ptr<Node> loopStmt(set<Token::Enum> follow);

        shared_ptr<Node> forStmt(set<Token::Enum> follow={});

        shared_ptr<Node> controlStmt(set<Token::Enum> follow={});

        shared_ptr<Node> selectStmt(set<Token::Enum> follow={});

        shared_ptr<Node> variableDecl(set<Token::Enum> follow={});

        shared_ptr<Node> functionDecl(set<Token::Enum> follows,bool need_definition=true);

        shared_ptr<Node> initDecl(set<Token::Enum> follows);

        shared_ptr<Node> operatorDecl(set<Token::Enum> follows);

        shared_ptr<Node> parameterList(set<Token::Enum> follows);

        shared_ptr<Node> subDecl(set<Token::Enum> follows,bool need_definition=true);

        shared_ptr<Node> ID(set<Token::Enum> follows={});

        shared_ptr<Node> enumDecl(set<Token::Enum> follows);

        shared_ptr<Node> DIGIT(set<Token::Enum> follow={});

        shared_ptr<Node> typeDecl(set<Token::Enum> follow={});

        shared_ptr<Node> moduleDecl(set<Token::Enum> follow);

        shared_ptr<Node> classDecl(set<Token::Enum> follow);

        shared_ptr<Node> classMember(const set<Token::Enum>& follow);

        shared_ptr<Node> moduleMember(const set<Token::Enum>& follow);

        shared_ptr<Node> DEMICAL();

        shared_ptr<Node> STRING();

        shared_ptr<Node> CHAR();

        shared_ptr<Node> globalDecl();

        AccessFlag accessFlag();

        MethodFlag methodFlag();

        shared_ptr<Node> interfaceDecl(set<Token::Enum> follows);
    };

}



#endif //EVOBASIC_PARSER_H

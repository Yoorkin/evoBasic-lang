//
// Created by yorkin on 10/27/21.
//

#ifndef EVOBASIC2_PARSER_H
#define EVOBASIC2_PARSER_H
#include "parseTree.h"
#include "lexer.h"
#include <set>
namespace evoBasic{
    using Follows = const std::set<Token::Enum>&;
    
    class Parser {
        Lexer *lexer;
    public:
        explicit Parser(Lexer *lexer);
        parseTree::Global *parseGlobal();
        parseTree::Class *parseClass(Follows follows);
        parseTree::Module *parseModule(Follows follows);
        parseTree::Type *parseType(Follows follows);
        parseTree::Enum *parseEnum(Follows follows);
        parseTree::Member *parseMember(Follows follows);
        AccessFlag parseAccessFlag();
        parseTree::Implement *parseImplement(Follows follows);

        parseTree::Import *parseImport(Follows follows);
        parseTree::Dim *parseDim(Follows follows);
        parseTree::Variable *parseVariable(Follows follows);
        parseTree::Function *parseFunction(Follows follows);
        parseTree::External *parseExternal(Follows follows);
        parseTree::Constructor *parseConstructor(Follows follows);
        parseTree::EnumMember *parseEnumMember(Follows follows);
        parseTree::Interface *parseInterface(Follows follows);
        parseTree::Function *parseFunctionInterface(Follows follows);
        parseTree::Function *parseSubInterface(Follows follows);


        parseTree::Parameter *parseParameter(Follows follows);
        parseTree::expr::ID *parseID(Follows follows);

        parseTree::stmt::Let *parseLet(Follows follows);
        parseTree::stmt::Select *parseSelect(Follows follows);
        parseTree::stmt::Loop *parseLoop(Follows follows);
        parseTree::stmt::If *parseIf(Follows follows);
        parseTree::stmt::For *parseFor(Follows follows);
        parseTree::stmt::Return *parseReturn(Follows follows);
        parseTree::stmt::Exit *parseExit(Follows follows);

        parseTree::stmt::ExprStmt *parseExprStmt(Follows follows);
        parseTree::expr::Expression *parseLogic(Follows follows);
        parseTree::expr::Expression *parseCmp(Follows follows);
        parseTree::expr::Expression *parseAdd(Follows follows);
        parseTree::expr::Expression *parseTerm(Follows follows);
        parseTree::expr::Expression *parseFactor(Follows follows);
        parseTree::expr::Expression *parseUnary(Follows follows);
        parseTree::expr::Expression *parseDot(Follows follows);
        parseTree::expr::Expression *parseTerminal(Follows follows);

        parseTree::Annotation *parseAnnotation(Follows follows);
        parseTree::AnnotationUnit *parseAnnotationUnit(Follows follows);

        //callee\id\index
        parseTree::expr::Expression *parseUnit(Follows follows);
        parseTree::expr::Callee::Argument *parseArg(Follows follows);

        parseTree::expr::Digit *parseDigit(Follows follows);
        parseTree::expr::Decimal *parseDecimal(Follows follows);
        parseTree::expr::String *parseString(Follows follows);
        parseTree::expr::Char *parseChar(Follows follows);
        parseTree::expr::Boolean *parseBoolean(Follows follows);
        parseTree::expr::Expression *parseParentheses(Follows follows);

        parseTree::Parameter *parseParameterList(Follows follows);
        parseTree::stmt::Statement *parseStmtList(Follows follows);
        parseTree::expr::Callee::Argument* parseArgsList(Follows follows);
        parseTree::expr::New *parseNew(Follows follows);
    };

    parseTree::Annotation *constructAnnotationAST (std::string code);
}


#endif //EVOBASIC2_PARSER_H

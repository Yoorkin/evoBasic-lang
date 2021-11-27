//
// Created by yorkin on 10/27/21.
//

#ifndef EVOBASIC2_PARSER_H
#define EVOBASIC2_PARSER_H
#include "ast.h"
#include "lexer.h"
#include <set>
namespace evoBasic{
    using Follows = const std::set<Token::Enum>&;
    
    class Parser {
        Lexer *lexer;
    public:
        explicit Parser(Lexer *lexer);
        ast::Global *parseGlobal();
        ast::Class *parseClass(Follows follows);
        ast::Module *parseModule(Follows follows);
        ast::Type *parseType(Follows follows);
        ast::Enum *parseEnum(Follows follows);
        ast::Member *parseMember(Follows follows);
        AccessFlag parseAccessFlag();
        ast::Implement *parseImplement(Follows follows);

        ast::Import *parseImport(Follows follows);
        ast::Dim *parseDim(Follows follows);
        ast::Variable *parseVariable(Follows follows);
        ast::Function *parseFunction(Follows follows);
        ast::External *parseExternal(Follows follows);
        ast::EnumMember *parseEnumMember(Follows follows);

        ast::Parameter *parseParameter(Follows follows);
        ast::expr::ID *parseID(Follows follows);

        ast::stmt::Let *parseLet(Follows follows);
        ast::stmt::Select *parseSelect(Follows follows);
        ast::stmt::Loop *parseLoop(Follows follows);
        ast::stmt::If *parseIf(Follows follows);
        ast::stmt::For *parseFor(Follows follows);
        ast::stmt::Return *parseReturn(Follows follows);
        ast::stmt::Exit *parseExit(Follows follows);

        ast::stmt::ExprStmt *parseExprStmt(Follows follows);
        ast::expr::Expression *parseLogic(Follows follows);
        ast::expr::Expression *parseCmp(Follows follows);
        ast::expr::Expression *parseAdd(Follows follows);
        ast::expr::Expression *parseTerm(Follows follows);
        ast::expr::Expression *parseFactor(Follows follows);
        ast::expr::Expression *parseUnary(Follows follows);
        ast::expr::Expression *parseDot(Follows follows);
        ast::expr::Expression *parseTerminal(Follows follows);

        ast::Annotation *parseAnnotation(Follows follows);
        ast::AnnotationUnit *parseAnnotationUnit(Follows follows);


        //callee\id\index
        ast::expr::Expression *parseUnit(Follows follows);
        ast::expr::Callee::Argument *parseArg(Follows follows);

        ast::expr::Digit *parseDigit(Follows follows);
        ast::expr::Decimal *parseDecimal(Follows follows);
        ast::expr::String *parseString(Follows follows);
        ast::expr::Char *parseChar(Follows follows);
        ast::expr::Boolean *parseBoolean(Follows follows);
        ast::expr::Expression *parseParentheses(Follows follows);

        ast::Parameter *parseParameterList(Follows follows);
        ast::stmt::Statement *parseStmtList(Follows follows);
        ast::expr::Callee::Argument* parseArgsList(Follows follows);
        ast::expr::New *parseNew(Follows follows);
    };

    ast::Annotation *constructAnnotationAST (std::string code);
}


#endif //EVOBASIC2_PARSER_H

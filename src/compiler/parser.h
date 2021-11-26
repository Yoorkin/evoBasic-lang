//
// Created by yorkin on 10/27/21.
//

#ifndef EVOBASIC2_PARSER_H
#define EVOBASIC2_PARSER_H
#include "ast.h"
#include "lexer.h"
#include <set>
namespace evoBasic{

    class Parser {
        Lexer *lexer;
    public:
        explicit Parser(Lexer *lexer);
        ast::Global *parseGlobal();
        ast::Class *parseClass(const std::set<Token::Enum>& follows);
        ast::Module *parseModule(const std::set<Token::Enum>& follows);
        ast::Type *parseType(const std::set<Token::Enum>& follows);
        ast::Enum *parseEnum(const std::set<Token::Enum>& follows);
        ast::Member *parseMember(const std::set<Token::Enum>& follows);
        AccessFlag parseAccessFlag();
        ast::Implement *parseImplement(const std::set<Token::Enum>& follows);

        ast::Import *parseImport(const std::set<Token::Enum>& follows);
        ast::Dim *parseDim(const std::set<Token::Enum>& follows);
        ast::Variable *parseVariable(const std::set<Token::Enum>& follows);
        ast::Function *parseFunction(const std::set<Token::Enum>& follows);
        ast::External *parseExternal(const std::set<Token::Enum>& follows);
        ast::EnumMember *parseEnumMember(const std::set<Token::Enum>& follows);

        ast::Parameter *parseParameter(const std::set<Token::Enum>& follows);
        ast::expr::ID *parseID(const std::set<Token::Enum>& follows);

        ast::stmt::Let *parseLet(const std::set<Token::Enum>& follows);
        ast::stmt::Select *parseSelect(const std::set<Token::Enum>& follows);
        ast::stmt::Loop *parseLoop(const std::set<Token::Enum>& follows);
        ast::stmt::If *parseIf(const std::set<Token::Enum>& follows);
        ast::stmt::For *parseFor(const std::set<Token::Enum>& follows);
        ast::stmt::Return *parseReturn(const std::set<Token::Enum>& follows);
        ast::stmt::Exit *parseExit(const std::set<Token::Enum>& follows);

        ast::stmt::ExprStmt *parseExprStmt(const std::set<Token::Enum>& follows);
        ast::expr::Expression *parseLogic(const std::set<Token::Enum>& follows);
        ast::expr::Expression *parseCmp(const std::set<Token::Enum>& follows);
        ast::expr::Expression *parseAdd(const std::set<Token::Enum>& follows);
        ast::expr::Expression *parseTerm(const std::set<Token::Enum>& follows);
        ast::expr::Expression *parseFactor(const std::set<Token::Enum>& follows);
        ast::expr::Expression *parseUnary(const std::set<Token::Enum>& follows);
        ast::expr::Expression *parseDot(const std::set<Token::Enum>& follows);
        ast::expr::Expression *parseTerminal(const std::set<Token::Enum>& follows);

        ast::Annotation *parseAnnotation(const std::set<Token::Enum> &follows);
        ast::AnnotationUnit *parseAnnotationUnit(const std::set<Token::Enum> &follows);


        //callee\id\index
        ast::expr::Expression *parseUnit(const std::set<Token::Enum>& follows);
        ast::expr::Callee::Argument *parseArg(const std::set<Token::Enum>& follows);

        ast::expr::Digit *parseDigit(const std::set<Token::Enum>& follows);
        ast::expr::Decimal *parseDecimal(const std::set<Token::Enum>& follows);
        ast::expr::String *parseString(const std::set<Token::Enum>& follows);
        ast::expr::Char *parseChar(const std::set<Token::Enum>& follows);
        ast::expr::Boolean *parseBoolean(const std::set<Token::Enum>& follows);
        ast::expr::Expression *parseParentheses(const std::set<Token::Enum>& follows);

        ast::Parameter *parseParameterList(const std::set<Token::Enum>& follows);
        ast::stmt::Statement *parseStmtList(const std::set<Token::Enum>& follows);
        ast::expr::Callee::Argument* parseArgsList(const std::set<Token::Enum>& follows);

    };

    ast::Annotation *constructAnnotationAST (std::string code);
}


#endif //EVOBASIC2_PARSER_H

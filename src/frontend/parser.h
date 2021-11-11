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
        ast::Class *parseClass(std::set<Token::Enum> follows);
        ast::Module *parseModule(std::set<Token::Enum> follows);
        ast::Type *parseType(std::set<Token::Enum> follows);
        ast::Enum *parseEnum(std::set<Token::Enum> follows);
        ast::Member *parseMember(std::set<Token::Enum> follows);
        AccessFlag parseAccessFlag();

        ast::Import *parseImport(std::set<Token::Enum> follows);
        ast::Dim *parseDim(std::set<Token::Enum> follows);
        ast::Variable *parseVariable(std::set<Token::Enum> follows);
        ast::Function *parseFunction(std::set<Token::Enum> follows);
        ast::External *parseExternal(std::set<Token::Enum> follows);
        ast::Init *parseInit(std::set<Token::Enum> follows);
        ast::Operator *parseOperator(std::set<Token::Enum> follows);
        ast::EnumMember parseEnumMember(std::set<Token::Enum> follows);
        ast::TypeMember parseTypeMember(std::set<Token::Enum> follows);

        std::list<ast::Parameter*> parseParameterList(std::set<Token::Enum> follows);
        std::list<ast::stmt::Statement*> parseStmtList(std::set<Token::Enum> follows);
        ast::Parameter *parseParameter(std::set<Token::Enum> follows);
        ast::ID *parseID(std::set<Token::Enum> follows);
        ast::Annotation *parseAnnotation(std::set<Token::Enum> follows);
        ast::AnnotationUnit *parseAnnotationUnit(std::set<Token::Enum> follows);
        ast::Generic *parseGeneric(std::set<Token::Enum> follows);
        
        ast::stmt::Let *parseLet(std::set<Token::Enum> follows);
        ast::stmt::Select *parseSelect(std::set<Token::Enum> follows);
        ast::stmt::Loop *parseLoop(std::set<Token::Enum> follows);
        ast::stmt::If *parseIf(std::set<Token::Enum> follows);
        ast::stmt::For *parseFor(std::set<Token::Enum> follows);
       // ast::stmt::Control *parseControl(std::set<Token::Enum> follows);
        ast::stmt::Return *parseReturn(std::set<Token::Enum> follows);
        ast::stmt::Exit *parseExit(std::set<Token::Enum> follows);

        ast::stmt::ExprStmt *parseExprStmt(std::set<Token::Enum> follows);
        ast::expr::Expression *parseLogic(std::set<Token::Enum> follows);
        ast::expr::Expression *parseCmp(std::set<Token::Enum> follows);
        ast::expr::Expression *parseAdd(std::set<Token::Enum> follows);
        ast::expr::Expression *parseTerm(std::set<Token::Enum> follows);
        ast::expr::Expression *parseFactor(std::set<Token::Enum> follows);
        ast::expr::Expression *parseUnit(std::set<Token::Enum> follows);
        ast::expr::Expression *parseTerminal(std::set<Token::Enum> follows);
        ast::expr::Callee *parseCallee(std::set<Token::Enum> follows);
        ast::expr::GenericArgs *parseGenericArgs(std::set<Token::Enum> follows);
        ast::expr::ArgsList *parseArgsList(std::set<Token::Enum> follows);
        ast::expr::Arg *parseArg(std::set<Token::Enum> follows);

        ast::expr::literal::Digit *parseDigit(std::set<Token::Enum> follows);
        ast::expr::literal::Decimal *parseDemical(std::set<Token::Enum> follows);
        ast::expr::literal::String *parseString(std::set<Token::Enum> follows);
        ast::expr::literal::Char *parseChar(std::set<Token::Enum> follows);
        ast::expr::literal::Boolean *parseBoolean(std::set<Token::Enum> follows);
    };

    ast::Annotation *constructAnnotationAST (std::string code);
}


#endif //EVOBASIC2_PARSER_H

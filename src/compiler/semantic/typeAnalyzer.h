//
// Created by yorkin on 11/24/21.
//

#ifndef EVOBASIC_TYPEANALYZER_H
#define EVOBASIC_TYPEANALYZER_H
#include "visitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"
#include "baseVisitor.h"

namespace evoBasic{
    class TypeAnalyzer : public BaseVisitor{
    public:
        std::any visitGlobal(ast::Global *global_node, BaseArgs args) override;
        std::any visitModule(ast::Module *mod_node, BaseArgs args) override;
        std::any visitClass(ast::Class *cls_node, BaseArgs args) override;
        std::any visitFunction(ast::Function *func_node, BaseArgs args) override;

        std::any visitLet(ast::stmt::Let *let_node,BaseArgs args)override;
        std::any visitSelect(ast::stmt::Select *select_node,BaseArgs args)override;
        std::any visitLoop(ast::stmt::Loop *loop_node,BaseArgs args)override;
        std::any visitIf(ast::stmt::If *ifstmt_node,BaseArgs args)override;
        std::any visitFor(ast::stmt::For *forstmt_node,BaseArgs args)override;
        std::any visitReturn(ast::stmt::Return *ret_node,BaseArgs args)override;
        std::any visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node,BaseArgs args)override;
        void visitStatementList(std::list<ast::stmt::Statement*> &stmt_list,BaseArgs args);


        std::any visitBinary(ast::expr::Binary *logic_node, BaseArgs args) override;
        std::any visitUnary(ast::expr::Unary *unit_node, BaseArgs args) override;

        std::any visitCallee(ast::expr::Callee *callee_node, BaseArgs args) override;
        std::any visitArg(ast::expr::Callee::Argument *arg_node, BaseArgs args) override;

        std::any visitID(ast::expr::ID *id_node, BaseArgs args) override;
        std::any visitDigit(ast::expr::Digit *digit_node, BaseArgs args) override;
        std::any visitDecimal(ast::expr::Decimal *decimal, BaseArgs args) override;
        std::any visitBoolean(ast::expr::Boolean *bl_node, BaseArgs args) override;
        std::any visitChar(ast::expr::Char *ch_node, BaseArgs args) override;
        std::any visitString(ast::expr::String *str_node, BaseArgs args) override;
        std::any visitParentheses(ast::expr::Parentheses *parentheses_node,BaseArgs args)override;

    };
}

#endif //EVOBASIC_TYPEANALYZER_H

//
// Created by yorkin on 11/24/21.
//

#ifndef EVOBASIC_TYPEANALYZER_H
#define EVOBASIC_TYPEANALYZER_H
#include "defaultVisitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"

namespace evoBasic{

    class TypeAnalyzer : public DefaultModifyVisitor{
    public:
        std::any visitGlobal(ast::Global **global_node, DefaultArgs args) override;
        std::any visitModule(ast::Module **mod_node, DefaultArgs args) override;
        std::any visitClass(ast::Class **cls_node, DefaultArgs args) override;
        std::any visitFunction(ast::Function **func_node, DefaultArgs args) override;

        std::any visitLet(ast::stmt::Let **let_node,DefaultArgs args)override;
        std::any visitVariable(ast::Variable **var_node, DefaultArgs args) override;
        std::any visitSelect(ast::stmt::Select **select_node,DefaultArgs args)override;
        std::any visitLoop(ast::stmt::Loop **loop_node,DefaultArgs args)override;
        std::any visitIf(ast::stmt::If **ifstmt_node,DefaultArgs args)override;
        std::any visitFor(ast::stmt::For **forstmt_node,DefaultArgs args)override;
        std::any visitReturn(ast::stmt::Return **ret_node,DefaultArgs args)override;
        std::any visitExprStmt(ast::stmt::ExprStmt **expr_stmt_node,DefaultArgs args)override;
        void visitStatementList(ast::stmt::Statement **stmt_list,DefaultArgs args);


        std::any visitBinary(ast::expr::Binary **logic_node, DefaultArgs args) override;
        std::any visitUnary(ast::expr::Unary **unit_node, DefaultArgs args) override;

        std::any visitCallee(ast::expr::Callee **callee_node, DefaultArgs args) override;
        std::any visitArg(ast::expr::Callee::Argument **argument_node, DefaultArgs args) override;

        std::any visitID(ast::expr::ID **id_node, DefaultArgs args) override;
        std::any visitDigit(ast::expr::Digit **digit_node, DefaultArgs args) override;
        std::any visitDecimal(ast::expr::Decimal **decimal, DefaultArgs args) override;
        std::any visitBoolean(ast::expr::Boolean **bl_node, DefaultArgs args) override;
        std::any visitChar(ast::expr::Char **ch_node, DefaultArgs args) override;
        std::any visitString(ast::expr::String **str_node, DefaultArgs args) override;
        std::any visitParentheses(ast::expr::Parentheses **parentheses_node,DefaultArgs args)override;

        std::any visitMember(ast::Member **member_node, DefaultArgs args) override;
        std::any visitStatement(ast::stmt::Statement **stmt_node, DefaultArgs args) override;
        std::any visitExpression(ast::expr::Expression **expr_node, DefaultArgs args) override;
        std::any visitNew(ast::expr::New **new_node, DefaultArgs args)override;
        void check_callee(Location *location,ast::expr::Argument *argument,type::Function *target, DefaultArgs args);
    };
}

#endif //EVOBASIC_TYPEANALYZER_H

//
// Created by yorkin on 11/24/21.
//

#ifndef EVOBASIC_TYPEANALYZER_H
#define EVOBASIC_TYPEANALYZER_H
#include "visitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"

namespace evoBasic{
    struct TypeAnalyzerArgs{
        Context *context = nullptr;
        type::UserFunction *user_function = nullptr;
        type::Domain *domain = nullptr;
        type::Symbol *dot_expression_context = nullptr;
        type::Class *parent_class = nullptr;
        int checking_args_index = 0;
        bool need_lookup = false;
    };
    
    class TypeAnalyzer : public ModifyVisitor<TypeAnalyzerArgs>{
    public:
        std::any visitGlobal(ast::Global **global_node, TypeAnalyzerArgs args) override;
        std::any visitModule(ast::Module **mod_node, TypeAnalyzerArgs args) override;
        std::any visitClass(ast::Class **cls_node, TypeAnalyzerArgs args) override;
        std::any visitFunction(ast::Function **func_node, TypeAnalyzerArgs args) override;

        std::any visitLet(ast::stmt::Let **let_node,TypeAnalyzerArgs args)override;
        std::any visitVariable(ast::Variable **var_node, TypeAnalyzerArgs args) override;
        std::any visitSelect(ast::stmt::Select **select_node,TypeAnalyzerArgs args)override;
        std::any visitLoop(ast::stmt::Loop **loop_node,TypeAnalyzerArgs args)override;
        std::any visitIf(ast::stmt::If **ifstmt_node,TypeAnalyzerArgs args)override;
        std::any visitFor(ast::stmt::For **forstmt_node,TypeAnalyzerArgs args)override;
        std::any visitReturn(ast::stmt::Return **ret_node,TypeAnalyzerArgs args)override;
        std::any visitExprStmt(ast::stmt::ExprStmt **expr_stmt_node,TypeAnalyzerArgs args)override;
        void visitStatementList(ast::stmt::Statement **stmt_list,TypeAnalyzerArgs args);


        std::any visitBinary(ast::expr::Binary **logic_node, TypeAnalyzerArgs args) override;
        std::any visitUnary(ast::expr::Unary **unit_node, TypeAnalyzerArgs args) override;

        std::any visitCallee(ast::expr::Callee **callee_node, TypeAnalyzerArgs args) override;
        std::any visitArg(ast::expr::Callee::Argument **argument_node, TypeAnalyzerArgs args) override;

        std::any visitID(ast::expr::ID **id_node, TypeAnalyzerArgs args) override;
        std::any visitDigit(ast::expr::Digit **digit_node, TypeAnalyzerArgs args) override;
        std::any visitDecimal(ast::expr::Decimal **decimal, TypeAnalyzerArgs args) override;
        std::any visitBoolean(ast::expr::Boolean **bl_node, TypeAnalyzerArgs args) override;
        std::any visitChar(ast::expr::Char **ch_node, TypeAnalyzerArgs args) override;
        std::any visitString(ast::expr::String **str_node, TypeAnalyzerArgs args) override;
        std::any visitParentheses(ast::expr::Parentheses **parentheses_node,TypeAnalyzerArgs args)override;

    };
}

#endif //EVOBASIC_TYPEANALYZER_H

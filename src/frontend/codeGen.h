//
// Created by yorkin on 11/2/21.
//

#ifndef EVOBASIC2_CODEGEN_H
#define EVOBASIC2_CODEGEN_H
#include "visitor.h"
#include "ir.h"
#include "context.h"
#include "semantic.h"
#include <variant>

namespace evoBasic{
    struct IRGenArgs{
        std::shared_ptr<type::Function> function;
        std::shared_ptr<type::Domain> domain;
        std::shared_ptr<type::Domain> *in_terminal_list = nullptr;
        bool need_lookup = false;
        bool reserve_address = false;
        bool is_last_terminal = false;
        bool need_dup_in_assignment = false;
        std::shared_ptr<Context> context;
        ir::Segment *previous_segment = nullptr;
        ir::Segment *next_segment = nullptr;
        ir::Segment *continue_segment = nullptr;
        std::shared_ptr<ir::IR> ir;
        int current_args_index = 0;
    };

    using DataSizeVariant = std::variant<vm::Data,data::u32>; // primitive\ptr or Array\Type

    class IRGen : public Visitor<IRGenArgs>{
    public:
        std::shared_ptr<ir::IR> gen(AST *ast,std::shared_ptr<Context> context);

        vm::Data convertSymbolToDataKind(std::shared_ptr<type::Symbol> symbol);

        std::any visitGlobal(ast::Global *global_node, IRGenArgs args) override;
        std::any visitModule(ast::Module *mod_node, IRGenArgs args) override;
        std::any visitClass(ast::Class *cls_node, IRGenArgs args) override;
        std::any visitEnum(ast::Enum *em_node, IRGenArgs args) override;
        std::any visitType(ast::Type *ty_node, IRGenArgs args) override;
        std::any visitDim(ast::Dim *dim_node, IRGenArgs args) override;
        std::any visitVariable(ast::Variable *var_node, IRGenArgs args) override;

        std::any visitFunction(ast::Function *func_node, IRGenArgs args) override;

        ir::Segment *visitStatementList(std::list<ast::stmt::Statement*> &stmt_list, IRGenArgs args);
        std::any visitLet(ast::stmt::Let *let_node, IRGenArgs args) override;
        std::any visitIf(ast::stmt::If *ifstmt_node, IRGenArgs args) override;
        std::any visitCase(ast::Case *ca_node, IRGenArgs args) override;
        std::any visitLoop(ast::stmt::Loop *loop_node, IRGenArgs args) override;
        std::any visitSelect(ast::stmt::Select *select_node, IRGenArgs args) override;
        std::any visitFor(ast::stmt::For *forstmt_node, IRGenArgs args) override;
        std::any visitContinue(ast::stmt::Continue *cont_node, IRGenArgs args) override;
        std::any visitReturn(ast::stmt::Return *ret_node, IRGenArgs args) override;
        std::any visitExit(ast::stmt::Exit *exit_node, IRGenArgs args) override;

        std::any visitBinary(ast::expr::Binary *logic_node, IRGenArgs args) override;
        std::any visitUnary(ast::expr::Unary *unit_node, IRGenArgs args) override;
        std::any visitCallee(ast::expr::Callee *callee_node, IRGenArgs args) override;
        std::any visitArg(ast::expr::Callee::Argument *arg_node, IRGenArgs args) override;

        std::any visitBoolean(ast::expr::Boolean *bl_node, IRGenArgs args) override;
        std::any visitChar(ast::expr::Char *ch_node, IRGenArgs args) override;
        std::any visitDigit(ast::expr::Digit *digit_node, IRGenArgs args) override;
        std::any visitDecimal(ast::expr::Decimal *decimal, IRGenArgs args) override;
        std::any visitString(ast::expr::String *str_node, IRGenArgs args) override;
        std::any visitParentheses(ast::expr::Parentheses *parentheses_node, IRGenArgs args) override;
        std::any visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, IRGenArgs args) override;

        DataSizeVariant visitFunctionCall(ast::expr::Callee *callee_node,IRGenArgs args,std::shared_ptr<type::Function> target);
        DataSizeVariant visitVariableCall(ast::expr::Callee *callee_node,IRGenArgs args,std::shared_ptr<type::Variable> target);
    };

}


#endif //EVOBASIC2_CODEGEN_H

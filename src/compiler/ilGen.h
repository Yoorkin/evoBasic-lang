//
// Created by yorkin on 12/13/21.
//

#ifndef EVOBASIC_ILGEN_H
#define EVOBASIC_ILGEN_H
#include <any>
#include <semantic.h>
#include "context.h"
#include "parseTree.h"
#include "il.h"
#include "visitor.h"
#include "defaultVisitor.h"

namespace evoBasic{
    
    struct ILGenArgs{
        type::Symbol *dot_expression_context = nullptr;
        bool need_lookup = false;
        Context *context = nullptr;
        il::FtnWithDefinition *ftn = nullptr;
        il::Block *previous_block = nullptr,
                  *next_block = nullptr;
    };
    
    class ILGen : public DefaultVisitor<ILGenArgs>{
        il::ILFactory factory;
    public:
        il::IL *gen(AST *ast,Context *context);

        std::any visitGlobal(ast::Global *global_node, ILGenArgs args) override;
        std::any visitModule(ast::Module *mod_node, ILGenArgs args) override;
        std::any visitClass(ast::Class *cls_node, ILGenArgs args) override;
        std::any visitEnum(ast::Enum *em_node, ILGenArgs args) override;
        std::any visitType(ast::Type *ty_node, ILGenArgs args) override;
        std::any visitDim(ast::Dim *dim_node, ILGenArgs args) override;
        std::any visitVariable(ast::Variable *var_node, ILGenArgs args) override;
        std::any visitFunction(ast::Function *func_node, ILGenArgs args) override;

        std::any visitLet(ast::stmt::Let *let_node, ILGenArgs args) override;
        std::any visitIf(ast::stmt::If *ifstmt_node, ILGenArgs args) override;
        std::any visitCase(ast::Case *ca_node, ILGenArgs args) override;
        std::any visitLoop(ast::stmt::Loop *loop_node, ILGenArgs args) override;
        std::any visitSelect(ast::stmt::Select *select_node, ILGenArgs args) override;
        std::any visitFor(ast::stmt::For *forstmt_node, ILGenArgs args) override;
        std::any visitContinue(ast::stmt::Continue *cont_node, ILGenArgs args) override;
        std::any visitReturn(ast::stmt::Return *ret_node, ILGenArgs args) override;
        std::any visitExit(ast::stmt::Exit *exit_node, ILGenArgs args) override;

        std::any visitBinary(ast::expr::Binary *logic_node, ILGenArgs args) override;
        std::any visitUnary(ast::expr::Unary *unit_node, ILGenArgs args) override;
        std::any visitCallee(ast::expr::Callee *callee_node, ILGenArgs args) override;
        std::any visitArg(ast::expr::Callee::Argument *arg_node, ILGenArgs args) override;

        std::any visitIndex(ast::expr::Index *index_node, ILGenArgs args) override;
        std::any visitCast(ast::expr::Cast *cast_node, ILGenArgs args) override;
        std::any visitAssign(ast::expr::Assign *assign_node, ILGenArgs args) override;
        std::any visitDot(ast::expr::Dot *dot_node, ILGenArgs args) override;

        std::any visitID(ast::expr::ID *id_node, ILGenArgs args) override;
        std::any visitBoolean(ast::expr::Boolean *bl_node, ILGenArgs args) override;
        std::any visitChar(ast::expr::Char *ch_node, ILGenArgs args) override;
        std::any visitDigit(ast::expr::Digit *digit_node, ILGenArgs args) override;
        std::any visitDecimal(ast::expr::Decimal *decimal, ILGenArgs args) override;
        std::any visitString(ast::expr::String *str_node, ILGenArgs args) override;
        std::any visitParentheses(ast::expr::Parentheses *parentheses_node, ILGenArgs args) override;
        std::any visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, ILGenArgs args) override;

        std::any visitExpression(ast::expr::Expression *expr_node, ILGenArgs args) override;

        std::any visitMember(ast::Member *member_node, ILGenArgs args) override;
        std::any visitStatement(ast::stmt::Statement *stmt_node, ILGenArgs args) override;

    };
}


#endif //EVOBASIC_ILGEN_H

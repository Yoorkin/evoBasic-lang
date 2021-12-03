//
// Created by yorkin on 11/1/21.
//

#ifndef EVOBASIC2_VISITOR_H
#define EVOBASIC2_VISITOR_H
#include <any>
#include "ast.h"
namespace evoBasic{
    template<typename ARGS>
    class Visitor {
    public:
        virtual std::any visitGlobal(ast::Global *global_node,ARGS args){PANIC;}
        virtual std::any visitClass(ast::Class *cls_node,ARGS args){PANIC;}
        virtual std::any visitModule(ast::Module *mod_node,ARGS args){PANIC;}

        virtual std::any visitImport(ast::Import *imp_node,ARGS args){PANIC;}
        virtual std::any visitDim(ast::Dim *dim_node,ARGS args){PANIC;}
        virtual std::any visitVariable(ast::Variable *var_node,ARGS args){PANIC;}
        virtual std::any visitFunction(ast::Function *func_node,ARGS args){PANIC;}
        virtual std::any visitExternal(ast::External *ext_node,ARGS args){PANIC;}
        virtual std::any visitInit(ast::Init *init_node,ARGS args){PANIC;}
        virtual std::any visitOperator(ast::Operator *op_node,ARGS args){PANIC;}
        virtual std::any visitEnum(ast::Enum *em_node,ARGS args){PANIC;}
        virtual std::any visitType(ast::Type *ty_node,ARGS args){PANIC;}
        virtual std::any visitParameter(ast::Parameter *param_node,ARGS args){PANIC;}

        virtual std::any visitLet(ast::stmt::Let *let_node,ARGS args){PANIC;}
        virtual std::any visitSelect(ast::stmt::Select *select_node,ARGS args){PANIC;}
        virtual std::any visitLoop(ast::stmt::Loop *loop_node,ARGS args){PANIC;}
        virtual std::any visitIf(ast::stmt::If *ifstmt_node,ARGS args){PANIC;}
        virtual std::any visitFor(ast::stmt::For *forstmt_node,ARGS args){PANIC;}
        virtual std::any visitReturn(ast::stmt::Return *ret_node,ARGS args){PANIC;}
        virtual std::any visitExit(ast::stmt::Exit *exit_node,ARGS args){PANIC;}
        virtual std::any visitContinue(ast::stmt::Continue *cont_node,ARGS args){PANIC;}
        virtual std::any visitCase(ast::Case *ca_node,ARGS args){PANIC;}
        virtual std::any visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node,ARGS args){PANIC;}

        virtual std::any visitBinary(ast::expr::Binary *logic_node, ARGS args){PANIC;}
        virtual std::any visitUnary(ast::expr::Unary *unit_node, ARGS args){PANIC;}
        virtual std::any visitIndex(ast::expr::Index *index_node, ARGS args){PANIC;}
        virtual std::any visitDot(ast::expr::Dot *dot_node, ARGS args){PANIC;}
        virtual std::any visitAssign(ast::expr::Assign *assign_node, ARGS args){PANIC;}
        virtual std::any visitCast(ast::expr::Cast *cast_node,ARGS args){PANIC;}

        virtual std::any visitCallee(ast::expr::Callee *callee_node, ARGS args){PANIC;}
        virtual std::any visitArg(ast::expr::Callee::Argument *arg_node, ARGS args){PANIC;}

        virtual std::any visitID(ast::expr::ID *id_node,ARGS args){PANIC;}
        virtual std::any visitDigit(ast::expr::Digit *digit_node,ARGS args){PANIC;}
        virtual std::any visitDecimal(ast::expr::Decimal *decimal, ARGS args){PANIC;}
        virtual std::any visitString(ast::expr::String *str_node,ARGS args){PANIC;}
        virtual std::any visitChar(ast::expr::Char *ch_node,ARGS args){PANIC;}
        virtual std::any visitBoolean(ast::expr::Boolean *bl_node,ARGS args){PANIC;}
        virtual std::any visitParentheses(ast::expr::Parentheses *parentheses_node,ARGS args){PANIC;}

        virtual std::any visitAnnotation(ast::Annotation *anno_node, ARGS args) {PANIC;}
        virtual std::any visitAnnotationUnit(ast::AnnotationUnit *unit_node, ARGS args) {PANIC;}

        virtual std::any visitMember(ast::Member *member_node,ARGS args){PANIC;}
        virtual std::any visitStatement(ast::stmt::Statement *stmt_node,ARGS args){PANIC;}
        virtual std::any visitExpression(ast::expr::Expression *expr_node,ARGS args){PANIC;}
        virtual std::any visitNew(ast::expr::New *new_node,ARGS args){PANIC;}
    };


    template<typename ARGS>
    class ModifyVisitor {
    public:
        virtual std::any visitGlobal(ast::Global **global_node,ARGS args){PANIC;}
        virtual std::any visitClass(ast::Class **cls_node,ARGS args){PANIC;}
        virtual std::any visitModule(ast::Module **mod_node,ARGS args){PANIC;}

        virtual std::any visitImport(ast::Import **imp_node,ARGS args){PANIC;}
        virtual std::any visitDim(ast::Dim **dim_node,ARGS args){PANIC;}
        virtual std::any visitVariable(ast::Variable **var_node,ARGS args){PANIC;}
        virtual std::any visitFunction(ast::Function **func_node,ARGS args){PANIC;}
        virtual std::any visitExternal(ast::External **ext_node,ARGS args){PANIC;}
        virtual std::any visitInit(ast::Init **init_node,ARGS args){PANIC;}
        virtual std::any visitOperator(ast::Operator **op_node,ARGS args){PANIC;}
        virtual std::any visitEnum(ast::Enum **em_node,ARGS args){PANIC;}
        virtual std::any visitType(ast::Type **ty_node,ARGS args){PANIC;}
        virtual std::any visitParameter(ast::Parameter **param_node,ARGS args){PANIC;}

        virtual std::any visitLet(ast::stmt::Let **let_node,ARGS args){PANIC;}
        virtual std::any visitSelect(ast::stmt::Select **select_node,ARGS args){PANIC;}
        virtual std::any visitLoop(ast::stmt::Loop **loop_node,ARGS args){PANIC;}
        virtual std::any visitIf(ast::stmt::If **ifstmt_node,ARGS args){PANIC;}
        virtual std::any visitFor(ast::stmt::For **forstmt_node,ARGS args){PANIC;}
        virtual std::any visitReturn(ast::stmt::Return **ret_node,ARGS args){PANIC;}
        virtual std::any visitExit(ast::stmt::Exit **exit_node,ARGS args){PANIC;}
        virtual std::any visitContinue(ast::stmt::Continue **cont_node,ARGS args){PANIC;}
        virtual std::any visitCase(ast::Case **ca_node,ARGS args){PANIC;}
        virtual std::any visitExprStmt(ast::stmt::ExprStmt **expr_stmt_node,ARGS args){PANIC;}

        virtual std::any visitBinary(ast::expr::Binary **logic_node, ARGS args){PANIC;}
        virtual std::any visitUnary(ast::expr::Unary **unit_node, ARGS args){PANIC;}
        virtual std::any visitIndex(ast::expr::Index **index_node, ARGS args){PANIC;}
        virtual std::any visitDot(ast::expr::Dot **dot_node, ARGS args){PANIC;}
        virtual std::any visitAssign(ast::expr::Assign **assign_node, ARGS args){PANIC;}
        virtual std::any visitCast(ast::expr::Cast **cast_node,ARGS args){PANIC;}

        virtual std::any visitCallee(ast::expr::Callee **callee_node, ARGS args){PANIC;}
        virtual std::any visitArg(ast::expr::Callee::Argument **arg_node, ARGS args){PANIC;}

        virtual std::any visitID(ast::expr::ID **id_node,ARGS args){PANIC;}
        virtual std::any visitDigit(ast::expr::Digit **digit_node,ARGS args){PANIC;}
        virtual std::any visitDecimal(ast::expr::Decimal **decimal, ARGS args){PANIC;}
        virtual std::any visitString(ast::expr::String **str_node,ARGS args){PANIC;}
        virtual std::any visitChar(ast::expr::Char **ch_node,ARGS args){PANIC;}
        virtual std::any visitBoolean(ast::expr::Boolean **bl_node,ARGS args){PANIC;}
        virtual std::any visitParentheses(ast::expr::Parentheses **parentheses_node,ARGS args){PANIC;}

        virtual std::any visitAnnotation(ast::Annotation **anno_node, ARGS args) {PANIC;}
        virtual std::any visitAnnotationUnit(ast::AnnotationUnit **unit_node, ARGS args) {PANIC;}

        virtual std::any visitMember(ast::Member **member_node,ARGS args){PANIC;}
        virtual std::any visitStatement(ast::stmt::Statement **stmt_node,ARGS args){PANIC;}
        virtual std::any visitExpression(ast::expr::Expression **expr_node,ARGS args){PANIC;}
        virtual std::any visitNew(ast::expr::New **new_node,ARGS args){PANIC;}
    };
}


#endif //EVOBASIC2_VISITOR_H

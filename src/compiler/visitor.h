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
        virtual std::any visitGlobal(ast::Global *global_node,ARGS args){return nullptr;}
        virtual std::any visitClass(ast::Class *cls_node,ARGS args){return nullptr;}
        virtual std::any visitModule(ast::Module *mod_node,ARGS args){return nullptr;}

        virtual std::any visitImport(ast::Import *imp_node,ARGS args){return nullptr;}
        virtual std::any visitDim(ast::Dim *dim_node,ARGS args){return nullptr;}
        virtual std::any visitVariable(ast::Variable *var_node,ARGS args){return nullptr;}
        virtual std::any visitFunction(ast::Function *func_node,ARGS args){return nullptr;}
        virtual std::any visitExternal(ast::External *ext_node,ARGS args){return nullptr;}
        virtual std::any visitInit(ast::Init *init_node,ARGS args){return nullptr;}
        virtual std::any visitOperator(ast::Operator *op_node,ARGS args){return nullptr;}
        virtual std::any visitEnum(ast::Enum *em_node,ARGS args){return nullptr;}
        virtual std::any visitType(ast::Type *ty_node,ARGS args){return nullptr;}
        virtual std::any visitParameter(ast::Parameter *param_node,ARGS args){return nullptr;}

        virtual std::any visitLet(ast::stmt::Let *let_node,ARGS args){return nullptr;}
        virtual std::any visitSelect(ast::stmt::Select *select_node,ARGS args){return nullptr;}
        virtual std::any visitLoop(ast::stmt::Loop *loop_node,ARGS args){return nullptr;}
        virtual std::any visitIf(ast::stmt::If *ifstmt_node,ARGS args){return nullptr;}
        virtual std::any visitFor(ast::stmt::For *forstmt_node,ARGS args){return nullptr;}
        virtual std::any visitReturn(ast::stmt::Return *ret_node,ARGS args){return nullptr;}
        virtual std::any visitExit(ast::stmt::Exit *exit_node,ARGS args){return nullptr;}
        virtual std::any visitContinue(ast::stmt::Continue *cont_node,ARGS args){return nullptr;}
        virtual std::any visitCase(ast::Case *ca_node,ARGS args){return nullptr;}
        virtual std::any visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node,ARGS args){
            visitExpression(expr_stmt_node->expr,args);
            return nullptr;
        }

        virtual std::any visitBinary(ast::expr::Binary *logic_node, ARGS args){return nullptr;}
        virtual std::any visitUnary(ast::expr::Unary *unit_node, ARGS args){return nullptr;}

        virtual std::any visitCallee(ast::expr::Callee *callee_node, ARGS args){return nullptr;}
        virtual std::any visitArg(ast::expr::Callee::Argument *arg_node, ARGS args){return nullptr;}

        virtual std::any visitID(ast::expr::ID *id_node,ARGS args){return nullptr;}
        virtual std::any visitDigit(ast::expr::Digit *digit_node,ARGS args){return nullptr;}
        virtual std::any visitDecimal(ast::expr::Decimal *decimal, ARGS args){return nullptr;}
        virtual std::any visitString(ast::expr::String *str_node,ARGS args){return nullptr;}
        virtual std::any visitChar(ast::expr::Char *ch_node,ARGS args){return nullptr;}
        virtual std::any visitBoolean(ast::expr::Boolean *bl_node,ARGS args){return nullptr;}
        virtual std::any visitParentheses(ast::expr::Parentheses *parentheses_node,ARGS args){return nullptr;}

        virtual std::any visitAnnotation(ast::Annotation *anno_node, ARGS args) {return nullptr;}
        virtual std::any visitAnnotationUnit(ast::AnnotationUnit *unit_node, ARGS args) {return nullptr;}

        virtual std::any visitMember(ast::Member *member_node,ARGS args){
            switch (member_node->member_kind) {
                case ast::Member::function_: return visitFunction((ast::Function*)member_node,args);
                case ast::Member::class_:    return visitClass((ast::Class*)member_node,args);
                case ast::Member::module_:   return visitModule((ast::Module*)member_node,args);
                case ast::Member::type_:     return visitType((ast::Type*)member_node,args);
                case ast::Member::enum_:     return visitEnum((ast::Enum*)member_node,args);
                case ast::Member::dim_:      return visitDim((ast::Dim*)member_node,args);
                case ast::Member::external_: return visitExternal((ast::External*)member_node,args);
            }
            return nullptr;
        }
        virtual std::any visitStatement(ast::stmt::Statement *stmt_node,ARGS args){
            switch (stmt_node->stmt_flag) {
                case ast::stmt::Statement::let_: return visitLet((ast::stmt::Let*)stmt_node,args);
                case ast::stmt::Statement::loop_:return visitLoop((ast::stmt::Loop*)stmt_node,args);
                case ast::stmt::Statement::if_:  return visitIf((ast::stmt::If*)stmt_node,args);
                case ast::stmt::Statement::for_: return visitFor((ast::stmt::For*)stmt_node,args);
                case ast::stmt::Statement::select_:return visitSelect((ast::stmt::Select*)stmt_node,args);
                case ast::stmt::Statement::return_:return visitReturn((ast::stmt::Return*)stmt_node,args);
                case ast::stmt::Statement::continue_:return visitContinue((ast::stmt::Continue*)stmt_node,args);
                case ast::stmt::Statement::exit_:return visitExit((ast::stmt::Exit*)stmt_node,args);
                case ast::stmt::Statement::expr_:return visitExprStmt((ast::stmt::ExprStmt*)stmt_node,args);
            }
            return nullptr;
        }
        virtual std::any visitExpression(ast::expr::Expression *expr_node,ARGS args){
            switch (expr_node->expression_kind) {
                case ast::expr::Expression::binary_:
                    return visitBinary((ast::expr::Binary*)expr_node,args);
                case ast::expr::Expression::unary_:
                    return visitUnary((ast::expr::Unary*)expr_node, args);
                case ast::expr::Expression::digit_:
                    return visitDigit((ast::expr::Digit*)expr_node,args);
                case ast::expr::Expression::decimal_:
                    return visitDecimal((ast::expr::Decimal *)expr_node,args);
                case ast::expr::Expression::string_:
                    return visitString((ast::expr::String*)expr_node,args);
                case ast::expr::Expression::char_:
                    return visitChar((ast::expr::Char*)expr_node,args);
                case ast::expr::Expression::parentheses_:
                    return visitParentheses((ast::expr::Parentheses*)expr_node,args);
                case ast::expr::Expression::callee_:
                    return visitCallee((ast::expr::Callee*)expr_node,args);
                case ast::expr::Expression::boolean_:
                    return visitBoolean((ast::expr::Boolean*)expr_node,args);
                case ast::expr::Expression::ID_:
                    return visitID((ast::expr::ID*)expr_node,args);
                case ast::expr::Expression::error_:
                    return {};
            }
            return {};
        }
    };


    template<typename ARGS>
    class ModifyVisitor {
    public:
        virtual std::any visitGlobal(ast::Global **global_node,ARGS args){return nullptr;}
        virtual std::any visitClass(ast::Class **cls_node,ARGS args){return nullptr;}
        virtual std::any visitModule(ast::Module **mod_node,ARGS args){return nullptr;}

        virtual std::any visitImport(ast::Import **imp_node,ARGS args){return nullptr;}
        virtual std::any visitDim(ast::Dim **dim_node,ARGS args){return nullptr;}
        virtual std::any visitVariable(ast::Variable **var_node,ARGS args){return nullptr;}
        virtual std::any visitFunction(ast::Function **func_node,ARGS args){return nullptr;}
        virtual std::any visitExternal(ast::External **ext_node,ARGS args){return nullptr;}
        virtual std::any visitInit(ast::Init **init_node,ARGS args){return nullptr;}
        virtual std::any visitOperator(ast::Operator **op_node,ARGS args){return nullptr;}
        virtual std::any visitEnum(ast::Enum **em_node,ARGS args){return nullptr;}
        virtual std::any visitType(ast::Type **ty_node,ARGS args){return nullptr;}
        virtual std::any visitParameter(ast::Parameter **param_node,ARGS args){return nullptr;}

        virtual std::any visitLet(ast::stmt::Let **let_node,ARGS args){return nullptr;}
        virtual std::any visitSelect(ast::stmt::Select **select_node,ARGS args){return nullptr;}
        virtual std::any visitLoop(ast::stmt::Loop **loop_node,ARGS args){return nullptr;}
        virtual std::any visitIf(ast::stmt::If **ifstmt_node,ARGS args){return nullptr;}
        virtual std::any visitFor(ast::stmt::For **forstmt_node,ARGS args){return nullptr;}
        virtual std::any visitReturn(ast::stmt::Return **ret_node,ARGS args){return nullptr;}
        virtual std::any visitExit(ast::stmt::Exit **exit_node,ARGS args){return nullptr;}
        virtual std::any visitContinue(ast::stmt::Continue **cont_node,ARGS args){return nullptr;}
        virtual std::any visitCase(ast::Case **ca_node,ARGS args){return nullptr;}
        virtual std::any visitExprStmt(ast::stmt::ExprStmt **expr_stmt_node,ARGS args){return nullptr;}

        virtual std::any visitBinary(ast::expr::Binary **logic_node, ARGS args){return nullptr;}
        virtual std::any visitUnary(ast::expr::Unary **unit_node, ARGS args){return nullptr;}
        virtual std::any visitCast(ast::expr::Cast **cast_node,ARGS args){return nullptr;}

        virtual std::any visitCallee(ast::expr::Callee **callee_node, ARGS args){return nullptr;}
        virtual std::any visitArg(ast::expr::Callee::Argument **arg_node, ARGS args){return nullptr;}

        virtual std::any visitID(ast::expr::ID **id_node,ARGS args){return nullptr;}
        virtual std::any visitDigit(ast::expr::Digit **digit_node,ARGS args){return nullptr;}
        virtual std::any visitDecimal(ast::expr::Decimal **decimal, ARGS args){return nullptr;}
        virtual std::any visitString(ast::expr::String **str_node,ARGS args){return nullptr;}
        virtual std::any visitChar(ast::expr::Char **ch_node,ARGS args){return nullptr;}
        virtual std::any visitBoolean(ast::expr::Boolean **bl_node,ARGS args){return nullptr;}
        virtual std::any visitParentheses(ast::expr::Parentheses **parentheses_node,ARGS args){return nullptr;}

        virtual std::any visitAnnotation(ast::Annotation **anno_node, ARGS args) {return nullptr;}
        virtual std::any visitAnnotationUnit(ast::AnnotationUnit **unit_node, ARGS args) {return nullptr;}

        virtual std::any visitMember(ast::Member **member_node,ARGS args){return nullptr;}
        virtual std::any visitStatement(ast::stmt::Statement **stmt_node,ARGS args){return nullptr;}
        virtual std::any visitExpression(ast::expr::Expression **expr_node,ARGS args){return nullptr;}
    };
}


#endif //EVOBASIC2_VISITOR_H

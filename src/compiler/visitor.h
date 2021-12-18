//
// Created by yorkin on 11/1/21.
//

#ifndef EVOBASIC2_VISITOR_H
#define EVOBASIC2_VISITOR_H
#include <any>
#include "parseTree.h"
namespace evoBasic{
    template<typename ARGS>
    class Visitor {
    public:
        virtual std::any visitGlobal(parseTree::Global *global_node, ARGS args){PANIC;}
        virtual std::any visitClass(parseTree::Class *cls_node, ARGS args){PANIC;}
        virtual std::any visitModule(parseTree::Module *mod_node, ARGS args){PANIC;}
        virtual std::any visitInterface(parseTree::Interface *interface_node, ARGS args){PANIC;}

        virtual std::any visitImport(parseTree::Import *imp_node, ARGS args){PANIC;}
        virtual std::any visitDim(parseTree::Dim *dim_node, ARGS args){PANIC;}
        virtual std::any visitVariable(parseTree::Variable *var_node, ARGS args){PANIC;}
        virtual std::any visitFunction(parseTree::Function *func_node, ARGS args){PANIC;}
        virtual std::any visitExternal(parseTree::External *ext_node, ARGS args){PANIC;}
        virtual std::any visitConstructor(parseTree::Constructor *constructor_node, ARGS args){PANIC;}
        virtual std::any visitOperator(parseTree::Operator *op_node, ARGS args){PANIC;}
        virtual std::any visitEnum(parseTree::Enum *em_node, ARGS args){PANIC;}
        virtual std::any visitType(parseTree::Type *ty_node, ARGS args){PANIC;}
        virtual std::any visitParameter(parseTree::Parameter *param_node, ARGS args){PANIC;}

        virtual std::any visitLet(parseTree::stmt::Let *let_node, ARGS args){PANIC;}
        virtual std::any visitSelect(parseTree::stmt::Select *select_node, ARGS args){PANIC;}
        virtual std::any visitLoop(parseTree::stmt::Loop *loop_node, ARGS args){PANIC;}
        virtual std::any visitIf(parseTree::stmt::If *ifstmt_node, ARGS args){PANIC;}
        virtual std::any visitFor(parseTree::stmt::For *forstmt_node, ARGS args){PANIC;}
        virtual std::any visitReturn(parseTree::stmt::Return *ret_node, ARGS args){PANIC;}
        virtual std::any visitExit(parseTree::stmt::Exit *exit_node, ARGS args){PANIC;}
        virtual std::any visitContinue(parseTree::stmt::Continue *cont_node, ARGS args){PANIC;}
        virtual std::any visitCase(parseTree::Case *ca_node, ARGS args){PANIC;}
        virtual std::any visitExprStmt(parseTree::stmt::ExprStmt *expr_stmt_node, ARGS args){PANIC;}

        virtual std::any visitBinary(parseTree::expr::Binary *logic_node, ARGS args){PANIC;}
        virtual std::any visitUnary(parseTree::expr::Unary *unit_node, ARGS args){PANIC;}
        virtual std::any visitIndex(parseTree::expr::Index *index_node, ARGS args){PANIC;}
        virtual std::any visitDot(parseTree::expr::Dot *dot_node, ARGS args){PANIC;}
        virtual std::any visitAssign(parseTree::expr::Assign *assign_node, ARGS args){PANIC;}
        virtual std::any visitCast(parseTree::expr::Cast *cast_node, ARGS args){PANIC;}
        virtual std::any visitColon(parseTree::expr::Colon *colon_node, ARGS args){PANIC;}

        virtual std::any visitCallee(parseTree::expr::Callee *callee_node, ARGS args){PANIC;}
        virtual std::any visitArg(parseTree::expr::Callee::Argument *arg_node, ARGS args){PANIC;}

        virtual std::any visitID(parseTree::expr::ID *id_node, ARGS args){PANIC;}
        virtual std::any visitDigit(parseTree::expr::Digit *digit_node, ARGS args){PANIC;}
        virtual std::any visitDecimal(parseTree::expr::Decimal *decimal, ARGS args){PANIC;}
        virtual std::any visitString(parseTree::expr::String *str_node, ARGS args){PANIC;}
        virtual std::any visitChar(parseTree::expr::Char *ch_node, ARGS args){PANIC;}
        virtual std::any visitBoolean(parseTree::expr::Boolean *bl_node, ARGS args){PANIC;}
        virtual std::any visitParentheses(parseTree::expr::Parentheses *parentheses_node, ARGS args){PANIC;}

        virtual std::any visitAnnotation(parseTree::Annotation *anno_node, ARGS args) {PANIC;}
        virtual std::any visitAnnotationUnit(parseTree::AnnotationUnit *unit_node, ARGS args) {PANIC;}

        virtual std::any visitMember(parseTree::Member *member_node, ARGS args){PANIC;}
        virtual std::any visitStatement(parseTree::stmt::Statement *stmt_node, ARGS args){PANIC;}
        virtual std::any visitExpression(parseTree::expr::Expression *expr_node, ARGS args){PANIC;}
        virtual std::any visitNew(parseTree::expr::New *new_node, ARGS args){PANIC;}
    };


    template<typename ARGS>
    class ModifyVisitor {
    public:
        virtual std::any visitGlobal(parseTree::Global **global_node, ARGS args){PANIC;}
        virtual std::any visitClass(parseTree::Class **cls_node, ARGS args){PANIC;}
        virtual std::any visitModule(parseTree::Module **mod_node, ARGS args){PANIC;}
        virtual std::any visitInterface(parseTree::Interface **interface_node, ARGS args){PANIC;}

        virtual std::any visitImport(parseTree::Import **imp_node, ARGS args){PANIC;}
        virtual std::any visitDim(parseTree::Dim **dim_node, ARGS args){PANIC;}
        virtual std::any visitVariable(parseTree::Variable **var_node, ARGS args){PANIC;}
        virtual std::any visitFunction(parseTree::Function **func_node, ARGS args){PANIC;}
        virtual std::any visitExternal(parseTree::External **ext_node, ARGS args){PANIC;}
        virtual std::any visitConstructor(parseTree::Constructor **constructor_node, ARGS args){PANIC;}
        virtual std::any visitOperator(parseTree::Operator **op_node, ARGS args){PANIC;}
        virtual std::any visitEnum(parseTree::Enum **em_node, ARGS args){PANIC;}
        virtual std::any visitType(parseTree::Type **ty_node, ARGS args){PANIC;}
        virtual std::any visitParameter(parseTree::Parameter **param_node, ARGS args){PANIC;}

        virtual std::any visitLet(parseTree::stmt::Let **let_node, ARGS args){PANIC;}
        virtual std::any visitSelect(parseTree::stmt::Select **select_node, ARGS args){PANIC;}
        virtual std::any visitLoop(parseTree::stmt::Loop **loop_node, ARGS args){PANIC;}
        virtual std::any visitIf(parseTree::stmt::If **ifstmt_node, ARGS args){PANIC;}
        virtual std::any visitFor(parseTree::stmt::For **forstmt_node, ARGS args){PANIC;}
        virtual std::any visitReturn(parseTree::stmt::Return **ret_node, ARGS args){PANIC;}
        virtual std::any visitExit(parseTree::stmt::Exit **exit_node, ARGS args){PANIC;}
        virtual std::any visitContinue(parseTree::stmt::Continue **cont_node, ARGS args){PANIC;}
        virtual std::any visitCase(parseTree::Case **ca_node, ARGS args){PANIC;}
        virtual std::any visitExprStmt(parseTree::stmt::ExprStmt **expr_stmt_node, ARGS args){PANIC;}

        virtual std::any visitBinary(parseTree::expr::Binary **logic_node, ARGS args){PANIC;}
        virtual std::any visitUnary(parseTree::expr::Unary **unit_node, ARGS args){PANIC;}
        virtual std::any visitIndex(parseTree::expr::Index **index_node, ARGS args){PANIC;}
        virtual std::any visitDot(parseTree::expr::Dot **dot_node, ARGS args){PANIC;}
        virtual std::any visitAssign(parseTree::expr::Assign **assign_node, ARGS args){PANIC;}
        virtual std::any visitCast(parseTree::expr::Cast **cast_node, ARGS args){PANIC;}

        virtual std::any visitCallee(parseTree::expr::Callee **callee_node, ARGS args){PANIC;}
        virtual std::any visitArg(parseTree::expr::Callee::Argument **arg_node, ARGS args){PANIC;}

        virtual std::any visitID(parseTree::expr::ID **id_node, ARGS args){PANIC;}
        virtual std::any visitDigit(parseTree::expr::Digit **digit_node, ARGS args){PANIC;}
        virtual std::any visitDecimal(parseTree::expr::Decimal **decimal, ARGS args){PANIC;}
        virtual std::any visitString(parseTree::expr::String **str_node, ARGS args){PANIC;}
        virtual std::any visitChar(parseTree::expr::Char **ch_node, ARGS args){PANIC;}
        virtual std::any visitBoolean(parseTree::expr::Boolean **bl_node, ARGS args){PANIC;}
        virtual std::any visitParentheses(parseTree::expr::Parentheses **parentheses_node, ARGS args){PANIC;}

        virtual std::any visitAnnotation(parseTree::Annotation **anno_node, ARGS args) {PANIC;}
        virtual std::any visitAnnotationUnit(parseTree::AnnotationUnit **unit_node, ARGS args) {PANIC;}

        virtual std::any visitMember(parseTree::Member **member_node, ARGS args){PANIC;}
        virtual std::any visitStatement(parseTree::stmt::Statement **stmt_node, ARGS args){PANIC;}
        virtual std::any visitExpression(parseTree::expr::Expression **expr_node, ARGS args){PANIC;}
        virtual std::any visitNew(parseTree::expr::New **new_node, ARGS args){PANIC;}
    };

#define Transform(AST,NAME) using AST##Tuple = std::tuple<parseTree::AST*,std::any>; \
    virtual AST##Tuple transform##AST(parseTree::AST *NAME##_node,ARGS... args){PANIC;}

    template<typename ...ARGS>
    class Transformer{
    public:
        Transform(Global,global)
        Transform(Class,class)
        Transform(Module,module)
        Transform(Interface,interface)
        Transform(Import,import)
        Transform(Dim,dim)
        Transform(Variable,variable)
        Transform(Function,function)
        Transform(External,exteranl)
        Transform(Constructor,ctor)
        Transform(Operator,op)
        Transform(Enum,enum)
        Transform(Type,type)
        Transform(Parameter,parameter)
        Transform(Let,let)
        Transform(Select,select)
        Transform(Loop,loop)
        Transform(If,if)
        Transform(For,for)
        Transform(Return,return)
        Transform(Exit,exit)
        Transform(Continue,continue)
        Transform(Case,case)
        Transform(ExprStmt,expr_stmt)
        Transform(Binary,binary)
        Transform(Unary,unary)
        Transform(Index,index)
        Transform(Dot,dot)
        Transform(Assign,assign)
        Transform(Cast,cast)
        Transform(Colon,colon)
        Transform(Callee,callee)
        Transform(Argument,arg)
        Transform(ID,id)
        Transform(Digit,digit)
        Transform(Decimal,decimal)
        Transform(String,string)
        Transform(Char,char)
        Transform(Boolean,boolean)
        Transform(Parentheses,parentheses)
        Transform(Annotation,annotation)
        Transform(AnnotationUnit,annotation_unit)
        Transform(Member,member)
        Transform(Statement,statement)
        Transform(Expression,expression)
        Transform(New,new)
    };
#undef Transform

}


#endif //EVOBASIC2_VISITOR_H

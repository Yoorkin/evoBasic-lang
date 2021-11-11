//
// Created by yorkin on 11/1/21.
//

#ifndef EVOBASIC2_SEMANTIC_H
#define EVOBASIC2_SEMANTIC_H
#include "visitor.h"
#include "type.h"
#include "context.h"
namespace evoBasic{

    using AST = ast::Global;

    std::string getID(ast::ID *id);

    class Semantic {
    public:
        static void collectSymbol(AST *ast,std::shared_ptr<Context> context);
        static void collectDetail(AST *ast,std::shared_ptr<Context> context);
        static void typeCheck(AST *ast,std::shared_ptr<Context> context);
        static bool solveTypeInferenceDependencies(std::shared_ptr<Context> context);
        static bool solveByteLengthDependencies(std::shared_ptr<Context> context);
    };


    struct SymbolCollectorArgs{
        std::shared_ptr<type::Domain> domain;
    };

    class SymbolCollector : public Visitor<SymbolCollectorArgs>{
    public:
        std::any visitGlobal(ast::Global *global, SymbolCollectorArgs args) override;
        std::any visitModule(ast::Module *mod, SymbolCollectorArgs args) override;
        std::any visitClass(ast::Class *cls, SymbolCollectorArgs args) override;
        std::any visitEnum(ast::Enum *em, SymbolCollectorArgs args) override;
        std::any visitType(ast::Type *ty, SymbolCollectorArgs args) override;
        std::any visitDim(ast::Dim *dim, SymbolCollectorArgs args) override;
        std::any visitVariable(ast::Variable *var, SymbolCollectorArgs args) override;
        std::any visitID(ast::ID *id, SymbolCollectorArgs args) override;
        std::any visitMember(ast::Member *member_node, SymbolCollectorArgs args) override;
    };

    struct BaseArgs{
        std::shared_ptr<Context> context;
        std::shared_ptr<type::Domain> domain;
        std::shared_ptr<type::Domain> in_terminal_list;
        int checking_args_index = 0;
        bool need_lookup = false;
    };

    class BaseVisitor : public Visitor<BaseArgs>{
    public:
        std::any visitAnnotation(ast::Annotation *anno_node, BaseArgs args) override;
        std::any visitAnnotationUnit(ast::AnnotationUnit *unit_node, BaseArgs args) override;
    };

    class DetailCollector : public BaseVisitor{
    public:
        std::any visitGlobal(ast::Global *global, BaseArgs args) override;
        std::any visitModule(ast::Module *mod, BaseArgs args) override;
        std::any visitClass(ast::Class *cls, BaseArgs args) override;
        std::any visitEnum(ast::Enum *em, BaseArgs args) override;
        std::any visitType(ast::Type *ty, BaseArgs args) override;
        std::any visitDim(ast::Dim *dim, BaseArgs args) override;
        std::any visitVariable(ast::Variable *var, BaseArgs args) override;
        std::any visitFunction(ast::Function *func_node, BaseArgs args) override;
        std::any visitExternal(ast::External *ext_node, BaseArgs args) override;
        std::any visitParameter(ast::Parameter *param_node, BaseArgs args) override;
        std::any visitMember(ast::Member *member_node, BaseArgs args) override;
    };

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
        std::any visitCast(ast::expr::Cast *cast_node, BaseArgs args) override;

        std::any visitLink(ast::expr::Link *link_node, BaseArgs args) override;
        std::any visitCallee(ast::expr::Callee *callee_node, BaseArgs args) override;
        std::any visitArgsList(ast::expr::ArgsList *args_list_node, BaseArgs args) override;
        std::any visitArg(ast::expr::Arg *arg_node, BaseArgs args) override;

        std::any visitDigit(ast::expr::literal::Digit *digit_node, BaseArgs args) override;
        std::any visitDecimal(ast::expr::literal::Decimal *decimal, BaseArgs args) override;
        std::any visitBoolean(ast::expr::literal::Boolean *bl_node, BaseArgs args) override;
        std::any visitChar(ast::expr::literal::Char *ch_node, BaseArgs args) override;
        std::any visitString(ast::expr::literal::String *str_node, BaseArgs args) override;
        std::any visitParentheses(ast::expr::Parentheses *parentheses_node,BaseArgs args)override;

    };

    struct ExpressionType{
        std::shared_ptr<type::Prototype> prototype;
        enum ValueKind {lvalue,rvalue,error} value_kind;
        ExpressionType()=default;
        ExpressionType(std::shared_ptr<type::Prototype> prototype_,ValueKind kind){
            this->prototype = prototype_;
            this->value_kind = kind;
        }
        static ExpressionType *Error;
    };
}



#endif //EVOBASIC2_SEMANTIC_H

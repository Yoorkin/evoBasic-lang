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

    struct TypeAnalyzerArgs{
        Context *context = nullptr;
        type::Domain *domain = nullptr;
        ExpressionType *dot_prefix = nullptr;
        type::Domain *current_class_or_module = nullptr;
        type::UserFunction *user_function = nullptr;
        type::Function *checking_function = nullptr;
        int checking_arg_index = 0;
        bool need_lookup = false;
    };

    class TypeAnalyzer : public Visitor<TypeAnalyzerArgs>{
    public:
        std::any visitGlobal(ast::Global *global_node, TypeAnalyzerArgs args) override;
        std::any visitModule(ast::Module *mod_node, TypeAnalyzerArgs args) override;
        std::any visitClass(ast::Class *cls_node, TypeAnalyzerArgs args) override;
        std::any visitFunction(ast::Function *func_node, TypeAnalyzerArgs args) override;
        std::any visitMember(ast::Member *member_node, TypeAnalyzerArgs args) override;
        std::any visitStatement(ast::stmt::Statement *stmt_node, TypeAnalyzerArgs args) override;

        std::any visitLet(ast::stmt::Let *let_node, TypeAnalyzerArgs args) override;
        std::any visitSelect(ast::stmt::Select *select_node, TypeAnalyzerArgs args) override;
        std::any visitLoop(ast::stmt::Loop *loop_node, TypeAnalyzerArgs args) override;
        std::any visitIf(ast::stmt::If *ifstmt_node, TypeAnalyzerArgs args) override;
        std::any visitFor(ast::stmt::For *forstmt_node, TypeAnalyzerArgs args) override;
        std::any visitReturn(ast::stmt::Return *ret_node, TypeAnalyzerArgs args) override;

        std::any visitParentheses(ast::expr::Parentheses *parentheses_node, TypeAnalyzerArgs args) override;
        std::any visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, TypeAnalyzerArgs args) override;
        std::any visitExpression(ast::expr::Expression *expr_node, TypeAnalyzerArgs args) override;
        std::any visitBinary(ast::expr::Binary *logic_node, TypeAnalyzerArgs args) override;
        std::any visitUnary(ast::expr::Unary *unit_node, TypeAnalyzerArgs args) override;
        std::any visitID(ast::expr::ID *id_node, TypeAnalyzerArgs args) override;
        std::any visitDot(ast::expr::Dot *dot_node, TypeAnalyzerArgs args) override;
        std::any visitIndex(ast::expr::Index *index_node, TypeAnalyzerArgs args) override;
        std::any visitAssign(ast::expr::Assign *assign_node, TypeAnalyzerArgs args) override;
        std::any visitCallee(ast::expr::Callee *callee_node, TypeAnalyzerArgs args) override;
        std::any visitArg(ast::expr::Callee::Argument *arg_node, TypeAnalyzerArgs args) override;
        std::any visitNew(ast::expr::New *new_node, TypeAnalyzerArgs args) override;
        std::any visitAnnotation(ast::Annotation *anno_node, TypeAnalyzerArgs args) override;
        std::any visitAnnotationUnit(ast::AnnotationUnit *unit_node, TypeAnalyzerArgs args) override;

        bool check_binary_op_valid(Location *code,ConversionRules &rules,
                                   type::Prototype *lhs,type::Prototype *rhs,ast::expr::Expression **lhs_node,ast::expr::Expression **rhs_node);
        void check_access(Location *code_location,type::Symbol *target,type::Domain *current,type::Domain *current_class_or_module);
        void check_callee(Location *location,ast::expr::Argument *argument,type::Function *target, TypeAnalyzerArgs args);
        void check_static_access(Location *code_location,ExpressionType *lhs,bool is_rhs_static);
        std::any visitDigit(ast::expr::Digit *digit_node, TypeAnalyzerArgs args) override;
        std::any visitDecimal(ast::expr::Decimal *decimal, TypeAnalyzerArgs args) override;
        std::any visitBoolean(ast::expr::Boolean *bl_node, TypeAnalyzerArgs args) override;
        std::any visitChar(ast::expr::Char *ch_node, TypeAnalyzerArgs args) override;
        std::any visitString(ast::expr::String *str_node, TypeAnalyzerArgs args) override;

        void visitStatementList(ast::stmt::Statement *stmt_list, TypeAnalyzerArgs args);
    };
}

#endif //EVOBASIC_TYPEANALYZER_H

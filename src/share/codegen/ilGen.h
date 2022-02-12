//
// Created by yorkin on 12/13/21.
//

#ifndef EVOBASIC_ILGEN_H
#define EVOBASIC_ILGEN_H
#include <any>

#include <analyzer/semantic.h>
#include <analyzer/context.h>
#include <analyzer/ast.h>
#include <loader/il.h>
#include <analyzer/visitor.h>
#include <analyzer/defaultVisitor.h>

namespace evoBasic{
    
    struct ILGenArgs{
        type::Symbol *dot_expression_context = nullptr;
        bool need_lookup = false;
        CompileTimeContext *context = nullptr;
        il::FunctionDefine *ftn = nullptr;
        il::BasicBlock *previous_block = nullptr,
                  *next_block = nullptr;
    };

    class ILGen{
        il::Document *document = nullptr;
        std::list<il::BasicBlock*> blocks;
    public:
        struct JumpOption{
            il::BasicBlock *looping_condition_block = nullptr;
            il::BasicBlock *after_for_block = nullptr;
            il::BasicBlock *after_loop_block = nullptr;
        };

        void visitGlobal(ast::Global *global_node,il::Document *document);
        auto visitClass(ast::Class *class_node)->il::Class*;
        auto visitModule(ast::Module *module_node)->il::Module*;
        auto visitInterface(ast::Interface *interface_node)->il::Interface*;
        auto visitType(ast::Type *type_node)->il::Record*;
        auto visitEnum(ast::Enum *enum_node)->il::Enum*;
        auto visitFunction(ast::Function *function_node)->il::FunctionDefine*;
        auto visitExternal(ast::External *external_node)->il::Ext*;
        auto visitConstructor(ast::Constructor *ctor_node)->il::Ctor*;

        void visitLet       (ast::Let *let_node, il::BasicBlock *current);
        auto visitSelect    (ast::Select *select_node, il::BasicBlock *current,     JumpOption jump_option)->il::BasicBlock*;
        auto visitLoop      (ast::Loop *loop_node, il::BasicBlock *current,         JumpOption jump_option)->il::BasicBlock*;
        auto visitIf        (ast::If *if_node, il::BasicBlock *current,             JumpOption jump_option)->il::BasicBlock*;
        auto visitCase      (ast::Case *case_node, il::BasicBlock *current,         JumpOption jump_option)->il::BasicBlock*;
        auto visitFor       (ast::For *for_node, il::BasicBlock *current,           JumpOption jump_option)->il::BasicBlock*;
        void visitExprStmt  (ast::ExprStmt *expr_stmt_node, il::BasicBlock *current);
        void visitReturn    (ast::Return *return_node, il::BasicBlock *current);
        void visitExit      (ast::Exit *exit_node, il::BasicBlock *current,         JumpOption jump_option);
        void visitContinue  (ast::Continue *continue_node, il::BasicBlock *current, JumpOption jump_option);
        auto visitStatement (ast::Statement *statement_node, il::BasicBlock *current,JumpOption jump_option)->il::BasicBlock*;

        void visitExpression(ast::Expression *expression_node, il::BasicBlock *current);
        void visitUnary(ast::Unary *unary_node, il::BasicBlock *current);
        void visitBinary(ast::Binary *binary_node, il::BasicBlock *current);
        void visitAssign(ast::Assign *assign_node, il::BasicBlock *current);
        void visitCast(ast::Cast *cast_node, il::BasicBlock *current);
        void visitParentheses(ast::Parentheses *parentheses_node, il::BasicBlock *current);
        void visitArrayElement(ast::ArrayElement *element_node, il::BasicBlock *current);
        void visitDelegate(ast::Delegate *delegate_node, il::BasicBlock *current);
        void visitArgument(ast::Argument *argument_node, il::BasicBlock *current);
        void visitNew(ast::New *new_node, il::BasicBlock *current);
        void visitFtnCall(ast::FtnCall *ftn_node, il::BasicBlock *current);
        void visitSFtnCall(ast::SFtnCall *sftn_node, il::BasicBlock *current);
        void visitVFtnCall(ast::VFtnCall *vftn_node, il::BasicBlock *current);
        void visitExtCall(ast::ExtCall *ext_node, il::BasicBlock *current);
        void visitSFld(ast::SFld *sfld_node, il::BasicBlock *current);
        void visitFld(ast::Fld *fld_node, il::BasicBlock *current);
        void visitLocal(ast::Local *local_node, il::BasicBlock *current);
        void visitArg(ast::Arg *arg_node, il::BasicBlock *current);
        void visitDigit(ast::Digit *digit_node, il::BasicBlock *current);
        void visitDecimal(ast::Decimal *decimal_node, il::BasicBlock *current);
        void visitString(ast::String *string_node, il::BasicBlock *current);
        void visitChar(ast::Char *char_node, il::BasicBlock *current);
        void visitBoolean(ast::Boolean *boolean_node, il::BasicBlock *current);
        void loadCalleeArguments(ast::Call *call, il::BasicBlock *current);
        std::list<il::Member*> visitMember(ast::Member *member);
        il::SFld *visitStaticField(ast::Variable *variable_node);
        il::Fld *visitField(ast::Variable *variable_node);
        std::list<il::Param*> visitParameter(type::Function *function);

        void visitEnumMember(ast::EnumMember *enum_member_node, il::BasicBlock *current);

        void loadArrayRef(ast::ArrayElement *array_element, il::BasicBlock *current);

        il::TokenRef *getArrayElementTokenRef(ast::ArrayElement *array_element);

        void loadAssignLhsValue(ast::Assign *assign_node, il::BasicBlock *current);

        void loadAssignLhsAddress(ast::Assign *assign_node, il::BasicBlock *current);

        il::TokenRef *getRecordOrArrayTypeTokenRef(DataTypeEnum il_type, ExpressionType *type);

        il::TokenRef *getTokenRef(ast::Fld *fld);

        il::TokenRef *getTokenRef(ast::SFld *sfld);

        il::DataType mapILType(type::Prototype *type);

        il::DataType mapILType(ExpressionType *type);

        void loadFtnRef(ast::Expression *ref, il::BasicBlock *current);

        void loadExpressionValue(ast::Expression *terminal, il::BasicBlock *current);
    };

}


#endif //EVOBASIC_ILGEN_H

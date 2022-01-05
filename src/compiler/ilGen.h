//
// Created by yorkin on 12/13/21.
//

#ifndef EVOBASIC_ILGEN_H
#define EVOBASIC_ILGEN_H
#include <any>
#include <semantic.h>
#include "context.h"
#include "ast.h"
#include "il.h"
#include "visitor.h"
#include "defaultVisitor.h"

namespace evoBasic{
    
    struct ILGenArgs{
        type::Symbol *dot_expression_context = nullptr;
        bool need_lookup = false;
        Context *context = nullptr;
        il::FunctionDefine *ftn = nullptr;
        il::BasicBlock *previous_block = nullptr,
                  *next_block = nullptr;
    };

    class ILGen{
        il::Document *document = nullptr;
        il::BasicBlock *for_next = nullptr,*loop_next = nullptr;
        std::list<il::BasicBlock*> blocks;
    public:
        void visitGlobal(ast::Global *global_node,il::Document *document);
        il::Class *visitClass(ast::Class *class_node);
        il::Module *visitModule(ast::Module *module_node);
        il::Interface *visitInterface(ast::Interface *interface_node);
        il::Record *visitType(ast::Type *type_node);
        il::Enum *visitEnum(ast::Enum *enum_node);
        il::FunctionDefine *visitFunction(ast::Function *function_node);
        il::Ext *visitExternal(ast::External *external_node);
        il::Ctor *visitConstructor(ast::Constructor *ctor_node);
        void visitLet(ast::Let *let_node, il::BasicBlock *current, il::BasicBlock *next);
        il::BasicBlock *visitSelect(ast::Select *select_node, il::BasicBlock *current, il::BasicBlock *next);
        il::BasicBlock *visitLoop(ast::Loop *loop_node, il::BasicBlock *current, il::BasicBlock *next);
        il::BasicBlock *visitIf(ast::If *if_node, il::BasicBlock *current, il::BasicBlock *next);
        void visitCase(ast::Case *case_node, il::BasicBlock *current, il::BasicBlock *next);
        il::BasicBlock *visitFor(ast::For *for_node, il::BasicBlock *current, il::BasicBlock *next);
        il::BasicBlock *visitExprStmt(ast::ExprStmt *expr_stmt_node, il::BasicBlock *current, il::BasicBlock *next);
        void visitReturn(ast::Return *return_node, il::BasicBlock *current, il::BasicBlock *next);
        il::BasicBlock *visitExit(ast::Exit *exit_node, il::BasicBlock *current);
        il::BasicBlock *visitContinue(ast::Continue *continue_node, il::BasicBlock *current, il::BasicBlock *next);
        il::BasicBlock *visitStatement(ast::Statement *statement_node, il::BasicBlock *current, il::BasicBlock *next);
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
    };

}


#endif //EVOBASIC_ILGEN_H

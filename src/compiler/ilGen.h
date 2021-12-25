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
        il::FtnWithDefinition *ftn = nullptr;
        il::Block *previous_block = nullptr,
                  *next_block = nullptr;
    };

    class ILGen{
        il::Document *document = new il::Document;
        il::Block *for_next = nullptr,*loop_next = nullptr;
    public:
        il::Document *visitGlobal(ast::Global *global_node);
        il::Class *visitClass(ast::Class *class_node);
        il::Module *visitModule(ast::Module *module_node);
        il::Interface *visitInterface(ast::Interface *interface_node);
        il::Record *visitType(ast::Type *type_node);
        il::Enum *visitEnum(ast::Enum *enum_node);
        il::FtnWithDefinition *visitFunction(ast::Function *function_node);
        il::Ext *visitExternal(ast::External *external_node);
        il::Ctor *visitConstructor(ast::Constructor *ctor_node);
        void visitLet(ast::Let *let_node, il::Block *current, il::Block *next);
        il::Block *visitSelect(ast::Select *select_node, il::Block *current, il::Block *next);
        il::Block *visitLoop(ast::Loop *loop_node, il::Block *current, il::Block *next);
        il::Block *visitIf(ast::If *if_node, il::Block *current, il::Block *next);
        void visitCase(ast::Case *case_node, il::Block *current, il::Block *next);
        il::Block *visitFor(ast::For *for_node, il::Block *current, il::Block *next);
        il::Block *visitExprStmt(ast::ExprStmt *expr_stmt_node, il::Block *current, il::Block *next);
        void visitReturn(ast::Return *return_node, il::Block *current, il::Block *next);
        il::Block *visitExit(ast::Exit *exit_node, il::Block *current);
        il::Block *visitContinue(ast::Continue *continue_node, il::Block *current, il::Block *next);
        il::Block *visitStatement(ast::Statement *statement_node, il::Block *current, il::Block *next);
        void visitExpression(ast::Expression *expression_node, il::Block *current);
        void visitUnary(ast::Unary *unary_node, il::Block *current);
        void visitBinary(ast::Binary *binary_node, il::Block *current);
        void visitAssign(ast::Assign *assign_node, il::Block *current);
        void visitCast(ast::Cast *cast_node, il::Block *current);
        void visitParentheses(ast::Parentheses *parentheses_node, il::Block *current);
        void visitArrayElement(ast::ArrayElement *element_node, il::Block *current);
        void visitDelegate(ast::Delegate *delegate_node, il::Block *current);
        void visitArgument(ast::Argument *argument_node, il::Block *current);
        void visitNew(ast::New *new_node, il::Block *current);
        void visitFtnCall(ast::FtnCall *ftn_node, il::Block *current);
        void visitSFtnCall(ast::SFtnCall *sftn_node, il::Block *current);
        void visitVFtnCall(ast::VFtnCall *vftn_node, il::Block *current);
        void visitExtCall(ast::ExtCall *ext_node, il::Block *current);
        void visitSFld(ast::SFld *sfld_node, il::Block *current);
        void visitFld(ast::Fld *fld_node, il::Block *current);
        void visitLocal(ast::Local *local_node, il::Block *current);
        void visitArg(ast::Arg *arg_node, il::Block *current);
        void visitDigit(ast::Digit *digit_node, il::Block *current);
        void visitDecimal(ast::Decimal *decimal_node, il::Block *current);
        void visitString(ast::String *string_node, il::Block *current);
        void visitChar(ast::Char *char_node, il::Block *current);
        void visitBoolean(ast::Boolean *boolean_node, il::Block *current);
        void loadCalleeArguments(ast::Call *call, il::Block *current);
        std::vector<il::Member*> visitMember(ast::Member *member);
        il::SFld *visitStaticField(ast::Variable *variable_node);
        il::Fld *visitField(ast::Variable *variable_node);
        std::vector<il::Param *> visitParameter(type::Function *function);

        void visitEnumMember(ast::EnumMember *enum_member_node, il::Block *current);
    };

}


#endif //EVOBASIC_ILGEN_H

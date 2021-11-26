//
// Created by yorkin on 11/2/21.
//

#ifndef EVOBASIC2_CODEGEN_H
#define EVOBASIC2_CODEGEN_H
#include "visitor.h"
#include "ir.h"
#include "context.h"
#include "semantic/semantic.h"
#include <variant>

namespace evoBasic{
    struct IRGenArgs{
        type::Function *function = nullptr;
        type::Domain *domain = nullptr;
        type::Symbol *dot_expression_context = nullptr;
        bool need_lookup = false;
        bool reserve_address = false;
        bool is_last_terminal = false;
        bool need_return_value = false;
        Context *context = nullptr;
        ir::Block *previous_block = nullptr;
        ir::Block *next_block = nullptr;
        ir::Block *continue_block = nullptr;
        ir::IR *ir = nullptr;
        int current_args_index = 0;
    };

    // primitive\ptr or Array\Type or Prototype
    struct EmptyType{};

    struct ClassType{
        data::ptr size;
        type::Class *cls;
    };

    struct ArrayType{
        data::ptr size;
        type::Array *array;
    };

    struct RecordType{
        data::ptr size;
        type::Record *record;
    };

    struct DataType{
        vm::Data data;
        type::Primitive *primitive;
    };

    struct AddressType;

    using OperandType = std::variant<EmptyType,DataType,ArrayType,RecordType,AddressType,ClassType,type::Symbol*>;
    enum class OperandEnum{EmptyType,DataType,ArrayType,RecordType,AddressType,ClassType,SymbolPtr};


    struct AddressType{
        OperandType *element = nullptr;
    };

    OperandType addressOf(OperandType type);


    class IRGen : public Visitor<IRGenArgs>{
        //std::stack<OperandType> operand;
    public:
        ir::IR *gen(AST *ast,Context *context);

        OperandType mapSymbolToOperandType(type::Symbol *symbol);
        OperandType visitAssign(ast::expr::Binary *node, IRGenArgs args);
        OperandType pushVariableAddress(type::Variable *variable, ir::Block *block, bool need_push_base);
        OperandType loadOperandAddress(OperandType top,ir::Block *block);
        OperandType visitArithmeticOp(OperandType lhs_operand,ast::expr::Binary *logic_node, IRGenArgs args);
        OperandType visitIndex(ast::expr::Binary *index,IRGenArgs args,bool need_push_base);
        OperandType visitDot(ast::expr::Expression *dot_node,IRGenArgs args,OperandType lhs = EmptyType{});
        OperandType visitLogicOp(ast::expr::Binary *logic_node, IRGenArgs args);

        std::any visitGlobal(ast::Global *global_node, IRGenArgs args) override;
        std::any visitModule(ast::Module *mod_node, IRGenArgs args) override;
        std::any visitClass(ast::Class *cls_node, IRGenArgs args) override;
        std::any visitEnum(ast::Enum *em_node, IRGenArgs args) override;
        std::any visitType(ast::Type *ty_node, IRGenArgs args) override;
        std::any visitDim(ast::Dim *dim_node, IRGenArgs args) override;
        std::any visitVariable(ast::Variable *var_node, IRGenArgs args) override;

        std::any visitFunction(ast::Function *func_node, IRGenArgs args) override;

        ir::Block *visitStatementList(ast::stmt::Statement *statement, IRGenArgs args);
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

        std::any visitID(ast::expr::ID *id_node, IRGenArgs args) override;
        std::any visitBoolean(ast::expr::Boolean *bl_node, IRGenArgs args) override;
        std::any visitChar(ast::expr::Char *ch_node, IRGenArgs args) override;
        std::any visitDigit(ast::expr::Digit *digit_node, IRGenArgs args) override;
        std::any visitDecimal(ast::expr::Decimal *decimal, IRGenArgs args) override;
        std::any visitString(ast::expr::String *str_node, IRGenArgs args) override;
        std::any visitParentheses(ast::expr::Parentheses *parentheses_node, IRGenArgs args) override;
        std::any visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, IRGenArgs args) override;

        std::any visitAnnotation(ast::Annotation *anno_node, IRGenArgs args)override;
        std::any visitExpression(ast::expr::Expression *expr_node, IRGenArgs args) override;

        OperandType visitVariableCall(ast::expr::Callee *callee_node, IRGenArgs args, type::Variable *target);

        std::any visitCast(ast::expr::Cast *cast_node, IRGenArgs args);
        void dereference(std::shared_ptr<type::Symbol> symbol);

        std::any visitCaseInSelectStmt(ast::Case *ca_node, IRGenArgs args);
    };

}


#endif //EVOBASIC2_CODEGEN_H

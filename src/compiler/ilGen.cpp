//
// Created by yorkin on 12/13/21.
//

#include "ilGen.h"
#include<variant>

namespace evoBasic{
    using namespace il;
    using namespace std;

    DataType mapILType(type::Prototype *type){
        switch(type->getKind()){
            case type::SymbolKind::Class:       return il::ref;
            case type::SymbolKind::Enum:        return il::u32;
            case type::SymbolKind::Record:      return il::record;
            case type::SymbolKind::Function:    return il::delegate;
            case type::SymbolKind::Array:       return il::array;
            case type::SymbolKind::Primitive:{
                auto primitive = type->as<type::Primitive*>();
                switch(primitive->getDataKind().getValue()){
                    case vm::Data::i8:      return il::i8;
                    case vm::Data::i16:     return il::i16;
                    case vm::Data::i32:     return il::i32;
                    case vm::Data::i64:     return il::i64;
                    case vm::Data::u8:      return il::u8;
                    case vm::Data::u16:     return il::u16;
                    case vm::Data::u32:     return il::u32;
                    case vm::Data::u64:     return il::u64;
                    case vm::Data::f32:     return il::f32;
                    case vm::Data::f64:     return il::f64;
                    case vm::Data::boolean: return il::boolean;
                    default: PANIC;
                }
            }
            default: PANIC;
        }
    }

#define Visit(RETURN,AST,VAR,...) RETURN ILGen::visit##AST(ast::AST *VAR##_node,##__VA_ARGS__)
    Visit(il::IL*,Global,global){

    }

    Visit(il::Class*,Class,class){

    }

    Visit(il::Module*,Module,module){

    }

    Visit(il::Interface*,Interface,interface){

    }

    Visit(il::Record*,Type,type){

    }

    Visit(il::Enum*,Enum,enum){

    }

    Visit(il::Ftn*,Function,function){

    }

    Visit(il::Ext*,External,external){

    }

    Visit(il::Ctor*,Constructor,ctor){

    }


    il::Block *ILGen::visitStatement(ast::Statement *statement_node, il::Block *current, il::Block *next) {

    }

    il::Block *ILGen::visitLet(ast::Let *let_node, il::Block *current, il::Block *next) {

    }

    il::Block *ILGen::visitSelect(ast::Select *select_node, il::Block *current, il::Block *next) {

    }

    il::Block *ILGen::visitLoop(ast::Loop *loop_node, il::Block *current, il::Block *next) {

    }

    il::Block *ILGen::visitIf(ast::If *if_node, il::Block *current, il::Block *next) {

    }

    il::Block *ILGen::visitCase(ast::Case *case_node, il::Block *current, il::Block *next) {

    }

    il::Block *ILGen::visitFor(ast::For *for_node, il::Block *current, il::Block *next) {

        switch(for_node->iterator->expression_kind){
            case ast::Expression::SFld:{
                auto sfld = (ast::SFld*)for_node->iterator;
                auto il_type = mapILType(sfld->type->getPrototype());
                visitExpression(for_node->begin,current);
                current->Stsfld(il_type,sfld->variable->getToken(factory));
                break;
            }
            case ast::Expression::Fld:{
                auto fld = (ast::Fld*)for_node->iterator;
                auto il_type = mapILType(fld->type->getPrototype());
                visitExpression(fld->ref,current);
                visitExpression(for_node->begin,current);
                current->Stfld(il_type,fld->variable->getToken(factory));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = (ast::Arg*)for_node->iterator;
                auto il_type = mapILType(arg->type->getPrototype());
                current->Push(il::u16,(data::u16)arg->variable->getLayoutIndex());
                visitExpression(for_node->begin,current);
                current->Starg(il_type);
                break;
            }
            case ast::Expression::Local:{
                auto local = (ast::Local*)for_node->iterator;
                auto il_type = mapILType(local->type->getPrototype());
                current->Push(il::u16,(data::u16)local->variable->getLayoutIndex());
                visitExpression(for_node->begin,current);
                current->Stloc(il_type);
                break;
            }
            case ast::Expression::Element:{
                auto element = (ast::ArrayElement*)for_node->iterator;
                auto il_type = mapILType(element->type->getPrototype());
                visitExpression(element->array,current);
                visitExpression(element->offset,current);
                visitExpression(for_node->begin,current);
                current->Stelem(il_type);
                break;
            }
            default: PANIC;
        }

        auto iter_il_type = mapILType(for_node->begin->type->getPrototype());

        current->Push(il::u16,(data::u16)for_node->begin_variable->getLayoutIndex());
        visitExpression(for_node->begin,current);
        current->Stloc(iter_il_type);

        current->Push(il::u16,(data::u16)for_node->end_variable->getLayoutIndex());
        visitExpression(for_node->end,current);
        current->Stloc(iter_il_type);

        if(for_node->step){
            current->Push(il::u16,(data::u16)for_node->step_variable->getLayoutIndex());
            visitExpression(for_node->step,current);
            current->Stloc(iter_il_type);
        }

        auto stmt_block = new il::Block;
        auto cond_block = new il::Block;
        auto after_block = new il::Block;

        current->Br(stmt_block);

        visitStatement(for_node->statement,current,cond_block);

        current->Br(cond_block);

        /*
         * iter = iter + step
         */
        switch(for_node->iterator->expression_kind){
            case ast::Expression::SFld:{
                auto sfld = (ast::SFld*)for_node->iterator;
                auto il_type = mapILType(sfld->type->getPrototype());
                visitExpression(for_node->iterator,cond_block);
                visitExpression(for_node->step,cond_block);
                cond_block->Add(il_type);
                cond_block->Stsfld(il_type,sfld->variable->getToken(factory));
                break;
            }
            case ast::Expression::Fld:{
                auto fld = (ast::Fld*)for_node->iterator;
                auto il_type = mapILType(fld->type->getPrototype());
                visitExpression(fld->ref,cond_block);
                visitExpression(for_node->iterator,cond_block);
                visitExpression(for_node->step,cond_block);
                cond_block->Add(il_type);
                cond_block->Stfld(il_type,fld->variable->getToken(factory));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = (ast::Arg*)for_node->iterator;
                auto il_type = mapILType(arg->type->getPrototype());
                cond_block->Push(il::u16,(data::u16)arg->variable->getLayoutIndex());
                visitExpression(for_node->iterator,cond_block);
                visitExpression(for_node->step,cond_block);
                cond_block->Add(il_type);
                cond_block->Starg(il_type);
                break;
            }
            case ast::Expression::Local:{
                auto local = (ast::Local*)for_node->iterator;
                auto il_type = mapILType(local->type->getPrototype());
                cond_block->Push(il::u16,(data::u16)local->variable->getLayoutIndex());
                visitExpression(for_node->iterator,cond_block);
                visitExpression(for_node->step,cond_block);
                cond_block->Add(il_type);
                cond_block->Stloc(il_type);
                break;
            }
            case ast::Expression::Element:{
                auto element = (ast::ArrayElement*)for_node->iterator;
                auto il_type = mapILType(element->type->getPrototype());
                visitExpression(element->array,cond_block);
                visitExpression(element->offset,cond_block);
                visitExpression(for_node->iterator,cond_block);
                visitExpression(for_node->step,cond_block);
                cond_block->Add(il_type);
                cond_block->Stelem(il_type);
                break;
            }
            default: PANIC;
        }


        /*
         *  beg < end && iter > end ||
         *  beg > end && iter < beg ||
         *  beg == end
         */
        cond_block->Push(il::u16,(data::u16)for_node->begin_variable->getLayoutIndex())
                   .Ldloc(iter_il_type)
                   .Push(il::u16,(data::u16)for_node->end_variable->getLayoutIndex())
                   .Ldloc(iter_il_type)
                   .LT(iter_il_type);
        visitExpression(for_node->iterator,cond_block);
        cond_block->Push(il::u16,(data::u16)for_node->end_variable->getLayoutIndex())
                   .Ldloc(iter_il_type)
                   .GT(iter_il_type);

        cond_block->Push(il::u16,(data::u16)for_node->begin_variable->getLayoutIndex())
                    .Ldloc(iter_il_type)
                    .Push(il::u16,(data::u16)for_node->end_variable->getLayoutIndex())
                    .Ldloc(iter_il_type)
                    .GT(iter_il_type);
        visitExpression(for_node->iterator,cond_block);
        cond_block->Or();
        cond_block->Push(il::u16,(data::u16)for_node->begin_variable->getLayoutIndex())
                .Ldloc(iter_il_type)
                .Push(il::u16,(data::u16)for_node->end_variable->getLayoutIndex())
                .Ldloc(iter_il_type)
                .EQ(iter_il_type)
                .Or();


        cond_block->Jif(after_block);
        cond_block->Br(stmt_block);

        return after_block;
    }

    il::Block *ILGen::visitExprStmt(ast::ExprStmt *expr_stmt_node, il::Block *current, il::Block *next) {
        visitExpression(expr_stmt_node->expr,current);
        return current;
        // todo: clean operand stack
    }

    il::Block *ILGen::visitExit(ast::Exit *exit_node, il::Block *current) {
        switch(exit_node->exit_flag){
            case ast::Exit::For:
                NotNull(for_next);
                current->Br(for_next);
                break;
            case ast::Exit::While:
                NotNull(loop_next);
                current->Br(loop_next);
                break;
            case ast::Exit::Sub:
                current->Ret();
                break;
        }
        return current;
    }

    il::Block *ILGen::visitContinue(ast::Continue *continue_node, il::Block *current, il::Block *next) {
        current->Br(next);
    }



    void ILGen::visitExpression(ast::Expression *expression_node, il::Block *current) {
        switch(expression_node->expression_kind){
            case ast::Expression::Element:
                visitArrayElement((ast::ArrayElement*)expression_node,current);
                break;
            case ast::Expression::Ftn:
                visitFtnCall((ast::FtnCall*)expression_node,current);
                break;
            case ast::Expression::VFtn:
                visitVFtnCall((ast::VFtnCall*)expression_node,current);
                break;
            case ast::Expression::SFtn:
                visitSFtnCall((ast::SFtnCall*)expression_node,current);
                break;
            case ast::Expression::Local:
                visitLocal((ast::Local*)expression_node,current);
                break;
            case ast::Expression::ArgUse:
                visitArg((ast::Arg*)expression_node,current);
                break;
            case ast::Expression::Fld:
                visitFld((ast::Fld*)expression_node,current);
                break;
            case ast::Expression::Assign:
                visitAssign((ast::Assign*)expression_node,current);
                break;
            case ast::Expression::SFld:
                visitSFld((ast::SFld*)expression_node,current);
                break;
            case ast::Expression::Digit:
                visitDigit((ast::Digit*)expression_node,current);
                break;
            case ast::Expression::Decimal:
                visitDecimal((ast::Decimal*)expression_node,current);
                break;
            case ast::Expression::String:
                visitString((ast::String*)expression_node,current);
                break;
            case ast::Expression::Boolean:
                visitBoolean((ast::Boolean*)expression_node,current);
                break;
            case ast::Expression::Char:
                visitChar((ast::Char*)expression_node,current);
                break;
            case ast::Expression::Unary:
                visitUnary((ast::Unary*)expression_node,current);
                break;
            case ast::Expression::Binary:
                visitBinary((ast::Binary*)expression_node,current);
                break;
            case ast::Expression::Cast:
                visitCast((ast::Cast*)expression_node,current);
                break;
            case ast::Expression::New:
                visitNew((ast::New*)expression_node,current);
                break;
            case ast::Expression::Parentheses:
                visitParentheses((ast::Parentheses*)expression_node,current);
                break;
            case ast::Expression::Empty:
            case ast::Expression::Argument:
            case ast::Expression::TmpPath:
                PANIC;
                break;
            case ast::Expression::Ext:
                visitExtCall((ast::ExtCall*)expression_node,current);
                break;
            case ast::Expression::Delegate:
                visitDelegate((ast::Delegate*)expression_node,current);
                break;
        }
    }

    void ILGen::visitUnary(ast::Unary *unary_node, il::Block *current) {
        visitExpression(unary_node->terminal,current);
        auto il_type = mapILType(unary_node->type->getPrototype());
        using Op = ast::Unary::Op;
        switch(unary_node->op){
            case Op::ADD:   /* do nothing */ break;
            case Op::MINUS: current->Neg(il_type); break;
            case Op::Empty: PANIC;
        }
    }

    void ILGen::visitBinary(ast::Binary *binary_node, il::Block *current) {
        visitExpression(binary_node->lhs,current);
        visitExpression(binary_node->rhs,current);
        auto il_type = mapILType(binary_node->type->getPrototype());
        using Op = ast::Binary::Op;
        switch (binary_node->op) {
            case Op::Empty: PANIC; break;
            case Op::And: current->And(); break;
            case Op::Or: current->Or(); break;
            case Op::Xor: current->Xor(); break;
            case Op::Not: current->Not(); break;
            case Op::EQ: current->EQ(il_type); break;
            case Op::NE: current->NE(il_type); break;
            case Op::GE: current->GE(il_type); break;
            case Op::LE: current->LE(il_type); break;
            case Op::GT: current->GT(il_type); break;
            case Op::LT: current->LT(il_type); break;
            case Op::ADD: current->Add(il_type); break;
            case Op::MINUS: current->Sub(il_type); break;
            case Op::MUL: current->Mul(il_type); break;
            case Op::DIV: current->Div(il_type); break;
            case Op::FDIV: current->FDiv(il_type); break;
        }
    }

    void ILGen::visitAssign(ast::Assign *assign_node, il::Block *current) {
        auto expr = assign_node->lhs;
        switch (expr->expression_kind) {
            case ast::Expression::Fld:{
                auto fld = (ast::Fld*)expr;
                visitFld(fld,current);
                visitExpression(assign_node->rhs,current);
                current->Stfld(mapILType(fld->variable->getPrototype()),fld->variable->getToken(factory));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = (ast::SFld*)expr;
                visitSFld(sfld,current);
                visitExpression(assign_node->rhs,current);
                current->Stsfld(mapILType(sfld->variable->getPrototype()),sfld->variable->getToken(factory));
                break;
            }
            case ast::Expression::Element:{
                auto element = (ast::ArrayElement*)expr;
                visitExpression(element->array,current);
                visitExpression(element->offset,current);
                visitExpression(assign_node->rhs,current);
                current->Stelem(mapILType(element->type->getPrototype()));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = (ast::Arg*)expr;
                current->Push(il::u16,(data::u16)arg->variable->getLayoutIndex());
                visitExpression(assign_node->rhs,current);
                current->Stloc(mapILType(arg->variable->getPrototype()));
                break;
            }
            default: PANIC;
        }
    }

    void ILGen::visitCast(ast::Cast *cast_node, il::Block *current) {
        visitExpression(cast_node->expr,current);
        current->Conv(mapILType(cast_node->expr->type->getPrototype()),mapILType(cast_node->target));
    }

    void ILGen::visitParentheses(ast::Parentheses *parentheses_node, il::Block *current) {
        visitExpression(parentheses_node->expr,current);
    }

    void ILGen::visitArrayElement(ast::ArrayElement *element_node, il::Block *current) {
        visitExpression(element_node->array,current);
        current->Ldelem(mapILType(element_node->type->getPrototype()));
    }

    void ILGen::visitDelegate(ast::Delegate *delegate_node, il::Block *current) {
        //todo
    }


    void ILGen::visitArgument(ast::Argument *argument_node, il::Block *current) {
        auto expr = argument_node->expr;
        if(argument_node->byval){
            // pass byval
            if(expr->expression_kind == ast::Expression::Assign){
                auto assign = (ast::Assign*)expr;
                visitAssign(assign,current);
                expr = assign->lhs;
            }

            switch (expr->expression_kind) {
                case ast::Expression::Ftn:
                    visitFtnCall((ast::FtnCall*)expr,current);
                    break;
                case ast::Expression::VFtn:
                    visitVFtnCall((ast::VFtnCall*)expr,current);
                    break;
                case ast::Expression::SFtn:
                    visitSFtnCall((ast::SFtnCall*)expr,current);
                    break;
                case ast::Expression::Fld:{
                    auto fld = (ast::Fld*)expr;
                    visitFld(fld,current);
                    current->Ldfld(mapILType(fld->variable->getPrototype()),fld->variable->getToken(factory));
                    break;
                }
                case ast::Expression::SFld:{
                    auto sfld = (ast::SFld*)expr;
                    visitSFld(sfld,current);
                    current->Ldsfld(mapILType(sfld->variable->getPrototype()),sfld->variable->getToken(factory));
                    break;
                }
                case ast::Expression::Element:{
                    auto element = (ast::ArrayElement*)expr;
                    visitExpression(element->array,current);
                    visitExpression(element->offset,current);
                    current->Ldelem(mapILType(element->type->getPrototype()));
                    break;
                }
                case ast::Expression::ArgUse:{
                    auto arg = (ast::Arg*)expr;
                    current->Push(il::u16,arg->variable->getLayoutIndex())
                            .Ldarg(mapILType(arg->variable->getPrototype()));
                    break;
                }
                case ast::Expression::Unary:
                case ast::Expression::Binary:
                case ast::Expression::New:
                case ast::Expression::Digit:
                case ast::Expression::Decimal:
                case ast::Expression::String:
                case ast::Expression::Boolean:
                case ast::Expression::Char:
                case ast::Expression::Delegate:
                case ast::Expression::Parentheses:
                case ast::Expression::Cast:
                    // do nothing
                    break;
                default: PANIC;
            }
        }
        else{
            // pass byref
            if(expr->expression_kind == ast::Expression::Assign){
                auto assign = (ast::Assign*)expr;
                visitAssign(assign,current);
                expr = assign->lhs;
            }

            switch (expr->expression_kind) {
                case ast::Expression::Fld:{
                    auto fld = (ast::Fld*)expr;
                    visitFld(fld,current);
                    current->Ldflda(fld->variable->getToken(factory));
                    break;
                }
                case ast::Expression::SFld:{
                    auto sfld = (ast::SFld*)expr;
                    visitSFld(sfld,current);
                    current->Ldsflda(sfld->variable->getToken(factory));
                    break;
                }
                case ast::Expression::Element:{
                    auto element = (ast::ArrayElement*)expr;
                    visitExpression(element->array,current);
                    visitExpression(element->offset,current);
                    current->Ldelema();
                    break;
                }
                case ast::Expression::ArgUse:{
                    auto arg = (ast::Arg*)expr;
                    current->Push(il::u16,(data::u16)arg->variable->getLayoutIndex())
                            .Ldarga();
                    break;
                }
                default: PANIC;
            }
        }
    }


    void ILGen::visitNew(ast::New *new_node, il::Block *current) {
        loadCalleeArguments(new_node,current);
        current->Newobj(new_node->target->getToken(factory));
    }

    void ILGen::visitFtnCall(ast::FtnCall *ftn_node, il::Block *current) {
        visitExpression(ftn_node->ref,current);
        current->Ldftn(ftn_node->function->getToken(factory));
        loadCalleeArguments(ftn_node,current);
        current->Call();
    }

    void ILGen::visitSFtnCall(ast::SFtnCall *sftn_node, il::Block *current) {
        loadCalleeArguments(sftn_node,current);
        current->Ldsftn(sftn_node->function->getToken(factory))
                .Callstatic();
    }

    void ILGen::loadCalleeArguments(ast::Call *call,il::Block *current){
        // load regular argument by declaration order
        FOR_EACH(iter,call->argument){
            visitArgument(iter,current);
        }
    }

    void ILGen::visitVFtnCall(ast::VFtnCall *vftn_node, il::Block *current) {
        visitExpression(vftn_node->ref,current);
        current->Ldvftn(vftn_node->function->getToken(factory));
        loadCalleeArguments(vftn_node,current);
        current->Callvirt();
    }

    void ILGen::visitExtCall(ast::ExtCall *ext_node, il::Block *current) {
        current->Invoke(ext_node->function->getToken(factory));
    }

    void ILGen::visitSFld(ast::SFld *sfld_node, il::Block *current) {
        auto token = sfld_node->variable->getToken(factory);
        current->Ldsfld(mapILType(sfld_node->variable->getPrototype()),token);
    }

    void ILGen::visitFld(ast::Fld *fld_node, il::Block *current) {
        auto token = fld_node->variable->getToken(factory);
        visitExpression(fld_node->ref,current);
        current->Ldfld(mapILType(fld_node->variable->getPrototype()),token);
    }

    void ILGen::visitLocal(ast::Local *local_node, il::Block *current) {
        auto index = local_node->variable->getLayoutIndex();
        current->Push(DataType::u16,(data::u16)index);
        current->Ldloc(mapILType(local_node->variable->getPrototype()));
    }

    void ILGen::visitArg(ast::Arg *arg_node, il::Block *current) {
        auto index = arg_node->variable->getLayoutIndex();
        current->Push(DataType::u16,(data::u16)index);
        current->Ldarg(mapILType(arg_node->variable->getPrototype()));
    }

    void ILGen::visitDigit(ast::Digit *digit_node, il::Block *current) {
        current->Push(DataType::i32,digit_node->value);
    }

    void ILGen::visitDecimal(ast::Decimal *decimal_node, il::Block *current) {
        current->Push(DataType::f64,decimal_node->value);
    }

    void ILGen::visitString(ast::String *string_node, il::Block *current) {
        current->Ldc(factory->createToken(string_node->value));
        //todo: call ctor for String
    }

    void ILGen::visitChar(ast::Char *char_node, il::Block *current) {
        current->Push(DataType::character,char_node->value);
    }

    void ILGen::visitBoolean(ast::Boolean *boolean_node, il::Block *current) {
        current->Push(DataType::boolean,boolean_node->value);
    }

}


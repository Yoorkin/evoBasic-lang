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

    il::Document *ILGen::visitGlobal(ast::Global *global_node) {
        auto members = visitMember(global_node->member);
        return factory->createDocument(members);
    }

    il::Class *ILGen::visitClass(ast::Class *class_node) {
        auto members = visitMember(class_node->member);
        auto cls = class_node->class_symbol;
        auto extend = factory->createExtend(cls->getExtend());
        vector<Impl*> impls;
        for(auto [_,interface] : cls->getImplMap()){
            impls.push_back(factory->createImplements(interface));
        }
        return factory->createClass(cls->getName(),cls->getAccessFlag(),extend,impls,members);
    }

    il::SFld *ILGen::visitStaticField(ast::Variable *variable_node){
        auto variable = variable_node->variable_symbol;
        return factory->createStaticField(variable->getName(),variable->getAccessFlag(),variable->getPrototype());
    }

    il::Fld *ILGen::visitField(ast::Variable *variable_node){
        auto variable = variable_node->variable_symbol;
        return factory->createField(variable->getName(),variable->getAccessFlag(),variable->getPrototype());
    }

    vector<Member*> ILGen::visitMember(ast::Member *member){
        vector<Member*> members;
        FOR_EACH(iter,member){
            switch(iter->member_kind){
                case ast::Member::function_:
                    members.push_back(visitFunction((ast::Function*)iter));
                    break;
                case ast::Member::class_:
                    members.push_back(visitClass((ast::Class*)iter));
                    break;
                case ast::Member::module_:
                    members.push_back(visitModule((ast::Module*)iter));
                    break;
                case ast::Member::type_:
                    members.push_back(visitType((ast::Type*)iter));
                    break;
                case ast::Member::enum_:
                    members.push_back(visitEnum((ast::Enum*)iter));
                    break;
                case ast::Member::dim_:{
                    auto dim = (ast::Dim*)iter;
                    FOR_EACH(variable,dim->variable){
                        if(variable->variable_symbol->isStatic()){
                            members.push_back(visitStaticField(variable));
                        }
                        else{
                            members.push_back(visitField(variable));
                        }
                    }
                    break;
                }
                case ast::Member::external_:
                    members.push_back(visitExternal((ast::External*)iter));
                    break;
                case ast::Member::interface_:
                    members.push_back(visitInterface((ast::Interface*)iter));
                    break;
            }
        }
        return members;
    }

    vector<il::Param*> ILGen::visitParameter(vector<type::Parameter*> parameters){
        vector<Param*> params;
        for(auto iter : parameters){
            if(iter->isOptional()){
                auto block = new il::Block;
                visitExpression(iter->getDefaultArgument()->expr,block);
                params.push_back(factory->createOption(iter->getName(),iter->getPrototype(),!iter->isByval(),block));
            }
            else if(iter->isParamArray()){
                params.push_back(factory->createParamArray(iter->getName(),iter->getPrototype(),!iter->isByval()));
            }
            else{
                params.push_back(factory->createParam(iter->getName(),iter->getPrototype(),!iter->isByval()));
            }
        }
        return params;
    }

    il::Module *ILGen::visitModule(ast::Module *module_node) {
        auto members = visitMember(module_node->member);
        auto mod = module_node->module_symbol;
        return factory->createModule(mod->getName(),mod->getAccessFlag(),members);
    }

    il::Interface *ILGen::visitInterface(ast::Interface *interface_node) {
        vector<FtnBase*> ftns;
        for(auto symbol : *(interface_node->interface_symbol)){
            auto member = symbol->as<type::Function*>();
            auto parameter = visitParameter(member->getArgsSignature());
            auto result = factory->createResult(member->getRetSignature());
            auto ftn = factory->createInterfaceFunction(member->getName(),member->getAccessFlag(),parameter,result);
            ftns.push_back(ftn);
        }
        auto itf = interface_node->interface_symbol;
        return factory->createInterface(itf->getName(),itf->getAccessFlag(),ftns);
    }

    il::Record *ILGen::visitType(ast::Type *type_node) {
        vector<Fld*> fields;
        for(auto symbol : *(type_node->type_symbol)){
            auto variable = symbol->as<type::Variable*>();
            fields.push_back(factory->createField(variable->getName(),variable->getAccessFlag(),variable->getPrototype()));
        }
        auto ty = type_node->type_symbol;
        return factory->createRecord(ty->getName(),ty->getAccessFlag(),fields);
    }

    il::Enum *ILGen::visitEnum(ast::Enum *enum_node) {
        vector<il::Pair*> pairs;
        for(auto symbol : *(enum_node->enum_symbol)){
            auto member = symbol->as<type::EnumMember*>();
            pairs.push_back(factory->createPair(member->getName(),member->getIndex()));
        }
        auto em = enum_node->enum_symbol;
        return factory->createEnum(em->getName(),em->getAccessFlag(),pairs);
    }

    il::Ftn *ILGen::visitFunction(ast::Function *function_node) {
        auto entry = new il::Block;
        visitStatement(function_node->statement, entry, nullptr);
        auto parameter = visitParameter(function_node->function_symbol->getArgsSignature());
        auto symbol = function_node->function_symbol;
        auto result = factory->createResult(symbol->getRetSignature());
        auto ftn = factory->createFunction(symbol->getName(), function_node->access, parameter,result,entry);
        return ftn;
    }

    il::Ext *ILGen::visitExternal(ast::External *external_node) {
        auto parameter = visitParameter(external_node->function_symbol->getArgsSignature());
        auto symbol = external_node->function_symbol;
        auto result = factory->createResult(symbol->getRetSignature());
        auto ext = factory->createExternalFunction(symbol->getName(),symbol->getLibName(),external_node->access,parameter,result);
        return ext;
    }

    il::Ctor *ILGen::visitConstructor(ast::Constructor *ctor_node) {
        auto entry = new il::Block;
        auto parameter = visitParameter(ctor_node->constructor_symbol->getArgsSignature());
        visitStatement(ctor_node->statement,entry,nullptr);
        auto ctor = factory->createConstructor(ctor_node->access,parameter,entry);
        return ctor;
    }


    il::Block *ILGen::visitStatement(ast::Statement *statement_node, il::Block *current, il::Block *next) {
        FOR_EACH(iter,statement_node){
            switch(iter->stmt_flag){
                case ast::Statement::error:
                    PANIC;
                    break;
                case ast::Statement::let_:
                    visitLet((ast::Let*)iter,current,next);
                    break;
                case ast::Statement::loop_:
                    visitLoop((ast::Loop*)iter,current,next);
                    break;
                case ast::Statement::if_:
                    current = visitIf((ast::If*)iter,current,next);
                    break;
                case ast::Statement::for_:
                    current = visitFor((ast::For*)iter,current,next);
                    break;
                case ast::Statement::select_:
                    current = visitSelect((ast::Select*)iter,current,next);
                    break;
                case ast::Statement::return_:
                    visitReturn((ast::Return*)iter,current,next);
                    break;
                case ast::Statement::continue_:
                    visitContinue((ast::Continue*)iter,current,next);
                    break;
                case ast::Statement::exit_:
                    visitExit((ast::Exit*)iter,current);
                    break;
                case ast::Statement::expr_:
                    visitExprStmt((ast::ExprStmt*)iter,current,next);
                    break;
            }
        }
        return current;
    }

    void ILGen::visitLet(ast::Let *let_node, il::Block *current, il::Block *next) {
        FOR_EACH(iter,let_node->variable){
            current->Push(il::u16,(data::u16)iter->variable_symbol->getLayoutIndex());
            visitExpression(iter->initial,current);
            current->Stloc(mapILType(iter->initial->type->getPrototype()));
        }
    }

    il::Block *ILGen::visitSelect(ast::Select *select_node, il::Block *current, il::Block *next) {
        //todo
    }

    il::Block *ILGen::visitLoop(ast::Loop *loop_node, il::Block *current, il::Block *next) {
        auto cond_block = new il::Block,
             loop_block = new il::Block,
             after_block = new il::Block;

        current->Br(cond_block);

        visitExpression(loop_node->condition,cond_block);
        cond_block->Jif(loop_block);
        cond_block->Br(after_block);

        loop_block = visitStatement(loop_node->statement,loop_block,after_block);
        loop_block->Br(cond_block);

        return after_block;
    }

    il::Block *ILGen::visitIf(ast::If *if_node, il::Block *current, il::Block *next) {
        auto after_block = new il::Block;
        visitCase(if_node->case_,current,after_block);
        return after_block;
    }

    void ILGen::visitCase(ast::Case *case_node, il::Block *current, il::Block *next) {
        map<ast::Case*,il::Block*> case_blocks;
        FOR_EACH(iter,case_node){
            case_blocks.insert({iter,new il::Block});
        }

        FOR_EACH(iter,case_node){
            auto case_block = case_blocks.find(iter);
            if(case_block != case_blocks.end()){
                visitExpression(iter->condition,current);
                current->Jif(case_block->second);
                auto after_case_block = visitStatement(iter->statement,case_block->second,next);
                after_case_block->Br(next);
            }
            else{
                auto after_case_block = visitStatement(iter->statement,current,next);
                after_case_block->Br(next);
            }
        }
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

        stmt_block = visitStatement(for_node->statement,current,cond_block);

        stmt_block->Br(cond_block);

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

    void ILGen::visitReturn(ast::Return *return_node, il::Block *current, il::Block *next) {
        visitExpression(return_node->expr,current);
        current->Ret();
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
                current->Starg(mapILType(arg->variable->getPrototype()));
                break;
            }
            case ast::Expression::Local:{
                auto local = (ast::Local*)expr;
                current->Push(il::u16,(data::u16)local->variable->getLayoutIndex());
                visitExpression(assign_node->rhs,current);
                current->Stloc(mapILType(local->variable->getPrototype()));
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


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
            case type::SymbolKind::Class:       return DataType::ref;
            case type::SymbolKind::Enum:        return DataType::u32;
            case type::SymbolKind::Record:      return DataType::record;
            case type::SymbolKind::Function:    return DataType::delegate;
            case type::SymbolKind::Array:       return DataType::array;
            case type::SymbolKind::Primitive:{
                auto primitive = type->as<type::Primitive*>();
                switch(primitive->getDataKind().getValue()){
                    case vm::Data::i8:      return DataType::i8;
                    case vm::Data::i16:     return DataType::i16;
                    case vm::Data::i32:     return DataType::i32;
                    case vm::Data::i64:     return DataType::i64;
                    case vm::Data::u8:      return DataType::u8;
                    case vm::Data::u16:     return DataType::u16;
                    case vm::Data::u32:     return DataType::u32;
                    case vm::Data::u64:     return DataType::u64;
                    case vm::Data::f32:     return DataType::f32;
                    case vm::Data::f64:     return DataType::f64;
                    case vm::Data::boolean: return DataType::boolean;
                    default: PANIC;
                }
            }
            default: PANIC;
        }
    }

    void ILGen::visitGlobal(ast::Global *global_node,Document *document) {
        this->document = document;
        auto members = visitMember(global_node->member);
        for(auto member : members){
            document->add(member);
        }
    }

    il::Class *ILGen::visitClass(ast::Class *class_node) {
        auto members = visitMember(class_node->member);
        auto cls = class_node->class_symbol;
        auto extend = document->getTokenRef(cls->getExtend()->getFullName());
        list<TokenRef*> impls;
        for(auto [_,interface] : cls->getImplMap()){
            impls.push_back(document->getTokenRef(interface->getFullName()));
        }
        return new il::Class(document,cls->getAccessFlag(),document->getTokenRef(cls->getName()), extend, impls, members);
    }

    il::SFld *ILGen::visitStaticField(ast::Variable *variable_node){
        auto variable = variable_node->variable_symbol;
        auto name = document->getTokenRef(variable->getName());
        auto type = document->getTokenRef(variable->getPrototype()->getName());
        return new SFld(document,variable->getAccessFlag(),name,type);
    }

    il::Fld *ILGen::visitField(ast::Variable *variable_node){
        auto variable = variable_node->variable_symbol;
        auto name = document->getTokenRef(variable->getName());
        auto type = document->getTokenRef(variable->getPrototype()->getFullName());
        return new Fld(document,variable->getAccessFlag(),name,type);
    }

    list<Member*> ILGen::visitMember(ast::Member *member){
        list<Member*> members;
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
                case ast::Member::constructor_:
                    members.push_back(visitConstructor((ast::Constructor*)iter));
                    break;
                default:
                    PANIC;
            }
        }
        return members;
    }

    list<il::Param*> ILGen::visitParameter(type::Function *function){
        list<Param*> params;

        for(auto iter : function->getArgsSignature()){
            auto name = document->getTokenRef(iter->getName());
            auto type = document->getTokenRef(iter->getPrototype()->getFullName());
            auto param = new Regular(document,name,type,!iter->isByval());
            params.push_back(param);
        }

        for(auto iter : function->getArgsOptions()){
            auto block = new il::BasicBlock(document);
            visitExpression(iter->getDefaultArgument()->expr,block);
            auto name = document->getTokenRef(iter->getName());
            auto type = document->getTokenRef(iter->getPrototype()->getFullName());
            auto param = new Opt(document,name,type,!iter->isByval(),block);
            params.push_back(param);
        }

        if(function->getParamArray()){
            auto name = document->getTokenRef(function->getParamArray()->getName());
            auto type = document->getTokenRef(function->getParamArray()->getPrototype()->getFullName());
            auto param = new Inf(document,name,type,!function->getParamArray()->isByval());
            params.push_back(param);
        }

        return params;
    }

    il::Module *ILGen::visitModule(ast::Module *module_node) {
        auto members = visitMember(module_node->member);
        auto mod = module_node->module_symbol;
        auto name = document->getTokenRef(mod->getName());
        return new Module(document,mod->getAccessFlag(),name,members);
    }

    il::Interface *ILGen::visitInterface(ast::Interface *interface_node) {
        list<InterfaceFunction*> ftns;
        for(auto symbol : *(interface_node->interface_symbol)){
            auto member = symbol->as<type::Function*>();
            auto parameter = visitParameter(member);
            Result *result = nullptr;
            if(member->getRetSignature()){
                auto type = document->getTokenRef(member->getRetSignature()->getFullName());
                result = new Result(document,type);
            }
            auto name = document->getTokenRef(member->getName());
            auto ftn = new InterfaceFunction(document,name,parameter,result);
            ftns.push_back(ftn);
        }
        auto itf = interface_node->interface_symbol;
        auto itf_name = document->getTokenRef(itf->getName());
        return new Interface(document,itf->getAccessFlag(),itf_name,ftns);
    }

    il::Record *ILGen::visitType(ast::Type *type_node) {
        list<Fld*> fields;
        for(auto symbol : *(type_node->type_symbol)){
            auto variable = symbol->as<type::Variable*>();
            auto name = document->getTokenRef(variable->getName());
            auto type = document->getTokenRef(variable->getPrototype()->getFullName());
            auto fld = new Fld(document,variable->getAccessFlag(),name,type);
            fields.push_back(fld);
        }
        auto ty = type_node->type_symbol;
        auto ty_name = document->getTokenRef(ty->getName());
        return new Record(document,ty->getAccessFlag(),ty_name,fields);
    }

    il::Enum *ILGen::visitEnum(ast::Enum *enum_node) {
        list<il::Enum::Pair> pairs;
        for(auto symbol : *(enum_node->enum_symbol)){
            auto member = symbol->as<type::EnumMember*>();
            il::Enum::Pair pair;
            pair.first = document->getTokenRef(member->getName());
            pair.second = member->getIndex();
            pairs.push_back(pair);
        }
        auto em = enum_node->enum_symbol;
        auto em_name = document->getTokenRef(em->getName());
        return new Enum(document,em->getAccessFlag(),em_name,pairs);
    }

    il::FunctionDefine *ILGen::visitFunction(ast::Function *function_node) {
        auto entry = new il::BasicBlock(document);
        blocks.clear();
        blocks.push_back(entry);
        auto symbol = function_node->function_symbol->as<type::UserFunction*>();
        auto tail_block = visitStatement(function_node->statement, entry, nullptr);
        tail_block->Ret();

        auto parameter = visitParameter(symbol);
        Result *result = nullptr;
        if(symbol->getRetSignature()){
            auto type = document->getTokenRef(symbol->getRetSignature()->getFullName());
            result = new Result(document,type);
        }

        list<il::Local*> locals;
        for(auto variable : symbol->getMemoryLayout()){
            auto name = document->getTokenRef(variable->getName());
            auto type = document->getTokenRef(variable->getPrototype()->getFullName());
            auto local = new Local(document,name,type,locals.size());
            locals.push_back(local);
        }

        FunctionDefine *ftn = nullptr;
        auto ftn_name = document->getTokenRef(symbol->getName());
        switch(symbol->getFunctionFlag()){
            case FunctionFlag::Method:
                ftn = new Ftn(document,function_node->access,ftn_name,parameter,result,locals,blocks);
                break;
            case FunctionFlag::Static:
                ftn = new SFtn(document,function_node->access,ftn_name,parameter,result,locals,blocks);
                break;
            case FunctionFlag::Virtual:
            case FunctionFlag::Override:
                ftn = new VFtn(document,function_node->access,ftn_name,parameter,result,locals,blocks);
                break;
        }
        return ftn;
    }

    il::Ext *ILGen::visitExternal(ast::External *external_node) {
        auto parameter = visitParameter(external_node->function_symbol);
        auto symbol = external_node->function_symbol;
        Result *result = nullptr;
        if(symbol->getRetSignature()){
            auto type = document->getTokenRef(symbol->getRetSignature()->getFullName());
            result = new Result(document,type);
        }

        TokenRef *alias = nullptr;
        auto library = document->getTokenRef(symbol->getLibName());
        if(!symbol->getAlias().empty()){
            alias = document->getTokenRef(symbol->getAlias());
        }
        auto name = document->getTokenRef(symbol->getName());
        return new Ext(document,symbol->getAccessFlag(),name,library,alias,parameter,result);
    }

    il::Ctor *ILGen::visitConstructor(ast::Constructor *ctor_node) {
        auto entry = new il::BasicBlock(document);
        blocks.clear();
        blocks.push_back(entry);
        auto parameter = visitParameter(ctor_node->constructor_symbol);
        auto tail_block = visitStatement(ctor_node->statement,entry,nullptr);
        tail_block->Ret();

        list<il::Local*> locals;
        for(auto variable : ctor_node->constructor_symbol->getMemoryLayout()){
            auto name = document->getTokenRef(variable->getName());
            auto type = document->getTokenRef(variable->getPrototype()->getFullName());
            auto local = new Local(document,name,type,locals.size());
            locals.push_back(local);
        }
        return new Ctor(document,parameter,locals,blocks);
    }


    il::BasicBlock *ILGen::visitStatement(ast::Statement *statement_node, il::BasicBlock *current, il::BasicBlock *next) {
        FOR_EACH(iter,statement_node){
            switch(iter->stmt_flag){
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
                default:
                    PANIC;
                    break;
            }
        }
        return current;
    }

    void ILGen::visitLet(ast::Let *let_node, il::BasicBlock *current, il::BasicBlock *next) {
        FOR_EACH(iter,let_node->variable){
            current->Push(DataType::u16,(data::u16)iter->variable_symbol->getLayoutIndex());
            visitExpression(iter->initial,current);
            current->Stloc(mapILType(iter->initial->type->getPrototype()));
        }
    }

    il::BasicBlock *ILGen::visitSelect(ast::Select *select_node, il::BasicBlock *current, il::BasicBlock *next) {
        //todo
        return current;
    }

    il::BasicBlock *ILGen::visitLoop(ast::Loop *loop_node, il::BasicBlock *current, il::BasicBlock *next) {
        auto cond_block = new il::BasicBlock(document),
             loop_block = new il::BasicBlock(document),
             after_block = new il::BasicBlock(document);

        blocks.push_back(cond_block);
        blocks.push_back(loop_block);
        blocks.push_back(after_block);

        current->Br(cond_block);

        visitExpression(loop_node->condition,cond_block);
        cond_block->Jif(loop_block);
        cond_block->Br(after_block);

        loop_block = visitStatement(loop_node->statement,loop_block,after_block);
        loop_block->Br(cond_block);

        return after_block;
    }

    il::BasicBlock *ILGen::visitIf(ast::If *if_node, il::BasicBlock *current, il::BasicBlock *next) {
        auto after_block = new il::BasicBlock(document);
        blocks.push_back(after_block);
        visitCase(if_node->case_,current,after_block);
        return after_block;
    }

    void ILGen::visitCase(ast::Case *case_node, il::BasicBlock *current, il::BasicBlock *next) {
        map<ast::Case*,il::BasicBlock*> case_blocks;
        FOR_EACH(iter,case_node){
            if(iter->condition){
                auto bl = new il::BasicBlock(document);
                blocks.push_back(bl);
                case_blocks.insert({iter,bl});
            }
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

    il::BasicBlock *ILGen::visitFor(ast::For *for_node, il::BasicBlock *current, il::BasicBlock *next) {

        switch(for_node->iterator->expression_kind){
            case ast::Expression::SFld:{
                auto sfld = (ast::SFld*)for_node->iterator;
                auto il_type = mapILType(sfld->type->getPrototype());
                visitExpression(for_node->begin,current);
                current->Stsfld(il_type, document->getTokenRef(sfld->variable->getFullName()));
                break;
            }
            case ast::Expression::Fld:{
                auto fld = (ast::Fld*)for_node->iterator;
                auto il_type = mapILType(fld->type->getPrototype());
                visitExpression(fld->ref,current);
                visitExpression(for_node->begin,current);
                current->Stfld(il_type, document->getTokenRef(fld->variable->getFullName()));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = (ast::Arg*)for_node->iterator;
                auto il_type = mapILType(arg->type->getPrototype());
                current->Push(DataType::u16,(data::u16)arg->variable->getLayoutIndex());
                visitExpression(for_node->begin,current);
                current->Starg(il_type);
                break;
            }
            case ast::Expression::Local:{
                auto local = (ast::Local*)for_node->iterator;
                auto il_type = mapILType(local->type->getPrototype());
                current->Push(DataType::u16,(data::u16)local->variable->getLayoutIndex());
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

        current->Push(DataType::u16,(data::u16)for_node->begin_variable->getLayoutIndex());
        visitExpression(for_node->begin,current);
        current->Stloc(iter_il_type);

        current->Push(DataType::u16,(data::u16)for_node->end_variable->getLayoutIndex());
        visitExpression(for_node->end,current);
        current->Stloc(iter_il_type);

        if(for_node->step){
            current->Push(DataType::u16,(data::u16)for_node->step_variable->getLayoutIndex());
            visitExpression(for_node->step,current);
            current->Stloc(iter_il_type);
        }

        auto stmt_block = new il::BasicBlock(document);
        auto cond_block = new il::BasicBlock(document);
        auto after_block = new il::BasicBlock(document);

        blocks.push_back(stmt_block);
        blocks.push_back(cond_block);
        blocks.push_back(after_block);

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
                cond_block->Stsfld(il_type, document->getTokenRef(sfld->variable->getFullName()));
                break;
            }
            case ast::Expression::Fld:{
                auto fld = (ast::Fld*)for_node->iterator;
                auto il_type = mapILType(fld->type->getPrototype());
                visitExpression(fld->ref,cond_block);
                visitExpression(for_node->iterator,cond_block);
                visitExpression(for_node->step,cond_block);
                cond_block->Add(il_type);
                cond_block->Stfld(il_type, document->getTokenRef(fld->variable->getFullName()));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = (ast::Arg*)for_node->iterator;
                auto il_type = mapILType(arg->type->getPrototype());
                cond_block->Push(DataType::u16,(data::u16)arg->variable->getLayoutIndex());
                visitExpression(for_node->iterator,cond_block);
                visitExpression(for_node->step,cond_block);
                cond_block->Add(il_type);
                cond_block->Starg(il_type);
                break;
            }
            case ast::Expression::Local:{
                auto local = (ast::Local*)for_node->iterator;
                auto il_type = mapILType(local->type->getPrototype());
                cond_block->Push(DataType::u16,(data::u16)local->variable->getLayoutIndex());
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
        cond_block->Push(DataType::u16,(data::u16)for_node->begin_variable->getLayoutIndex())
                   .Ldloc(iter_il_type)
                   .Push(DataType::u16,(data::u16)for_node->end_variable->getLayoutIndex())
                   .Ldloc(iter_il_type)
                   .LT(iter_il_type);
        visitExpression(for_node->iterator,cond_block);
        cond_block->Push(DataType::u16,(data::u16)for_node->end_variable->getLayoutIndex())
                   .Ldloc(iter_il_type)
                   .GT(iter_il_type);

        cond_block->Push(DataType::u16,(data::u16)for_node->begin_variable->getLayoutIndex())
                    .Ldloc(iter_il_type)
                    .Push(DataType::u16,(data::u16)for_node->end_variable->getLayoutIndex())
                    .Ldloc(iter_il_type)
                    .GT(iter_il_type);
        visitExpression(for_node->iterator,cond_block);
        cond_block->Or();
        cond_block->Push(DataType::u16,(data::u16)for_node->begin_variable->getLayoutIndex())
                .Ldloc(iter_il_type)
                .Push(DataType::u16,(data::u16)for_node->end_variable->getLayoutIndex())
                .Ldloc(iter_il_type)
                .EQ(iter_il_type)
                .Or();

        cond_block->Jif(after_block);
        cond_block->Br(stmt_block);

        return after_block;
    }

    il::BasicBlock *ILGen::visitExprStmt(ast::ExprStmt *expr_stmt_node, il::BasicBlock *current, il::BasicBlock *next) {
        visitExpression(expr_stmt_node->expr,current);
        return current;
        // todo: clean operand stack
    }

    il::BasicBlock *ILGen::visitExit(ast::Exit *exit_node, il::BasicBlock *current) {
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

    il::BasicBlock *ILGen::visitContinue(ast::Continue *continue_node, il::BasicBlock *current, il::BasicBlock *next) {
        current->Br(next);
        return current;
    }

    void ILGen::visitReturn(ast::Return *return_node, il::BasicBlock *current, il::BasicBlock *next) {
        visitExpression(return_node->expr,current);
        current->Ret();
    }



    void ILGen::visitExpression(ast::Expression *expression_node, il::BasicBlock *current) {
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
            case ast::Expression::EnumMember:
                visitEnumMember((ast::EnumMember*)expression_node,current);
                break;
        }
    }

    void ILGen::visitEnumMember(ast::EnumMember* enum_member_node, il::BasicBlock *current){
        current->Push(DataType::i32,enum_member_node->member->getIndex());
    }

    void ILGen::visitUnary(ast::Unary *unary_node, il::BasicBlock *current) {
        visitExpression(unary_node->terminal,current);
        auto il_type = mapILType(unary_node->type->getPrototype());
        using Op = ast::Unary::Op;
        switch(unary_node->op){
            case Op::ADD:   /* do nothing */ break;
            case Op::MINUS: current->Neg(il_type); break;
            case Op::Empty: PANIC;
        }
    }

    void ILGen::visitBinary(ast::Binary *binary_node, il::BasicBlock *current) {
        visitExpression(binary_node->lhs,current);
        visitExpression(binary_node->rhs,current);
        auto il_type = mapILType(binary_node->lhs->type->getPrototype());
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

    void ILGen::visitAssign(ast::Assign *assign_node, il::BasicBlock *current) {
        auto expr = assign_node->lhs;
        switch (expr->expression_kind) {
            case ast::Expression::Fld:{
                auto fld = (ast::Fld*)expr;
                visitFld(fld,current);
                visitExpression(assign_node->rhs,current);
                current->Stfld(mapILType(fld->variable->getPrototype()),
                               document->getTokenRef(fld->variable->getFullName()));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = (ast::SFld*)expr;
                visitSFld(sfld,current);
                visitExpression(assign_node->rhs,current);
                current->Stsfld(mapILType(sfld->variable->getPrototype()),
                                document->getTokenRef(sfld->variable->getFullName()));
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
                current->Push(DataType::u16,(data::u16)arg->variable->getLayoutIndex());
                visitExpression(assign_node->rhs,current);
                auto data_type = mapILType(arg->variable->getPrototype());
                if(arg->is_ref){
                    current->Stargr(data_type);
                }
                else{
                    current->Starg(data_type);
                }
                break;
            }
            case ast::Expression::Local:{
                auto local = (ast::Local*)expr;
                current->Push(DataType::u16,(data::u16)local->variable->getLayoutIndex());
                visitExpression(assign_node->rhs,current);
                current->Stloc(mapILType(local->variable->getPrototype()));
                break;
            }
            default: PANIC;
        }
    }

    void ILGen::visitCast(ast::Cast *cast_node, il::BasicBlock *current) {
        visitExpression(cast_node->expr,current);
        current->Conv(mapILType(cast_node->expr->type->getPrototype()),mapILType(cast_node->target));
    }

    void ILGen::visitParentheses(ast::Parentheses *parentheses_node, il::BasicBlock *current) {
        visitExpression(parentheses_node->expr,current);
    }

    void ILGen::visitArrayElement(ast::ArrayElement *element_node, il::BasicBlock *current) {
        visitExpression(element_node->array,current);
        current->Ldelem(mapILType(element_node->type->getPrototype()));
    }

    void ILGen::visitDelegate(ast::Delegate *delegate_node, il::BasicBlock *current) {
        //todo
    }


    void ILGen::visitArgument(ast::Argument *argument_node, il::BasicBlock *current) {
        auto expr = argument_node->expr;
        if(argument_node->byval){
            // pass byval
            visitExpression(expr,current);
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
                    current->Ldflda(document->getTokenRef(fld->variable->getFullName()));
                    break;
                }
                case ast::Expression::SFld:{
                    auto sfld = (ast::SFld*)expr;
                    visitSFld(sfld,current);
                    current->Ldsflda(document->getTokenRef(sfld->variable->getFullName()));
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
                    current->Push(DataType::u16,(data::u16)arg->variable->getLayoutIndex());
                    if(arg->is_ref){
                        current->Ldarg(DataType::ref);
                    }
                    else{
                        current->Ldarga();
                    }
                    break;
                }
                case ast::Expression::Local:{
                    auto local = (ast::Local*)expr;
                    current->Push(DataType::u16,(data::u16)local->variable->getLayoutIndex())
                            .Ldloca();
                    break;
                }
                default: PANIC;
            }
        }
    }


    void ILGen::visitNew(ast::New *new_node, il::BasicBlock *current) {
        loadCalleeArguments(new_node,current);
        current->Newobj(document->getTokenRef(new_node->target->getFullName()));
    }

    void ILGen::visitFtnCall(ast::FtnCall *ftn_node, il::BasicBlock *current) {
        visitExpression(ftn_node->ref,current);
        current->Ldftn(document->getTokenRef(ftn_node->function->getFullName()));
        loadCalleeArguments(ftn_node,current);
        current->Call();
    }

    void ILGen::visitSFtnCall(ast::SFtnCall *sftn_node, il::BasicBlock *current) {
        current->Ldsftn(document->getTokenRef(sftn_node->function->getFullName()));
        loadCalleeArguments(sftn_node,current);
        current->Callstatic();
    }

    void ILGen::loadCalleeArguments(ast::Call *call,il::BasicBlock *current){
        // load regular argument by declaration order
        FOR_EACH(iter,call->argument){
            visitArgument(iter,current);
        }
    }

    void ILGen::visitVFtnCall(ast::VFtnCall *vftn_node, il::BasicBlock *current) {
        visitExpression(vftn_node->ref,current);
        current->Ldvftn(document->getTokenRef(vftn_node->function->getFullName()));
        loadCalleeArguments(vftn_node,current);
        current->Callvirt();
    }

    void ILGen::visitExtCall(ast::ExtCall *ext_node, il::BasicBlock *current) {
        loadCalleeArguments(ext_node,current);
        current->Invoke(document->getTokenRef(ext_node->function->getFullName()));
    }

    void ILGen::visitSFld(ast::SFld *sfld_node, il::BasicBlock *current) {
        auto token = document->getTokenRef(sfld_node->variable->getFullName());
        current->Ldsfld(mapILType(sfld_node->variable->getPrototype()),token);
    }

    void ILGen::visitFld(ast::Fld *fld_node, il::BasicBlock *current) {
        auto token = document->getTokenRef(fld_node->variable->getFullName());
        visitExpression(fld_node->ref,current);
        current->Ldfld(mapILType(fld_node->variable->getPrototype()),token);
    }

    void ILGen::visitLocal(ast::Local *local_node, il::BasicBlock *current) {
        auto index = local_node->variable->getLayoutIndex();
        current->Push(DataType::u16,(data::u16)index);
        current->Ldloc(mapILType(local_node->variable->getPrototype()));
    }

    void ILGen::visitArg(ast::Arg *arg_node, il::BasicBlock *current) {
        auto index = arg_node->variable->getLayoutIndex();
        current->Push(DataType::u16,(data::u16)index);
        auto data_type = mapILType(arg_node->variable->getPrototype());
        if(arg_node->is_ref){
            current->Ldargr(data_type);
        }
        else{
            current->Ldarg(data_type);
        }
    }

    void ILGen::visitDigit(ast::Digit *digit_node, il::BasicBlock *current) {
        current->Push(DataType::i32,digit_node->value);
    }

    void ILGen::visitDecimal(ast::Decimal *decimal_node, il::BasicBlock *current) {
        current->Push(DataType::f64,decimal_node->value);
    }

    void ILGen::visitString(ast::String *string_node, il::BasicBlock *current) {
        current->Ldc(document->getTokenRef(string_node->value));
        //todo: call ctor for String
    }

    void ILGen::visitChar(ast::Char *char_node, il::BasicBlock *current) {
        current->Push(DataType::character,char_node->value);
    }

    void ILGen::visitBoolean(ast::Boolean *boolean_node, il::BasicBlock *current) {
        current->Push(DataType::boolean,boolean_node->value);
    }

}


//
// Created by yorkin on 11/2/21.
//

#include "codeGen.h"
#include <variant>
#include "utils.h"
using namespace std;
namespace evoBasic{
    using namespace vm;
    using namespace ir;
    using namespace type;
    
    ConstBase *convertNumberToConst(Data kind,long long number){
        switch (kind.getValue()) {
            case Data::void_: ASSERT(true,"error");
            case Data::boolean: return new Const<data::boolean>(number);
            case Data::i8:      return new Const<data::i8>(number);
            case Data::i16:     return new Const<data::i16>(number);
            case Data::i32:     return new Const<data::i32>(number);
            case Data::i64:     return new Const<data::i64>(number);
            case Data::f32:     return new Const<data::f32>(number);
            case Data::f64:     return new Const<data::f64>(number);
            case Data::u8:      return new Const<data::u8>(number);
            case Data::u16:     return new Const<data::u16>(number);
            case Data::u32:     return new Const<data::u32>(number);
            case Data::u64:     return new Const<data::u64>(number);
        }
    }
    
    shared_ptr<IR> IRGen::gen(AST *ast,std::shared_ptr<Context> context) {
        auto ir = std::make_shared<IR>();
        IRGenArgs args;
        args.ir = ir;
        args.domain = context->getGlobal();
        args.context = context;
        visitGlobal(ast,args);
        return ir;
    }

    std::any IRGen::visitGlobal(ast::Global *global_node, IRGenArgs args) {
        for(auto& m:global_node->member_list)
            visitMember(m,args);
        return nullptr;
    }

    std::any IRGen::visitModule(ast::Module *mod_node, IRGenArgs args) {
        args.domain = args.domain->find(getID(mod_node->name))->as_shared<Domain>();
        for(auto& m:mod_node->member_list)
            visitMember(m,args);
        return nullptr;
    }

    std::any IRGen::visitClass(ast::Class *cls_node, IRGenArgs args) {
        list<Pair*> members;
        auto ty = args.domain->find(getID(cls_node->name))->as_shared<type::Record>();
        for(auto &p : ty->getFields()){
            auto field_type_ir = new Mark(p->getPrototype()->mangling(),false,false);
            members.push_back(new Pair(p->getName(),field_type_ir));
        }
        auto record_ir = new ir::Record(members);
        args.ir->addMeta(new Pair(ty->mangling(), record_ir));
        return nullptr;
    }


    std::any IRGen::visitType(ast::Type *ty_node, IRGenArgs args) {
        list<Pair*> members;
        auto ty = args.domain->find(getID(ty_node->name))->as_shared<type::Record>();
        for(auto &p : ty->getFields()){
            auto field_type_ir = new Mark(p->getPrototype()->mangling(),false,false);
            members.push_back(new Pair(p->getName(),field_type_ir));
        }
        auto record_ir = new ir::Record(members);
        args.ir->addMeta(new Pair(ty->mangling(), record_ir));
        return nullptr;
    }

    std::any IRGen::visitEnum(ast::Enum *em_node, IRGenArgs args) {
        auto name = getID(em_node->name);
        auto em = args.domain->find(name)->as_shared<Enumeration>();
        list<pair<string,int>> members;
        for(auto p = em->begin();p!=em->end();p++){
            auto key = p->getName();
            auto value = p->as_shared<EnumMember>()->getIndex();
            members.emplace_back(key,value);
        }
        auto enum_ir = new Enum(members);
        args.ir->addMeta(new Pair(em->mangling(), enum_ir));
        return nullptr;
    }

    std::any IRGen::visitDim(ast::Dim *dim_node, IRGenArgs args) {
        if(args.domain->getKind() == DeclarationEnum::Module){
            for(auto &var:dim_node->variable_list)
                visitVariable(var,args);
        }
        return nullptr;
    }

    std::any IRGen::visitVariable(ast::Variable *var_node, IRGenArgs args) {
        auto name = getID(var_node->name);
        auto field = args.domain->find(name)->as_shared<Variable>();
        auto type_ref = new Mark(field->getPrototype()->mangling(),false,false);
        args.ir->addMeta(new Pair(field->mangling(), type_ref));
        return nullptr;
    }

    std::any IRGen::visitFunction(ast::Function *func_node, IRGenArgs args) {
        auto func = args.domain->find(getID(func_node->name))->as_shared<type::Function>();
        args.function = func;

        list<Pair*> params;
        Mark *ret_mark = nullptr;
        for(auto &param : func->getArgsSignature()){
            auto isArray = false,isRef = !param->isByval();
            string name;
            if(param->getPrototype()->getKind() == DeclarationEnum::Array){
                isArray = true;
                name = param->getPrototype()->as_shared<Array>()->getElementPrototype()->mangling();
            }
            else{
                isArray = false;
                name = param->getPrototype()->mangling();
            }
            auto mark = new Mark(name,isRef,isArray);
            params.push_back(new Pair(param->getName(),mark));
        }

        if(func->getRetSignature()){
            bool isArray;
            string name;
            if(func->getRetSignature()->getKind() == DeclarationEnum::Array){
                isArray = true;
                name = func->getRetSignature()->as_shared<Array>()->getElementPrototype()->mangling();
            }
            else{
                isArray = false;
                name = func->getRetSignature()->mangling();
            }
            ret_mark = new Mark(name,false,isArray);
        }


        args.domain = func;
        //create function segment
        auto function_block = new Block(args.domain->mangling());
        auto func_ir = new ir::Function(params, ret_mark, function_block);
        args.ir->addMeta(new Pair(func->mangling(), func_ir));
        args.ir->addBlock(function_block);

        args.previous_block = function_block;

        for(int i = func->getArgsSignature().size()-1;i>=0;i--){
            auto argument = func->getArgsSignature()[i];
            auto variable = func->find(argument->getName())->as_shared<Variable>();
            args.previous_block->PushFrameBase()
                    .Push(Data::ptr,new Const<data::ptr>(variable->getOffset()))
                    .Add(Data::ptr);
            if(argument->isByval()){
                switch (argument->getPrototype()->getKind()) {
                    case DeclarationEnum::Array:
                    case DeclarationEnum::Type:
                        args.previous_block->StmR(argument->getPrototype()->as_shared<Domain>()->getByteLength());
                        break;
                    case DeclarationEnum::Class:
                        args.previous_block->StoreR(Data::ptr);
                        break;
                    case DeclarationEnum::Primitive:
                        args.previous_block->StoreR(argument->getPrototype()->as_shared<primitive::Primitive>()->getDataKind());
                        break;
                }
            }
            else{
                args.previous_block->StoreR(Data::ptr);
            }
        }

        auto after_block = visitStatementList(func_node->statement_list,args);
        after_block->Ret();
        return nullptr;
    }

    Block *IRGen::visitStatementList(std::list<ast::stmt::Statement*> &stmt_list, IRGenArgs args) {
        for(auto& s:stmt_list){
            auto *after_block = any_cast<Block*>(visitStatement(s,args));
            if(after_block)args.previous_block = after_block;
        }
        return args.previous_block;
    }

    std::any IRGen::visitLet(ast::stmt::Let *let_node, IRGenArgs args) {
        for(auto &var : let_node->variable_list){
            auto tmp = args.domain->find(getID(var->name));
            auto field = tmp->as_shared<Variable>();
            auto offset = field->getOffset();
            args.previous_block->PushFrameBase()
                                .Push(Data::ptr,new Const<data::u32>(field->getOffset()))
                                .Add(Data::ptr);

            if(var->initial){
                auto operand_type = any_cast<OperandTopType>(visitExpression(var->initial, args));
                auto data = tryLoadOperandTop(operand_type,args.previous_block);
                switch(operand_type.index()){
                    case 1: /* DataType */
                        args.previous_block->Store(data);
                        break;
                    case 2: /* MemType */
                        args.previous_block->Stm(get<MemType>(operand_type).size);
                        break;
                    case 4: /* ClassType */
                        args.previous_block->Store(Data::ptr);
                        break;
                    default:
                        ASSERT(true,"invalid");
                }
            }
            else switch(field->getPrototype()->getKind()){
                case DeclarationEnum::Primitive:{
                    auto data = field->getPrototype()->as_shared<primitive::Primitive>()->getDataKind();
                    args.previous_block->Push(data,convertNumberToConst(data,0));
                    args.previous_block->Store(data);
                    break;
                }
                case DeclarationEnum::Type:{
                    break;
                }
                case DeclarationEnum::Array:{
                    break;
                }
                case DeclarationEnum::Class:{
                    args.previous_block->Push(Data::ptr,new Const<data::ptr>(0));
                    args.previous_block->Store(Data::ptr);
                    break;
                }
            }
        }
        return (Block*)nullptr;
    }

    std::any IRGen::visitIf(ast::stmt::If *ifstmt_node, IRGenArgs args) {
        auto after_if_block = new Block(args.domain->mangling() + "_if_after");
        args.next_block = after_if_block;
        for(auto &ca:ifstmt_node->case_list){
            visitCase(ca,args);
        }
        args.previous_block->Jmp(after_if_block);
        args.ir->addBlock(after_if_block);
        return after_if_block;
    }

    std::any IRGen::visitCase(ast::Case *ca_node, IRGenArgs args) {
        if(ca_node->condition) {
            visitExpression(ca_node->condition, args);
            auto case_block = new Block(args.function->mangling() + "_case");
            args.ir->addBlock(case_block);
            args.previous_block->Jif(case_block);
            args.previous_block = case_block;
            auto after_stmts_block = visitStatementList(ca_node->statement_list, args);
            after_stmts_block->Jmp(args.next_block);
        }
        else{
            visitStatementList(ca_node->statement_list, args);
        }
        return (Segment*)nullptr;
    }

    std::any IRGen::visitLoop(ast::stmt::Loop *loop_node, IRGenArgs args) {
        auto condition_block = new Block(args.function->mangling() + "_condition");
        auto loop_block = new Block(args.function->mangling() + "_loop");
        auto after_loop_block = new Block(args.function->mangling() + "_loop_after");

        //jump from previous segment to condition segment
        args.previous_block->Jmp(condition_block);

        //loop condition ir
        args.ir->addBlock(condition_block);
        args.previous_block = condition_block;
        visitExpression(loop_node->condition,args);
        condition_block->Jif(loop_block);
        condition_block->Jmp(after_loop_block);

        //loop body ir
        args.ir->addBlock(loop_block);
        args.previous_block = loop_block;
        args.next_block = after_loop_block;
        visitStatementList(loop_node->statement_list,args);
        loop_block->Jmp(condition_block);

        args.ir->addBlock(after_loop_block);
        return after_loop_block;
    }

    std::any IRGen::visitSelect(ast::stmt::Select *select_node, IRGenArgs args) {
        auto after_select_block = new Block(args.domain->mangling() + "_select_after");
        args.next_block = after_select_block;
        auto operand_type = any_cast<OperandTopType>(visitExpression(select_node->condition,args));
        auto data = tryLoadOperandTop(operand_type,args.previous_block);
        //duplicate condition value for each Case
        int need_value_count = 0;
        for(auto &ca:select_node->case_list){
            if(ca->condition)need_value_count++;
        }
        for(int i=0;i<need_value_count-1;i++){
            args.previous_block->Dup(data);
        }
        for(auto &ca:select_node->case_list){
            visitCaseInSelectStmt(ca,args);
        }
        args.previous_block->Jmp(after_select_block);
        args.ir->addBlock(after_select_block);
        return after_select_block;
    }

    std::any IRGen::visitCaseInSelectStmt(ast::Case *ca_node, IRGenArgs args) {
        if(ca_node->condition) {
            auto operand_type = any_cast<OperandTopType>(visitExpression(ca_node->condition, args));
            auto data = tryLoadOperandTop(operand_type,args.previous_block);
            auto case_block = new Block(args.function->mangling() + "_case");
            args.ir->addBlock(case_block);
            args.previous_block->EQ(data);
            args.previous_block->Jif(case_block);
            args.previous_block = case_block;
            auto after_stmts_block = visitStatementList(ca_node->statement_list, args);
            after_stmts_block->Jmp(args.next_block);
        }
        else{
            visitStatementList(ca_node->statement_list, args);
        }
        return (Segment*)nullptr;
    }

    std::any IRGen::visitFor(ast::stmt::For *forstmt_node, IRGenArgs args) {
        auto condition_block = new Block(args.function->mangling() + "_condition");
        auto loop_block = new Block(args.function->mangling() + "_loop");
        auto after_loop_block = new Block(args.function->mangling() + "_loop_after");


        //load iterator address and begin expression value
        args.reserve_address = true;
        visitExpression(forstmt_node->iterator,args);
        args.reserve_address = false;
        auto data = any_cast<Data>(visitExpression(forstmt_node->begin,args));
        //store value in address
        args.previous_block->Store(data);
        //jump from previous previous_block to looping segment
        args.previous_block->Jmp(loop_block);
        //generate ir in loop previous_block


        args.ir->addBlock(condition_block);
        //generate condition previous_block ir
        // increase iterator
        args.previous_block = condition_block;
        visitExpression(forstmt_node->iterator,args);
        if(forstmt_node->step)
            visitExpression(forstmt_node->step,args);
        else
            condition_block->Push(data, convertNumberToConst(data, 1));
        args.previous_block->Add(data);
        //duplicate increased value and store in iterator address
        args.previous_block->Dup(data);
        args.reserve_address = true;
        visitExpression(forstmt_node->iterator,args);
        args.reserve_address = false;
        args.previous_block->Store(data);
        //load end expression value
        visitExpression(forstmt_node->end,args);
        //compare iterator and end,if less than or equal the end value,jump to loop previous_block
        args.previous_block->LE(data);
        args.previous_block->Jif(loop_block);
        //otherwise jump to after loop segment
        args.previous_block->Jmp(after_loop_block);



        args.ir->addBlock(loop_block);
        args.previous_block = loop_block;
        args.next_block = after_loop_block;
        args.continue_block = condition_block;
        auto after_stmts_segment = visitStatementList(forstmt_node->statement_list,args);
        //jump from loop end to condition previous_block
        after_stmts_segment->Jmp(condition_block);

        args.ir->addBlock(after_loop_block);
        return after_loop_block;
    }

    std::any IRGen::visitContinue(ast::stmt::Continue *cont_node, IRGenArgs args) {
        args.previous_block->Jmp(args.continue_block);
        return (Block*)nullptr;
    }

    std::any IRGen::visitReturn(ast::stmt::Return *ret_node, IRGenArgs args) {
        args.need_return_value = true;
        auto operand_type = any_cast<OperandTopType>(visitExpression(ret_node->expr,args));
        auto data = tryLoadOperandTop(operand_type,args.previous_block);
        args.previous_block->Ret();
        return (Block*)nullptr;
    }

    std::any IRGen::visitExit(ast::stmt::Exit *exit_node, IRGenArgs args) {
        switch (exit_node->exit_flag) {
            case ast::stmt::Exit::For:
            case ast::stmt::Exit::While:
                args.previous_block->Jmp(args.next_block);
                break;
            case ast::stmt::Exit::Sub:
                args.previous_block->Ret();
                break;
        }
        return (Block*)nullptr;
    }

    vm::Data IRGen::tryLoadOperandTop(OperandTopType type,Block *block){
        switch(type.index()){
            case 0: ASSERT(true,"invalid"); break;
            case 1: /* DataType */
                return get<DataType>(type).data;
            case 2: /* MemType */
                return vm::Data::void_;
            case 3: /* AddressType */{
                auto element = get<AddressType>(type).element;
                switch(element->index()){
                    case 0: ASSERT(true,"invalid");
                    case 1: /* DataType */
                        block->Load(get<DataType>(*element).data);
                        return get<DataType>(*element).data;
                    case 2: /* MemType */
                        block->Ldm(get<MemType>(*element).size);
                        return vm::Data::void_;
                    case 3: /* AddressType, for ByRef Argument */
                        block->Load(Data::ptr);
                        return tryLoadOperandTop(*element,block);
                    case 4: /* ClassType */
                        block->Load(Data::ptr);
                        return Data::ptr;
                }
            }
            case 4: /* ClassType */
                return vm::Data::void_;
                break;
        }
    }

    OperandTopType IRGen::visitAssign(ast::expr::Binary *node, IRGenArgs args) {
        bool need_return_value = args.need_return_value;
        args.need_return_value = true;

        auto lhs_data_type = any_cast<OperandTopType>(visitExpression(node->lhs, args));
        auto rhs_data_type = any_cast<OperandTopType>(visitExpression(node->rhs, args));

        switch(rhs_data_type.index()){
            case 0: ASSERT(true,"invalid"); break;
            case 1: /* DataType */
                args.previous_block->Store(get<DataType>(rhs_data_type).data);
                break;
            case 2: /* MemType */
                args.previous_block->Stm(get<MemType>(rhs_data_type).size);
                break;
            case 3: /* AddressType */{
                auto element = get<AddressType>(rhs_data_type).element;
                switch(element->index()){
                    case 0: ASSERT(true,"invalid");
                    case 1: /* DataType */
                        args.previous_block->Load(get<DataType>(*element).data);
                        args.previous_block->Store(get<DataType>(*element).data);
                        break;
                    case 2: /* MemType */
                        args.previous_block->Ldm(get<MemType>(*element).size);
                        args.previous_block->Stm(get<MemType>(*element).size);
                        break;
                    case 3: /* AddressType */
                        ASSERT(true,"invalid");
                    case 4: /* ClassType */
                        args.previous_block->Load(Data::ptr);
                        args.previous_block->Store(Data::ptr);
                        break;
                }
                break;
            }
            case 4: /* ClassType */
                args.previous_block->Store(Data::ptr);
                break;
        }

        if(need_return_value)
            return any_cast<OperandTopType>(visitExpression(node->lhs, args));
        else
            return OperandTopType(EmptyType{});
    }

    std::any IRGen::visitBinary(ast::expr::Binary *logic_node, IRGenArgs args) {
        using Op = ast::expr::Binary::Enum;
        set<Op> logic_op = {Op::And,Op::Or,Op::Xor,Op::Not,Op::Not};
        set<Op> calculate_op = {Op::EQ,Op::NE,Op::LE,Op::GE,Op::LT,Op::GT,Op::ADD,Op::MINUS,Op::MUL,Op::DIV,Op::FDIV};
        auto boolean = OperandTopType(DataType{Data::boolean});
        if(logic_op.contains(logic_node->op)){
            switch (logic_node->op) {
                case Op::And:
                    args.previous_block->And();
                    return boolean;
                case Op::Or:
                    args.previous_block->Or();
                    return boolean;
                case Op::Xor:
                    args.previous_block->Xor();
                    return boolean;
                case Op::Not:
                    args.previous_block->Not();
                    return boolean;
            }
        }
        else if(calculate_op.contains(logic_node->op)){
            auto lhs_data_type = any_cast<OperandTopType>(visitExpression(logic_node->lhs, args));
            auto lhs_data = tryLoadOperandTop(lhs_data_type,args.previous_block);

            auto rhs_data_type = any_cast<OperandTopType>(visitExpression(logic_node->rhs,args));
            tryLoadOperandTop(rhs_data_type,args.previous_block);

            switch (logic_node->op) {
                case Op::EQ:
                    args.previous_block->EQ(lhs_data);
                    return boolean;
                case Op::NE:
                    args.previous_block->NE(lhs_data);
                    return boolean;
                case Op::GE:
                    args.previous_block->GE(lhs_data);
                    return boolean;
                case Op::LE:
                    args.previous_block->LE(lhs_data);
                    return boolean;
                case Op::GT:
                    args.previous_block->GT(lhs_data);
                    return boolean;
                case Op::LT:
                    args.previous_block->LT(lhs_data);
                    return boolean;
                case Op::ADD:
                    args.previous_block->Add(lhs_data);
                    return OperandTopType(DataType{lhs_data});
                case Op::MINUS:
                    args.previous_block->Sub(lhs_data);
                    return OperandTopType(DataType{lhs_data});
                case Op::MUL:
                    args.previous_block->Mul(lhs_data);
                    return OperandTopType(DataType{lhs_data});
                case Op::DIV:
                    args.previous_block->Div(lhs_data);
                    return OperandTopType(DataType{lhs_data});
                case Op::FDIV:
                    args.previous_block->FDiv(lhs_data);
                    return OperandTopType(DataType{lhs_data});//TODO FIDV support
            }
        }
        else if(logic_node->op == Op::ASSIGN){
            args.need_return_value = false;
            return visitAssign(logic_node,args);
        }
        else if(logic_node->op == Op::Dot){
            args.dot_expression_context = args.domain;
            auto prototype = visitDot(logic_node,args);
            return mapSymbolToOperandTopType(prototype);
        }
    }

    std::any IRGen::visitID(ast::expr::ID *id_node, IRGenArgs args) {
        auto name = getID(id_node);
        if(args.need_lookup) {
            return args.dot_expression_context->as_shared<Domain>()->lookUp(name);
        }
        else{
            return args.dot_expression_context->as_shared<Domain>()->find(name);
        }
    }


    std::any IRGen::visitCallee(ast::expr::Callee *callee_node, IRGenArgs args) {
        shared_ptr<type::Function> function = any_cast<shared_ptr<Symbol>>(visitID(callee_node->name,args))->as_shared<type::Function>();
        args.function = function;
        for(int i=0;i<function->getArgsSignature().size();i++){
            args.current_args_index = i;
            visitArg(callee_node->arg_list[i],args);
        }
        args.previous_block->Invoke(function->mangling());
        return function->getRetSignature();
    }

//    void loadFromAddress(OperandTopType operand_variant, Block *block){
//        switch(operand_variant.index()){
//            case 0: ASSERT(true,"invalid");break;
//            case 1: block->Ldm(get<MemType>(operand_variant).size); break;
//            case 2: block->Load(get<PtrType>(operand_variant).data); break;
//        }
//    }

    void IRGen::pushVariableAddress(const std::shared_ptr<Variable> &variable, Block *block, bool need_push_base){
        if(need_push_base){
            if(variable->isGlobal())
                block->PushGlobalBase();
            else
                block->PushFrameBase();
        }
        block->Push(Data::ptr,new Const<data::ptr>(variable->getOffset()));
        block->Add(Data::ptr);
        //push variable address to operand

        auto argument = variable->as_shared<Argument>();
        if(argument && !argument->isByval()){
            block->Load(Data::ptr);
            //push referencing address to operand
        }
    }

    shared_ptr<Prototype> IRGen::visitIndex(ast::expr::Binary *index,IRGenArgs args,bool need_push_base){
        auto symbol = any_cast<shared_ptr<Symbol>>(visitID((ast::expr::ID*)index->lhs,args));
        auto variable = symbol->as_shared<Variable>();
        pushVariableAddress(variable, args.previous_block,need_push_base);
        auto array = variable->getPrototype()->as_shared<Array>();
        NotNull(array.get());

        visitExpression(index->rhs,args);
        args.previous_block->Push(Data::ptr,new Const<data::ptr>(array->getElementPrototype()->getByteLength()));
        args.previous_block->Mul(Data::ptr);
        args.previous_block->Add(Data::ptr);
        return array->getElementPrototype();
    }
    std::any IRGen::visitArg(ast::expr::Callee::Argument *arg_node, IRGenArgs args) {
        /*
          *   Param\Arg          ByVal                  ByRef            Undefined
          *   ByVal      Yes,allow implicit conversion  Error      Yes,allow implicit conversion
          *   ByRef      store value to tmp address,                   Error when arg
          *              allow implicit conversion      Yes             is not lvalue
          */
        NotNull(arg_node);
        auto &param = args.function->getArgsSignature()[args.current_args_index];
        auto arg_type = arg_node->expr->type;
        if(param->isByval()){ // ByVal Parameter
            switch (arg_node->pass_kind) {
                case ast::expr::Callee::Argument::byref:
                    //do nothing
                    break;
                case ast::expr::Callee::Argument::undefined:
                case ast::expr::Callee::Argument::byval:
                    auto operand_variant = any_cast<OperandTopType>(visitExpression(arg_node->expr, args));
                    tryLoadOperandTop(operand_variant,args.previous_block);
                    break;
            }
        }
        else{ // ByRef Parameter
            switch (arg_node->pass_kind) {
                case ast::expr::Callee::Argument::byval: {
                    pushVariableAddress(arg_node->temp_address, args.previous_block, true);
                    auto data_type = any_cast<OperandTopType>(visitExpression(arg_node->expr, args));
                    switch (data_type.index()) {
                        case 0: ASSERT(true, "invalid");
                        case 1: /* DataType */
                            args.previous_block->Store(get<DataType>(data_type).data);
                            break;
                        case 2: /* MemType */
                            args.previous_block->Stm(get<MemType>(data_type).size);
                            break;
                        case 4: /* ClassType */
                            args.previous_block->Store(Data::ptr);
                            break;
                        case 3: /* AddressType */{
                            auto element = get<AddressType>(data_type).element;
                            switch (element->index()) {
                                case 0: ASSERT(true, "invalid");
                                case 1: /* DataType */
                                    args.previous_block->Load(get<DataType>(*element).data);
                                    args.previous_block->Store(get<DataType>(*element).data);
                                    break;
                                case 2: /* MemType */
                                    args.previous_block->Ldm(get<MemType>(*element).size);
                                    args.previous_block->Stm(get<MemType>(*element).size);
                                    break;
                                case 3: /* AddressType */
                                    ASSERT(true, "invalid");
                                case 4: /* ClassType */
                                    args.previous_block->Load(Data::ptr);
                                    args.previous_block->Store(Data::ptr);
                                    return Data::ptr;
                            }
                        }
                    }
                    pushVariableAddress(arg_node->temp_address, args.previous_block, true);
                    break;
                }
                case ast::expr::Callee::Argument::byref:
                case ast::expr::Callee::Argument::undefined:
                    visitExpression(arg_node->expr,args);
                    break;
            }
        }
        return nullptr;
    }

    OperandTopType IRGen::mapSymbolToOperandTopType(shared_ptr<Symbol> symbol) {
        NotNull(symbol.get());
        switch (symbol->getKind()) {
            case DeclarationEnum::Class:
                return AddressType{new OperandTopType(MemType{symbol->as_shared<Class>()->getByteLength()})};
            case DeclarationEnum::Enum_:
                return DataType{Data::u32};
            case DeclarationEnum::Array:
            case DeclarationEnum::Type:
                return MemType{symbol->as_shared<Domain>()->getByteLength()};
            case DeclarationEnum::Argument:{
                auto argument = symbol->as_shared<Argument>();
                if(argument->isByval()) {
                    auto element = mapSymbolToOperandTopType(argument->getPrototype());
                    return AddressType{new OperandTopType(element)};
                }
                else{
                    auto element = mapSymbolToOperandTopType(argument->getPrototype());
                    return AddressType{new OperandTopType(AddressType{new OperandTopType(element)})};
                }
            }
            case DeclarationEnum::Variable: {
                auto variable = symbol->as_shared<Variable>();
                auto element = mapSymbolToOperandTopType(variable->getPrototype());
                return AddressType{new OperandTopType(element)};
            }
            case DeclarationEnum::Primitive:
                return DataType{symbol->as_shared<primitive::Primitive>()->getDataKind()};
            default:
                ASSERT(true,"error");
        }
    }

    shared_ptr<Prototype> IRGen::visitDot(ast::expr::Expression *dot_node,IRGenArgs args){
        using namespace ast::expr;
        using Op = Binary::Enum;
        auto node = (Binary*)dot_node;
        shared_ptr<Symbol> lhs,rhs;


        args.need_lookup = true;
        args.dot_expression_context = args.domain;
        if(node->lhs->expression_kind == Expression::ID_){
            lhs = any_cast<shared_ptr<Symbol>>(visitID((ID*)node->lhs,args));
            if(auto variable = lhs->as_shared<Variable>()){
                pushVariableAddress(variable, args.previous_block,true);
                lhs = variable->getPrototype();
            }
        }
        else if(node->lhs->expression_kind == Expression::callee_){
            if(node->temp_address){
                //push temp address,store returned value in chained method later.
                pushVariableAddress(node->temp_address,args.previous_block,true);
            }
            lhs = any_cast<shared_ptr<Prototype>>(visitCallee((Callee*)node->lhs,args));
            if(node->temp_address){
                args.previous_block->Stm(lhs->as_shared<Domain>()->getByteLength());
                pushVariableAddress(node->temp_address,args.previous_block,true);
            }
        }
        else if(node->lhs->expression_kind == Expression::binary_){
            auto bin_node = (Binary*)node->lhs;
            if(bin_node->op == Op::Dot)
                lhs = visitDot(node->lhs,args);
            else if(bin_node->op == Op::Index)
                lhs = visitIndex((Binary*)node->lhs,args,true);
        }

        NotNull(lhs.get());

        args.need_lookup = false;
        args.dot_expression_context = lhs;
        if(node->rhs->expression_kind == Expression::ID_){
            rhs = any_cast<shared_ptr<Symbol>>(visitID((ID*)node->rhs,args));
            if(auto variable = rhs->as_shared<Variable>()){
                switch(lhs->getKind()){
                    case DeclarationEnum::Array:
                    case DeclarationEnum::Type:
                        args.previous_block->Push(Data::ptr,new Const<data::u32>(variable->getOffset()));
                        args.previous_block->Add(Data::ptr);
                        break;
                    default:
                        pushVariableAddress(variable, args.previous_block,true);
                        break;
                }
                rhs = variable->getPrototype();
            }
        }
        else if(node->rhs->expression_kind == Expression::callee_){
            args.need_lookup = false;
            if(node->temp_address){
                //push temp address,store returned value in chained method later.
                pushVariableAddress(node->temp_address,args.previous_block,true);
            }
            rhs = any_cast<shared_ptr<Prototype>>(visitCallee((Callee*)node->lhs,args));
            if(node->temp_address){
                args.previous_block->Stm(rhs->as_shared<Domain>()->getByteLength());
                pushVariableAddress(node->temp_address,args.previous_block,true);
            }
        }
        else if(node->rhs->expression_kind == Expression::binary_){
            auto bin_node = (Binary*)node->rhs;
            if(bin_node->op == Op::Index){
                switch(lhs->getKind()){
                    case DeclarationEnum::Array:
                    case DeclarationEnum::Type:
                        rhs = visitIndex(bin_node,args,false);
                        break;
                    default:
                        rhs = visitIndex(bin_node,args,true);
                        break;
                }
            }
        }

        NotNull(rhs.get());
        return rhs->as_shared<Prototype>();
    }


    std::any IRGen::visitExpression(ast::expr::Expression *expr_node, IRGenArgs args) {
        switch (expr_node->expression_kind) {
            case ast::expr::Expression::binary_:
                return visitBinary((ast::expr::Binary*)expr_node,args);
            case ast::expr::Expression::unary_:
                return visitUnary((ast::expr::Unary*)expr_node, args);
            case ast::expr::Expression::digit_:
                return visitDigit((ast::expr::Digit*)expr_node,args);
            case ast::expr::Expression::decimal_:
                return visitDecimal((ast::expr::Decimal *)expr_node,args);
            case ast::expr::Expression::string_:
                return visitString((ast::expr::String*)expr_node,args);
            case ast::expr::Expression::char_:
                return visitChar((ast::expr::Char*)expr_node,args);
            case ast::expr::Expression::parentheses_:
                return visitParentheses((ast::expr::Parentheses*)expr_node,args);
            case ast::expr::Expression::boolean_:
                return visitBoolean((ast::expr::Boolean*)expr_node,args);
            case ast::expr::Expression::callee_:{
                args.dot_expression_context = args.domain;
                args.need_lookup = true;
                auto prototype = any_cast<shared_ptr<Prototype>>(visitCallee((ast::expr::Callee*)expr_node,args));
                if(!prototype)return OperandTopType(EmptyType{});
                return mapSymbolToOperandTopType(prototype);
            }
            case ast::expr::Expression::ID_:{
                args.dot_expression_context = args.domain;
                args.need_lookup = true;
                auto symbol = any_cast<shared_ptr<Symbol>>(visitID((ast::expr::ID*)expr_node,args));
                if(auto variable = symbol->as_shared<Variable>())
                    pushVariableAddress(variable,args.previous_block,true);
                return mapSymbolToOperandTopType(symbol);
            }
        }
    }


    std::any IRGen::visitUnary(ast::expr::Unary *unit_node, IRGenArgs args) {
        auto operand_type = any_cast<OperandTopType>(visitExpression(unit_node->terminal, args));
        auto data = tryLoadOperandTop(operand_type,args.previous_block);
        args.previous_block->Neg(data);
        return OperandTopType(DataType{data});
    }



    std::any IRGen::visitCast(ast::expr::Cast *cast_node, IRGenArgs args) {
        auto dst_data = any_cast<Data>(visitAnnotation(cast_node->annotation,args));
        auto src_data = any_cast<Data>(visitExpression(cast_node->expr,args));
        args.previous_block->Cast(src_data,dst_data);
        return dst_data;
        //TODO
    }

    std::any IRGen::visitBoolean(ast::expr::Boolean *bl_node, IRGenArgs args) {
        args.previous_block->Push(Data::boolean,new Const<data::boolean>(bl_node->value));
        return OperandTopType(DataType{Data::boolean});
    }

    std::any IRGen::visitChar(ast::expr::Char *ch_node, IRGenArgs args) {
        args.previous_block->Push(Data::i8,new Const<data::i8>(ch_node->value));
        return OperandTopType(DataType{Data::i8});
    }

    std::any IRGen::visitDigit(ast::expr::Digit *digit_node, IRGenArgs args) {
        args.previous_block->Push(Data::i32, new Const<data::i32>(digit_node->value));
        return OperandTopType(DataType{Data::i32});
    }

    std::any IRGen::visitDecimal(ast::expr::Decimal *decimal_node, IRGenArgs args) {
        args.previous_block->Push(Data::f64, new Const<data::f64>(decimal_node->value));
        return OperandTopType(DataType{Data::f64});
    }

    std::any IRGen::visitString(ast::expr::String *str_node, IRGenArgs args) {
        args.previous_block->Psm(str_node->value.size(),str_node->value.c_str());
        return OperandTopType(MemType{(data::u32)str_node->value.size()});
    }

    std::any IRGen::visitParentheses(ast::expr::Parentheses *parentheses_node, IRGenArgs args) {
        return visitExpression(parentheses_node->expr,args);
    }


    std::any IRGen::visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, IRGenArgs args) {
        Visitor::visitExprStmt(expr_stmt_node, args);
        return (Block*)nullptr;
    }

    std::any IRGen::visitAnnotation(ast::Annotation *anno_node, IRGenArgs args) {
        if(anno_node->array_size)return Data(Data::u32);

        shared_ptr<Symbol> target = args.domain;
        for(auto &unit:anno_node->unit_list){
            auto name = getID(unit->name);
            auto domain = target->as_shared<Domain>();
            NotNull(domain.get());
            if(&unit == &anno_node->unit_list.front()){
                target = domain->lookUp(name);
            }
            else{
                target = domain->find(name);
            }
            NotNull(target.get());
        }

        switch (target->getKind()) {
            case DeclarationEnum::Class:
            case DeclarationEnum::Type:
            case DeclarationEnum::Function:
            case DeclarationEnum::Interface:
            case DeclarationEnum::Array:
            case DeclarationEnum::Enum_:
                return Data(Data::u32);
            case DeclarationEnum::Primitive:
                return target->as_shared<primitive::Primitive>()->getDataKind();
            case DeclarationEnum::Argument:
            case DeclarationEnum::TmpDomain:
            case DeclarationEnum::EnumMember:
            case DeclarationEnum::Module:
            case DeclarationEnum::Variable:
            case DeclarationEnum::Error:
            case DeclarationEnum::Variant:
                ASSERT(true,"invalid annotation");
        }
    }


}

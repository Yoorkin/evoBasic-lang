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

    OperandType addressOf(OperandType type){
        return AddressType{new OperandType(type)};
    }

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
    
    IR *IRGen::gen(AST *ast,Context *context) {
        auto ir = new IR;
        IRGenArgs args;
        args.ir = ir;
        args.domain = context->getGlobal();
        args.context = context;
        visitGlobal(ast,args);
        return ir;
    }

    std::any IRGen::visitGlobal(ast::Global *global_node, IRGenArgs args) {
        NotNull(global_node);
        args.domain = args.context->getGlobal();
        for(auto iter = (*global_node).member;iter!=nullptr;iter=iter->next_sibling){
            visitMember(iter,args);
        }
        return {};
    }

    std::any IRGen::visitModule(ast::Module *module_node, IRGenArgs args) {
        NotNull(module_node);
        args.domain = (*module_node).module_symbol;
        for(auto iter = (*module_node).member;iter!=nullptr;iter=iter->next_sibling){
            visitMember(iter,args);
        }
        return {};
    }

    std::any IRGen::visitClass(ast::Class *cls_node, IRGenArgs args) {
        list<Pair*> members;
        auto ty = cls_node->class_symbol;
        for(auto &p : ty->getFields()){
            auto field_type_ir = new Mark(p->getPrototype()->mangling(),false,false);
            members.push_back(new Pair(p->getName(),field_type_ir));
        }
        auto record_ir = new ir::Record(members);
        args.ir->addMeta(new Pair(ty->mangling(), record_ir));

        args.domain = ty;
        
        auto iter = cls_node->member;
        while(iter){
            visitMember(iter,args);
            iter = iter->next_sibling;
        }

        return {};
    }


    std::any IRGen::visitType(ast::Type *ty_node, IRGenArgs args) {
        list<Pair*> members;
        auto ty = ty_node->type_symbol;
        auto iter = ty_node->member;
        while(iter){
            auto p = iter->variable_symbol;
            auto field_type_ir = new Mark(p->getPrototype()->mangling(),false,false);
            members.push_back(new Pair(p->getName(),field_type_ir));
            iter = iter->next_sibling;
        }
        auto record_ir = new ir::Record(members);
        args.ir->addMeta(new Pair(ty->mangling(), record_ir));
        return nullptr;
    }

    std::any IRGen::visitEnum(ast::Enum *em_node, IRGenArgs args) {
        auto name = getID(em_node->name);
        auto em = em_node->enum_symbol;
        list<pair<string,int>> members;
        for(auto p = em->begin();p!=em->end();p++){
            auto key = p->getName();
            auto value = p->as<EnumMember*>()->getIndex();
            members.emplace_back(key,value);
        }
        auto enum_ir = new Enum(members);
        args.ir->addMeta(new Pair(em->mangling(), enum_ir));
        return nullptr;
    }

    std::any IRGen::visitDim(ast::Dim *dim_node, IRGenArgs args) {
        if(args.domain->getKind() == SymbolKind::Module){
            auto iter = dim_node->variable;
            while(iter){
                visitVariable(iter,args);
                iter = iter->next_sibling;
            }
        }
        return {};
    }

    std::any IRGen::visitVariable(ast::Variable *var_node, IRGenArgs args) {
        auto name = getID(var_node->name);
        auto field = var_node->variable_symbol;
        auto type_ref = new Mark(field->getPrototype()->mangling(),false,false);
        args.ir->addMeta(new Pair(field->mangling(), type_ref));
        return nullptr;
    }

    std::any IRGen::visitFunction(ast::Function *func_node, IRGenArgs args) {
        auto func = func_node->function_symbol;
        args.function = func;

        list<Pair*> params;
        Mark *ret_mark = nullptr;
        for(auto &param : func->getArgsSignature()){
            auto isArray = false,isRef = !param->isByval();
            string name;
            if(param->getPrototype()->getKind() == SymbolKind::Array){
                isArray = true;
                name = param->getPrototype()->as<Array*>()->getElementPrototype()->mangling();
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
            if(func->getRetSignature()->getKind() == SymbolKind::Array){
                isArray = true;
                name = func->getRetSignature()->as<Array*>()->getElementPrototype()->mangling();
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
            auto variable = func->find(argument->getName())->as<Variable*>();
            args.previous_block->PushFrameBase()
                    .Push(Data::ptr,new Const<data::ptr>(variable->getOffset()))
                    .Add(Data::ptr);
            if(argument->isByval()){
                switch (argument->getPrototype()->getKind()) {
                    case SymbolKind::Array:
                    case SymbolKind::Record:
                        args.previous_block->StmR(argument->getPrototype()->as<Domain*>()->getByteLength());
                        break;
                    case SymbolKind::Class:
                        args.previous_block->StoreR(Data::ptr);
                        break;
                    case SymbolKind::Primitive:
                        args.previous_block->StoreR(argument->getPrototype()->as<type::Primitive*>()->getDataKind());
                        break;
                }
            }
            else{
                args.previous_block->StoreR(Data::ptr);
            }
        }

        auto after_block = visitStatementList(func_node->statement,args);
        after_block->Ret();
        return nullptr;
    }

    Block *IRGen::visitStatementList(ast::stmt::Statement *statement, IRGenArgs args) {
        while(statement){
            auto *after_block = any_cast<Block*>(visitStatement(statement,args));
            if(after_block)args.previous_block = after_block;
            statement = statement->next_sibling;
        }
        return args.previous_block;
    }

    std::any IRGen::visitLet(ast::stmt::Let *let_node, IRGenArgs args) {
        auto var = let_node->variable;
        while(var){
            auto tmp = args.domain->find(getID(var->name));
            auto field = tmp->as<Variable*>();
            auto offset = field->getOffset();
            args.previous_block->PushFrameBase()
                                .Push(Data::ptr,new Const<data::u32>(field->getOffset()))
                                .Add(Data::ptr);

            if(var->initial){
                auto operand = any_cast<OperandType>(visitExpression(var->initial, args));
                auto inner_operand = loadOperandAddress(operand,args.previous_block);
                switch((OperandEnum)inner_operand.index()){
                    case OperandEnum::DataType:
                        args.previous_block->Store(get<DataType>(inner_operand).data);
                        break;
                    case OperandEnum::ArrayType:
                        args.previous_block->Stm(get<ArrayType>(inner_operand).size);
                        break;
                    case OperandEnum::RecordType:
                        args.previous_block->Stm(get<RecordType>(inner_operand).size);
                        break;
                    case OperandEnum::AddressType:
                        args.previous_block->Dup(Data::ptr);
                        args.previous_block->RcInc();
                        args.previous_block->Store(Data::ptr);
                        break;
                    default:
                        ASSERT(true,"invalid");
                }
            }
            else switch(field->getPrototype()->getKind()){
                case SymbolKind::Primitive:{
                    auto data = field->getPrototype()->as<Primitive*>()->getDataKind();
                    args.previous_block->Push(data,convertNumberToConst(data,0));
                    args.previous_block->Store(data);
                    break;
                }
                case SymbolKind::Record:{
                    break;
                }
                case SymbolKind::Array:{
                    break;
                }
                case SymbolKind::Class:{
                    args.previous_block->Push(Data::ptr,new Const<data::ptr>(0));
                    args.previous_block->Store(Data::ptr);
                    break;
                }
            }
            var = var->next_sibling;
        }
        return (Block*)nullptr;
    }

    std::any IRGen::visitIf(ast::stmt::If *ifstmt_node, IRGenArgs args) {
        auto after_if_block = new Block(args.domain->mangling() + "_if_after");
        args.next_block = after_if_block;
        
        auto ca = ifstmt_node->case_;
        while(ca){
            visitCase(ca,args);
            ca = ca->next_sibling;
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
            auto after_stmts_block = visitStatementList(ca_node->statement, args);
            after_stmts_block->Jmp(args.next_block);
        }
        else{
            visitStatementList(ca_node->statement, args);
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
        visitStatementList(loop_node->statement,args);
        loop_block->Jmp(condition_block);

        args.ir->addBlock(after_loop_block);
        return after_loop_block;
    }

    std::any IRGen::visitSelect(ast::stmt::Select *select_node, IRGenArgs args) {
        auto after_select_block = new Block(args.domain->mangling() + "_select_after");
        args.next_block = after_select_block;
        auto operand = any_cast<OperandType>(visitExpression(select_node->condition, args));
        auto inner_operand = loadOperandAddress(operand,args.previous_block);

        auto data = get<DataType>(inner_operand).data;
        //duplicate condition value for each Case
        int need_value_count = 0;
        auto ca = select_node->case_;
        while(ca){
            if(ca->condition)need_value_count++;
            ca = ca->next_sibling;
        }
        for(int i=0;i<need_value_count-1;i++){
            args.previous_block->Dup(data);
        }
        ca = select_node->case_;
        while(ca){
            visitCaseInSelectStmt(ca,args);
            ca = ca->next_sibling;
        }
        args.previous_block->Jmp(after_select_block);
        args.ir->addBlock(after_select_block);
        return after_select_block;
    }

    std::any IRGen::visitCaseInSelectStmt(ast::Case *ca_node, IRGenArgs args) {
        if(ca_node->condition) {
            auto operand = any_cast<OperandType>(visitExpression(ca_node->condition, args));
            auto inner_operand = loadOperandAddress(operand,args.previous_block);
            // operand top value must be DataType
            auto data = get<DataType>(inner_operand).data;
            auto case_block = new Block(args.function->mangling() + "_case");
            args.ir->addBlock(case_block);
            args.previous_block->EQ(data);
            args.previous_block->Jif(case_block);
            args.previous_block = case_block;
            auto after_stmts_block = visitStatementList(ca_node->statement, args);
            after_stmts_block->Jmp(args.next_block);
        }
        else{
            visitStatementList(ca_node->statement, args);
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
        auto after_stmts_segment = visitStatementList(forstmt_node->statement,args);
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
        auto operand = any_cast<OperandType>(visitExpression(ret_node->expr, args));
        loadOperandAddress(operand,args.previous_block);
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


    OperandType IRGen::visitAssign(ast::expr::Binary *node, IRGenArgs args) {
        bool need_return_value = args.need_return_value;
        args.need_return_value = true;

        auto lhs_operand = any_cast<OperandType>(visitExpression(node->lhs, args));

        switch((OperandEnum)lhs_operand.index()){
            case OperandEnum::AddressType: {
                auto element = get<AddressType>(lhs_operand).element;
                switch((OperandEnum)element->index()){
                    case OperandEnum::DataType:
                    case OperandEnum::ArrayType:
                    case OperandEnum::RecordType:
                        // do nothing
                        break;
                    case OperandEnum::AddressType: {
                        auto second_element = get<AddressType>(*element).element;
                        switch ((OperandEnum)second_element->index()) {
                            case OperandEnum::ClassType: {
                                // ByVal Class Parameter or Class Variable,
                                // operand top value is ptr->ptr->class memory begin
                                args.previous_block->Dup(Data::ptr)
                                                    .Load(Data::ptr) // ptr->class memory begin
                                                    .RcDec();
                                break;
                            }
                            case OperandEnum::AddressType: {
                                // AddressType,must be ByRef Class Parameter,
                                // operand top value is ptr->ptr->ptr->class memory
                                args.previous_block->Load(Data::ptr) // ptr->ptr->class memory begin
                                                    .Dup(Data::ptr)
                                                    .Load(Data::ptr) // ptr->class memory begin
                                                    .RcDec();
                                break;
                            }
                        }
                        break;
                    }
                    default: PANIC;
                }
                break;
            }
            default: PANIC;
        }

        auto rhs_operand = any_cast<OperandType>(visitExpression(node->rhs, args));
        rhs_operand = loadOperandAddress(rhs_operand,args.previous_block);

        switch((OperandEnum)rhs_operand.index()){
            case OperandEnum::DataType:
                args.previous_block->Store(get<DataType>(rhs_operand).data);
                break;
            case OperandEnum::ArrayType:
                args.previous_block->Stm(get<ArrayType>(rhs_operand).size);
                break;
            case OperandEnum::RecordType:
                args.previous_block->Stm(get<RecordType>(rhs_operand).size);
                break;
            case OperandEnum::ClassType:
                PANIC;
                break;
            case OperandEnum::AddressType:{
                args.previous_block->Dup(Data::ptr)
                                    .RcInc()
                                    .Store(Data::ptr);
                break;
            }
            default: PANIC;
        }

        if(need_return_value) return any_cast<OperandType>(visitExpression(node->lhs, args));
        else return EmptyType{};
    }

    OperandType IRGen::loadOperandAddress(OperandType top,Block *block){
        /*
         * return the innermost OperandType
         * do nothing for Data,Record,Array,Class and ptr->Class.
         * load ptr->Data, ptr->ptr->Data to Data
         * load ptr->Record, ptr->ptr->Record to Record
         * load ptr->Array, ptr->ptr->Array to Array
         * load ptr->ptr->Class, ptr->ptr->ptr->Class to ptr->Class
         *
         */
        switch((OperandEnum)top.index()){
            case OperandEnum::ArrayType:
            case OperandEnum::RecordType:
            case OperandEnum::DataType:
                return top;
            case OperandEnum::AddressType: {
                auto element = get<AddressType>(top).element;
                switch ((OperandEnum)element->index()) {
                    case OperandEnum::ClassType: {
                        // ptr->Class
                        return top;
                    }
                    case OperandEnum::ArrayType:{
                        block->Ldm(get<ArrayType>(*element).size);
                        return loadOperandAddress(*element,block);
                    }
                    case OperandEnum::RecordType:{
                        block->Ldm(get<RecordType>(*element).size);
                        return loadOperandAddress(*element,block);
                    }
                    case OperandEnum::AddressType:{
                        block->Load(Data::ptr);
                        return loadOperandAddress(*element,block);
                    }
                    case OperandEnum::DataType: {
                        block->Load(get<DataType>(*element).data);
                        return loadOperandAddress(*element,block);
                    }
                    default: PANIC;
                }
                break;
            }
            default: PANIC;
        }
    }

    OperandType IRGen::visitLogicOp(ast::expr::Binary *logic_node, IRGenArgs args){
        using Op = ast::expr::Binary::Enum;
        auto boolean = OperandType(DataType{Data::boolean,args.context->getBuiltIn().getPrimitive(vm::Data::boolean)});

        auto lhs_operand = any_cast<OperandType>(visitExpression(logic_node->lhs, args));
        lhs_operand = loadOperandAddress(lhs_operand,args.previous_block);

        auto rhs_operand = any_cast<OperandType>(visitExpression(logic_node->rhs, args));
        rhs_operand = loadOperandAddress(rhs_operand,args.previous_block);


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
        return {};
    }


    OperandType IRGen::visitArithmeticOp(OperandType lhs_operand,ast::expr::Binary *logic_node, IRGenArgs args){
        // The lhs,rhs operand is guaranteed to be DataType
        using Op = ast::expr::Binary::Enum;
        auto boolean = OperandType(DataType{Data::boolean});

        auto rhs_operand = any_cast<OperandType>(visitExpression(logic_node->rhs, args));
        rhs_operand = loadOperandAddress(rhs_operand,args.previous_block);

        auto data = get<DataType>(rhs_operand).data;

        switch (logic_node->op) {
            case Op::EQ:
                args.previous_block->EQ(data);
                return boolean;
            case Op::NE:
                args.previous_block->NE(data);
                return boolean;
            case Op::GE:
                args.previous_block->GE(data);
                return boolean;
            case Op::LE:
                args.previous_block->LE(data);
                return boolean;
            case Op::GT:
                args.previous_block->GT(data);
                return boolean;
            case Op::LT:
                args.previous_block->LT(data);
                return boolean;
            case Op::ADD:
                args.previous_block->Add(data);
                return rhs_operand;
            case Op::MINUS:
                args.previous_block->Sub(data);
                return rhs_operand;
            case Op::MUL:
                args.previous_block->Mul(data);
                return rhs_operand;
            case Op::DIV:
                args.previous_block->Div(data);
                return rhs_operand;
            case Op::FDIV:
                args.previous_block->FDiv(data);
                return rhs_operand;//TODO FIDV support
        }
        return {};
    }

    OperandType visitArithmeticOverride(type::Class *class_node,ast::expr::Binary *logic_node, IRGenArgs args){
        // The lhs operand is guaranteed to be Class, which prototype is type::Class
        //TODO operator override support
        return {};
    }

    std::any IRGen::visitBinary(ast::expr::Binary *logic_node, IRGenArgs args) {

        using Op = ast::expr::Binary::Enum;
        set<Op> logic_op = {Op::And,Op::Or,Op::Xor,Op::Not};
        set<Op> calculate_op = {Op::EQ,Op::NE,Op::LE,Op::GE,Op::LT,Op::GT,Op::ADD,Op::MINUS,Op::MUL,Op::DIV,Op::FDIV};

        if(logic_op.contains(logic_node->op)){
            return visitLogicOp(logic_node,args);
        }
        else if(calculate_op.contains(logic_node->op)){
            auto lhs_operand = any_cast<OperandType>(visitExpression(logic_node->lhs, args));
            lhs_operand = loadOperandAddress(lhs_operand,args.previous_block);

            switch((OperandEnum)lhs_operand.index()){
                case OperandEnum::AddressType:
                    return visitArithmeticOverride(get<ClassType>(*get<AddressType>(lhs_operand).element).cls,logic_node,args);
                    break;
                case OperandEnum::DataType:
                    return visitArithmeticOp(lhs_operand,logic_node,args);
                    break;
                default: PANIC;
            }
        }
//        else if(logic_node->op == Op::ASSIGN){
//            return visitAssign(logic_node,args);
//        }
//        else if(logic_node->op == Op::Dot || logic_node->op == Op::Index ){
//            return visitDot(logic_node,args);
//        }
        return {};
    }

    std::any IRGen::visitID(ast::expr::ID *id_node, IRGenArgs args) {
        auto name = getID(id_node);
        Symbol *ret = nullptr;
        if(args.need_lookup) {
            return args.dot_expression_context->as<Domain*>()->lookUp(name);
        }
        else{
            return args.dot_expression_context->as<Domain*>()->find(name);
        }
    }


    std::any IRGen::visitCallee(ast::expr::Callee *callee_node, IRGenArgs args) {
        auto function = any_cast<Symbol*>(visitID(callee_node->name,args))->as<type::Function*>();
        args.function = function;
        args.need_lookup = true;
        args.dot_expression_context = nullptr;

        auto argument = callee_node->argument;
        int i = 0;
        while(argument){
            args.current_args_index = i;
            visitArg(argument,args);
            argument = argument->next_sibling;
            i++;
        }
        args.previous_block->Invoke(function->mangling());

        auto ret = function->getRetSignature();
        switch(ret->getKind()){
            case SymbolKind::Primitive: {
                auto pr = ret->as<type::Primitive*>();
                return OperandType(DataType{pr->getDataKind(),pr});
                break;
            }
            case SymbolKind::Record: {
                auto record = ret->as<type::Record*>();
                return OperandType(RecordType{record->getByteLength(),record});
            }
            case SymbolKind::Array: {
                auto array = ret->as<type::Array*>();
                return OperandType(RecordType{array->getByteLength(),array});
            }
            case SymbolKind::Class: {
                auto cls = ret->as<type::Class*>();
                return OperandType(addressOf(ClassType{cls->getByteLength(),cls}));
            }
            default: PANIC;
        }
    }


    OperandType IRGen::pushVariableAddress(Variable *variable, Block *block, bool need_push_base){
        if(need_push_base){
            switch(variable->getParent()->getKind()){
                case SymbolKind::Function:
                    block->PushFrameBase();
                    break;
                case SymbolKind::Module:
                    block->PushGlobalBase();
                    break;
            }
        }
        block->Push(Data::ptr,new Const<data::ptr>(variable->getOffset()))
              .Add(Data::ptr);
        //push variable address to operand

        if(auto argument = variable->as<Parameter*>()){
            if(!argument->isByval()){
                //push referencing address to operand
                block->Load(Data::ptr);
            }
        }

        return addressOf(mapSymbolToOperandType(variable->getPrototype()));
    }

    OperandType IRGen::visitIndex(ast::expr::Binary *index,IRGenArgs args,bool need_push_base){
        auto symbol = any_cast<Symbol*>(visitID((ast::expr::ID*)index->lhs,args));
        auto variable = symbol->as<Variable*>();
        NotNull(variable);
        auto lhs_operand = pushVariableAddress(variable,args.previous_block,need_push_base);

        switch ((OperandEnum)lhs_operand.index()) {
            case OperandEnum::AddressType: {
                auto element = get<AddressType>(lhs_operand).element;
                switch ((OperandEnum) element->index()) {
                    case OperandEnum::ArrayType: {
                        visitExpression(index->rhs, args);
                        auto array_type = get<ArrayType>(*element);
                        args.previous_block->Push(Data::ptr,new Const<data::ptr>(array_type.array->getElementPrototype()->getByteLength()))
                                            .Mul(Data::ptr)
                                            .Add(Data::ptr);
                        return addressOf(mapSymbolToOperandType(array_type.array->getElementPrototype()));
                    }
                    case OperandEnum::ClassType:{
                        //TODO index operator override support
                        break;
                    }
                    default: PANIC; break;
                }
                break;
            }
            default: PANIC; break;
        }
        return {};
    }

    std::any IRGen::visitArg(ast::expr::Callee::Argument *arg_node, IRGenArgs args) {
//        /*
//          *   Param\Arg          ByVal                  ByRef            Undefined
//          *   ByVal      Yes,allow implicit conversion  Error      Yes,allow implicit conversion
//          *   ByRef      store value to tmp address,                   Error when arg
//          *              allow implicit conversion      Yes             is not lvalue
//          */
//        NotNull(arg_node);
//        int param_index = (args.function->getFunctionFlag() != type::FunctionFlag::Static) ? args.current_args_index : args.current_args_index+1;
//        auto &param =  args.function->getArgsSignature()[param_index];
//        auto arg_type = arg_node->expr->type;
//        if(param->isByval()){ // ByVal Parameter
//            switch (arg_node->pass_kind) {
//                case ast::expr::Callee::Parameter::byref:
//                    //do nothing
//                    break;
//                case ast::expr::Callee::Parameter::undefined:
//                case ast::expr::Callee::Parameter::byval: {
//                    auto arg_operand = any_cast<OperandType>(visitExpression(arg_node->expr, args));
//                    loadOperandAddress(arg_operand,args.previous_block);
//                    break;
//                }
//            }
//        }
//        else{ // ByRef Parameter
//            switch (arg_node->pass_kind) {
//                case ast::expr::Callee::Parameter::byval: {
//                    // store value in temporary address
//                    pushVariableAddress(arg_node->temp_address, args.previous_block, true);
//                    auto operand = any_cast<OperandType>(visitExpression(arg_node->expr, args));
//                    loadOperandAddress(operand,args.previous_block);
//                    auto arg_operand = operand;
//
//                    switch ((OperandEnum)arg_operand.index()) {
//                        case OperandEnum::DataType:
//                            args.previous_block->Store(get<DataType>(arg_operand).data);
//                            break;
//                        case OperandEnum::ArrayType:
//                            args.previous_block->Stm(get<ArrayType>(arg_operand).size);
//                            break;
//                        case OperandEnum::RecordType:
//                            args.previous_block->Stm(get<RecordType>(arg_operand).size);
//                            break;
//                        case OperandEnum::AddressType:
//                            args.previous_block->Store(Data::ptr);
//                            break;
//                        default:
//                            PANIC;
//                    }
//
//                    // pass temporary address to ByRef parameter
//                    pushVariableAddress(arg_node->temp_address, args.previous_block, true);
//                    break;
//                }
//                case ast::expr::Callee::Parameter::byref:
//                case ast::expr::Callee::Parameter::undefined:
//                    visitExpression(arg_node->expr,args);
//                    break;
//            }
//        }
//        return nullptr;
    }

    OperandType IRGen::mapSymbolToOperandType(Symbol *symbol) {
        NotNull(symbol);
        switch (symbol->getKind()) {
            case SymbolKind::Class: {
                auto cls = symbol->as<Class*>();
                return addressOf(ClassType{cls->getByteLength(),cls});
            }
            case SymbolKind::Enum: {
                return DataType{Data::ptr};
            }
            case SymbolKind::Array: {
                auto array = symbol->as<type::Array*>();
                return ArrayType{array->getByteLength(),array};
            }
            case SymbolKind::Record: {
                auto record = symbol->as<type::Record*>();
                return RecordType{record->getByteLength(),record};
            }
            case SymbolKind::Parameter: {
                auto argument = symbol->as<Parameter*>();
                auto element = mapSymbolToOperandType(argument->getPrototype());
                if(argument->isByval()) {
                    return addressOf(element);
                }
                else{
                    return addressOf(addressOf(element));
                }
            }
            case SymbolKind::Variable: {
                auto variable = symbol->as<Variable*>();
                auto element = mapSymbolToOperandType(variable->getPrototype());
                return addressOf(element);
            }
            case SymbolKind::Primitive: {
                auto primitive = symbol->as<primitive::Primitive*>();
                return DataType{primitive->getDataKind(),primitive};
            }
            default: PANIC;
        }
    }

    std::any IRGen::visitExpression(ast::expr::Expression *expr_node, IRGenArgs args) {
        switch (expr_node->expression_kind) {
            case ast::expr::Expression::binary_:
                return visitBinary((ast::expr::Binary*)expr_node,args);

            case ast::expr::Expression::unary_:
                return visitUnary((ast::expr::Unary*)expr_node, args);

            case ast::expr::Expression::parentheses_:
                return visitParentheses((ast::expr::Parentheses*)expr_node,args);

            case ast::expr::Expression::boolean_:
            case ast::expr::Expression::callee_:
            case ast::expr::Expression::ID_:
            case ast::expr::Expression::digit_:
            case ast::expr::Expression::decimal_:
            case ast::expr::Expression::string_:
            case ast::expr::Expression::char_:
                return visitDot(expr_node,args);
        }
        return {};
    }

    Symbol *stripOperandType(OperandType type){
        switch ((OperandEnum)type.index()) {
            case OperandEnum::DataType: return get<DataType>(type).primitive;
            case OperandEnum::ClassType: return get<ClassType>(type).cls;
            case OperandEnum::ArrayType: return get<ArrayType>(type).array;
            case OperandEnum::RecordType: return get<RecordType>(type).record;
            case OperandEnum::AddressType: return stripOperandType(*get<AddressType>(type).element);
            case OperandEnum::SymbolPtr: return get<type::Symbol*>(type);
        }
        return nullptr;
    }

    OperandType IRGen::visitDot(ast::expr::Expression *node,IRGenArgs args,OperandType lhs){
//        using namespace ast;
//        using namespace ast::expr;
//
//        bool push_base_address = true;
//        switch ((OperandEnum)lhs.index()) {
//            case OperandEnum::EmptyType:
//                args.need_lookup = true;
//                args.dot_expression_context = args.domain;
//                break;
//            case OperandEnum::AddressType:{
//                auto element = get<AddressType>(lhs).element;
//                switch ((OperandEnum)element->index()) {
//                    case OperandEnum::RecordType:
//                    case OperandEnum::ArrayType:
//                        push_base_address = false;
//                        break;
//                    case OperandEnum::AddressType:{
//                        push_base_address = false;
//                        args.previous_block->Load(Data::ptr);
//                        break;
//                    }
//                    case OperandEnum::ClassType:
//                        push_base_address = false;
//                        break;
//                    default: PANIC;
//                }
//                break;
//            }
//            case OperandEnum::SymbolPtr:
//                // do nothing
//                break;
//            default: PANICMSG(to_string(lhs.index()));
//        }
//
//        if((OperandEnum)lhs.index()!=OperandEnum::EmptyType){
//            args.dot_expression_context = stripOperandType(lhs);
//        }
//
//        switch(node->expression_kind){
//            case ast::expr::Expression::binary_: {
//                auto bin_node = (Binary*)node;
//                switch(bin_node->op){
//                    case Binary::Dot:{
//                        //lhs
//                        auto operand = visitDot(bin_node->lhs,args);
//                        //rhs
//                        return visitDot(bin_node->rhs,args,operand);
//                    }
//                    case Binary::Index:{
//                        return visitIndex(bin_node,args,push_base_address);
//                    }
//                }
//            }
//            case ast::expr::Expression::ID_: {
//                auto symbol = any_cast<Symbol*>(visitID((ID*)node,args));
//                if(auto variable = symbol->as<type::Variable*>())
//                    return pushVariableAddress(variable,args.previous_block,push_base_address);
//                return symbol;
//            }
//            case ast::expr::Expression::cast_:
//                return any_cast<OperandType>(visitCast((Cast*)node,args));
//
//            case ast::expr::Expression::digit_:
//                return any_cast<OperandType>(visitDigit((Digit*)node,args));
//
//            case ast::expr::Expression::decimal_:
//                return any_cast<OperandType>(visitDecimal((Decimal*)node,args));
//
//            case ast::expr::Expression::string_:
//                return any_cast<OperandType>(visitString((String*)node,args));
//
//            case ast::expr::Expression::char_:
//                return any_cast<OperandType>(visitChar((Char*)node,args));
//
//            case ast::expr::Expression::boolean_:
//                return any_cast<OperandType>(visitBoolean((Boolean*)node,args));
//
//            case ast::expr::Expression::callee_:
//                return any_cast<OperandType>(visitCallee((Callee*)node,args));
//
//        }
//        return {};
    }



    std::any IRGen::visitUnary(ast::expr::Unary *unit_node, IRGenArgs args) {
        auto operand = any_cast<OperandType>(visitExpression(unit_node->terminal, args));
        auto inner = loadOperandAddress(operand,args.previous_block);
        switch((OperandEnum)inner.index()){
            case OperandEnum::DataType:
                args.previous_block->Neg(get<DataType>(inner).data);
                return inner;
            case OperandEnum::RecordType:
            case OperandEnum::ArrayType:
                PANICMSG("MemType");
            case OperandEnum::AddressType:
                PANICMSG("AddressType");
                //TODO unary operator override for Ptr->Class
            case OperandEnum::ClassType:
                PANICMSG("ClassType");
        }
        return {};
    }



    std::any IRGen::visitCast(ast::expr::Cast *cast_node, IRGenArgs args) {
        auto dst_data = any_cast<Data>(visitAnnotation(cast_node->annotation,args));
        visitExpression(cast_node->expr,args);
        //auto src_data = get operand.top()
        //args.previous_block->Cast(src_data,dst_data);//TODO
        return dst_data;
        //TODO class conversion support
    }

    std::any IRGen::visitBoolean(ast::expr::Boolean *bl_node, IRGenArgs args) {
        args.previous_block->Push(Data::boolean,new Const<data::boolean>(bl_node->value));
        auto boolean = args.context->getBuiltIn().getPrimitive(Data::boolean);
        return OperandType(DataType{Data::boolean,boolean});
    }

    std::any IRGen::visitChar(ast::expr::Char *ch_node, IRGenArgs args) {
        args.previous_block->Push(Data::i8,new Const<data::i8>(ch_node->value));
        auto char_ = args.context->getBuiltIn().getPrimitive(Data::i8);
        return OperandType(DataType{Data::i8,char_});
    }

    std::any IRGen::visitDigit(ast::expr::Digit *digit_node, IRGenArgs args) {
        args.previous_block->Push(Data::i32, new Const<data::i32>(digit_node->value));
        auto digit = args.context->getBuiltIn().getPrimitive(Data::i32);
        return OperandType(DataType{Data::i32,digit});
    }

    std::any IRGen::visitDecimal(ast::expr::Decimal *decimal_node, IRGenArgs args) {
        args.previous_block->Push(Data::f64, new Const<data::f64>(decimal_node->value));
        auto decimal = args.context->getBuiltIn().getPrimitive(Data::f64);
        return OperandType(DataType{Data::f64,decimal});
    }

    std::any IRGen::visitString(ast::expr::String *str_node, IRGenArgs args) {
        args.previous_block->Psm(str_node->value.size(),str_node->value.c_str());
        return OperandType(DataType{Data::ptr,nullptr});
       //TODO String class
    }

    std::any IRGen::visitParentheses(ast::expr::Parentheses *parentheses_node, IRGenArgs args) {
        return visitExpression(parentheses_node->expr,args);
    }


    std::any IRGen::visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, IRGenArgs args) {
        args.need_return_value = false;
        visitExpression(expr_stmt_node->expr,args);
        return (Block*)nullptr;
    }

    std::any IRGen::visitAnnotation(ast::Annotation *anno_node, IRGenArgs args) {
        if(anno_node->array_size)return Data(Data::u32);

        Symbol *target = args.domain;
        auto unit = anno_node->unit;
        while(unit){
            auto name = getID(unit->name);
            auto domain = target->as<Domain*>();
            NotNull(domain);
            if(unit == anno_node->unit){
                target = domain->lookUp(name);
            }
            else{
                target = domain->find(name);
            }
            NotNull(target);
            unit = unit->next_sibling;
        }

        switch (target->getKind()) {
            case SymbolKind::Class:
            case SymbolKind::Record:
            case SymbolKind::Function:
            case SymbolKind::Interface:
            case SymbolKind::Array:
            case SymbolKind::Enum:
                return Data(Data::ptr);
            case SymbolKind::Primitive:
                return target->as<Primitive*>()->getDataKind();
            case SymbolKind::Parameter:
            case SymbolKind::TmpDomain:
            case SymbolKind::EnumMember:
            case SymbolKind::Module:
            case SymbolKind::Variable:
            case SymbolKind::Error:
                ASSERT(true,"invalid annotation");
        }
    }

    std::any IRGen::visitMember(ast::Member *member_node, IRGenArgs args) {
        switch (member_node->member_kind) {
            case ast::Member::function_: return visitFunction((ast::Function*)member_node,args);
            case ast::Member::class_:    return visitClass((ast::Class*)member_node,args);
            case ast::Member::module_:   return visitModule((ast::Module*)member_node,args);
            case ast::Member::type_:     return visitType((ast::Type*)member_node,args);
            case ast::Member::enum_:     return visitEnum((ast::Enum*)member_node,args);
            case ast::Member::dim_:      return visitDim((ast::Dim*)member_node,args);
            case ast::Member::external_: return visitExternal((ast::External*)member_node,args);
        }
        return {};
    }

    std::any IRGen::visitStatement(ast::stmt::Statement *stmt_node, IRGenArgs args) {
        switch (stmt_node->stmt_flag) {
            case ast::stmt::Statement::let_: return visitLet((ast::stmt::Let*)stmt_node,args);
            case ast::stmt::Statement::loop_:return visitLoop((ast::stmt::Loop*)stmt_node,args);
            case ast::stmt::Statement::if_:  return visitIf((ast::stmt::If*)stmt_node,args);
            case ast::stmt::Statement::for_: return visitFor((ast::stmt::For*)stmt_node,args);
            case ast::stmt::Statement::select_:return visitSelect((ast::stmt::Select*)stmt_node,args);
            case ast::stmt::Statement::return_:return visitReturn((ast::stmt::Return*)stmt_node,args);
            case ast::stmt::Statement::continue_:return visitContinue((ast::stmt::Continue*)stmt_node,args);
            case ast::stmt::Statement::exit_:return visitExit((ast::stmt::Exit*)stmt_node,args);
            case ast::stmt::Statement::expr_:return visitExprStmt((ast::stmt::ExprStmt*)stmt_node,args);
        }
        return {};
    }


}

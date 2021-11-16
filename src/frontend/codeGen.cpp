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
    
    IRBase *convertNumberToConst(Data kind,long long number){
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
        args.previous_segment = ir->getMetaSegment();
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
        args.previous_segment->add(new Pair(ty->mangling(), record_ir));
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
        args.previous_segment->add(new Pair(ty->mangling(), record_ir));
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
        args.previous_segment->add(new Pair(em->mangling(), enum_ir));
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
        args.previous_segment->add(new Pair(field->mangling(), type_ref));
        return nullptr;
    }


    std::any IRGen::visitFunction(ast::Function *func_node, IRGenArgs args) {
        auto func = args.domain->find(getID(func_node->name))->as_shared<type::Function>();
        args.function = func;
        list<Pair*> params;
        Mark *ret_mark;
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
        auto function_segment = args.ir->createSegment(args.domain->mangling());
        auto func_ir = new ir::Function(params,ret_mark,function_segment);
        args.previous_segment->add(new Pair(func->mangling(), func_ir));
        args.ir->add(function_segment);

        args.previous_segment = function_segment;
        auto after_segment = visitStatementList(func_node->statement_list,args);
        after_segment->add(Instruction::WithoutType(Bytecode::Ret));
        return nullptr;
    }


    Segment *IRGen::visitStatementList(std::list<ast::stmt::Statement*> &stmt_list, IRGenArgs args) {
        for(auto& s:stmt_list){
            auto *after_segment = any_cast<Segment*>(visitStatement(s,args));
            if(after_segment)args.previous_segment = after_segment;
        }
        return args.previous_segment;
    }

    std::any IRGen::visitLet(ast::stmt::Let *let_node, IRGenArgs args) {
        for(auto &var : let_node->variable_list){
            auto tmp = args.domain->find(getID(var->name));
            auto field = tmp->as_shared<Variable>();
            auto offset = field->getOffset();
            args.previous_segment->add(Instruction::WithoutType(Bytecode::PushFrameBase))
                                 ->add(Instruction::Push(IR::ptr,new Const<data::u32>(field->getOffset())))
                                 ->add(Instruction::WithType(Bytecode::Add, IR::ptr));

            auto field_kind = field->getPrototype()->getKind();
            if(field_kind == DeclarationEnum::Primitive){
                auto primitive_type = field->getPrototype()->as_shared<primitive::Primitive>();
                if(var->initial) {
                    visitExpression(var->initial, args);
                }
                else {
                    auto data =  primitive_type->getDataKind();
                    args.previous_segment->add(Instruction::Push(data,convertNumberToConst(data,0)));
                }
                args.previous_segment->add(Instruction::WithType(Bytecode::Store, primitive_type->getDataKind()));
            }
            else if(field_kind == DeclarationEnum::Type){
                //TODO
            }
            else if(field_kind == DeclarationEnum::Array){

            }
            else if(field_kind == DeclarationEnum::Class){
                //TODO
            }
        }
        return (Segment*)nullptr;
    }

    std::any IRGen::visitIf(ast::stmt::If *ifstmt_node, IRGenArgs args) {
        auto after_if_segment = args.ir->createSegment(args.domain->mangling() + "_if_after");
        args.next_segment = after_if_segment;
        for(auto &ca:ifstmt_node->case_list){
            visitCase(ca,args);
        }
        args.previous_segment->add(Instruction::Jmp(after_if_segment));
        args.ir->add(after_if_segment);
        return after_if_segment;
    }

    std::any IRGen::visitCase(ast::Case *ca_node, IRGenArgs args) {
        if(ca_node->condition) {
            visitExpression(ca_node->condition, args);
            auto case_segment = args.ir->createSegment(args.function->mangling() + "_case");
            args.ir->add(case_segment);
            args.previous_segment->add(Instruction::Jif(case_segment));
            args.previous_segment = case_segment;
            auto after_stmts_segment = visitStatementList(ca_node->statement_list, args);
            after_stmts_segment->add(Instruction::Jmp(args.next_segment));
        }
        else{
            visitStatementList(ca_node->statement_list, args);
        }
        return (Segment*)nullptr;
    }

    std::any IRGen::visitLoop(ast::stmt::Loop *loop_node, IRGenArgs args) {
        auto condition_segment = args.ir->createSegment(args.function->mangling() + "_condition");
        auto loop_segment = args.ir->createSegment(args.function->mangling() + "_loop");
        auto after_loop_segment = args.ir->createSegment(args.function->mangling() + "_loop_after");

        //jump from previous segment to condition segment
        args.previous_segment->add(Instruction::Jmp(condition_segment));

        //loop condition ir
        args.ir->add(condition_segment);
        args.previous_segment = condition_segment;
        visitExpression(loop_node->condition,args);
        condition_segment->add(Instruction::Jif(loop_segment));
        condition_segment->add(Instruction::Jmp(after_loop_segment));

        //loop body ir
        args.ir->add(loop_segment);
        args.previous_segment = loop_segment;
        args.next_segment = after_loop_segment;
        visitStatementList(loop_node->statement_list,args);
        loop_segment->add(Instruction::Jmp(condition_segment));

        args.ir->add(after_loop_segment);
        return after_loop_segment;
    }

    std::any IRGen::visitSelect(ast::stmt::Select *select_node, IRGenArgs args) {
        auto after_select_segment = args.ir->createSegment(args.domain->mangling() + "_select_after");
        args.next_segment = after_select_segment;
        for(auto &ca:select_node->case_list){
            visitCase(ca,args);
        }
        args.previous_segment->add(Instruction::Jmp(after_select_segment));
        args.ir->add(after_select_segment);
        return after_select_segment;
    }

    std::any IRGen::visitFor(ast::stmt::For *forstmt_node, IRGenArgs args) {
        auto condition_segment = args.ir->createSegment(args.function->mangling() + "_condition");
        auto loop_segment = args.ir->createSegment(args.function->mangling() + "_loop");
        auto after_loop_segment = args.ir->createSegment(args.function->mangling() + "_loop_after");


        //load iterator address and begin expression value
        args.reserve_address = true;
        visitExpression(forstmt_node->iterator,args);
        args.reserve_address = false;
        auto data = any_cast<Data>(visitExpression(forstmt_node->begin,args));
        //store value in address
        args.previous_segment->add(Instruction::WithType(Bytecode::Store, data));
        //jump from previous previous_segment to looping segment
        args.previous_segment->add(Instruction::Jmp(loop_segment));
        //generate ir in loop previous_segment


        args.ir->add(condition_segment);
        //generate condition previous_segment ir
        // increase iterator
        args.previous_segment = condition_segment;
        visitExpression(forstmt_node->iterator,args);
        if(forstmt_node->step)
            visitExpression(forstmt_node->step,args);
        else
            condition_segment->add(Instruction::Push(data, convertNumberToConst(data,1)));
        args.previous_segment->add(Instruction::WithType(Bytecode::Add, data));
        //duplicate increased value and store in iterator address
        args.previous_segment->add(Instruction::WithType(Bytecode::Dup, data));
        args.reserve_address = true;
        visitExpression(forstmt_node->iterator,args);
        args.reserve_address = false;
        args.previous_segment->add(Instruction::WithType(Bytecode::Store, data));
        //load end expression value
        visitExpression(forstmt_node->end,args);
        //compare iterator and end,if less than or equal the end value,jump to loop previous_segment
        args.previous_segment->add(Instruction::WithType(Bytecode::LE, data));
        args.previous_segment->add(Instruction::Jif(loop_segment));
        //otherwise jump to after loop segment
        args.previous_segment->add(Instruction::Jmp(after_loop_segment));



        args.ir->add(loop_segment);
        args.previous_segment = loop_segment;
        args.next_segment = after_loop_segment;
        args.continue_segment = condition_segment;
        auto after_stmts_segment = visitStatementList(forstmt_node->statement_list,args);
        //jump from loop end to condition previous_segment
        after_stmts_segment->add(Instruction::Jmp(condition_segment));

        args.ir->add(after_loop_segment);
        return after_loop_segment;
    }

    std::any IRGen::visitContinue(ast::stmt::Continue *cont_node, IRGenArgs args) {
        args.previous_segment->add(Instruction::Jmp(args.continue_segment));
        return (Segment*)nullptr;
    }

    std::any IRGen::visitReturn(ast::stmt::Return *ret_node, IRGenArgs args) {
        args.need_dup_in_assignment = true;
        visitExpression(ret_node->expr,args);
        args.previous_segment->add(Instruction::WithoutType(Bytecode::Ret));
        return (Segment*)nullptr;
    }

    std::any IRGen::visitExit(ast::stmt::Exit *exit_node, IRGenArgs args) {
        switch (exit_node->exit_flag) {
            case ast::stmt::Exit::For:
            case ast::stmt::Exit::While:
                args.previous_segment->add(Instruction::Jmp(args.next_segment));
                break;
            case ast::stmt::Exit::Sub:
                args.previous_segment->add(Instruction::WithoutType(Bytecode::Ret));
                break;
        }
        return (Segment*)nullptr;
    }

    std::any IRGen::visitBinary(ast::expr::Binary *logic_node, IRGenArgs args) {
        using Op = ast::expr::Binary::Enum;
        set<Op> logic_op = {Op::And,Op::Or,Op::Xor,Op::Not,Op::Not};
        set<Op> calculate_op = {Op::EQ,Op::NE,Op::LE,Op::GE,Op::LT,Op::GT,Op::ADD,Op::MINUS,Op::MUL,Op::DIV,Op::FDIV};
        if(logic_op.contains(logic_node->op)){
            switch (logic_node->op) {
                case Op::And:
                    args.previous_segment->add(Instruction::WithoutType(Bytecode::And));
                    return DataSizeVariant(Data::boolean);
                case Op::Or:
                    args.previous_segment->add(Instruction::WithoutType(Bytecode::Or));
                    return DataSizeVariant(Data::boolean);
                case Op::Xor:
                    args.previous_segment->add(Instruction::WithoutType(Bytecode::Xor));
                    return DataSizeVariant(Data::boolean);
                case Op::Not:
                    args.previous_segment->add(Instruction::WithoutType(Bytecode::Not));
                    return DataSizeVariant(Data::boolean);
            }
        }
        else if(calculate_op.contains(logic_node->op)){
            auto lhs_data = any_cast<Data>(visitExpression(logic_node->lhs,args));
            visitExpression(logic_node->rhs,args);
            switch (logic_node->op) {
                case Op::EQ:
                    args.previous_segment->add(Instruction::WithType(Bytecode::EQ, lhs_data));
                    return DataSizeVariant(Data::boolean);
                case Op::NE:
                    args.previous_segment->add(Instruction::WithType(Bytecode::NE, lhs_data));
                    return DataSizeVariant(Data::boolean);
                case Op::GE:
                    args.previous_segment->add(Instruction::WithType(Bytecode::GE, lhs_data));
                    return DataSizeVariant(Data::boolean);
                case Op::LE:
                    args.previous_segment->add(Instruction::WithType(Bytecode::LE, lhs_data));
                    return DataSizeVariant(Data::boolean);
                case Op::GT:
                    args.previous_segment->add(Instruction::WithType(Bytecode::GT, lhs_data));
                    return DataSizeVariant(Data::boolean);
                case Op::LT:
                    args.previous_segment->add(Instruction::WithType(Bytecode::LT, lhs_data));
                    return DataSizeVariant(Data::boolean);
                case Op::ADD:
                    args.previous_segment->add(Instruction::WithType(Bytecode::Add, lhs_data));
                    return DataSizeVariant(lhs_data);
                case Op::MINUS:
                    args.previous_segment->add(Instruction::WithType(Bytecode::Sub, lhs_data));
                    return DataSizeVariant(lhs_data);
                case Op::MUL:
                    args.previous_segment->add(Instruction::WithType(Bytecode::Mul, lhs_data));
                    return DataSizeVariant(lhs_data);
                case Op::DIV:
                    args.previous_segment->add(Instruction::WithType(Bytecode::Div, lhs_data));
                    return DataSizeVariant(lhs_data);
                case Op::FDIV:
                    args.previous_segment->add(Instruction::WithType(Bytecode::FDiv, lhs_data));
                    return DataSizeVariant(lhs_data);//TODO FIDV support
            }
        }
        else if(logic_node->op == Op::ASSIGN){
            args.reserve_address = true;
            auto lhs_data = any_cast<Data>(visitExpression(logic_node->lhs,args));
            args.reserve_address = false;
            if(args.need_dup_in_assignment) args.previous_segment->add(Instruction::WithType(Bytecode::Dup, lhs_data));
            visitExpression(logic_node->rhs,args);
            args.previous_segment->add(Instruction::WithType(Bytecode::Store, lhs_data));
            return DataSizeVariant(lhs_data);
        }
        else if(logic_node->op == Op::Dot){
            args.dot_expression_context = args.domain;
            auto prototype = visitDot(logic_node,args);
            return convertSymbolToDataKind(prototype);
        }
    }

    std::any IRGen::visitID(ast::expr::ID *id_node, IRGenArgs args) {
        auto name = getID(id_node);
        if(args.need_lookup)
            return args.domain->lookUp(name);
        else
            return args.domain->find(name);
    }


    std::any IRGen::visitCallee(ast::expr::Callee *callee_node, IRGenArgs args) {
        shared_ptr<type::Function> function = any_cast<Symbol>(visitID(callee_node->name,args)).as_shared<type::Function>();
        //TODO args code gen
        return function->getRetSignature();
    }

    DataSizeVariant IRGen::convertSymbolToDataKind(std::shared_ptr<Symbol> symbol) {
        NotNull(symbol.get());
        switch (symbol->getKind()) {
            case DeclarationEnum::Class:
                return Data::ptr;
            case DeclarationEnum::Enum_:
                return Data::u32;
            case DeclarationEnum::Array:
            case DeclarationEnum::Type:
                return symbol->as_shared<Domain>()->getByteLength();
            case DeclarationEnum::Argument:{
                auto argument = symbol->as_shared<Argument>();
                if(argument->isByval())
                    return Data::ptr;
                else
                    return convertSymbolToDataKind(argument->getPrototype());
            }
            case DeclarationEnum::Variable:
                return convertSymbolToDataKind(symbol->as_shared<Variable>()->getPrototype());
            case DeclarationEnum::Primitive:
                return (symbol->as_shared<primitive::Primitive>()->getDataKind());
            default:
                ASSERT(true,"error");
        }
    }

    void IRGen::dereference(std::shared_ptr<type::Symbol> symbol){

    }

    shared_ptr<Prototype> IRGen::visitDot(ast::expr::Expression *dot_node,IRGenArgs args){
        using namespace ast::expr;
        using Op = Binary::Enum;
        using PrototypePtr = shared_ptr<Prototype>;
        auto node = (Binary*)dot_node;
        PrototypePtr lhs,rhs;
        if(node->expression_kind == Expression::ID_){
            args.need_lookup = true;
            lhs = any_cast<PrototypePtr>(visitID((ID*)node->lhs,args));
            switch(lhs->getKind()){

            }
            if(lhs->getKind() == DeclarationEnum::Argument){
                auto argument = lhs->as_shared<Argument>();
                if(!argument->isByval()){
                    //dereference ByRef argument
                    args.previous_segment->add(Instruction::WithType(Bytecode::Load,Data::u32));
                }
            }
        }
        else if(node->expression_kind == Expression::callee_){
            args.need_lookup = true;
            lhs = any_cast<PrototypePtr>(visitCallee((Callee*)node->lhs,args));
        }




        if(dot_node->expression_kind == Expression::ID_){
            return any_cast<shared_ptr<Prototype>>(visitID((ID*)dot_node,args));
        }
        else if(dot_node->expression_kind == Expression::callee_){
            return any_cast<shared_ptr<Prototype>>(visitCallee((Callee*)dot_node,args));
        }
        else if(dot_node->expression_kind == Expression::binary_){
            switch (((Binary*)dot_node)->op) {
                case Op::Dot: {
                    auto lhs_prototype = visitDot(((Binary*) dot_node)->lhs, args);
                    auto rhs_prototype = visitDot(((Binary*) dot_node)->lhs, args);
                    using ty = DeclarationEnum;
                    switch (lhs_prototype->getKind()) {
                        case ty::Array:
                        case ty::Type:
                        case ty::Variable:
                        case ty::Argument:
                        break;
                    }
                }
                break;
                case Op::Index:
                    auto array = visitDot(((Binary*)dot_node)->lhs,args);
                    visitExpression(((Binary*)dot_node)->rhs,args);
                    args.previous_segment->add(Instruction::WithType(Bytecode::Add,Data::ptr));
                    return array->as_shared<Array>()->getElementPrototype();
            }
        }
    }


    std::any IRGen::visitUnary(ast::expr::Unary *unit_node, IRGenArgs args) {
        auto data_size_variant = any_cast<DataSizeVariant>(visitExpression(unit_node->terminal,args));
        args.previous_segment->add(Instruction::WithType(Bytecode::Neg, get<0>(data_size_variant)));
        return data_size_variant;
    }

    DataSizeVariant IRGen::visitVariableCall(ast::expr::Callee *callee_node,IRGenArgs args,shared_ptr<Variable> target){
        if(target->getKind() == DeclarationEnum::Variable || target->getKind() == DeclarationEnum::Argument){
            auto variable = target->as_shared<evoBasic::Variable>();
            auto base = variable->isGlobal() ? Bytecode::PushGlobalBase : Bytecode::PushFrameBase;
            args.previous_segment->add(Instruction::WithoutType(base));
            args.previous_segment->add(Instruction::Push(IR::ptr,new Const<data::u32>(variable->getOffset())));
            args.previous_segment->add(Instruction::WithType(Bytecode::Add, IR::ptr));
            //*args.in_terminal_list=(variable->getPrototype()->as_shared<Domain>());
        }
        else{
            //*args.in_terminal_list=(target->as_shared<Domain>());
        }

        if(args.is_last_terminal){
            auto field = target->as_shared<Variable>();
            auto data = convertSymbolToDataKind(field);
            if(!args.reserve_address)
                args.previous_segment->add(Instruction::WithType(Bytecode::Load, data));
            return DataSizeVariant(data);
        }
    }


    std::any IRGen::visitArg(ast::expr::Callee::Argument *arg_node, IRGenArgs args) {
        /*
          *   Param\Arg          ByVal                  ByRef            Undefined
          *   ByVal      Yes,allow implicit conversion  Error      Yes,allow implicit conversion
          *   ByRef      store value to tmp address,                   Error when arg
          *              allow implicit conversion      Yes             is not lvalue
          */
        auto &param = args.function->getArgsSignature()[args.current_args_index];
        auto arg_type = arg_node->expr->type;
        if(param->isByval()){
            switch (arg_node->pass_kind) {
                case ast::expr::Callee::Argument::undefined:
                case ast::expr::Callee::Argument::byval:
                    visitExpression(arg_node->expr,args);
                    break;
            }
        }
        else{
            switch (arg_node->pass_kind) {
                case ast::expr::Callee::Argument::byval:
                    visitExpression(arg_node->expr,args);
                    switch (param->getPrototype()->getKind()) {
                        case DeclarationEnum::Type:
                        case DeclarationEnum::Array:
                            args.previous_segment->add(Instruction::Push(
                                    Data::ptr,new Const<data::u32>(arg_node->temp_address->getOffset())));
                            args.previous_segment->add(Instruction::StoreMemory(
                                    new Const<data::u32>(param->getRealByteLength())));
                            break;
                        case DeclarationEnum::Primitive:
                            args.previous_segment->add(Instruction::Push(
                                    Data::ptr,new Const<data::u32>(arg_node->temp_address->getOffset())));
                            args.previous_segment->add(Instruction::WithType(
                                    Bytecode::Store,
                                    arg_node->temp_address->as_shared<primitive::Primitive>()->getDataKind()
                                    ));
                            break;
                    }
                    break;
                case ast::expr::Callee::Argument::byref:
                case ast::expr::Callee::Argument::undefined:
                    args.reserve_address = true;
                    visitExpression(arg_node->expr,args);
                    break;
            }
        }
        return nullptr;
    }

    std::any IRGen::visitCast(ast::expr::Cast *cast_node, IRGenArgs args) {
        auto dst_data = any_cast<Data>(visitAnnotation(cast_node->annotation,args));
        auto src_data = any_cast<Data>(visitExpression(cast_node->expr,args));
        args.previous_segment->add(Instruction::Cast(dst_data,src_data));
        return dst_data;
    }

    std::any IRGen::visitBoolean(ast::expr::Boolean *bl_node, IRGenArgs args) {
        auto push = Instruction::Push(Data::boolean,new Const<data::boolean>(bl_node->value));
        args.previous_segment->add(push);
        return DataSizeVariant(Data::boolean);
    }

    std::any IRGen::visitChar(ast::expr::Char *ch_node, IRGenArgs args) {
        auto push = Instruction::Push(Data::i8,new Const<data::i8>(ch_node->value));
        args.previous_segment->add(push);
        return DataSizeVariant(Data::i8);
    }

    std::any IRGen::visitDigit(ast::expr::Digit *digit_node, IRGenArgs args) {
        auto push = Instruction::Push(Data::i32, new Const<data::i32>(digit_node->value));
        args.previous_segment->add(push);
        return DataSizeVariant(Data::i32);
    }

    std::any IRGen::visitDecimal(ast::expr::Decimal *decimal_node, IRGenArgs args) {
        auto push = Instruction::Push(Data::f64, new Const<data::f64>(decimal_node->value));
        args.previous_segment->add(push);
        return DataSizeVariant(Data::f64);
    }

    std::any IRGen::visitString(ast::expr::String *str_node, IRGenArgs args) {
        auto push = Instruction::PushMemory(new Const<data::u32>((data::u32)str_node->value.size()),str_node->value);
        args.previous_segment->add(push);
        return DataSizeVariant((data::u32)str_node->value.size());
    }

    std::any IRGen::visitParentheses(ast::expr::Parentheses *parentheses_node, IRGenArgs args) {
        return visitExpression(parentheses_node->expr,args);
    }


    std::any IRGen::visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, IRGenArgs args) {
        Visitor::visitExprStmt(expr_stmt_node, args);
        return (Segment*)nullptr;
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

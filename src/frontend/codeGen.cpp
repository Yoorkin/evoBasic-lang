//
// Created by yorkin on 11/2/21.
//

#include "codeGen.h"
#include <variant>
using namespace std;
namespace evoBasic{
    using namespace vm;

    ir::IRBase *convertNumberToConst(vm::Data kind,long long number){
        switch (kind.getValue()) {
            case Data::void_: ASSERT(true,"error");
            case Data::boolean: return new ir::Const<data::boolean>(number);
            case Data::i8:      return new ir::Const<data::i8>(number);
            case Data::i16:     return new ir::Const<data::i16>(number);
            case Data::i32:     return new ir::Const<data::i32>(number);
            case Data::i64:     return new ir::Const<data::i64>(number);
            case Data::f32:     return new ir::Const<data::f32>(number);
            case Data::f64:     return new ir::Const<data::f64>(number);
            case Data::u8:      return new ir::Const<data::u8>(number);
            case Data::u16:     return new ir::Const<data::u16>(number);
            case Data::u32:     return new ir::Const<data::u32>(number);
            case Data::u64:     return new ir::Const<data::u64>(number);
        }
    }
    
    shared_ptr<ir::IR> IRGen::gen(AST *ast,std::shared_ptr<Context> context) {
        auto ir = std::make_shared<ir::IR>();
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
        args.domain = args.domain->find(getID(mod_node->name))->as_shared<type::Domain>();
        for(auto& m:mod_node->member_list)
            visitMember(m,args);
        return nullptr;
    }

    std::any IRGen::visitClass(ast::Class *cls_node, IRGenArgs args) {
        list<ir::Pair*> members;
        auto ty = args.domain->find(getID(cls_node->name))->as_shared<type::Record>();
        for(auto &p : ty->getFields()){
            auto field_type_ir = new ir::Mark(p->getPrototype()->mangling(),false,false);
            members.push_back(new ir::Pair(p->getName(),field_type_ir));
        }
        auto record_ir = new ir::Record(members);
        args.previous_segment->add(new ir::Pair(ty->mangling(), record_ir));
        return nullptr;
    }


    std::any IRGen::visitType(ast::Type *ty_node, IRGenArgs args) {
        list<ir::Pair*> members;
        auto ty = args.domain->find(getID(ty_node->name))->as_shared<type::Record>();
        for(auto &p : ty->getFields()){
            auto field_type_ir = new ir::Mark(p->getPrototype()->mangling(),false,false);
            members.push_back(new ir::Pair(p->getName(),field_type_ir));
        }
        auto record_ir = new ir::Record(members);
        args.previous_segment->add(new ir::Pair(ty->mangling(), record_ir));
        return nullptr;
    }

    std::any IRGen::visitEnum(ast::Enum *em_node, IRGenArgs args) {
        auto name = getID(em_node->name);
        auto em = args.domain->find(name)->as_shared<type::Enumeration>();
        list<pair<string,int>> members;
        for(auto p = em->begin();p!=em->end();p++){
            auto key = p->getName();
            auto value = p->as_shared<type::EnumMember>()->getIndex();
            members.emplace_back(key,value);
        }
        auto enum_ir = new ir::Enum(members);
        args.previous_segment->add(new ir::Pair(em->mangling(), enum_ir));
        return nullptr;
    }

    std::any IRGen::visitDim(ast::Dim *dim_node, IRGenArgs args) {
        if(args.domain->getKind() == type::DeclarationEnum::Module){
            for(auto &var:dim_node->variable_list)
                visitVariable(var,args);
        }
        return nullptr;
    }

    std::any IRGen::visitVariable(ast::Variable *var_node, IRGenArgs args) {
        auto name = getID(var_node->name);
        auto field = args.domain->find(name)->as_shared<type::Variable>();
        auto type_ref = new ir::Mark(field->getPrototype()->mangling(),false,false);
        args.previous_segment->add(new ir::Pair(field->mangling(), type_ref));
        return nullptr;
    }


    std::any IRGen::visitFunction(ast::Function *func_node, IRGenArgs args) {
        auto func = args.domain->find(getID(func_node->name))->as_shared<type::Function>();
        args.function = func;
        list<ir::Pair*> params;
        ir::Mark *ret_mark;
        for(auto &param : func->getArgsSignature()){
            auto isArray = false,isRef = !param->isByval();
            string name;
            if(param->getPrototype()->getKind() == type::DeclarationEnum::Array){
                isArray = true;
                name = param->getPrototype()->as_shared<type::Array>()->getElementPrototype()->mangling();
            }
            else{
                isArray = false;
                name = param->getPrototype()->mangling();
            }
            auto mark = new ir::Mark(name,isRef,isArray);
            params.push_back(new ir::Pair(param->getName(),mark));
        }

        if(func->getRetSignature()){
            bool isArray;
            string name;
            if(func->getRetSignature()->getKind() == type::DeclarationEnum::Array){
                isArray = true;
                name = func->getRetSignature()->as_shared<type::Array>()->getElementPrototype()->mangling();
            }
            else{
                isArray = false;
                name = func->getRetSignature()->mangling();
            }
            ret_mark = new ir::Mark(name,false,isArray);
        }



        args.domain = func;
        //create function segment
        auto function_segment = args.ir->createSegment(args.domain->mangling());
        auto func_ir = new ir::Function(params,ret_mark,function_segment);
        args.previous_segment->add(new ir::Pair(func->mangling(), func_ir));
        args.ir->add(function_segment);

        args.previous_segment = function_segment;
        auto after_segment = visitStatementList(func_node->statement_list,args);
        after_segment->add(ir::Instruction::WithoutType(Bytecode::Ret));
        return nullptr;
    }


    ir::Segment *IRGen::visitStatementList(std::list<ast::stmt::Statement*> &stmt_list, IRGenArgs args) {
        for(auto& s:stmt_list){
            auto *after_segment = any_cast<ir::Segment*>(visitStatement(s,args));
            if(after_segment)args.previous_segment = after_segment;
        }
        return args.previous_segment;
    }

    std::any IRGen::visitLet(ast::stmt::Let *let_node, IRGenArgs args) {
        for(auto &var : let_node->variable_list){
            auto tmp = args.domain->find(getID(var->name));
            auto field = tmp->as_shared<type::Variable>();
            auto offset = field->getOffset();
            args.previous_segment->add(ir::Instruction::WithoutType(Bytecode::PushFrameBase))
                                 ->add(ir::Instruction::Push(ir::IR::ptr,new ir::Const<data::u32>(field->getOffset())))
                                 ->add(ir::Instruction::WithType(Bytecode::Add, ir::IR::ptr));

            auto field_kind = field->getPrototype()->getKind();
            if(field_kind == type::DeclarationEnum::Primitive){
                auto primitive_type = field->getPrototype()->as_shared<type::primitive::Primitive>();
                if(var->initial) {
                    visitExpression(var->initial, args);
                }
                else {
                    auto data =  primitive_type->getDataKind();
                    args.previous_segment->add(ir::Instruction::Push(data,convertNumberToConst(data,0)));
                }
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::Store, primitive_type->getDataKind()));
            }
            else if(field_kind == type::DeclarationEnum::Type){
                //TODO
            }
            else if(field_kind == type::DeclarationEnum::Array){

            }
            else if(field_kind == type::DeclarationEnum::Class){
                //TODO
            }
        }
        return (ir::Segment*)nullptr;
    }

    std::any IRGen::visitIf(ast::stmt::If *ifstmt_node, IRGenArgs args) {
        auto after_if_segment = args.ir->createSegment(args.domain->mangling() + "_if_after");
        args.next_segment = after_if_segment;
        for(auto &ca:ifstmt_node->case_list){
            visitCase(ca,args);
        }
        args.previous_segment->add(ir::Instruction::Jmp(after_if_segment));
        args.ir->add(after_if_segment);
        return after_if_segment;
    }

    std::any IRGen::visitCase(ast::Case *ca_node, IRGenArgs args) {
        if(ca_node->condition) {
            visitExpression(ca_node->condition, args);
            auto case_segment = args.ir->createSegment(args.function->mangling() + "_case");
            args.ir->add(case_segment);
            args.previous_segment->add(ir::Instruction::Jif(case_segment));
            args.previous_segment = case_segment;
            auto after_stmts_segment = visitStatementList(ca_node->statement_list, args);
            after_stmts_segment->add(ir::Instruction::Jmp(args.next_segment));
        }
        else{
            visitStatementList(ca_node->statement_list, args);
        }
        return (ir::Segment*)nullptr;
    }

    std::any IRGen::visitLoop(ast::stmt::Loop *loop_node, IRGenArgs args) {
        auto condition_segment = args.ir->createSegment(args.function->mangling() + "_condition");
        auto loop_segment = args.ir->createSegment(args.function->mangling() + "_loop");
        auto after_loop_segment = args.ir->createSegment(args.function->mangling() + "_loop_after");

        //jump from previous segment to condition segment
        args.previous_segment->add(ir::Instruction::Jmp(condition_segment));

        //loop condition ir
        args.ir->add(condition_segment);
        args.previous_segment = condition_segment;
        visitExpression(loop_node->condition,args);
        condition_segment->add(ir::Instruction::Jif(loop_segment));
        condition_segment->add(ir::Instruction::Jmp(after_loop_segment));

        //loop body ir
        args.ir->add(loop_segment);
        args.previous_segment = loop_segment;
        args.next_segment = after_loop_segment;
        visitStatementList(loop_node->statement_list,args);
        loop_segment->add(ir::Instruction::Jmp(condition_segment));

        args.ir->add(after_loop_segment);
        return after_loop_segment;
    }

    std::any IRGen::visitSelect(ast::stmt::Select *select_node, IRGenArgs args) {
        auto after_select_segment = args.ir->createSegment(args.domain->mangling() + "_select_after");
        args.next_segment = after_select_segment;
        for(auto &ca:select_node->case_list){
            visitCase(ca,args);
        }
        args.previous_segment->add(ir::Instruction::Jmp(after_select_segment));
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
        auto data = any_cast<vm::Data>(visitExpression(forstmt_node->begin,args));
        //store value in address
        args.previous_segment->add(ir::Instruction::WithType(Bytecode::Store, data));
        //jump from previous previous_segment to looping segment
        args.previous_segment->add(ir::Instruction::Jmp(loop_segment));
        //generate ir in loop previous_segment


        args.ir->add(condition_segment);
        //generate condition previous_segment ir
        // increase iterator
        args.previous_segment = condition_segment;
        visitExpression(forstmt_node->iterator,args);
        if(forstmt_node->step)
            visitExpression(forstmt_node->step,args);
        else
            condition_segment->add(ir::Instruction::Push(data, convertNumberToConst(data,1)));
        args.previous_segment->add(ir::Instruction::WithType(Bytecode::Add, data));
        //duplicate increased value and store in iterator address
        args.previous_segment->add(ir::Instruction::WithType(Bytecode::Dup, data));
        args.reserve_address = true;
        visitExpression(forstmt_node->iterator,args);
        args.reserve_address = false;
        args.previous_segment->add(ir::Instruction::WithType(Bytecode::Store, data));
        //load end expression value
        visitExpression(forstmt_node->end,args);
        //compare iterator and end,if less than or equal the end value,jump to loop previous_segment
        args.previous_segment->add(ir::Instruction::WithType(Bytecode::LE, data));
        args.previous_segment->add(ir::Instruction::Jif(loop_segment));
        //otherwise jump to after loop segment
        args.previous_segment->add(ir::Instruction::Jmp(after_loop_segment));



        args.ir->add(loop_segment);
        args.previous_segment = loop_segment;
        args.next_segment = after_loop_segment;
        args.continue_segment = condition_segment;
        auto after_stmts_segment = visitStatementList(forstmt_node->statement_list,args);
        //jump from loop end to condition previous_segment
        after_stmts_segment->add(ir::Instruction::Jmp(condition_segment));

        args.ir->add(after_loop_segment);
        return after_loop_segment;
    }

    std::any IRGen::visitContinue(ast::stmt::Continue *cont_node, IRGenArgs args) {
        args.previous_segment->add(ir::Instruction::Jmp(args.continue_segment));
        return (ir::Segment*)nullptr;
    }

    std::any IRGen::visitReturn(ast::stmt::Return *ret_node, IRGenArgs args) {
        args.need_dup_in_assignment = true;
        visitExpression(ret_node->expr,args);
        args.previous_segment->add(ir::Instruction::WithoutType(Bytecode::Ret));
        return (ir::Segment*)nullptr;
    }

    std::any IRGen::visitExit(ast::stmt::Exit *exit_node, IRGenArgs args) {
        switch (exit_node->exit_flag) {
            case ast::stmt::Exit::For:
            case ast::stmt::Exit::While:
                args.previous_segment->add(ir::Instruction::Jmp(args.next_segment));
                break;
            case ast::stmt::Exit::Sub:
                args.previous_segment->add(ir::Instruction::WithoutType(Bytecode::Ret));
                break;
        }
        return (ir::Segment*)nullptr;
    }

    std::any IRGen::visitBinary(ast::expr::Binary *logic_node, IRGenArgs args) {
        IRGenArgs new_args = args;
        if(logic_node->op == ast::expr::Binary::ASSIGN){
            new_args.reserve_address = true;
            new_args.need_dup_in_assignment = true;
        }
        auto lhs_data = any_cast<vm::Data>(visitExpression(logic_node->lhs,new_args));
        new_args.reserve_address = false;
        visitExpression(logic_node->rhs,new_args);

        switch (logic_node->op) {
            case ast::expr::Binary::And:
                args.previous_segment->add(ir::Instruction::WithoutType(Bytecode::And));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::Or:
                args.previous_segment->add(ir::Instruction::WithoutType(Bytecode::Or));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::Xor:
                args.previous_segment->add(ir::Instruction::WithoutType(Bytecode::Xor));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::Not:
                args.previous_segment->add(ir::Instruction::WithoutType(Bytecode::Not));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::EQ:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::EQ, lhs_data));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::NE:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::NE, lhs_data));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::GE:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::GE, lhs_data));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::LE:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::LE, lhs_data));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::GT:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::GT, lhs_data));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::LT:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::LT, lhs_data));
                return DataSizeVariant(vm::Data::boolean);
            case ast::expr::Binary::ADD:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::Add, lhs_data));
                return DataSizeVariant(lhs_data);
            case ast::expr::Binary::MINUS:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::Sub, lhs_data));
                return DataSizeVariant(lhs_data);
            case ast::expr::Binary::MUL:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::Mul, lhs_data));
                return DataSizeVariant(lhs_data);
            case ast::expr::Binary::DIV:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::Div, lhs_data));
                return DataSizeVariant(lhs_data);
            case ast::expr::Binary::FDIV:
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::FDiv, lhs_data));
                return DataSizeVariant(lhs_data);//TODO FIDV support
            case ast::expr::Binary::ASSIGN:
                if(args.need_dup_in_assignment){
                    args.previous_segment->add(ir::Instruction::WithType(Bytecode::Dup, lhs_data));
                }
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::Store, lhs_data));
                return DataSizeVariant(lhs_data);
            case ast::expr::Binary::Empty:
                ASSERT(true,"unexpected instruction");
                break;
        }
    }

    std::any IRGen::visitUnary(ast::expr::Unary *unit_node, IRGenArgs args) {
        auto data_size_variant = any_cast<DataSizeVariant>(visitExpression(unit_node->terminal,args));
        args.previous_segment->add(ir::Instruction::WithType(Bytecode::Neg, get<0>(data_size_variant)));
        return data_size_variant;
    }

//    std::any IRGen::visitLink(ast::expr::Link *link_node, IRGenArgs args) {
//        DataSizeVariant link_data_size_variant(vm::Data::void_);
//        shared_ptr<type::Domain> iterator = args.domain;
//        args.in_terminal_list = &iterator;
//        for(auto &ter:link_node->terminal_list){
//            switch (ter->terminal_kind) {
//                case ast::expr::Expression::parentheses_:
//                case ast::expr::Expression::digit_:
//                case ast::expr::Expression::decimal_:
//                case ast::expr::Expression::string_:
//                case ast::expr::Expression::char_:
//                case ast::expr::Expression::boolean_:
//                    if(&ter != &link_node->terminal_list.front()) ASSERT(true,"invalid link");
//                    link_data_size_variant = any_cast<DataSizeVariant>(visitTerminal(ter,args));
//                    break;
//                case ast::expr::Expression::callee_:
//                    args.need_lookup = (&ter == &link_node->terminal_list.front());
//                    args.is_last_terminal = (&ter == &link_node->terminal_list.back());
//                    link_data_size_variant = any_cast<DataSizeVariant>(visitTerminal(ter,args));
//                    break;
//            }
//        }
//        args.in_terminal_list = nullptr;
//        return link_data_size_variant;
//    }

    DataSizeVariant IRGen::visitFunctionCall(ast::expr::Callee *callee_node,IRGenArgs args,shared_ptr<type::Function> target){
        args.function = target->as_shared<type::Function>();
        //visitArgsList(callee_node->args,args);
        args.previous_segment->add(ir::Instruction::Invoke(target->mangling()));
        return DataSizeVariant(convertSymbolToDataKind(callee_node->type->prototype));
    }

    DataSizeVariant IRGen::visitVariableCall(ast::expr::Callee *callee_node,IRGenArgs args,shared_ptr<type::Variable> target){
        if(target->getKind() == type::DeclarationEnum::Variable || target->getKind() == type::DeclarationEnum::Argument){
            auto variable = target->as_shared<evoBasic::type::Variable>();
            auto base = variable->isGlobal() ? Bytecode::PushGlobalBase : Bytecode::PushFrameBase;
            args.previous_segment->add(ir::Instruction::WithoutType(base));
            args.previous_segment->add(ir::Instruction::Push(ir::IR::ptr,new ir::Const<data::u32>(variable->getOffset())));
            args.previous_segment->add(ir::Instruction::WithType(Bytecode::Add, ir::IR::ptr));
            *args.in_terminal_list=(variable->getPrototype()->as_shared<type::Domain>());
        }
        else{
            *args.in_terminal_list=(target->as_shared<type::Domain>());
        }

        if(args.is_last_terminal){
            auto field = target->as_shared<type::Variable>();
            auto data = convertSymbolToDataKind(field);
            if(!args.reserve_address)
                args.previous_segment->add(ir::Instruction::WithType(Bytecode::Load, data));
            return DataSizeVariant(data);
        }
    }

    std::any IRGen::visitCallee(ast::expr::Callee *callee_node, IRGenArgs args) {
        NotNull(args.in_terminal_list);
        NotNull(args.in_terminal_list->get());
        shared_ptr<type::Symbol> target;
        if(args.need_lookup)
            target = (*args.in_terminal_list)->lookUp(getID(callee_node->name));
        else
            target = (*args.in_terminal_list)->find(getID(callee_node->name));
        NotNull(target.get());

//        if(callee_node->args){
//            //function call
//            return visitFunctionCall(callee_node,args,target->as_shared<type::Function>());
//        }
//        else if(target->getKind() == type::DeclarationEnum::Variable){
//            //variable
//            return visitVariableCall(callee_node,args,target->as_shared<type::Variable>());
//        }
//        else {
//            (*args.in_terminal_list) = target->as_shared<type::Domain>();
//            return DataSizeVariant(vm::Data::void_);
//        }
    }
//
//    std::any IRGen::visitArgsList(ast::expr::ArgsList *args_list_node, IRGenArgs args) {
//        args.current_args_index = 0;
//        for(auto &arg : args_list_node->arg_list){
//            visitArg(arg,args);
//            args.current_args_index++;
//        }
//        return nullptr;
//    }

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
                        case type::DeclarationEnum::Type:
                        case type::DeclarationEnum::Array:
                            args.previous_segment->add(ir::Instruction::Push(
                                    vm::Data::ptr,new ir::Const<data::u32>(arg_node->temp_address->getOffset())));
                            args.previous_segment->add(ir::Instruction::StoreMemory(
                                    new ir::Const<data::u32>(param->getRealByteLength())));
                            break;
                        case type::DeclarationEnum::Primitive:
                            args.previous_segment->add(ir::Instruction::Push(
                                    vm::Data::ptr,new ir::Const<data::u32>(arg_node->temp_address->getOffset())));
                            args.previous_segment->add(ir::Instruction::WithType(
                                    Bytecode::Store,
                                    arg_node->temp_address->as_shared<type::primitive::Primitive>()->getDataKind()
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


//    std::any IRGen::visitCast(ast::expr::Cast *cast_node, IRGenArgs args) {
//        auto dst_data = any_cast<vm::Data>(visitAnnotation(cast_node->dst,args));
//        auto src_data = any_cast<vm::Data>(visitExpression(cast_node->src,args));
//        args.previous_segment->add(ir::Instruction::Cast(dst_data,src_data));
//        return dst_data;
//    }

    std::any IRGen::visitBoolean(ast::expr::Boolean *bl_node, IRGenArgs args) {
        auto push = ir::Instruction::Push(vm::Data::boolean,new ir::Const<data::boolean>(bl_node->value));
        args.previous_segment->add(push);
        return DataSizeVariant(vm::Data::boolean);
    }

    std::any IRGen::visitChar(ast::expr::Char *ch_node, IRGenArgs args) {
        auto push = ir::Instruction::Push(vm::Data::i8,new ir::Const<data::i8>(ch_node->value));
        args.previous_segment->add(push);
        return DataSizeVariant(vm::Data::i8);
    }

    std::any IRGen::visitDigit(ast::expr::Digit *digit_node, IRGenArgs args) {
        auto push = ir::Instruction::Push(vm::Data::i32, new ir::Const<data::i32>(digit_node->value));
        args.previous_segment->add(push);
        return DataSizeVariant(vm::Data::i32);
    }

    std::any IRGen::visitDecimal(ast::expr::Decimal *decimal_node, IRGenArgs args) {
        auto push = ir::Instruction::Push(vm::Data::f64, new ir::Const<data::f64>(decimal_node->value));
        args.previous_segment->add(push);
        return DataSizeVariant(vm::Data::f64);
    }

    std::any IRGen::visitString(ast::expr::String *str_node, IRGenArgs args) {
        auto push = ir::Instruction::PushMemory(new ir::Const<data::u32>((data::u32)str_node->value.size()),str_node->value);
        args.previous_segment->add(push);
        return DataSizeVariant((data::u32)str_node->value.size());
    }

    std::any IRGen::visitParentheses(ast::expr::Parentheses *parentheses_node, IRGenArgs args) {
        return visitExpression(parentheses_node->expr,args);
    }

    vm::Data IRGen::convertSymbolToDataKind(std::shared_ptr<type::Symbol> symbol) {
        NotNull(symbol.get());
        auto ptr = vm::Data::u32;
        switch (symbol->getKind()) {
            case type::DeclarationEnum::Variant:
            case type::DeclarationEnum::Class:
                return vm::Data::u32;
            case type::DeclarationEnum::Enum_:
                return vm::Data::u32;
            case type::DeclarationEnum::Array:
            case type::DeclarationEnum::Type:
            case type::DeclarationEnum::Function:
                return ptr;
            case type::DeclarationEnum::Argument:
            case type::DeclarationEnum::Variable:
                return convertSymbolToDataKind(symbol->as_shared<type::Variable>()->getPrototype());
            case type::DeclarationEnum::Primitive:
                return (symbol->as_shared<type::primitive::Primitive>()->getDataKind());
            case type::DeclarationEnum::Interface:
            case type::DeclarationEnum::Error:
            case type::DeclarationEnum::TmpDomain:
            case type::DeclarationEnum::EnumMember:
            case type::DeclarationEnum::Module:
                ASSERT(true,"error");
        }
    }

    std::any IRGen::visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, IRGenArgs args) {
        Visitor::visitExprStmt(expr_stmt_node, args);
        return (ir::Segment*)nullptr;
    }
//
//    std::any IRGen::visitAnnotation(ast::Annotation *anno_node, IRGenArgs args) {
//        if(anno_node->array_size)return vm::Data(vm::Data::u32);
//
//        shared_ptr<type::Symbol> target = args.domain;
//        for(auto &unit:anno_node->unit_list){
//            auto name = getID(unit->name);
//            auto domain = target->as_shared<type::Domain>();
//            NotNull(domain.get());
//            if(&unit == &anno_node->unit_list.front()){
//                target = domain->lookUp(name);
//            }
//            else{
//                target = domain->find(name);
//            }
//            NotNull(target.get());
//        }
//
//        switch (target->getKind()) {
//            case type::DeclarationEnum::Class:
//            case type::DeclarationEnum::Type:
//            case type::DeclarationEnum::Function:
//            case type::DeclarationEnum::Interface:
//            case type::DeclarationEnum::Array:
//            case type::DeclarationEnum::Enum_:
//                return vm::Data(vm::Data::u32);
//            case type::DeclarationEnum::Primitive:
//                return target->as_shared<type::primitive::Primitive>()->getDataKind();
//            case type::DeclarationEnum::Argument:
//            case type::DeclarationEnum::TmpDomain:
//            case type::DeclarationEnum::EnumMember:
//            case type::DeclarationEnum::Module:
//            case type::DeclarationEnum::Variable:
//            case type::DeclarationEnum::Error:
//            case type::DeclarationEnum::Variant:
//                ASSERT(true,"invalid annotation");
//        }
//    }
}

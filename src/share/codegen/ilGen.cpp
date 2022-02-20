//
// Created by yorkin on 12/13/21.
//

#include "ilGen.h"
#include<variant>
#include<execution/intrinsic.h>

namespace evoBasic{
    using namespace il;
    using namespace std;

    DataType ILGen::mapILType(type::Prototype *type){
        switch(type->getKind()){
            case type::SymbolKind::Record:      return {DataTypeEnum::record,document->getTokenRef(type->getFullName())};
            case type::SymbolKind::Array:       return {DataTypeEnum::array,document->getTokenRef(type->getFullName())};
            case type::SymbolKind::Class:       return {DataTypeEnum::ref};
            case type::SymbolKind::Enum:        return {DataTypeEnum::u32};
            case type::SymbolKind::Function:    return {DataTypeEnum::delegate};
            case type::SymbolKind::Primitive:{
                auto primitive = type->as<type::Primitive*>();
                switch(primitive->getDataKind().getValue()){
                    case vm::Data::i8:      return {DataTypeEnum::i8};
                    case vm::Data::i16:     return {DataTypeEnum::i16};
                    case vm::Data::i32:     return {DataTypeEnum::i32};
                    case vm::Data::i64:     return {DataTypeEnum::i64};
                    case vm::Data::u8:      return {DataTypeEnum::u8};
                    case vm::Data::u16:     return {DataTypeEnum::u16};
                    case vm::Data::u32:     return {DataTypeEnum::u32};
                    case vm::Data::u64:     return {DataTypeEnum::u64};
                    case vm::Data::f32:     return {DataTypeEnum::f32};
                    case vm::Data::f64:     return {DataTypeEnum::f64};
                    case vm::Data::boolean: return {DataTypeEnum::boolean};
                    default: PANIC;
                }
            }
            default: PANIC;
        }
    }

    DataType ILGen::mapILType(ExpressionType *type) {
        return mapILType(type->getPrototype());
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
        auto type = document->getTokenRef(variable->getPrototype()->getFullName());
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
            auto block = new il::BasicBlock(document,"optional");
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
        auto entry = new il::BasicBlock(document,"function");
        blocks.clear();
        blocks.push_back(entry);
        auto symbol = function_node->function_symbol->as<type::UserFunction*>();
        auto tail_block = visitStatement(function_node->statement,entry,{});
        tail_block->Ret();

        auto parameter = visitParameter(symbol);
        Result *result = nullptr;
        if(symbol->getRetSignature()){
            auto type = document->getTokenRef(symbol->getRetSignature()->getFullName());
            result = new Result(document,type);
        }

        list<il::Local*> locals;
        for(int i=parameter.size();i<symbol->getMemoryLayout().size();i++){
            auto &variable = symbol->getMemoryLayout()[i];
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
            default:
                PANIC;
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
        auto entry = new il::BasicBlock(document,"ctor");
        blocks.clear();
        blocks.push_back(entry);
        auto parameter = visitParameter(ctor_node->constructor_symbol);
        auto tail_block = visitStatement(ctor_node->statement,entry,{});
        tail_block->Ret();

        list<il::Local*> locals;
        for(int i=parameter.size();i<ctor_node->constructor_symbol->getMemoryLayout().size();i++){
            auto &variable = ctor_node->constructor_symbol->getMemoryLayout()[i];
            auto name = document->getTokenRef(variable->getName());
            auto type = document->getTokenRef(variable->getPrototype()->getFullName());
            auto local = new Local(document,name,type,locals.size());
            locals.push_back(local);
        }
        return new Ctor(document,parameter,locals,blocks);
    }


    il::BasicBlock *ILGen::visitStatement(ast::Statement *statement_node, il::BasicBlock *current, JumpOption jump_option) {
        FOR_EACH(iter,statement_node){
            switch(iter->stmt_flag){
                case ast::Statement::let_:
                    visitLet((ast::Let*)iter,current);
                    break;
                case ast::Statement::loop_:
                    current = visitLoop((ast::Loop*)iter,current,jump_option);
                    break;
                case ast::Statement::if_:
                    current = visitIf((ast::If*)iter,current,jump_option);
                    break;
                case ast::Statement::for_:
                    current = visitFor((ast::For*)iter,current,jump_option);
                    break;
                case ast::Statement::select_:
                    current = visitSelect((ast::Select*)iter,current,jump_option);
                    break;
                case ast::Statement::return_:
                    visitReturn((ast::Return*)iter,current);
                    break;
                case ast::Statement::continue_:
                    visitContinue((ast::Continue*)iter,current,jump_option);
                    break;
                case ast::Statement::exit_:
                    visitExit((ast::Exit*)iter,current,jump_option);
                    break;
                case ast::Statement::expr_:
                    visitExprStmt((ast::ExprStmt*)iter,current);
                    break;
                default:
                    PANIC;
                    break;
            }
        }
        return current;
    }

    void ILGen::visitLet(ast::Let *let_node, il::BasicBlock *current) {
        FOR_EACH(iter,let_node->variable){
            if(iter->initial){
                auto data_type = mapILType(iter->variable_symbol->getPrototype());
                loadExpressionValue(iter->initial,current);
                current->Push(DataTypeEnum::u16, (data::u16)iter->variable_symbol->getLayoutIndex());
                current->Stloc(data_type);
            }
        }
    }

    il::BasicBlock *ILGen::visitSelect(ast::Select *select_node, il::BasicBlock *current, JumpOption jump_option) {
        //todo
        return current;
    }

    il::BasicBlock *ILGen::visitLoop(ast::Loop *loop_node, il::BasicBlock *current, JumpOption jump_option) {
        auto cond_block = new il::BasicBlock(document,"while_cond"),
             loop_block = new il::BasicBlock(document,"while_loop"),
             after_block = new il::BasicBlock(document,"while_succ");

        blocks.push_back(cond_block);
        blocks.push_back(loop_block);
        blocks.push_back(after_block);

        current->Br(cond_block);

        visitExpression(loop_node->condition,cond_block);
        cond_block->Jif(loop_block);
        cond_block->Br(after_block);

        auto loop_tail_block = visitStatement(loop_node->statement,loop_block,{cond_block,jump_option.after_for_block,after_block});
        loop_tail_block->Br(cond_block);

        return after_block;
    }

    il::BasicBlock *ILGen::visitIf(ast::If *if_node, il::BasicBlock *current, JumpOption jump_option) {
        return visitCase(if_node->case_,current,jump_option);
    }

    il::BasicBlock *ILGen::visitCase(ast::Case *case_node, il::BasicBlock *current, JumpOption jump_option) {
        auto after_block = new il::BasicBlock(document,"case_succ");
        blocks.push_back(after_block);

        map<ast::Case*,il::BasicBlock*> case_blocks;
        bool has_else_case = false;
        FOR_EACH(iter,case_node){
            if(iter->condition){
                auto bl = new il::BasicBlock(document,"case");
                blocks.push_back(bl);
                case_blocks.insert({iter,bl});
            }
            else{
                has_else_case = true;
            }
        }

        FOR_EACH(iter,case_node){
            auto case_block = case_blocks.find(iter);
            if(case_block != case_blocks.end()){
                loadExpressionValue(iter->condition,current);
                current->Jif(case_block->second);
                auto case_tail_block = visitStatement(iter->statement,case_block->second,jump_option);
                case_tail_block->Br(after_block);
            }
            else{
                auto case_tail_block = visitStatement(iter->statement,current,jump_option);
                case_tail_block->Br(after_block);
            }
        }

        if(!has_else_case){
            current->Br(after_block);
        }

        return after_block;
    }

    il::BasicBlock *ILGen::visitFor(ast::For *for_node, il::BasicBlock *current, JumpOption jump_option) {

        loadExpressionValue(for_node->begin,current);
        switch(for_node->iterator->expression_kind){
            case ast::Expression::SFld:{
                auto sfld = (ast::SFld*)for_node->iterator;
                visitSFld(sfld,current);
                current->Stsfld(mapILType(sfld->type),getTokenRef(sfld));
                break;
            }
            case ast::Expression::Fld:{
                auto fld = (ast::Fld*)for_node->iterator;
                visitFld(fld,current);
                current->Stfld(mapILType(fld->type), getTokenRef(fld));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = (ast::Arg*)for_node->iterator;
                visitArg(arg,current);
                current->Starg(mapILType(arg->type));
                break;
            }
            case ast::Expression::Local:{
                auto local = (ast::Local*)for_node->iterator;
                visitLocal(local,current);
                current->Stloc(mapILType(local->type));
                break;
            }
            case ast::Expression::Element:{
                auto element = (ast::ArrayElement*)for_node->iterator;
                visitArrayElement(element,current);
                current->Stelem(mapILType(element->type));
                break;
            }
            default: PANIC;
        }

        auto iter_il_type = mapILType(for_node->begin->type);

        // store value of begin/end/step expression into begin/end/step variable
        loadExpressionValue(for_node->begin,current);
        current->Push(DataTypeEnum::u16, (data::u16)for_node->begin_variable->getLayoutIndex());
        current->Stloc(iter_il_type);

        loadExpressionValue(for_node->end,current);
        current->Push(DataTypeEnum::u16, (data::u16)for_node->end_variable->getLayoutIndex());
        current->Stloc(iter_il_type);

        if(for_node->step){
            visitExpression(for_node->step,current);
            current->Push(DataTypeEnum::u16, (data::u16)for_node->step_variable->getLayoutIndex());
            current->Stloc(iter_il_type);
        }

        auto stmt_block = new il::BasicBlock(document,"for_loop");
        auto cond_block = new il::BasicBlock(document,"for_cond");
        auto after_block = new il::BasicBlock(document,"for_succ");

        blocks.push_back(stmt_block);
        blocks.push_back(cond_block);
        blocks.push_back(after_block);

        current->Br(stmt_block);

        auto for_tail_block = visitStatement(for_node->statement,stmt_block,{cond_block,after_block,jump_option.after_loop_block});
        for_tail_block->Br(cond_block);

        /*
         * iter = iter + step
         */
        loadExpressionValue(for_node->iterator,cond_block);
        cond_block->Push(DataTypeEnum::u16, (data::u16)for_node->step_variable->getLayoutIndex())
                        .Ldloc(iter_il_type)
                        .Add(iter_il_type);

        switch(for_node->iterator->expression_kind){
            case ast::Expression::SFld:{
                auto sfld = (ast::SFld*)for_node->iterator;
                visitSFld(sfld,cond_block);
                cond_block->Stsfld(iter_il_type, getTokenRef(sfld));
                break;
            }
            case ast::Expression::Fld:{
                auto fld = (ast::Fld*)for_node->iterator;
                visitFld(fld,cond_block);
                cond_block->Stfld(iter_il_type, getTokenRef(fld));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = (ast::Arg*)for_node->iterator;
                visitArg(arg,cond_block);
                cond_block->Starg(iter_il_type);//todo fix bug
                break;
            }
            case ast::Expression::Local:{
                auto local = (ast::Local*)for_node->iterator;
                visitLocal(local,cond_block);
                cond_block->Stloc(iter_il_type);
                break;
            }
            case ast::Expression::Element:{
                auto element = (ast::ArrayElement*)for_node->iterator;
                visitArrayElement(element,cond_block);
                cond_block->Stelem(iter_il_type);
                break;
            }
            default: PANIC;
        }

        /*
         *  beg < end & (iter < beg | iter > end) ||
         *  beg > end & (iter < end | iter > beg) ||
         *  beg == end & iter != beg
         */
        // store value of begin/end/step expression into begin/end/step variable
        loadExpressionValue(for_node->iterator,cond_block);
        cond_block->Push(DataTypeEnum::u16, (data::u16)for_node->begin_variable->getLayoutIndex())
                .Ldloc(iter_il_type);
        cond_block->Push(DataTypeEnum::u16, (data::u16)for_node->end_variable->getLayoutIndex())
                .Ldloc(iter_il_type);
        cond_block->Intrinsic(vm::IntrinsicEnum::ItNotInRange)
                   .Jif(after_block);

        cond_block->Br(stmt_block);

        return after_block;
    }

    void ILGen::visitExprStmt(ast::ExprStmt *expr_stmt_node, il::BasicBlock *current) {
        visitExpression(expr_stmt_node->expr,current);
        // todo: clean operand stack
    }

    void ILGen::visitExit(ast::Exit *exit_node, il::BasicBlock *current, JumpOption jump_option) {
        switch(exit_node->exit_flag){
            case ast::Exit::For:
                NotNull(jump_option.after_for_block);
                current->Br(jump_option.after_for_block);
                break;
            case ast::Exit::While:
                NotNull(jump_option.after_loop_block);
                current->Br(jump_option.after_loop_block);
                break;
            case ast::Exit::Sub:
                current->Ret();
                break;
        }
    }

    void ILGen::visitContinue(ast::Continue *continue_node, il::BasicBlock *current, JumpOption jump_option) {
        current->Br(jump_option.looping_condition_block);
    }

    void ILGen::visitReturn(ast::Return *return_node, il::BasicBlock *current) {
        loadExpressionValue(return_node->expr,current);
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
        current->Push(DataTypeEnum::i32, enum_member_node->member->getIndex());
    }

    void ILGen::loadExpressionValue(ast::Expression *terminal, il::BasicBlock *current) {
        switch(terminal->expression_kind){
            case ast::Expression::Binary:
            case ast::Expression::Cast:
            case ast::Expression::Parentheses:
            case ast::Expression::Ftn:
            case ast::Expression::VFtn:
            case ast::Expression::SFtn:
            case ast::Expression::Ext:
            case ast::Expression::Digit:
            case ast::Expression::Decimal:
            case ast::Expression::Char:
            case ast::Expression::Boolean:
                visitExpression(terminal,current);
                break;
            case ast::Expression::Assign:{
                auto assign = static_cast<ast::Assign*>(terminal);
                visitAssign(assign,current);
                loadAssignLhsValue(assign,current);
                break;
            }
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(terminal);
                visitArrayElement(element,current);
                current->Ldelem(mapILType(element->type));
                break;
            }
            case ast::Expression::Local:{
                auto local = static_cast<ast::Local*>(terminal);
                visitLocal(local,current);
                current->Ldloc(mapILType(local->type));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(terminal);
                visitArg(arg,current);
                if(arg->is_ref){
                    current->Ldarg(DataTypeEnum::ref)
                            .Load(mapILType(arg->type));
                }
                else{
                    current->Ldarg(mapILType(arg->type));
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(terminal);
                visitFld(fld,current);
                current->Ldfld(mapILType(fld->type), getTokenRef(fld));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(terminal);
                visitSFld(sfld,current);
                current->Ldsfld(mapILType(sfld->type), getTokenRef(sfld));
                break;
            }
        }
    }

    void ILGen::visitUnary(ast::Unary *unary_node, il::BasicBlock *current) {
        loadExpressionValue(unary_node->terminal, current);
        auto il_type = mapILType(unary_node->type->getPrototype());
        using Op = ast::Unary::Op;
        switch(unary_node->op){
            case Op::ADD:   /* do nothing */ break;
            case Op::MINUS: current->Neg(il_type); break;
            case Op::Empty: PANIC;
        }
    }

    void ILGen::visitBinary(ast::Binary *binary_node, il::BasicBlock *current) {
        loadExpressionValue(binary_node->lhs, current);
        loadExpressionValue(binary_node->rhs, current);
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

    TokenRef *ILGen::getArrayElementTokenRef(ast::ArrayElement *element_node){
        return document->getTokenRef(element_node->array->type->getPrototype()->getFullName());
    }
    
    void ILGen::loadAssignLhsValue(ast::Assign *assign_node, il::BasicBlock *current){
        switch (assign_node->lhs->expression_kind) {
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(assign_node->lhs);
                visitArrayElement(element,current);
                auto element_token = getArrayElementTokenRef(element);
                current->Ldelem(mapILType(element->type));
                break;
            }
            case ast::Expression::Local:{
                auto local = static_cast<ast::Local*>(assign_node->lhs);
                visitLocal(local,current);
                current->Ldloc(mapILType(local->type));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(assign_node->lhs);
                auto il_type = mapILType(arg->type);
                visitArg(arg,current);
                if(arg->is_ref){
                    current->Ldarg(DataTypeEnum::ref)
                            .Load(il_type);
                }
                else{
                    current->Ldarg(il_type);
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(assign_node->lhs);
                auto fld_token = document->getTokenRef(fld->variable->getPrototype()->getFullName());
                visitFld(fld,current);
                current->Ldfld(mapILType(fld->type),fld_token);
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(assign_node->lhs);
                visitSFld(sfld,current);
                current->Ldsfld(mapILType(sfld->type), getTokenRef(sfld));
                break;
            }
        }
    }
    
    void ILGen::loadAssignLhsAddress(ast::Assign *assign_node, il::BasicBlock *current){
        switch (assign_node->lhs->expression_kind) {
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(assign_node->lhs);
                visitArrayElement(element,current);
                current->Ldelema(mapILType(element->type));
                break;
            }
            case ast::Expression::Local:{
                auto local = static_cast<ast::Local*>(assign_node->lhs);
                visitLocal(local,current);
                current->Ldloca();
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(assign_node->lhs);
                visitArg(arg,current);
                if(arg->is_ref){
                    current->Ldarg(DataTypeEnum::ref);
                }
                else{
                    current->Ldarga();
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(assign_node->lhs);
                visitFld(fld,current);
                current->Ldflda(getTokenRef(fld));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(assign_node->lhs);
                visitSFld(sfld,current);
                current->Ldsflda(getTokenRef(sfld));
                break;
            }
        }
    }
    
    void ILGen::visitAssign(ast::Assign *assign_node, il::BasicBlock *current) {
        switch(assign_node->rhs->expression_kind){
            case ast::Expression::Unary:
                visitUnary(static_cast<ast::Unary*>(assign_node->rhs),current);
                break;
            case ast::Expression::Binary:
                visitBinary(static_cast<ast::Binary*>(assign_node->rhs),current);
                break;
            case ast::Expression::Cast:
                visitCast(static_cast<ast::Cast*>(assign_node->rhs),current);
                break;
            case ast::Expression::Assign:{
                auto assign_rhs = static_cast<ast::Assign*>(assign_node->rhs);
                visitAssign(assign_rhs,current);
                loadAssignLhsValue(assign_rhs,current);
                break;
            }
            case ast::Expression::Parentheses:
                visitParentheses(static_cast<ast::Parentheses*>(assign_node->rhs),current);
                break;
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(assign_node->rhs);
                visitArrayElement(element,current);
                current->Ldelem(mapILType(element->type));
                break;
            }
            case ast::Expression::Ftn:
                visitFtnCall(static_cast<ast::FtnCall*>(assign_node->rhs),current);
                break;
            case ast::Expression::VFtn:
                visitVFtnCall(static_cast<ast::VFtnCall*>(assign_node->rhs),current);
                break;
            case ast::Expression::SFtn:
                visitSFtnCall(static_cast<ast::SFtnCall*>(assign_node->rhs),current);
                break;
            case ast::Expression::Ext:
                visitExtCall(static_cast<ast::ExtCall*>(assign_node->rhs),current);
                break;
            case ast::Expression::Local: {
                auto local = static_cast<ast::Local*>(assign_node->rhs);
                visitLocal(local,current);
                current->Ldloc(mapILType(local->type));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(assign_node->rhs);
                visitArg(arg,current);
                if(arg->is_ref){
                    current->Ldarg(DataTypeEnum::ref)
                            .Load(mapILType(arg->type));
                }
                else{
                    current->Ldarg(mapILType(arg->type));
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(assign_node->rhs);
                current->Ldfld(mapILType(fld->type), getTokenRef(fld));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(assign_node->rhs);
                visitSFld(sfld,current);
                current->Ldsfld(mapILType(sfld->type), getTokenRef(sfld));
                break;
            }
            case ast::Expression::Digit:
            case ast::Expression::Decimal:
            case ast::Expression::String:
            case ast::Expression::Char:
            case ast::Expression::Boolean:
            case ast::Expression::EnumMember:
                loadExpressionValue(assign_node->rhs,current);
                break;
        }

        TokenRef *record_or_array = nullptr;
        switch(assign_node->lhs->expression_kind){
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(assign_node->lhs);
                visitArrayElement(element,current);
                current->Stelem(mapILType(element->type));
                break;
            }
            case ast::Expression::Local:{
                auto local = static_cast<ast::Local*>(assign_node->lhs);
                visitLocal(local,current);
                current->Stloc(mapILType(local->type));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(assign_node->lhs);
                visitArg(arg,current);
                if(arg->is_ref){
                    current->Ldarg(DataTypeEnum::ref)
                            .Store(mapILType(arg->type));
                }
                else{
                    current->Starg(mapILType(arg->type));
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(assign_node->lhs);
                visitFld(fld,current);
                current->Stfld(mapILType(fld->type), getTokenRef(fld));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(assign_node->lhs);
                visitSFld(sfld,current);
                current->Stsfld(mapILType(sfld->type), getTokenRef(sfld));
                break;
            }
        }
    }

    void ILGen::visitCast(ast::Cast *cast_node, il::BasicBlock *current) {
        switch(cast_node->expr->expression_kind){
            case ast::Expression::Unary:
            case ast::Expression::Binary:
            case ast::Expression::Cast:
            case ast::Expression::Parentheses:
            case ast::Expression::Ftn:
            case ast::Expression::VFtn:
            case ast::Expression::SFtn:
            case ast::Expression::Ext:
            case ast::Expression::Digit:
            case ast::Expression::Decimal:
            case ast::Expression::String:
            case ast::Expression::Char:
            case ast::Expression::Boolean:
            case ast::Expression::EnumMember:
                visitExpression(cast_node->expr,current);
                break;
            case ast::Expression::Assign:{
                auto assign = static_cast<ast::Assign*>(cast_node->expr);
                visitAssign(assign,current);
                loadAssignLhsValue(assign,current);
                break;
            }
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(cast_node->expr);
                visitArrayElement(element,current);
                current->Ldelem(mapILType(element->type));
                break;
            }
            case ast::Expression::Local:{
                auto local = static_cast<ast::Local*>(cast_node->expr);
                visitLocal(local,current);
                current->Ldloc(mapILType(local->type));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(cast_node->expr);
                visitArg(arg,current);
                if(arg->is_ref){
                    current->Ldarg(DataTypeEnum::ref)
                            .Load(mapILType(arg->type));
                }
                else{
                    current->Ldarg(mapILType(arg->type));
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(cast_node->expr);
                visitFld(fld,current);
                current->Ldfld(mapILType(fld->type), getTokenRef(fld));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(cast_node->expr);
                visitSFld(sfld,current);
                current->Ldsfld(mapILType(sfld->type), getTokenRef(sfld));
                break;
            }
        }
        current->Conv(mapILType(cast_node->expr->type), mapILType(cast_node->target));
    }

    void ILGen::visitParentheses(ast::Parentheses *parentheses_node, il::BasicBlock *current) {
        PANIC;
    }

    void ILGen::visitArrayElement(ast::ArrayElement *element_node, il::BasicBlock *current) {
        switch(element_node->array->expression_kind){
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(element_node->array);
                visitArrayElement(element,current);
                current->Ldelema(mapILType(element->type));
                break;
            }
            case ast::Expression::Local:{
                auto local = static_cast<ast::Local*>(element_node->array);
                visitLocal(local,current);
                current->Ldloca();
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(element_node->array);
                visitArg(arg,current);
                if(arg->is_ref){
                    current->Ldarg(DataTypeEnum::ref);
                }
                else{
                    current->Ldarga();
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(element_node->array);
                visitFld(fld,current);
                current->Ldflda(getTokenRef(fld));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(element_node->array);
                visitSFld(sfld,current);
                current->Ldsflda(getTokenRef(sfld));
                break;
            }
        }

        switch(element_node->offset->expression_kind){
            case ast::Expression::Unary:
            case ast::Expression::Binary:
            case ast::Expression::Cast:
            case ast::Expression::Parentheses:
            case ast::Expression::Ftn:
            case ast::Expression::SFtn:
            case ast::Expression::VFtn:
            case ast::Expression::Ext:
            case ast::Expression::Digit:
                visitExpression(element_node->offset,current);
                break;
            case ast::Expression::Assign:{
                auto assign = static_cast<ast::Assign*>(element_node->offset);
                visitAssign(assign,current);
                loadAssignLhsValue(assign,current);
                break;
            }
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(element_node->offset);
                current->Ldelem(mapILType(element->type));
                break;
            }
            case ast::Expression::Local:{
                auto local = static_cast<ast::Local*>(element_node->offset);
                visitLocal(local,current);
                current->Ldloc(mapILType(local->type));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(element_node->offset);
                visitArg(arg,current);
                if(arg->is_ref){
                    current->Ldarg(DataTypeEnum::ref)
                            .Load(mapILType(arg->type));
                }
                else{
                    current->Ldarg(mapILType(arg->type));
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(element_node->offset);
                current->Ldfld(mapILType(fld->type), getTokenRef(fld));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(element_node->offset);
                current->Ldsfld(mapILType(sfld->type), getTokenRef(sfld));
                break;
            }
        }
    }

    TokenRef *ILGen::getTokenRef(ast::Fld *fld){
        return document->getTokenRef(fld->variable->getFullName());
    }

    TokenRef *ILGen::getTokenRef(ast::SFld *sfld){
        return document->getTokenRef(sfld->variable->getFullName());
    }


    void ILGen::visitDelegate(ast::Delegate *delegate_node, il::BasicBlock *current) {
        //todo
    }

    void ILGen::visitArgument(ast::Argument *argument_node, il::BasicBlock *current) {
        if(argument_node->byval){
            switch(argument_node->expr->expression_kind){
                case ast::Expression::Unary:
                case ast::Expression::Binary:
                case ast::Expression::Cast:
                case ast::Expression::Ftn:
                case ast::Expression::SFtn:
                case ast::Expression::VFtn:
                case ast::Expression::Ext:
                case ast::Expression::Digit:
                case ast::Expression::Decimal:
                case ast::Expression::String:
                case ast::Expression::Char:
                case ast::Expression::Boolean:
                case ast::Expression::Parentheses:
                case ast::Expression::EnumMember:{
                    visitExpression(argument_node->expr,current);
                    break;
                }
                case ast::Expression::Assign:{
                    auto assign = static_cast<ast::Assign*>(argument_node->expr);
                    visitAssign(assign,current);
                    loadAssignLhsValue(assign,current);
                    break;
                }
                case ast::Expression::Element:{
                    auto element = static_cast<ast::ArrayElement*>(argument_node->expr);
                    visitArrayElement(element,current);
                    current->Ldelem(mapILType(element->type));
                    break;
                }
                case ast::Expression::Local:{
                    auto local = static_cast<ast::Local*>(argument_node->expr);
                    visitLocal(local,current);
                    current->Ldloc(mapILType(local->type));
                    break;
                }
                case ast::Expression::ArgUse:{
                    auto arg = static_cast<ast::Arg*>(argument_node->expr);
                    visitArg(arg,current);
                    if(arg->is_ref){
                        current->Ldarg(DataTypeEnum::ref)
                                .Load(mapILType(arg->type));
                    }
                    else{
                        current->Ldarg(mapILType(arg->type));
                    }
                    break;
                }
                case ast::Expression::Fld:{
                    auto fld = static_cast<ast::Fld*>(argument_node->expr);
                    visitFld(fld,current);
                    current->Ldfld(mapILType(fld->type), getTokenRef(fld));
                    break;
                }
                case ast::Expression::SFld:{
                    auto sfld = static_cast<ast::SFld*>(argument_node->expr);
                    visitSFld(sfld,current);
                    current->Ldsfld(mapILType(sfld->type), getTokenRef(sfld));
                    break;
                }
            }
        }
        else{
            switch(argument_node->expr->expression_kind){
                case ast::Expression::Assign:{
                    auto assign = static_cast<ast::Assign*>(argument_node->expr);
                    visitAssign(assign,current);
                    loadAssignLhsAddress(assign,current);
                    break;
                }
                case ast::Expression::Element:{
                    auto element = static_cast<ast::ArrayElement*>(argument_node->expr);
                    visitArrayElement(element,current);
                    current->Ldelema(mapILType(element->type));
                    break;
                }
                case ast::Expression::Local:{
                    auto local = static_cast<ast::Local*>(argument_node->expr);
                    visitLocal(local,current);
                    current->Ldloca();
                    break;
                }
                case ast::Expression::ArgUse:{
                    auto arg = static_cast<ast::Arg*>(argument_node->expr);
                    visitArg(arg,current);

                    if(arg->is_ref){
                        current->Ldarg(DataTypeEnum::ref);
                    }
                    else{
                        current->Ldarga();
                    }
                    break;
                }
                case ast::Expression::Fld:{
                    auto fld = static_cast<ast::Fld*>(argument_node->expr);
                    visitFld(fld,current);
                    current->Ldflda(getTokenRef(fld));
                    break;
                }
                case ast::Expression::SFld:{
                    auto sfld = static_cast<ast::SFld*>(argument_node->expr);
                    visitSFld(sfld,current);
                    current->Ldsflda(getTokenRef(sfld));
                    break;
                }
            }
        }
    }


    void ILGen::visitNew(ast::New *new_node, il::BasicBlock *current) {
        loadCalleeArguments(new_node,current);
        current->Newobj(document->getTokenRef(new_node->target->getFullName()));
    }

    void ILGen::loadFtnRef(ast::Expression *ref, il::BasicBlock *current){
        switch(ref->expression_kind){
            case ast::Expression::Cast:
            case ast::Expression::Parentheses:
            case ast::Expression::Ftn:
            case ast::Expression::SFtn:
            case ast::Expression::Ext:{
                visitExpression(ref,current);
                break;
            }
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(ref);
                current->Ldelem(mapILType(element->type));
                break;
            }
            case ast::Expression::Local:{
                auto local = static_cast<ast::Local*>(ref);
                current->Ldloc(mapILType(local->type));
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(ref);
                if(arg->is_ref){
                    current->Ldarg(DataTypeEnum::ref)
                            .Load(mapILType(arg->type));
                }
                else{
                    current->Ldarg(mapILType(arg->type));
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(ref);
                current->Ldfld(mapILType(fld->type),getTokenRef(fld));
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(ref);
                current->Ldsfld(mapILType(sfld->type),getTokenRef(sfld));
                break;
            }
        }
    }

    void ILGen::visitFtnCall(ast::FtnCall *ftn_node, il::BasicBlock *current) {
        loadCalleeArguments(ftn_node,current);
        loadFtnRef(ftn_node->ref,current);
        current->Ldftn(document->getTokenRef(ftn_node->function->getFullName()));
        current->Call();
    }

    void ILGen::visitSFtnCall(ast::SFtnCall *sftn_node, il::BasicBlock *current) {
        loadCalleeArguments(sftn_node,current);
        current->Ldsftn(document->getTokenRef(sftn_node->function->getFullName()));
        current->Callstatic();
    }

    void ILGen::loadCalleeArguments(ast::Call *call, il::BasicBlock *current){
        stack<ast::Argument*> arguments;
        FOR_EACH(iter,call->argument){
            arguments.push(iter);
        }

        while(!arguments.empty()){
            visitArgument(arguments.top(),current);
            arguments.pop();
        }
    }

    void ILGen::visitVFtnCall(ast::VFtnCall *vftn_node, il::BasicBlock *current) {
        loadCalleeArguments(vftn_node,current);
        loadFtnRef(vftn_node->ref,current);
        current->Ldvftn(document->getTokenRef(vftn_node->function->getFullName()));
        current->Callvirt();
    }

    void ILGen::visitExtCall(ast::ExtCall *ext_node, il::BasicBlock *current) {
        loadCalleeArguments(ext_node,current);
        auto ext_symbol = static_cast<type::ExternalFunction*>(ext_node->function);
        if(ext_symbol->getLibName() == "intrinsic"){
            auto alias = ext_symbol->getAlias();
            current->Intrinsic((vm::IntrinsicEnum)stoi(alias.substr(1, alias.count()-1)));
        }
        else{
            current->Invoke(document->getTokenRef(ext_node->function->getFullName()));
        }
    }

    void ILGen::visitSFld(ast::SFld *sfld_node, il::BasicBlock *current) {
        // do nothing
    }

    void ILGen::visitFld(ast::Fld *fld_node, il::BasicBlock *current) {
        switch(fld_node->ref->expression_kind){
            case ast::Expression::Cast:
            case ast::Expression::Parentheses:
                visitExpression(fld_node,current);
                break;
            case ast::Expression::Element:{
                auto element = static_cast<ast::ArrayElement*>(fld_node->ref);
                auto il_type = mapILType(element->type);
                visitArrayElement(element,current);
                if(il_type.getKind() == DataTypeEnum::record){
                    current->Ldelema(il_type);
                }
                else{
                    current->Ldelem(il_type);
                }
                break;
            }
            case ast::Expression::Local:{
                auto local = static_cast<ast::Local*>(fld_node->ref);
                auto il_type = mapILType(local->type);
                visitLocal(local,current);
                if(il_type.getKind() == DataTypeEnum::record) {
                    current->Ldloca();
                }
                else{
                    current->Ldloc(il_type);
                }
                break;
            }
            case ast::Expression::ArgUse:{
                auto arg = static_cast<ast::Arg*>(fld_node->ref);
                auto il_type = mapILType(arg->type);
                visitArg(arg,current);
                if(arg->is_ref){
                    if(il_type.getKind() == DataTypeEnum::record){
                        current->Ldarg(DataTypeEnum::ref);
                    }
                    else{
                        current->Ldarg(DataTypeEnum::ref)
                                .Load(mapILType(arg->type));
                    }
                }
                else{
                    if(il_type.getKind() == DataTypeEnum::record){
                        current->Ldarga();
                    }
                    else{
                        current->Ldarg(mapILType(arg->type));
                    }
                }
                break;
            }
            case ast::Expression::Fld:{
                auto fld = static_cast<ast::Fld*>(fld_node->ref);
                auto il_type = mapILType(fld->type);
                visitFld(fld,current);
                if(il_type.getKind() == DataTypeEnum::record){
                    current->Ldflda(getTokenRef(fld));
                }
                else{
                    current->Ldfld(il_type, getTokenRef(fld));
                }
                break;
            }
            case ast::Expression::SFld:{
                auto sfld = static_cast<ast::SFld*>(fld_node->ref);
                auto il_type = mapILType(sfld->type);
                visitSFld(sfld,current);
                if(il_type.getKind() == DataTypeEnum::record){
                    current->Ldsflda(getTokenRef(sfld));
                }
                else{
                    current->Ldsfld(mapILType(sfld->type),getTokenRef(sfld));
                }
                break;
            }
        }
    }

    void ILGen::visitLocal(ast::Local *local_node, il::BasicBlock *current) {
        auto index = local_node->variable->getLayoutIndex();
        current->Push(DataTypeEnum::u16, (data::u16)index);
    }

    void ILGen::visitArg(ast::Arg *arg_node, il::BasicBlock *current) {
        auto index = arg_node->variable->getLayoutIndex();
        current->Push(DataTypeEnum::u16, (data::u16)index);
    }

    void ILGen::visitDigit(ast::Digit *digit_node, il::BasicBlock *current) {
        current->Push(DataTypeEnum::i32, digit_node->value);
    }

    void ILGen::visitDecimal(ast::Decimal *decimal_node, il::BasicBlock *current) {
        current->Push(DataTypeEnum::f64, decimal_node->value);
    }

    void ILGen::visitString(ast::String *string_node, il::BasicBlock *current) {
        //current->Ldc(document->getTokenRef(string_node->value));
        //todo: call ctor for String
    }

    void ILGen::visitChar(ast::Char *char_node, il::BasicBlock *current) {
        current->Push(DataTypeEnum::character, char_node->value);
    }

    void ILGen::visitBoolean(ast::Boolean *boolean_node, il::BasicBlock *current) {
        current->Push(DataTypeEnum::boolean, (data::boolean)boolean_node->value);
    }

}


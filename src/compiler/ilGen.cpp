//
// Created by yorkin on 12/13/21.
//

#include "ilGen.h"
#include<variant>

namespace evoBasic{
    using namespace il;
    using namespace std;

    using MemberList = list<Member*>;

    struct EmptyOperand {};

    struct DataOperand {
        il::DataType data;
        type::Primitive *symbol = nullptr;
    };

    struct RefOperand;

    struct FldOperand {
        enum FldKind{local,sfld,fld,arg}kind;
        type::Variable *variable = nullptr;
    };

    struct ElementOperand {
        type::Array *array = nullptr;
    };

    struct FtnOperand {
        il::DataType ftn;
        type::Function *symbol = nullptr;
    };

    struct RecordOperand {
        type::Record *symbol = nullptr;
    };

    struct ClassOperand {
        type::Class *symbol = nullptr;
    };

    struct ArrayOperand;

    struct TokenOperand {
        il::Token *token = nullptr;
        type::Symbol *symbol = nullptr;
    };

    using OperandInfo = std::variant<EmptyOperand,DataOperand,RefOperand,FtnOperand,RecordOperand,ClassOperand,ArrayOperand,TokenOperand,FldOperand,ElementOperand>;

    struct RefOperand {
        OperandInfo *ref = nullptr;
        //~RefOperand(){delete ref;}
    };

    struct ArrayOperand {
        OperandInfo *element = nullptr;
        type::Array *symbol = nullptr;
    };

    enum class OperandKind : int{empty,data,ref,ftn,record,cls,array,token,fld,element};

    RefOperand refTo(OperandInfo info){
        return RefOperand{new OperandInfo(info)};
    }

    type::Symbol *stripOperandInfo(OperandInfo type){
        switch ((OperandKind)type.index()) {
            case OperandKind::empty:    PANIC;
            case OperandKind::data:     return get<DataOperand>(type).symbol;
            case OperandKind::ref:      return stripOperandInfo(*get<RefOperand>(type).ref);
            case OperandKind::ftn:      return get<FtnOperand>(type).symbol;
            case OperandKind::record:   return get<RecordOperand>(type).symbol;
            case OperandKind::cls:      return get<ClassOperand>(type).symbol;
            case OperandKind::array:    return get<ArrayOperand>(type).symbol;
            case OperandKind::token:    return get<TokenOperand>(type).symbol;
            case OperandKind::fld:      return get<FldOperand>(type).variable;
            case OperandKind::element:  return get<ElementOperand>(type).array;
        }
        return nullptr;
    }

    il::IL *ILGen::gen(AST *ast, Context *context) {
        return nullptr;
    }

    std::any ILGen::visitGlobal(ast::Global *global_node, ILGenArgs args) {
//        vector<Member*> members;
//        for(auto iter = global_node->member; iter != nullptr; iter = iter->next_sibling){
//            members.push_back(any_cast<Member*>(visitMember(iter,args)));
//        }
//        return (Member*)factory.createIL(global_node->module_symbol->getName(),global_node->access,members);
    }

    std::any ILGen::visitModule(ast::Module *mod_node, ILGenArgs args) {
        vector<Member*> members;
        FOR_EACH(iter,mod_node->member){
            auto ls = any_cast<MemberList>(visitMember(iter,args));
            members.insert(members.end(),ls.begin(),ls.end());
        }

        return MemberList{
            factory.createModule(mod_node->module_symbol->getName(),mod_node->access,members)
        };
    }

    std::any ILGen::visitClass(ast::Class *cls_node, ILGenArgs args) {
        vector<Member*> members;
        vector<Impl*> impls;
        for(auto [_,interface]:cls_node->class_symbol->getImplMap()){
            impls.push_back(factory.createImplements(interface));
        }

        Extend *extend = factory.createExtend(cls_node->class_symbol->getExtend());

        FOR_EACH(iter,cls_node->member){
            auto ls = any_cast<MemberList>(visitMember(iter,args));
            members.insert(members.end(),ls.begin(),ls.end());
        }

        return MemberList{
            factory.createClass(cls_node->class_symbol->getName(),cls_node->access,extend,impls,members)
        };
    }

    std::any ILGen::visitEnum(ast::Enum *em_node, ILGenArgs args) {
        vector<Pair*> pairs;
        FOR_EACH(iter,em_node->member){
            pairs.push_back(factory.createPair(getID(iter->name), getDigit(iter->value)));
        }

        return MemberList{
            factory.createEnum(em_node->enum_symbol->getName(),em_node->enum_symbol->getAccessFlag(),pairs)
        };
    }

    std::any ILGen::visitType(ast::Type *ty_node, ILGenArgs args) {
        vector<Fld*> fields;
        for(auto symbol : *(ty_node->type_symbol)){
            auto var = symbol->as<type::Variable*>();
            fields.push_back(factory.createField(var->getName(),AccessFlag::Public,var->getPrototype()));
        }

        return MemberList{
            factory.createRecord(ty_node->type_symbol->getName(),ty_node->type_symbol->getAccessFlag(),fields)
        };
    }

    std::any ILGen::visitDim(ast::Dim *dim_node, ILGenArgs args) {
        MemberList ret;
        for(auto var=dim_node->variable; var!=nullptr; var=var->next_sibling){
            auto [name,prototype] = any_cast<tuple<string,type::Prototype*>>(visitVariable(var,args));
            Member *fld = dim_node->is_static ?
                          factory.createStaticField(name,dim_node->access,prototype) :
                          factory.createField(name,dim_node->access,prototype);

            ret.push_back(any_cast<Member*>(visitVariable(var,args)));
        }
        return ret;
    }

    std::any ILGen::visitVariable(ast::Variable *var_node, ILGenArgs args) {
        auto name = var_node->variable_symbol->getName();
        auto prototype = var_node->variable_symbol->as<type::Variable*>()->getPrototype();
        return make_tuple(name,prototype);
    }

    std::any ILGen::visitFunction(ast::Function *func_node, ILGenArgs args) {
        auto function = func_node->function_symbol->as<type::Function*>();

        vector<Param*> params;
        Result *result = nullptr;
        auto name = function->getName();
        auto access = function->getAccessFlag();

        if(function->getRetSignature())factory.createResult(function->getRetSignature());

        for(auto arg : function->getArgsSignature()){
            params.push_back(factory.createParam(arg->getName(),arg->getPrototype()));
        }

        for(auto arg : function->getArgsOptions()){
            //params.push_back(factory.createOption(arg->getName(),arg->getPrototype(),nullptr));//todo
            PANIC;
        }

        if(function->getParamArray()){
            params.push_back(factory.createParamArray(function->getParamArray()->getName(),function->getParamArray()->getPrototype()));
        }

        Member *member;
        switch (function->getFunctionKind()) {
            case type::FunctionKind::Constructor:{
                Block *entry = nullptr;
                auto ctor = function->as<type::Constructor*>();
                factory.createConstructor(access,params,entry);
            }
            case type::FunctionKind::UserFunction:{
                auto user_fn = function->as<type::UserFunction*>();
                Block *entry = nullptr;
                switch (user_fn->getFunctionFlag()) {
                    case type::FunctionFlag::Override:
                    case type::FunctionFlag::Virtual:
                        member = factory.createVirtualFunction(name,access,params,result,entry);
                        break;
                    case type::FunctionFlag::Static:
                        member = factory.createStaticFunction(name,access,params,result,entry);
                        break;
                    case type::FunctionFlag::Method:
                        member = factory.createFunction(name,access,params,result,entry);
                        break;
                }
            }
            case type::FunctionKind::External:{
                auto external_fn = function->as<type::ExternalFunction*>();
                member = factory.createExternalFunction(name,external_fn->getLibName(),access,params,result);
            }
        }

        return MemberList{member};
    }

    std::any ILGen::visitLet(ast::Let *let_node, ILGenArgs args) {
        for(auto var = let_node->variable; var!=nullptr; var = var->next_sibling){
            auto [name,prototype] = any_cast<tuple<string,type::Prototype*>>(visitVariable(var,args));
            args.ftn->addLocal(factory.createLocal(name,prototype));
        }
        return {};
    }

    std::any ILGen::visitIf(ast::stmt::If *ifstmt_node, ILGenArgs args) {

        return Visitor::visitIf(ifstmt_node, args);
    }

    std::any ILGen::visitCase(ast::Case *ca_node, ILGenArgs args) {
        return Visitor::visitCase(ca_node, args);
    }

    std::any ILGen::visitLoop(ast::stmt::Loop *loop_node, ILGenArgs args) {

        return Visitor::visitLoop(loop_node, args);
    }

    std::any ILGen::visitSelect(ast::stmt::Select *select_node, ILGenArgs args) {
        return Visitor::visitSelect(select_node, args);
    }

    std::any ILGen::visitFor(ast::stmt::For *for_node, ILGenArgs args) {
        // add iterator variable to locals
        if(for_node->iterator_has_let){
            auto name = for_node->iterator->type->symbol->getName();
            auto prototype = for_node->iterator->type->symbol->as<type::Variable*>()->getPrototype();
            auto iterator = factory.createLocal(name,prototype);
            args.ftn->addLocal(iterator);
        }
        for_node->iterator->type->symbol->as<type::Variable*>()->getOffset();

        // create blocks
        auto condition_block = new Block;
        auto next_block = new Block;
        auto previous_block = args.previous_block;

//        previous_block->Push(DataType::u16,)
//                            .Ldloca();
        visitExpression(for_node->begin,args);
        previous_block->Stloc(for_node->iterator->type->il_type);
        previous_block->Br(condition_block);
//
//        condition_block->Ldloc()
    }

    std::any ILGen::visitContinue(ast::stmt::Continue *cont_node, ILGenArgs args) {
        args.previous_block->Br(args.next_block);
        return {};
    }

    std::any ILGen::visitReturn(ast::stmt::Return *ret_node, ILGenArgs args) {
        args.previous_block->Ret();
        return {};
    }

    std::any ILGen::visitExit(ast::stmt::Exit *exit_node, ILGenArgs args) {
        //todo
    }

    std::any ILGen::visitBinary(ast::expr::Binary *logic_node, ILGenArgs args) {

    }

    std::any ILGen::visitUnary(ast::expr::Unary *unit_node, ILGenArgs args) {

    }

    std::any ILGen::visitCallee(ast::expr::Callee *callee_node, ILGenArgs args) {

    }

    std::any ILGen::visitArg(ast::expr::Callee::Argument *arg_node, ILGenArgs args) {

    }

    std::any ILGen::visitIndex(ast::expr::Index *index_node, ILGenArgs args) {

    }

    std::any ILGen::visitCast(ast::expr::Cast *cast_node, ILGenArgs args) {

    }

    std::any ILGen::visitAssign(ast::expr::Assign *assign_node, ILGenArgs args) {


    }

    std::any ILGen::visitDot(ast::expr::Dot *dot_node, ILGenArgs args) {
        auto lhs = visitExpression(dot_node->lhs,args);
        args.dot_expression_context = any_cast<type::Symbol*>(lhs);
        return visitExpression(dot_node->rhs,args);
    }

    std::any ILGen::visitID(ast::expr::ID *id_node, ILGenArgs args) {
        auto name = getID(id_node);
        if(args.need_lookup) {
            return args.dot_expression_context->as<type::Domain*>()->lookUp(name);
        }
        else{
            return args.dot_expression_context->as<type::Domain*>()->find(name);
        }
    }

    std::any ILGen::visitBoolean(ast::expr::Boolean *bl_node, ILGenArgs args) {
        args.previous_block->Push(il::boolean,bl_node->value);
        return OperandInfo(DataOperand{il::boolean});
    }

    std::any ILGen::visitChar(ast::expr::Char *ch_node, ILGenArgs args) {
        args.previous_block->Push(il::character,ch_node->value);
        return OperandInfo(DataOperand{il::character});
    }

    std::any ILGen::visitDigit(ast::expr::Digit *digit_node, ILGenArgs args) {
        args.previous_block->Push(il::i32,digit_node->value);
        return OperandInfo(DataOperand{il::i32});
    }

    std::any ILGen::visitDecimal(ast::expr::Decimal *decimal, ILGenArgs args) {
        args.previous_block->Push(il::f64,decimal->value);
        return OperandInfo(DataOperand{il::f64});
    }

    std::any ILGen::visitString(ast::expr::String *str_node, ILGenArgs args) {
        //todo: load raw string and initialize class string
        return OperandInfo(refTo(ClassOperand{args.context->getBuiltIn().getStringClass()}));
    }

    std::any ILGen::visitParentheses(ast::expr::Parentheses *parentheses_node, ILGenArgs args) {
        return visitExpression(parentheses_node->expr,args);
    }

    std::any ILGen::visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, ILGenArgs args) {
        return visitExpression(expr_stmt_node->expr,args);
    }

    std::any ILGen::visitExpression(ast::expr::Expression *expr_node, ILGenArgs args) {
        using exp = ast::expr::Expression;
        using namespace ast::expr;
        switch ((*expr_node).expression_kind) {
            case exp::binary_:      return visitBinary((Binary*)expr_node,args);
            case exp::unary_:       return visitUnary((Unary*)expr_node, args);
            case exp::digit_:       return visitDigit((Digit*)expr_node,args);
            case exp::decimal_:     return visitDecimal((Decimal*)expr_node,args);
            case exp::string_:      return visitString((String*)expr_node,args);
            case exp::char_:        return visitChar((Char*)expr_node,args);
            case exp::parentheses_: return visitParentheses((Parentheses*)expr_node,args);
            case exp::boolean_:     return visitBoolean((Boolean*)expr_node,args);
            case exp::new_:         return visitNew((New*)expr_node,args);
            case exp::assign_:      return visitAssign((Assign*)expr_node,args);
            case exp::index_:       return visitIndex((Index*)expr_node,args);
            case exp::dot_:         return visitDot((Dot*)expr_node,args);
            case exp::callee_:      return visitCallee((Callee*)expr_node,args);
            case exp::ID_:          return visitID((ID*)expr_node,args);
            case exp::colon_:       return visitColon((Colon*)expr_node,args);
        }
        PANIC;
    }

    std::any ILGen::visitStatement(ast::stmt::Statement *stmt_node, ILGenArgs args) {
        switch ((*stmt_node).stmt_flag) {
            case ast::stmt::Statement::let_:        return visitLet((ast::stmt::Let*)stmt_node,args);
            case ast::stmt::Statement::loop_:       return visitLoop((ast::stmt::Loop*)stmt_node,args);
            case ast::stmt::Statement::if_:         return visitIf((ast::stmt::If*)stmt_node,args);
            case ast::stmt::Statement::for_:        return visitFor((ast::stmt::For*)stmt_node,args);
            case ast::stmt::Statement::select_:     return visitSelect((ast::stmt::Select*)stmt_node,args);
            case ast::stmt::Statement::return_:     return visitReturn((ast::stmt::Return*)stmt_node,args);
            case ast::stmt::Statement::continue_:   return visitContinue((ast::stmt::Continue*)stmt_node,args);
            case ast::stmt::Statement::exit_:       return visitExit((ast::stmt::Exit*)stmt_node,args);
            case ast::stmt::Statement::expr_:       return visitExprStmt((ast::stmt::ExprStmt*)stmt_node,args);
        }
        PANIC;
    }

    std::any ILGen::visitMember(ast::Member *member_node, ILGenArgs args) {
        switch (member_node->member_kind) {
            case ast::Member::class_:    return visitClass((ast::Class*)member_node,args);
            case ast::Member::module_:   return visitModule((ast::Module*)member_node,args);
            case ast::Member::type_:     return visitType((ast::Type*)member_node,args);
            case ast::Member::enum_:     return visitEnum((ast::Enum*)member_node,args);
            case ast::Member::dim_:      return visitDim((ast::Dim*)member_node,args);
            case ast::Member::interface_:return visitInterface((ast::Interface*)member_node,args);
        }
        PANIC;
    }

}


//
// Created by yorkin on 12/13/21.
//

#include "ilGen.h"
#include<variant>

namespace evoBasic{
    using namespace il;
    using namespace std;

    using MemberList = list<Member*>;

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
        for(auto iter = mod_node->member; iter != nullptr; iter = iter->next_sibling){
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

        for(auto iter = cls_node->member; iter != nullptr; iter = iter->next_sibling){
            auto ls = any_cast<MemberList>(visitMember(iter,args));
            members.insert(members.end(),ls.begin(),ls.end());
        }

        return MemberList{
            factory.createClass(cls_node->class_symbol->getName(),cls_node->access,extend,impls,members)
        };
    }

    std::any ILGen::visitEnum(ast::Enum *em_node, ILGenArgs args) {
        vector<Pair*> pairs;
        for(auto iter = em_node->member; iter!=nullptr; iter=iter->next_sibling){
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
            params.push_back(factory.createOption(arg->getName(),arg->getPrototype()));
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

    std::any ILGen::visitLet(ast::Let *let_node, ILGenArgs args) {
        for(auto var = let_node->variable; var!=nullptr; var = var->next_sibling){
            auto [name,prototype] = any_cast<tuple<string,type::Prototype*>>(visitVariable(var,args));
            args.ftn->locals.push_back(factory.createLocal(name,prototype));
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

        previous_block->Push(DataType::u16,)
                            .Ldloca();
        visitExpression(for_node->begin,args);
        previous_block->Stloc(for_node->iterator->type->il_type);
        previous_block->Br(condition_block);

        condition_block->Ldloc()
    }

    std::any ILGen::visitContinue(ast::stmt::Continue *cont_node, ILGenArgs args) {
        return Visitor::visitContinue(cont_node, args);
    }

    std::any ILGen::visitReturn(ast::stmt::Return *ret_node, ILGenArgs args) {
        return Visitor::visitReturn(ret_node, args);
    }

    std::any ILGen::visitExit(ast::stmt::Exit *exit_node, ILGenArgs args) {
        return Visitor::visitExit(exit_node, args);
    }

    std::any ILGen::visitBinary(ast::expr::Binary *logic_node, ILGenArgs args) {
        return Visitor::visitBinary(logic_node, args);
    }

    std::any ILGen::visitUnary(ast::expr::Unary *unit_node, ILGenArgs args) {
        return Visitor::visitUnary(unit_node, args);
    }

    std::any ILGen::visitCallee(ast::expr::Callee *callee_node, ILGenArgs args) {
        return Visitor::visitCallee(callee_node, args);
    }

    std::any ILGen::visitArg(ast::expr::Callee::Argument *arg_node, ILGenArgs args) {
        return Visitor::visitArg(arg_node, args);
    }
}


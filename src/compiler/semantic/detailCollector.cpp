//
// Created by yorkin on 11/24/21.
//

#include "detailCollector.h"
#include "logger.h"
#include "semantic.h"
#include "ast.h"

using namespace std;
using namespace evoBasic::type;
using namespace evoBasic::ast;
using namespace evoBasic::ast::expr;
namespace evoBasic{

    std::any DetailCollector::visitGlobal(ast::Global *global, BaseArgs args) {
        NotNull(global);
        for(const auto& member:global->member_list)
            visitMember(member,args);
        return nullptr;
    }

    std::any DetailCollector::visitModule(ast::Module *mod_node, BaseArgs args) {
        NotNull(mod_node);
        auto name = getID(mod_node->name);
        auto mod = args.domain->find(name);
        NotNull(mod.get());
        args.domain = mod->as_shared<type::Domain>();
        for(const auto& member:mod_node->member_list)
            visitMember(member,args);
        return nullptr;
    }

    std::any DetailCollector::visitClass(ast::Class *cls_node, BaseArgs args) {
        NotNull(cls_node);
        auto name = getID(cls_node->name);
        auto cls = args.domain->find(name)->as_shared<type::Domain>();
        NotNull(cls.get());
        args.context->byteLengthDependencies.addIsolate(cls);
        args.domain = cls;
        args.parent_class = cls->as_shared<type::Class>();
        args.context->byteLengthDependencies.addIsolate(cls);
        for(const auto& member:cls_node->member_list)
            visitMember(member,args);
        return nullptr;
    }

    std::any DetailCollector::visitEnum(ast::Enum *em_node, BaseArgs args) {
        NotNull(em_node);
        auto name = getID(em_node->name);
        auto em = args.domain->find(name)->as_shared<type::Enumeration>();
        NotNull(em.get());
        int index = 0;
        for(auto& child:em_node->member_list){
            NotNull(child.first);
            if(child.second != nullptr){
                index = getDigit(child.second);
            }
            auto member_name = getID(child.first);
            if(is_name_valid(member_name,child.first->location,args.domain)){
                auto member = make_shared<type::EnumMember>(index);
                member->setName(member_name);
                member->setLocation(child.first->location);
                em->add(member);
                index++;
            }
        }
        return nullptr;
    }

    std::any DetailCollector::visitType(ast::Type *ty_node, BaseArgs args) {
        NotNull(ty_node);
        auto name = getID(ty_node->name);
        auto ty = args.domain->find(name)->as_shared<type::Record>();
        NotNull(ty.get());
        for(auto& var_node:ty_node->member_list){
            auto var_name = getID(var_node->name);
            auto variable = ty->find(var_name)->as_shared<type::Variable>();
            NotNull(variable.get());
            auto prototype = any_cast<shared_ptr<Prototype>>(visitAnnotation(var_node->annotation,args));
            switch (prototype->getKind()) {
                case type::DeclarationEnum::Type:
                case type::DeclarationEnum::Array:
                    args.context->byteLengthDependencies.addDependent(ty,prototype->as_shared<Domain>());
            }
            args.context->byteLengthDependencies.addIsolate(ty);
            variable->setPrototype(prototype);
        }
        return nullptr;
    }

    std::any DetailCollector::visitDim(ast::Dim *dim_node, BaseArgs args) {
        for(auto& variable_node:dim_node->variable_list){
            visitVariable(variable_node,args);
        }
        return nullptr;
    }

    std::any DetailCollector::visitVariable(ast::Variable *var_node, BaseArgs args) {
        auto name = getID(var_node->name);
        auto var = args.domain->find(name)->as_shared<type::Variable>();
        NotNull(var.get());
        auto prototype = any_cast<shared_ptr<Prototype>>(visitAnnotation(var_node->annotation,args));
        auto parent_kind = args.domain->getKind();

        switch(args.domain->getKind()){
            case DeclarationEnum::Function:
                switch (prototype->getKind()) {
                    case type::DeclarationEnum::Type:
                    case type::DeclarationEnum::Array:
                        args.context->byteLengthDependencies.addDependent(args.user_function,prototype->as_shared<Domain>());
                }
                break;
            case DeclarationEnum::Module:
                args.context->getGlobal()->addMemoryLayout(var);
                switch (prototype->getKind()) {
                    case type::DeclarationEnum::Type:
                    case type::DeclarationEnum::Array:
                        args.context->byteLengthDependencies.addDependent(args.context->getGlobal(),prototype->as_shared<Domain>());
                }
                break;
        }

        var->setPrototype(prototype);
        return nullptr;
    }


    std::any DetailCollector::visitFunction(ast::Function *func_node, BaseArgs args) {
        auto name = getID(func_node->name);
        if(is_name_valid(name,func_node->name->location,args.domain)){
            type::Function::Flag flag;
            switch(func_node->method_flag){
                case MethodFlag::Static:
                    flag = type::Function::Flag::Static;
                    if(args.domain->getKind() == type::DeclarationEnum::Class){
                        Logger::error(func_node->location,"Function in Module cannot be marked by 'Static'");
                    }
                    break;
                case MethodFlag::Virtual:
                case MethodFlag::Override:
                    if(args.domain->getKind() == type::DeclarationEnum::Class){
                        flag = type::Function::Flag::Virtual;
                    }
                    else{
                        Logger::error(func_node->location,"Function in Module cannot be marked by 'Virtual' or 'Override'");
                    }
                    break;
                case MethodFlag::None:
                    if(args.domain->getKind() == type::DeclarationEnum::Class){
                        flag = type::Function::Flag::Method;
                    }
                    break;
            }
            auto func = make_shared<type::UserFunction>(flag,func_node);
            func->setLocation(func_node->name->location);
            func->setName(name);

            args.context->byteLengthDependencies.addIsolate(func);

            args.domain->add(func);
            if(func_node->return_type){
                auto prototype = any_cast<shared_ptr<Prototype>>(visitAnnotation(func_node->return_type,args));
                func->setRetSignature(prototype);
            }

            switch (func->getFunctionFlag()) {
                case type::Function::Flag::Virtual:
                case type::Function::Flag::Method:
                    if(args.parent_class){
                        auto self = make_shared<Argument>("self",args.parent_class,true,false);
                        func->add(self);
                    }
                    break;
            }

            args.user_function = func;
            args.domain = func;
            for(auto& param_node:func_node->parameter_list){
                visitParameter(param_node,args);
            }

            if(name == "main" && args.domain->equal(args.context->getGlobal())){
                args.context->setEntrance(func);
            }
        }
        return nullptr;
    }

    std::any DetailCollector::visitExternal(ast::External *ext_node, BaseArgs args) {
        auto name = getID(ext_node->name);
        if(is_name_valid(name,ext_node->name->location,args.domain)){
            auto lib = getString(ext_node->lib);
            //auto alias = getString(ext_node->alias); TODO alias
            auto func = make_shared<type::ExternalFunction>(lib,name);
            func->setLocation(ext_node->name->location);
            func->setName(name);
            args.domain->add(func);
            if(ext_node->return_annotation){
                auto prototype = any_cast<shared_ptr<Prototype>>(visitAnnotation(ext_node->return_annotation,args));
                func->setRetSignature(prototype);
            }
            args.domain = func;
            for(auto& param_node:ext_node->parameter_list){
                visitParameter(param_node,args);
            }
        }
        return nullptr;
    }

    std::any DetailCollector::visitParameter(ast::Parameter *param_node, BaseArgs args) {
        auto name = getID(param_node->name);
        auto prototype = any_cast<shared_ptr<Prototype>>(visitAnnotation(param_node->annotation,args));
        NotNull(prototype.get());
        auto arg = make_shared<type::Argument>(name,prototype,param_node->is_byval,param_node->is_optional);
        if(param_node->is_byval){
            switch (prototype->getKind()) {
                case type::DeclarationEnum::Type:
                case type::DeclarationEnum::Array:
                    args.context->byteLengthDependencies.addDependent(args.user_function,prototype->as_shared<Domain>());
            }
        }
        if(is_name_valid(name,param_node->name->location,args.domain)){
            args.domain->add(arg);
        }
        return nullptr;
    }

    std::any DetailCollector::visitMember(ast::Member *member_node, BaseArgs args) {
        switch (member_node->member_kind) {
            case ast::Member::function_: return visitFunction((ast::Function*)member_node,args);
            case ast::Member::class_:    return visitClass((ast::Class*)member_node,args);
            case ast::Member::module_:   return visitModule((ast::Module*)member_node,args);
            case ast::Member::type_:     return visitType((ast::Type*)member_node,args);
            case ast::Member::enum_:     return visitEnum((ast::Enum*)member_node,args);
            case ast::Member::dim_:      return visitDim((ast::Dim*)member_node,args);
            case ast::Member::external_: return visitExternal((ast::External*)member_node,args);
                //case ast::Member::operator_: return visitOperator((ast::Operator*)member_node,args);
                //case ast::Member::init_:     return visitInit((ast::Init*)member_node,args);
        }
        return {};
    }

    std::any DetailCollector::visitBinary(ast::expr::Binary *logic_node, BaseArgs args) {
        switch (logic_node->op) {
            case ast::expr::Binary::Dot:{
                auto lhs_type = any_cast<ExpressionType*>(visitExpression(logic_node->lhs,args));
                if(lhs_type->value_kind == ExpressionType::error)return lhs_type;
                auto rhs_name = getID((ID*)logic_node->rhs);
                auto domain = lhs_type->prototype->as_shared<Domain>();
                shared_ptr<Prototype> target;
                if(domain && (target = domain->find(rhs_name)->as_shared<Prototype>())){
                    return new ExpressionType(target,ExpressionType::path);
                }
                else{
                    Logger::error(logic_node->location,"object not find");
                    return ExpressionType::Error;
                }
            }
                break;
            default:
                Logger::error(logic_node->location,"invalid expression");
                return ExpressionType::Error;
        }
    }

    std::any DetailCollector::visitID(ast::expr::ID *id_node, BaseArgs args) {
        auto name = getID(id_node);
        auto target = args.domain->lookUp(name)->as_shared<Prototype>();
        if(!target){
            Logger::error(id_node->location,"object not find");
            return ExpressionType::Error;
        }
        return new ExpressionType(target,ExpressionType::path);
    }

}
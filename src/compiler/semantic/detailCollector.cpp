//
// Created by yorkin on 11/24/21.
//

#include "detailCollector.h"
#include "logger.h"
#include "semantic.h"
#include "parseTree.h"
#include "i18n.h"

using namespace std;
using namespace evoBasic::type;
using namespace evoBasic::parseTree;
using namespace evoBasic::parseTree::expr;
using namespace evoBasic::i18n;
namespace evoBasic{

    std::any DetailCollector::visitAllMember(type::Domain *domain, parseTree::Member *member, DetailArgs args){
        args.domain = args.parent_class_or_module = domain;
        FOR_EACH(iter,member){
            visitMember(iter,args);
        }
        return {};
    }
    
    std::any DetailCollector::visitGlobal(parseTree::Global *global_node, DetailArgs args) {
        args.context->byteLengthDependencies.addIsolate(args.context->getGlobal());
        return visitAllMember(global_node->global_symbol,global_node->member,args);
    }

    std::any DetailCollector::visitModule(parseTree::Module *module_node, DetailArgs args) {
        return visitAllMember(module_node->module_symbol,module_node->member,args);
    }

    std::any DetailCollector::visitClass(parseTree::Class *class_node, DetailArgs args) {
        NotNull(class_node);
        auto class_symbol = class_node->class_symbol;
        args.domain = args.parent_class_or_module = class_symbol;

        type::Class *base_class = nullptr,*object_class = args.context->getBuiltIn().getObjectClass();
        if(class_node->extend){
            auto symbol = visitAnnotation(class_node->extend,args);
            if(symbol.has_value())
                base_class = any_cast<Prototype*>(symbol)->as<type::Class*>();
            else
                base_class = object_class;
        }
        else{
            base_class = object_class;
        }

        if(is_extend_valid(class_symbol,base_class)){
            class_symbol->setExtend(base_class);
            args.context->byteLengthDependencies.addDependent(class_symbol,base_class);
            args.context->inheritDependencies.addDependent(class_symbol,base_class);
        }
        else{
            Logger::error(class_node->extend->location,lang->msgInheritRecursive());
        }

        FOR_EACH(impl,class_node->impl){
            auto symbol = visitAnnotation(impl->annotation,args);
            if(symbol.has_value()){
                auto interface = any_cast<Prototype*>(symbol)->as<type::Interface*>();
                class_symbol->addImpl(interface);
            }
        }

        FOR_EACH(iter,class_node->member){
            visitMember(iter,args); 
        }

        return {};
    }

    std::any DetailCollector::visitInterface(parseTree::Interface *interface_node, DetailArgs args) {
        return visitAllMember(interface_node->interface_symbol,interface_node->function,args);
    }

    std::any DetailCollector::visitEnum(parseTree::Enum *enum_node, DetailArgs args) {
        args.domain = enum_node->enum_symbol;
        int index = 0;
        FOR_EACH(iter,enum_node->member){
            if(iter->value != nullptr){
                index = getDigit(iter->value);
            }
            auto member_name = getID(iter->name);
            if(is_name_valid(member_name,iter->location,args.domain)){
                auto member = new type::EnumMember(index);
                member->setName(member_name);
                member->setAccessFlag(AccessFlag::Public);
                member->setLocation(iter->location);
                args.domain->add(member);
                index++;
            }
        }
        return {};
    }

    std::any DetailCollector::visitType(parseTree::Type *type_node, DetailArgs args) {
        auto name = getID(type_node->name);
        auto type = type_node->type_symbol;
        NotNull(type);
        FOR_EACH(iter,type_node->member){
            auto var_name = getID(iter->name);
            auto variable = iter->variable_symbol;
            NotNull(variable);
            auto variable_prototype = any_cast<type::Prototype*>(visitAnnotation(iter->annotation, args));
            variable_prototype->setAccessFlag(AccessFlag::Public);
            switch (variable_prototype->getKind()) {
                case type::SymbolKind::Record:
                case type::SymbolKind::Array:
                    args.context->byteLengthDependencies.addDependent(type, variable_prototype->as<Domain*>());
            }
            variable->setPrototype(variable_prototype);
        }
        
        args.context->byteLengthDependencies.addIsolate(type);
        return {};
    }

    std::any DetailCollector::visitDim(parseTree::Dim *dim_node, DetailArgs args) {
        FOR_EACH(iter,dim_node->variable){
            auto variable = any_cast<Symbol*>(visitVariable(iter,args))->as<type::Variable*>();

            switch(args.domain->getKind()){
                case type::SymbolKind::Module:
                    args.context->getGlobal()->addMemoryLayout(variable);
                    switch (variable->getPrototype()->getKind()) {
                        case type::SymbolKind::Record:
                        case type::SymbolKind::Array:
                            args.context->byteLengthDependencies.addDependent(args.context->getGlobal(),
                                                                              variable->getPrototype()->as<Domain*>());
                    }
                    break;
                case type::SymbolKind::Class:
                    switch (variable->getPrototype()->getKind()) {
                        case type::SymbolKind::Record:
                        case type::SymbolKind::Array:
                            args.context->byteLengthDependencies.addDependent(args.domain,
                                                                              variable->getPrototype()->as<Domain*>());
                    }
                    break;
            }
        }
        return {};
    }

    std::any DetailCollector::visitVariable(parseTree::Variable *variable_node, DetailArgs args) {
        auto name = getID(variable_node->name);
        auto variable = variable_node->variable_symbol;
        NotNull(variable);
        auto prototype = any_cast<Prototype*>(visitAnnotation(variable_node->annotation,args));
        variable->setPrototype(prototype);
        return variable->as<Symbol*>();
    }

    std::any DetailCollector::visitFunction(parseTree::Function *function_node, DetailArgs args) {
        string name;
        if(function_node->name){
            name = getID(function_node->name);
        }

        if(is_name_valid(name,function_node->name->location,args.domain)){
            type::Function *function = nullptr;

            FunctionFlag flag = FunctionFlag::Static;

            if(function_node->is_static){
                flag = FunctionFlag::Static;
                if(args.domain->getKind() == type::SymbolKind::Module){
                    Logger::error(function_node->location,lang->msgFtnInModuleCannotMarkStatic());
                }
            }

            switch(function_node->method_flag){
                case MethodFlag::Virtual:
                    if(function_node->is_static){
                        Logger::error(function_node->location,lang->msgSFtnCannotMarkVirtual());
                    }

                    if(args.domain->getKind() == SymbolKind::Class){
                        flag = FunctionFlag::Virtual;
                    }
                    else if(args.domain->getKind() == SymbolKind::Module){
                        Logger::error(function_node->location,lang->msgSFtnCannotMarkVirtual());
                    }
                    break;
                case MethodFlag::Override:
                    if(function_node->is_static){
                        Logger::error(function_node->location,lang->msgSFtnCannotMarkOverride());
                    }

                    if(args.domain->getKind() == SymbolKind::Class){
                        flag = FunctionFlag::Override;
                    }
                    else if(args.domain->getKind() == SymbolKind::Module){
                        Logger::error(function_node->location,lang->msgFtnInModuleCannotMarkOverride());
                    }
                    break;
                case MethodFlag::None:
                    if(args.domain->getKind() == type::SymbolKind::Class && !function_node->is_static){
                        flag = FunctionFlag::Method;
                    }
                    break;
            }

            function = new type::UserFunction(flag);
            function->setStatic(function_node->is_static);
            function->setLocation(function_node->name->location);
            function->setName(name);
            function->setAccessFlag(function_node->access);

            args.context->byteLengthDependencies.addIsolate(function);

            if(function_node->return_annotation){
                auto prototype = any_cast<Prototype*>(visitAnnotation(function_node->return_annotation,args));
                function->setRetSignature(prototype);
            }

            args.domain->add(function);
            function_node->function_symbol = function;

            args.function = function;
            args.domain = function;
            FOR_EACH(parameter,function_node->parameter){
                visitParameter(parameter,args);
            }

            if(name == "main" && args.parent_class_or_module->equal(args.context->getGlobal())){
                args.context->setEntrance(function);
            }
        }
        return {};
    }

    std::any DetailCollector::visitExternal(parseTree::External *external_node, DetailArgs args) {
        auto name = getID(external_node->name);

        if(is_name_valid(name,external_node->name->location,args.domain)){
            auto lib = getString(external_node->lib);
            auto name = getID(external_node->name);
            auto alias = name;
            if(external_node->alias)alias = getString(external_node->alias);
            auto function = new type::ExternalFunction(lib,alias);
            function->setLocation(external_node->name->location);
            function->setName(name);
            args.domain->add(function);
            if(external_node->return_annotation){
                auto prototype = any_cast<Prototype*>(visitAnnotation(external_node->return_annotation,args));
                function->setRetSignature(prototype);
            }
            args.domain = function;
            FOR_EACH(parameter,external_node->parameter){
                visitParameter(parameter,args); 
            }
            external_node->function_symbol = function;
        }
        return {};
    }

    std::any DetailCollector::visitConstructor(parseTree::Constructor *ctor_node, DetailArgs args) {
        switch (args.parent_class_or_module->getKind()) {
            case SymbolKind::Module:
                Logger::error(ctor_node->location,lang->msgCtorOnlyAllowedInCls());
                break;
            case SymbolKind::Class:{
                auto cls = args.parent_class_or_module->as<type::Class*>();
                if(cls->getConstructor()){
                    Logger::error({ctor_node->location,cls->getConstructor()->getLocation()},
                                  lang->fmtCtorRedefine(cls->mangling('.')));
                }
                else{
                    auto constructor = new type::Constructor(ctor_node);
                    constructor->setLocation(ctor_node->location);
                    cls->setConstructor(constructor);
                    ctor_node->constructor_symbol = constructor;
                }
            }
        }
        return {};
    }

    std::any DetailCollector::visitParameter(parseTree::Parameter *parameter_node, DetailArgs args) {
        auto name = getID(parameter_node->name);
        auto prototype = any_cast<Prototype*>(visitAnnotation(parameter_node->annotation,args));
        NotNull(prototype);
        auto parameter = new type::Parameter(name, prototype, parameter_node->is_byval, parameter_node->is_optional, parameter_node->is_param_array);
        if(!parameter_node->is_optional && parameter_node->initial){
            Logger::error(parameter_node->location,lang->msgNotOptCannotSetDefaultVal());
        }

        if(parameter_node->is_byval){
            switch (prototype->getKind()) {
                case type::SymbolKind::Record:
                case type::SymbolKind::Array:
                    args.context->byteLengthDependencies.addDependent(args.function, prototype->as<Domain*>());
            }
        }
        if(is_name_valid(name,parameter_node->name->location,args.domain)){
            args.domain->add(parameter);
            parameter_node->parameter_symbol = parameter;
        }
        return {};
    }

    std::any DetailCollector::visitMember(parseTree::Member *member_node, DetailArgs args) {
        switch (member_node->member_kind) {
            case parseTree::Member::function_:    return visitFunction((parseTree::Function*)member_node, args);
            case parseTree::Member::class_:       return visitClass((parseTree::Class*)member_node, args);
            case parseTree::Member::module_:      return visitModule((parseTree::Module*)member_node, args);
            case parseTree::Member::type_:        return visitType((parseTree::Type*)member_node, args);
            case parseTree::Member::enum_:        return visitEnum((parseTree::Enum*)member_node, args);
            case parseTree::Member::dim_:         return visitDim((parseTree::Dim*)member_node, args);
            case parseTree::Member::external_:    return visitExternal((parseTree::External*)member_node, args);
            case parseTree::Member::interface_:   return visitInterface((parseTree::Interface*)member_node, args);
            case parseTree::Member::constructor_: return visitConstructor((parseTree::Constructor*)member_node, args);
        }
        PANIC;
    }
    
    bool DetailCollector::is_extend_valid(type::Class *class_symbol,type::Class *base_class){
        for(auto extend = base_class; extend != nullptr; extend = extend->getExtend()){
            if(extend == class_symbol) return false;
        }
        return true;
    }

}
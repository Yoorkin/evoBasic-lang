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

    std::any DetailCollector::visitGlobal(ast::Global **global_node, DefaultArgs args) {
        NotNull(*global_node);
        args.domain = args.parent_class_or_module = args.context->getGlobal();
        for(auto iter = (**global_node).member;iter!=nullptr;iter=iter->next_sibling){
            visitMember(&iter,args);
        }
        args.context->byteLengthDependencies.addIsolate(args.context->getGlobal());
        return {};
    }

    std::any DetailCollector::visitModule(ast::Module **module_node, DefaultArgs args) {
        NotNull(*module_node);
        args.domain = args.parent_class_or_module = (**module_node).module_symbol;
        for(auto iter = (**module_node).member;iter!=nullptr;iter=iter->next_sibling){
            visitMember(&iter,args);
        }
        return {};
    }

    std::any DetailCollector::visitInterface(ast::Interface **interface_node, DefaultArgs args) {
        NotNull(*interface_node);
        args.domain = args.parent_class_or_module = (**interface_node).interface_symbol;
        for(auto iter = (Member*)(**interface_node).function;iter!=nullptr;iter=iter->next_sibling){
            visitMember(&iter,args);
        }
        return {};
    }

    std::any DetailCollector::visitClass(ast::Class **class_node, DefaultArgs args) {
        NotNull(*class_node);
        auto class_symbol =  (**class_node).class_symbol;
        args.domain = args.parent_class_or_module = class_symbol;

        type::Class *base_class = nullptr,*object_class = args.context->getBuiltIn().getObjectClass();
        if((**class_node).extend){
            auto symbol = visitAnnotation(&(**class_node).extend,args);
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
            Logger::error((**class_node).extend->location,"inheritance is recursive");
        }

        for(auto impl=(**class_node).impl; impl!=nullptr; impl=impl->next_sibling){
            auto symbol = visitAnnotation(&impl->annotation,args);
            if(symbol.has_value()){
                auto interface = any_cast<Prototype*>(symbol)->as<type::Interface*>();
                class_symbol->addImpl(interface);
            }
        }

        for(auto iter = (**class_node).member;iter!=nullptr;iter=iter->next_sibling){
            visitMember(&iter,args);
        }

        return {};
    }


    bool DetailCollector::is_extend_valid(type::Class *class_symbol,type::Class *base_class){
        for(auto extend = base_class;extend!=nullptr;extend = extend->getExtend()){
            if(extend == class_symbol) return false;
        }
        return true;
    }

    std::any DetailCollector::visitEnum(ast::Enum **enum_node, DefaultArgs args) {
        NotNull(*enum_node);
        args.domain = (**enum_node).enum_symbol;
        int index = 0;
        for(auto iter = (**enum_node).member;iter != nullptr;iter=iter->next_sibling){
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

    std::any DetailCollector::visitType(ast::Type **type_node, DefaultArgs args) {
        NotNull(*type_node);
        auto name = getID((**type_node).name);
        auto type = (**type_node).type_symbol;
        NotNull(type);
        for(ast::Variable *iter = (**type_node).member;iter!=nullptr;iter=iter->next_sibling){
            auto var_name = getID(iter->name);
            auto variable = iter->variable_symbol;
            NotNull(variable);
            auto variable_prototype = any_cast<type::Prototype*>(visitAnnotation(&iter->annotation, args));
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

    std::any DetailCollector::visitDim(ast::Dim **dim_node, DefaultArgs args) {
        for(ast::Variable *iter=(**dim_node).variable;iter!=nullptr;iter=iter->next_sibling){
            auto variable = any_cast<Symbol*>(visitVariable(&iter,args))->as<type::Variable*>();

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

    std::any DetailCollector::visitVariable(ast::Variable **variable_node, DefaultArgs args) {
        auto name = getID((**variable_node).name);
        auto variable = (**variable_node).variable_symbol;
        NotNull(variable);
        auto prototype = any_cast<Prototype*>(visitAnnotation(&(**variable_node).annotation,args));
        variable->setPrototype(prototype);
        return variable->as<Symbol*>();
    }

    std::any DetailCollector::visitFunction(ast::Function **function_node, DefaultArgs args) {
        string name;
        if((**function_node).name){
            name = getID((**function_node).name);
        }

        if(is_name_valid(name,(**function_node).name->location,args.domain)){
            type::Function *function = nullptr;

            type::FunctionFlag flag = FunctionFlag::Static;

            if((**function_node).is_static){
                flag = type::FunctionFlag::Static;
                if(args.domain->getKind() == type::SymbolKind::Module){
                    Logger::error((**function_node).location,"Function in Module cannot be marked by 'Static'");
                }
            }

            switch((**function_node).method_flag){
                case MethodFlag::Virtual:
                    if((**function_node).is_static){
                        Logger::error((**function_node).location,"Static Method cannot be marked by 'Virtual'");
                    }

                    if(args.domain->getKind() == SymbolKind::Class){
                        flag = type::FunctionFlag::Virtual;
                    }
                    else if(args.domain->getKind() == SymbolKind::Module){
                        Logger::error((**function_node).location,"Function in Module cannot be marked by 'Virtual'");
                    }
                    break;
                case MethodFlag::Override:
                    if((**function_node).is_static){
                        Logger::error((**function_node).location,"Static Method cannot be marked by 'Override'");
                    }

                    if(args.domain->getKind() == SymbolKind::Class){
                        flag = type::FunctionFlag::Override;
                    }
                    else if(args.domain->getKind() == SymbolKind::Module){
                        Logger::error((**function_node).location,"Function in Module cannot be marked by 'Override'");
                    }
                    break;
                case MethodFlag::None:
                    if(args.domain->getKind() == type::SymbolKind::Class){
                        flag = type::FunctionFlag::Method;
                    }
                    break;
            }

            if((**function_node).is_constructor){
                function = new type::Constructor(*function_node);
            }
            else{
                function = new type::UserFunction(flag, *function_node);
            }

            function->setStatic((**function_node).is_static);
            function->setLocation((**function_node).name->location);
            function->setName(name);
            function->setAccessFlag((**function_node).access);

            args.context->byteLengthDependencies.addIsolate(function);
            
            if((**function_node).return_annotation){
                auto prototype = any_cast<Prototype*>(visitAnnotation(&(**function_node).return_annotation,args));
                function->setRetSignature(prototype);
            }
            
            args.domain->add(function);
            (**function_node).function_symbol = function;
            
            args.function = function;
            args.domain = function;
            for(ast::Parameter *parameter=(**function_node).parameter;parameter!=nullptr;parameter=parameter->next_sibling){
                visitParameter(&parameter,args);
            }

            if(name == "main" && args.domain->equal(args.context->getGlobal())){
                args.context->setEntrance(function);
            }
        }
        return {};
    }

    std::any DetailCollector::visitExternal(ast::External **external_node, DefaultArgs args) {
        auto name = getID((**external_node).name);
        
        if(is_name_valid(name,(**external_node).name->location,args.domain)){
            auto lib = getString((**external_node).lib);
            //auto alias = getString(ext_node->alias); TODO alias
            auto function = new type::ExternalFunction(lib, name);
            function->setLocation((**external_node).name->location);
            function->setName(name);
            args.domain->add(function);
            if((**external_node).return_annotation){
                auto prototype = any_cast<Prototype*>(visitAnnotation(&(**external_node).return_annotation,args));
                function->setRetSignature(prototype);
            }
            args.domain = function;
            for(auto parameter=(**external_node).parameter;parameter!=nullptr;parameter=parameter->next_sibling){
                visitParameter(&parameter,args);
            }
        }
        return nullptr;
    }

    std::any DetailCollector::visitParameter(ast::Parameter **parameter_node, DefaultArgs args) {
        auto name = getID((**parameter_node).name);
        auto prototype = any_cast<Prototype*>(visitAnnotation(&(**parameter_node).annotation,args));
        NotNull(prototype);
        auto arg = new type::Parameter(name, prototype, (**parameter_node).is_byval, (**parameter_node).is_optional,(**parameter_node).is_param_array);
        if((**parameter_node).is_byval){
            switch (prototype->getKind()) {
                case type::SymbolKind::Record:
                case type::SymbolKind::Array:
                    args.context->byteLengthDependencies.addDependent(args.function, prototype->as<Domain*>());
            }
        }
        if(is_name_valid(name,(**parameter_node).name->location,args.domain)){
            args.domain->add(arg);
        }
        return nullptr;
    }

    std::any DetailCollector::visitMember(ast::Member **member_node, DefaultArgs args) {
        switch ((**member_node).member_kind) {
            case ast::Member::function_: return visitFunction((ast::Function**)member_node,args);
            case ast::Member::class_:    return visitClass((ast::Class**)member_node,args);
            case ast::Member::module_:   return visitModule((ast::Module**)member_node,args);
            case ast::Member::type_:     return visitType((ast::Type**)member_node,args);
            case ast::Member::enum_:     return visitEnum((ast::Enum**)member_node,args);
            case ast::Member::dim_:      return visitDim((ast::Dim**)member_node,args);
            case ast::Member::external_: return visitExternal((ast::External**)member_node,args);
            case ast::Member::interface_:return visitInterface((ast::Interface**)member_node,args);
        }
        return {};
    }


}
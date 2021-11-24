//
// Created by yorkin on 11/24/21.
//

#include "symbolCollector.h"

using namespace std;

namespace evoBasic{

    std::any SymbolCollector::visitGlobal(evoBasic::ast::Global *global, evoBasic::SymbolCollectorArgs args) {
        NotNull(global);
        args.domain->setLocation(global->location);
        for(const auto& member:global->member_list){
            auto object = visitMember(member,args);
            if(!object.has_value())continue;
            auto symbol = any_cast<shared_ptr<type::Symbol>>(object);
            if(is_name_valid(symbol->getName(), symbol->getLocation(), args.domain)){
                args.domain->add(symbol);
            }
        }
        return nullptr;
    }

    std::any SymbolCollector::visitModule(ast::Module *mod_node, SymbolCollectorArgs args) {
        NotNull(mod_node);
        auto mod = make_shared<type::Module>();
        auto name = any_cast<string>(visitID(mod_node->name,args));
        mod->setName(name);
        mod->setAccessFlag(mod_node->access);
        mod->setLocation(mod_node->location);
        args.domain = mod;
        for(const auto& member:mod_node->member_list){
            auto object = visitMember(member,args);
            if(!object.has_value())continue;
            auto symbol = any_cast<shared_ptr<type::Symbol>>(object);
            args.domain->add(symbol);
        }
        return mod->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitClass(ast::Class *cls_node, SymbolCollectorArgs args) {
        NotNull(cls_node);
        auto cls = make_shared<type::Class>();
        cls->setName(any_cast<string>(visitID(cls_node->name,args)));
        cls->setAccessFlag(cls_node->access);
        cls->setLocation(cls_node->location);
        args.domain = cls;
        for(const auto& member:cls_node->member_list){
            auto object = visitMember(member,args);
            if(!object.has_value())continue;
            auto symbol = any_cast<shared_ptr<type::Symbol>>(object);
            args.domain->add(symbol);
        }
        return cls->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitEnum(ast::Enum *em_node, SymbolCollectorArgs args) {
        NotNull(em_node);
        auto em = make_shared<type::Enumeration>();
        em->setName(any_cast<string>(visitID(em_node->name,args)));
        em->setAccessFlag(em_node->access);
        em->setLocation(em_node->location);
        return em->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitType(ast::Type *ty_node, SymbolCollectorArgs args) {
        NotNull(ty_node);
        auto ty = make_shared<type::Record>();
        ty->setName(any_cast<string>(visitID(ty_node->name,args)));
        ty->setAccessFlag(ty_node->access);
        ty->setLocation(ty_node->location);
        for(auto &var_node:ty_node->member_list){
            auto symbol = any_cast<shared_ptr<type::Symbol>>(visitVariable(var_node,args));
            symbol->setAccessFlag(AccessFlag::Public);
            if(is_name_valid(symbol->getName(), var_node->location,ty)){
                ty->add(symbol);
            }
        }
        return ty->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitDim(ast::Dim *dim, SymbolCollectorArgs args) {
        NotNull(dim);
        shared_ptr<type::Symbol> symbol;
        for(const auto& var:dim->variable_list){
            symbol = any_cast<shared_ptr<type::Symbol>>(visitVariable(var,args));
            symbol->setAccessFlag(dim->access);
            if(is_name_valid(symbol->getName(), var->location, args.domain)){
                args.domain->add(symbol);
            }
        }
        return {};
    }

    std::any SymbolCollector::visitVariable(ast::Variable *var, SymbolCollectorArgs args) {
        NotNull(var);
        auto field = make_shared<type::Variable>();
        auto name = any_cast<string>(visitID(var->name,args));
        field->setName(name);
        field->setLocation(var->location);
        return field->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitID(ast::expr::ID *id, SymbolCollectorArgs args) {
        NotNull(id);
        return id->lexeme;
    }

    std::any SymbolCollector::visitMember(ast::Member *member_node, SymbolCollectorArgs args) {
        switch (member_node->member_kind) {
            case ast::Member::class_:    return visitClass((ast::Class*)member_node,args);
            case ast::Member::module_:   return visitModule((ast::Module*)member_node,args);
            case ast::Member::type_:     return visitType((ast::Type*)member_node,args);
            case ast::Member::enum_:     return visitEnum((ast::Enum*)member_node,args);
            case ast::Member::dim_:      return visitDim((ast::Dim*)member_node,args);
        }
        return {};
    }

}
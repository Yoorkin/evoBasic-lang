//
// Created by yorkin on 7/20/21.
//

#include "Semantic.h"
#include "Exception.h"
#include<stack>
#include <utility>

namespace evoBasic{


    void SymbolTable::collectSymbol(const shared_ptr<Node>& ast){
        list<pair<shared_ptr<Type::Domain>,shared_ptr<Node>>> next_list{{global,ast}};
        while(!next_list.empty()){
            auto domain = next_list.front().first;
            auto current_ast = next_list.front().second;
            next_list.pop_front();
            for(const auto& tree:current_ast->child) {
                shared_ptr<Type::DeclarationSymbol> ptr;
                AccessFlag access;
                switch(tree->tag){
                    case Tag::Module:
                        ptr = make_shared<Type::Module>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    case Tag::Class:
                        ptr = make_shared<Type::Class>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    case Tag::Type:
                        ptr = make_shared<Type::Class>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    case Tag::Enum:
                        ptr = make_shared<Type::Enumeration>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    default:
                        continue;
                        break;
                }
                auto next_domain = dynamic_pointer_cast<Type::Domain>(ptr);
                tree->set(Attr::DeclarationSymbol, ptr);
                ptr->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
                if(next_domain){
                    next_list.push_back(make_pair(next_domain,tree));
                }
                domain->add(access,ptr);
            }
        }
    }

    void SymbolTable::collectDetail(AST ast){
        list<pair<shared_ptr<Type::Domain>,shared_ptr<Node>>> next_list{{global,ast.root}};
        while(!next_list.empty()){
            auto domain = next_list.front().first;
            auto current_ast = next_list.front().second;
            next_list.pop_front();
            for(const auto& tree:current_ast->child) {
                auto tag = tree->tag;
                if(tag == Tag::Import){
                    if(auto target = visitPath(domain,tree->child[0])){
                        domain->addImport(target);
                    }
                }
                else if(tag == Tag::ExternalFunction){
                    auto c0 = tree->child[0];
                    auto c1 = tree->child[1];
                    auto c2 = tree->child[2];
                    auto lib_name = c1->tag==Tag::Empty ? "" : c1->get<std::string>(Attr::Value);
                    auto entry_name = c2->tag==Tag::Empty ? c0->get<std::string>(Attr::Lexeme)
                                                        : c2->get<std::string>(Attr::Value);
                    auto func = make_shared<Type::ExternalFunction>(lib_name,entry_name);
                    func->setName(c0->get<std::string>(Attr::Lexeme));
                    visitParameterList(domain,func,tree->child[3]);
                    if(tree->child[4]->tag!=Tag::Empty){
                        auto ptr = visitPath(domain,tree->child[4]);
                        func->setRetSignature(ptr);
                    }
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    domain->add(access,func);
                }
                else if(tag == Tag::Function){
                    auto func = std::make_shared<Type::UserFunction>(tree->child[3]);
                    func->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
                    visitParameterList(domain,func,tree->child[1]);
                    if(tree->child[2]->tag!=Tag::Empty){
                        auto ptr = visitPath(domain,tree->child[2]->child[0]);
                        func->setRetSignature(ptr);
                    }
                    else if(!entrance && func->getName()=="main" && domain->getName()=="global"){
                        entrance = func;
                    }
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    domain->add(access,func);
                }
                else if(tag == Tag::Enum){
                    auto em = std::make_shared<Type::Enumeration>();
                    em->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
                    int value = 0;
                    for(int i=1;i<tree->child.size();i++){
                        auto member = tree->child[i];
                        auto name = member->child[0]->get<std::string>(Attr::Lexeme);
                        if(member->child.size()>1)value = member->child[1]->get<int>(Attr::Value);
                        em->addEnumMember(value,name);
                        value++;
                    }
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    domain->add(access,em);
                }
                else if(tag == Tag::Type){
                    //TODO
                }
                else if(tag == Tag::Class){
                    auto symbol = tree->get<shared_ptr<Type::DeclarationSymbol>>(Attr::DeclarationSymbol);
                    auto cls = dynamic_pointer_cast<Type::Class>(symbol);
                    cls->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
                    next_list.emplace_back(cls,tree);
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    domain->add(access,cls);
                }
                else if(tag == Tag::Module){
                    auto symbol = tree->get<shared_ptr<Type::DeclarationSymbol>>(Attr::DeclarationSymbol);
                    auto mod = dynamic_pointer_cast<Type::Module>(symbol);
                    mod->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
                    next_list.emplace_back(mod,tree);
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    domain->add(access,mod);
                }
            }
        }
    }

    SymbolTable::SymbolTable(const vector<AST>& ast_list): global(new Type::Module) {
        global->setName("global");
        for(auto& ast:ast_list){
            logger = ast.logger;
            collectSymbol(ast.root);
        }
        for(auto& ast:ast_list){
            logger = ast.logger;
            collectDetail(ast);
        }
    }

    list<pair<Position,string>> getPositionLexemeListFromDot(const shared_ptr<Node> ptr){
        if(ptr->tag == Tag::ID) return {make_pair(ptr->pos(),ptr->get<string>(Attr::Lexeme))};
        auto ls = getPositionLexemeListFromDot(ptr->child[0]);
        auto rhs_lexeme = ptr->child[1]->get<string>(Attr::Lexeme);
        auto rhs_pos = ptr->child[1]->pos();
        ls.emplace_back(rhs_pos,rhs_lexeme);
        return ls;
    }

    std::shared_ptr<Type::DeclarationSymbol>
    SymbolTable::visitPath(std::shared_ptr<Type::Domain> domain, const std::shared_ptr<Node> path_node) {
        shared_ptr<Type::DeclarationSymbol> target;
        auto path = getPositionLexemeListFromDot(path_node->child[0]);
        auto beginMember = domain->lookUp(path.begin()->second);
        if(beginMember==Type::Member::Empty){
            logger->error(path.begin()->first,Format()<<"找不到对象'"<<path.begin()->second<<"'");
            return {};
        }
        path.pop_front();
        shared_ptr<Type::Domain> ptr = dynamic_pointer_cast<Type::Domain>(beginMember.symbol);
        for(const auto& p : path){
            const auto& id = p.second;
            auto next = ptr->find(id);
            if(next!=Type::Member::Empty){
                if(&p != &path.back()){
                    ptr = dynamic_pointer_cast<Type::Domain>(next.symbol);
                    if(!ptr){
                        logger->error(p.first,Format()<<"'"<<id<<"'不是个模块");
                        break;
                    }
                }
                else{
                    target = next.symbol;
                }
            }else{
                logger->error(p.first,Format()<<"找不到对象'"<<id<<"'");
                break;
            }
        }
        return target;
    }


    std::shared_ptr<Type::DeclarationSymbol>
    SymbolTable::visitAnnotation(std::shared_ptr<Type::Domain> domain, const std::shared_ptr<Node> path) {
        return visitPath(domain,path->child[0]);
    }


    void SymbolTable::visitParameterList(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Type::Function> function,
                                         const std::shared_ptr<Node> parameter_list) {
        for(auto param : parameter_list->child){
            auto isByval = param->get<bool>(Attr::IsByval);
            auto isOptional = param->get<bool>(Attr::IsOptional);
            auto name = param->child[0]->get<string>(Attr::Lexeme);
            auto symbol = visitAnnotation(domain,param->child[1]);
            function->addArgument(Type::Function::Argument(name,symbol,isByval,isOptional));
        }
    }
}
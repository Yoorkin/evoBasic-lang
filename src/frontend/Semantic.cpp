//
// Created by yorkin on 7/20/21.
//

#include "Semantic.h"
#include "Exception.h"
#include<stack>

namespace evoBasic{
    void collectDomain(const shared_ptr<Type::Domain>& domain,const shared_ptr<Node>& ast){
        for(const auto& tree:ast->child) {
            shared_ptr<Type::DeclarationSymbol> ptr;
            switch(tree->tag){
                case Tag::Module:
                    ptr = make_shared<Type::Module>();
                    break;
                case Tag::Class:
                    ptr = make_shared<Type::Class>();
                    break;
                case Tag::Type:
                    ptr = make_shared<Type::Class>();
                    break;
                case Tag::Enum:
                    ptr = make_shared<Type::Enumeration>();
                    break;
                default:
                    continue;
                    break;
            }
            auto next_domain = dynamic_pointer_cast<Type::Domain>(ptr);
            if(next_domain)collectDomain(next_domain,tree);
            tree->set(Attr::DeclarationSymbol, ptr);
            ptr->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
            domain->add(ptr);
        }
    }

    void SymbolTable::collectSymbol(const shared_ptr<Node>& ast) const {
        collectDomain(dynamic_pointer_cast<Type::Domain>(this->global), ast);
    }



    SymbolTable::SymbolTable(vector<AST> ast_list): global(new Type::Module) {
        for(auto& ast:ast_list){
            collectSymbol(ast.root);
        }
    }
}
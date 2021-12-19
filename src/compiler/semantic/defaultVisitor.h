//
// Created by yorkin on 11/27/21.
//

#ifndef EVOBASIC_DEFAULTVISITOR_H
#define EVOBASIC_DEFAULTVISITOR_H
#include "visitor.h"
#include "context.h"
#include "i18n.h"
#include "utils.h"
#include "logger.h"

namespace evoBasic{

    
    template<typename ARGS>
    class DefaultVisitor : public Visitor<ARGS> {
    private:
        bool need_lookup = false;
        Context *context = nullptr;
        Location *exception_location = nullptr;
        std::string not_found_name;
    public:
        std::any visitAnnotationUnit(parseTree::AnnotationUnit *unit_node, ARGS args) override{
            auto name = getID(unit_node->name);
            type::Symbol *symbol = nullptr;
            if(need_lookup){
                symbol = args.dot_expression_context->template as<type::Domain*>()->lookUp(name);
            }
            else{
                symbol = args.dot_expression_context->template as<type::Domain*>()->find(name);
            }

            if(!symbol){
                not_found_name = args.dot_expression_context->mangling('.') + name;
                exception_location = unit_node->name->location;
                throw std::exception();
            }
            return symbol;
        }
        std::any visitAnnotation(parseTree::Annotation *annotation_node, ARGS args) override{
            try{
                auto iter = annotation_node->unit;
                need_lookup = true;
                args.dot_expression_context = args.parent_class_or_module;;
                auto symbol = visitAnnotationUnit(iter,args);
                iter = iter->next_sibling;
                args.dot_expression_context = any_cast<type::Symbol*>(symbol);

                need_lookup = false;
                while(iter!=nullptr){
                    symbol = visitAnnotationUnit(iter,args);
                    args.dot_expression_context = any_cast<type::Symbol*>(symbol);
                    iter=iter->next_sibling;
                }

                auto element = args.dot_expression_context->template as<type::Prototype*>();
                auto ret_prototype = element;

                if(annotation_node->array_size){
                    ret_prototype = new type::Array(ret_prototype, getDigit(annotation_node->array_size));

                    if(element->getKind() == type::SymbolKind::Record)
                        context->byteLengthDependencies.addDependent(ret_prototype->template as<type::Domain*>(),element->template as<type::Domain*>());
                }

                return ret_prototype;
            }
            catch (std::exception&){
                Logger::error(exception_location, i18n::lang->fmtObjectNotFound(not_found_name));
                return context->getBuiltIn().getErrorPrototype()->as<type::Prototype*>();
            }
        }
    };

}


#endif //EVOBASIC_DEFAULTVISITOR_H

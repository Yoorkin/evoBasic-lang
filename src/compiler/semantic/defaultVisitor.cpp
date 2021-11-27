//
// Created by yorkin on 11/27/21.
//

#include "defaultVisitor.h"
#include "type.h"
#include "utils.h"
#include "logger.h"

using namespace evoBasic::type;
namespace evoBasic{
    std::any DefaultModifyVisitor::visitAnnotation(evoBasic::ast::Annotation **annotation_node, evoBasic::DefaultArgs args) {
        try{
            auto iter = (**annotation_node).unit;
            args.need_lookup = true;
            args.dot_expression_context = args.domain;
            auto symbol = visitAnnotationUnit(&iter,args);
            iter = iter->next_sibling;
            args.dot_expression_context = any_cast<Symbol*>(symbol);

            args.need_lookup = false;
            while(iter!=nullptr){
                symbol = visitAnnotationUnit(&iter,args);
                args.dot_expression_context = any_cast<Symbol*>(symbol);
                iter=iter->next_sibling;
            }

            auto element = args.dot_expression_context->as<Prototype*>();
            auto ret_prototype = element;

            if((**annotation_node).array_size){
                ret_prototype = new type::Array(ret_prototype, getDigit((**annotation_node).array_size));

                if(element->getKind() == SymbolKind::Record)
                    args.context->byteLengthDependencies.addDependent(ret_prototype->as<Domain*>(),element->as<Domain*>());
            }

            return ret_prototype;
        }
        catch (SymbolNotFound& e){
            Logger::error(e.location,format()<<"cannot find object '"
                                             <<e.search_domain->mangling('.')
                                             <<'.'<<e.search_name<<"'");
            return args.context->getBuiltIn().getErrorPrototype()->as<Prototype*>();
        }
    }

    std::any DefaultModifyVisitor::visitAnnotationUnit(evoBasic::ast::AnnotationUnit **unit_node,
                                                                 evoBasic::DefaultArgs args) {
        auto name = getID((**unit_node).name);
        Symbol *symbol = nullptr;
        if(args.need_lookup){
            symbol = args.dot_expression_context->as<Domain*>()->lookUp(name);
        }
        else{
            symbol = args.dot_expression_context->as<Domain*>()->find(name);
        }

        if(!symbol){
            throw SymbolNotFound((**unit_node).name->location,args.dot_expression_context,name);
        }
        return symbol;
    }

}

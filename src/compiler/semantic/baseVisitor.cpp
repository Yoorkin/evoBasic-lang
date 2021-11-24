//
// Created by yorkin on 11/24/21.
//

#include "baseVisitor.h"
#include "logger.h"
#include "semantic.h"

using namespace std;
using namespace evoBasic::type;

namespace evoBasic{

    //return shared_ptr<Prototype>
    std::any BaseVisitor::visitAnnotation(ast::Annotation *anno_node, BaseArgs args) {
        shared_ptr<type::Symbol> ptr;
        for(auto& unit_node:anno_node->unit_list){
            auto name = any_cast<string>(visitAnnotationUnit(unit_node,args));
            if(&unit_node == &anno_node->unit_list.front()){
                ptr = args.domain->lookUp(name);
            }
            else{
                if(!ptr){
                    Logger::error(anno_node->location,"Cannot find Object");
                    break;
                }
                auto domain = ptr->as_shared<type::Domain>();
                if(!domain){
                    Logger::error(unit_node->location,format()<<"'"<<name<<"' is not a Class,Enum,Type,Or Module");
                    break;
                }
                ptr = domain->find(name);
            }
        }

        auto prototype = ptr->as_shared<type::Prototype>();
        if(!prototype){
            Logger::error(anno_node->location,"Cannot find Object");
            return ExpressionType::Error->prototype;
        }
        if(anno_node->array_size){
            auto element_prototype = prototype;
            prototype = make_shared<type::Array>(prototype, getDigit(anno_node->array_size));

            if(element_prototype->getKind() == DeclarationEnum::Type)
                args.context->byteLengthDependencies.addDependent(prototype->as_shared<Domain>(),element_prototype->as_shared<Domain>());
        }
        return prototype;
    }

    std::any BaseVisitor::visitAnnotationUnit(ast::AnnotationUnit *unit_node, BaseArgs args) {
        NotNull(unit_node);
        return getID(unit_node->name);
    }

}
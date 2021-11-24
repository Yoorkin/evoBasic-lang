//
// Created by yorkin on 11/24/21.
//

#ifndef EVOBASIC_BASEVISITOR_H
#define EVOBASIC_BASEVISITOR_H
#include "visitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"

namespace evoBasic{

    struct BaseArgs{
        std::shared_ptr<Context> context;
        std::shared_ptr<type::UserFunction> user_function;
        std::shared_ptr<type::Domain> domain;
        std::shared_ptr<type::Symbol> dot_expression_context;
        std::shared_ptr<type::Class> parent_class;
        int checking_args_index = 0;
        bool need_lookup = false;
    };

    class BaseVisitor : public Visitor<BaseArgs>{
    public:
        std::any visitAnnotation(ast::Annotation *anno_node, BaseArgs args) override;
        std::any visitAnnotationUnit(ast::AnnotationUnit *unit_node, BaseArgs args) override;
    };

}

#endif //EVOBASIC_BASEVISITOR_H

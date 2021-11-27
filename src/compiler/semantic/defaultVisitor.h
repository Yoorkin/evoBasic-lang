//
// Created by yorkin on 11/27/21.
//

#ifndef EVOBASIC_DEFAULTVISITOR_H
#define EVOBASIC_DEFAULTVISITOR_H
#include "visitor.h"
#include "context.h"

namespace evoBasic{

    struct DefaultArgs{
        Context *context = nullptr;
        type::UserFunction *user_function = nullptr;
        type::Domain *domain = nullptr;
        type::Symbol *dot_expression_context = nullptr;
        type::Class *parent_class = nullptr;
        int checking_args_index = 0;
        bool need_lookup = false;
    };

    class DefaultModifyVisitor : public ModifyVisitor<DefaultArgs> {
        std::any visitAnnotationUnit(ast::AnnotationUnit **unit_node, DefaultArgs args) override;

    protected:
        std::any visitAnnotation(ast::Annotation **annotation_node, DefaultArgs args) override;
    };

}


#endif //EVOBASIC_DEFAULTVISITOR_H
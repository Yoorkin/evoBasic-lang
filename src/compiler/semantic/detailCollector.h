//
// Created by yorkin on 11/24/21.
//

#ifndef EVOBASIC_DETAILCOLLECTOR_H
#define EVOBASIC_DETAILCOLLECTOR_H
#include "visitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"

namespace evoBasic{
    struct DetailArgs{
        Context *context = nullptr;
        type::UserFunction *user_function = nullptr;
        type::Domain *domain = nullptr;
        type::Symbol *dot_expression_context = nullptr;
        type::Class *parent_class = nullptr;
        int checking_args_index = 0;
        bool need_lookup = false;
    };
    
    class DetailCollector : public ModifyVisitor<DetailArgs>{
    public:
        std::any visitGlobal(ast::Global **global_node, DetailArgs args) override;
        std::any visitModule(ast::Module **module_node, DetailArgs args) override;
        std::any visitClass(ast::Class **class_node, DetailArgs args) override;
        std::any visitEnum(ast::Enum **enum_node, DetailArgs args) override;
        std::any visitType(ast::Type **type_node, DetailArgs args) override;
        std::any visitDim(ast::Dim **dim_node, DetailArgs args) override;
        std::any visitVariable(ast::Variable **variable_node, DetailArgs args) override;
        std::any visitFunction(ast::Function **function_node, DetailArgs args) override;
        std::any visitExternal(ast::External **external_node, DetailArgs args) override;
        std::any visitParameter(ast::Parameter **parameter_node, DetailArgs args) override;
        std::any visitMember(ast::Member **member_node, DetailArgs args) override;
        std::any visitBinary(ast::expr::Binary **binary_node, DetailArgs args) override;
        std::any visitID(ast::expr::ID **id_node, DetailArgs args) override;
        std::any visitAnnotation(ast::Annotation **annotation_node, DetailArgs args) override;
        std::any visitAnnotationUnit(ast::AnnotationUnit **unit_node, DetailArgs args) override;
    };

}


#endif //EVOBASIC_DETAILCOLLECTOR_H

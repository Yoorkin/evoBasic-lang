//
// Created by yorkin on 11/24/21.
//

#ifndef EVOBASIC_DETAILCOLLECTOR_H
#define EVOBASIC_DETAILCOLLECTOR_H
#include "defaultVisitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"

namespace evoBasic{

    
    class DetailCollector : public DefaultModifyVisitor{
    public:
        std::any visitGlobal(ast::Global **global_node, DefaultArgs args) override;
        std::any visitModule(ast::Module **module_node, DefaultArgs args) override;
        std::any visitClass(ast::Class **class_node, DefaultArgs args) override;
        std::any visitEnum(ast::Enum **enum_node, DefaultArgs args) override;
        std::any visitType(ast::Type **type_node, DefaultArgs args) override;
        std::any visitDim(ast::Dim **dim_node, DefaultArgs args) override;
        std::any visitVariable(ast::Variable **variable_node, DefaultArgs args) override;
        std::any visitFunction(ast::Function **function_node, DefaultArgs args) override;
        std::any visitExternal(ast::External **external_node, DefaultArgs args) override;
        std::any visitParameter(ast::Parameter **parameter_node, DefaultArgs args) override;
        std::any visitMember(ast::Member **member_node, DefaultArgs args) override;
        std::any visitBinary(ast::expr::Binary **binary_node, DefaultArgs args) override;
        std::any visitID(ast::expr::ID **id_node, DefaultArgs args) override;
        bool is_extend_valid(type::Class *class_symbol,type::Class *base_class);
    };

}


#endif //EVOBASIC_DETAILCOLLECTOR_H

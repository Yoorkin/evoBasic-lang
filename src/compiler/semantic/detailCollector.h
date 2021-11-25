//
// Created by yorkin on 11/24/21.
//

#ifndef EVOBASIC_DETAILCOLLECTOR_H
#define EVOBASIC_DETAILCOLLECTOR_H
#include "visitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"
#include "baseVisitor.h"

namespace evoBasic{

    class DetailCollector : public ModifyVisitor<BaseArgs>{
    public:
        std::any visitGlobal(ast::Global **global_node, BaseArgs args) override;
        std::any visitModule(ast::Module **module_node, BaseArgs args) override;
        std::any visitClass(ast::Class **class_node, BaseArgs args) override;
        std::any visitEnum(ast::Enum **enum_node, BaseArgs args) override;
        std::any visitType(ast::Type **type_node, BaseArgs args) override;
        std::any visitDim(ast::Dim **dim_node, BaseArgs args) override;
        std::any visitVariable(ast::Variable **variable_node, BaseArgs args) override;
        std::any visitFunction(ast::Function **function_node, BaseArgs args) override;
        std::any visitExternal(ast::External **external_node, BaseArgs args) override;
        std::any visitParameter(ast::Parameter **parameter_node, BaseArgs args) override;
        std::any visitMember(ast::Member **member_node, BaseArgs args) override;
        std::any visitBinary(ast::expr::Binary **binary_node, BaseArgs args) override;
        std::any visitID(ast::expr::ID **id_node, BaseArgs args) override;
    };

}


#endif //EVOBASIC_DETAILCOLLECTOR_H

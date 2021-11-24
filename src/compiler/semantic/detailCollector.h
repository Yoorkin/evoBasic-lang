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

    class DetailCollector : public BaseVisitor{
    public:
        std::any visitGlobal(ast::Global *global, BaseArgs args) override;
        std::any visitModule(ast::Module *mod, BaseArgs args) override;
        std::any visitClass(ast::Class *cls, BaseArgs args) override;
        std::any visitEnum(ast::Enum *em, BaseArgs args) override;
        std::any visitType(ast::Type *ty, BaseArgs args) override;
        std::any visitDim(ast::Dim *dim, BaseArgs args) override;
        std::any visitVariable(ast::Variable *var, BaseArgs args) override;
        std::any visitFunction(ast::Function *func_node, BaseArgs args) override;
        std::any visitExternal(ast::External *ext_node, BaseArgs args) override;
        std::any visitParameter(ast::Parameter *param_node, BaseArgs args) override;
        std::any visitMember(ast::Member *member_node, BaseArgs args) override;
        std::any visitBinary(ast::expr::Binary *logic_node, BaseArgs args) override;
        std::any visitID(ast::expr::ID *id_node, BaseArgs args) override;
    };

}


#endif //EVOBASIC_DETAILCOLLECTOR_H

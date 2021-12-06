//
// Created by yorkin on 11/24/21.
//

#ifndef EVOBASIC_SYMBOLCOLLECTOR_H
#define EVOBASIC_SYMBOLCOLLECTOR_H
#include "visitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"

namespace evoBasic{
    struct SymbolCollectorArgs{
        type::Domain *domain = nullptr;
        Context *context = nullptr;
    };

    class SymbolCollector : public Visitor<SymbolCollectorArgs>{
    public:
        std::any visitGlobal(ast::Global *global, SymbolCollectorArgs args) override;
        std::any visitModule(ast::Module *mod, SymbolCollectorArgs args) override;
        std::any visitClass(ast::Class *cls, SymbolCollectorArgs args) override;
        std::any visitInterface(ast::Interface *interface_node, SymbolCollectorArgs args) override;
        std::any visitEnum(ast::Enum *em, SymbolCollectorArgs args) override;
        std::any visitType(ast::Type *ty, SymbolCollectorArgs args) override;
        std::any visitDim(ast::Dim *dim, SymbolCollectorArgs args) override;
        std::any visitVariable(ast::Variable *variable_node, SymbolCollectorArgs args) override;
        std::any visitID(ast::expr::ID *id, SymbolCollectorArgs args) override;
        std::any visitMember(ast::Member *member_node, SymbolCollectorArgs args) override;
    };

}


#endif //EVOBASIC_SYMBOLCOLLECTOR_H

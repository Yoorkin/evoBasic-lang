/*
 * Created by yorkin on 11/24/21.
 *
 * symbol collector.
 *
 * This phase is aims to support forward declaration
 *
 *  - bind name,access flag,static flag and location of source to below symbols:
 *      Module,Class,Interface,Enum,Type,Static/Non-static field
 *
 *  - Store those symbols in hierarchical structure and AST
 *
 *  - verify if a name has been declared
 *
 */


#ifndef EVOBASIC_SYMBOLCOLLECTOR_H
#define EVOBASIC_SYMBOLCOLLECTOR_H
#include "visitor.h"
#include "type.h"
#include "context.h"
#include <utils/utils.h>

#include <any>

namespace evoBasic{
    struct SymbolCollectorArgs{
        type::Domain *domain = nullptr;
        CompileTimeContext *context = nullptr;
    };

    class SymbolCollector : public Visitor<SymbolCollectorArgs>{
    public:
        std::any visitGlobal(parseTree::Global *global, SymbolCollectorArgs args) override;
        std::any visitModule(parseTree::Module *mod, SymbolCollectorArgs args) override;
        std::any visitClass(parseTree::Class *cls, SymbolCollectorArgs args) override;
        std::any visitInterface(parseTree::Interface *interface_node, SymbolCollectorArgs args) override;
        std::any visitEnum(parseTree::Enum *em, SymbolCollectorArgs args) override;
        std::any visitType(parseTree::Type *ty, SymbolCollectorArgs args) override;
        std::any visitDim(parseTree::Dim *dim, SymbolCollectorArgs args) override;
        std::any visitVariable(parseTree::Variable *variable_node, SymbolCollectorArgs args) override;
        std::any visitID(parseTree::expr::ID *id, SymbolCollectorArgs args) override;
        std::any visitMember(parseTree::Member *member_node, SymbolCollectorArgs args) override;
    };

}


#endif //EVOBASIC_SYMBOLCOLLECTOR_H

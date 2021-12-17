/*
 * Created by yorkin on 11/24/21.
 *
 * detail collector.
 *
 * This phase
 *   - bind name,access flag,static flag and locations of source to below symbols:
 *          Function/Sub,Constructor,External Function/Sub,Parameter,Enum members,Type fields
 *
 *   - verify if a name has been declared in the same scope
 *
 *   - collect information of extend and implements relationships and store in inheritDependencies
 *
 *   - collect relationships between primitive type and constructed type, store into byteLengthDependencies.
 *     The constructed type can be Type,Class,global variables table and local variables table of function/sub/constructor.
 *              E.g.
 *                  Type MyConstructedRecord
 *                      A as Long
 *                      B as AnotherConstructedRecord
 *                  End Type
 *
 *                  MyConstructedRecord is consist of primitive type long and AnotherConstructedRecord.
 *                  So the size of MyConstructedRecord is depends on size of long + AnotherConstructedRecord.
 *
 *   - collect information of parameter. E.g. is_optional,is_param_array,is_byval
 *
 *   - collect information of function like method flag
 *
 *   - lookup prototype for parameter,field and variable
 *
 */

#ifndef EVOBASIC_DETAILCOLLECTOR_H
#define EVOBASIC_DETAILCOLLECTOR_H
#include "defaultVisitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"

namespace evoBasic{
    struct DetailArgs{
        Context *context = nullptr;
        type::Function *function = nullptr;
        type::Domain *domain = nullptr;
        type::Symbol *dot_expression_context = nullptr;
        bool is_dot_expression_context_static = false;
        type::Domain *parent_class_or_module = nullptr;
        int checking_args_index = 0;
        bool need_lookup = false;
    };
    
    class DetailCollector : public DefaultVisitor<DetailArgs>{
    public:
        std::any visitGlobal(ast::Global *global_node, DetailArgs args) override;
        std::any visitModule(ast::Module *module_node, DetailArgs args) override;
        std::any visitClass(ast::Class *class_node, DetailArgs args) override;
        std::any visitInterface(ast::Interface *interface_node,DetailArgs args)override;

        std::any visitEnum(ast::Enum *enum_node, DetailArgs args) override;
        std::any visitType(ast::Type *type_node, DetailArgs args) override;
        std::any visitDim(ast::Dim *dim_node, DetailArgs args) override;
        std::any visitVariable(ast::Variable *variable_node, DetailArgs args) override;
        std::any visitFunction(ast::Function *function_node, DetailArgs args) override;
        std::any visitExternal(ast::External *external_node, DetailArgs args) override;
        std::any visitConstructor(ast::Constructor *ctor_node, DetailArgs args) override;
        std::any visitParameter(ast::Parameter *parameter_node, DetailArgs args) override;
        std::any visitMember(ast::Member *member_node, DetailArgs args) override;

        std::any visitAllMember(type::Domain *domain, ast::Member *member, DetailArgs args);
        bool is_extend_valid(type::Class *class_symbol,type::Class *base_class);
    };

}


#endif //EVOBASIC_DETAILCOLLECTOR_H

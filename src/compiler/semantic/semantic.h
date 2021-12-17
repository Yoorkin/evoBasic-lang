//
// Created by yorkin on 11/1/21.
//

#ifndef EVOBASIC2_SEMANTIC_H
#define EVOBASIC2_SEMANTIC_H
#include "visitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"
#include "il.h"

namespace evoBasic{

    using AST = ast::Global;

    class Semantic {
    public:
        static void collectSymbol(AST *ast,Context *context);
        static void collectDetail(AST *ast,Context *context);
        static void typeCheck(AST *ast,Context *context);
        static bool solveTypeInferenceDependencies(Context *context);
        static bool solveInheritDependencies(Context *context);
        static bool solveByteLengthDependencies(Context *context);
    };


    struct ExpressionType{
        type::Symbol *symbol = nullptr;
        /*
         *   ========= Dot Expression =======
         *   lhs\rhs    Static   Non-Static
         *   static      yes       error
         *   Non-Static  error      yes
         *
         *   ======================= Access Control =================
         *   domain\target          Static Member    Non-Static Member
         *   Static Function            yes                 error
         *   None-Static Function       yes                 yes
         *
         */
        il::DataType il_type;
        bool is_static = false;
        enum ValueKind {lvalue,rvalue,path,error,void_} value_kind;
        ExpressionType()=default;
        ExpressionType(type::Symbol *symbol,ValueKind kind,bool is_static = false){
            NotNull(symbol);
            this->value_kind = kind;
            this->symbol = symbol;
            this->is_static = is_static;
        }
        type::Prototype *getPrototype(){
            auto tmp = symbol->as<type::Prototype*>();
            NotNull(tmp);
            return tmp;
        }
        static ExpressionType *Error,*Void;
    };

    class SymbolNotFound : std::exception{
    public:
        type::Symbol *search_domain = nullptr;
        Location *location = nullptr;
        std::string search_name;
        SymbolNotFound(Location *location,type::Symbol *search_domain,std::string search_name)
                : search_domain(search_domain),search_name(search_name),location(location){}
    };
}



#endif //EVOBASIC2_SEMANTIC_H

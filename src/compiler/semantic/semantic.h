//
// Created by yorkin on 11/1/21.
//

#ifndef EVOBASIC2_SEMANTIC_H
#define EVOBASIC2_SEMANTIC_H
#include "visitor.h"
#include "type.h"
#include "context.h"
#include "utils.h"

namespace evoBasic{

    using AST = ast::Global;

    class Semantic {
    public:
        static void collectSymbol(AST *ast,std::shared_ptr<Context> context);
        static void collectDetail(AST *ast,std::shared_ptr<Context> context);
        static void typeCheck(AST *ast,std::shared_ptr<Context> context);
        static bool solveTypeInferenceDependencies(std::shared_ptr<Context> context);
        static bool solveByteLengthDependencies(std::shared_ptr<Context> context);
    };


    struct ExpressionType{
        std::shared_ptr<type::Prototype> prototype;
        enum ValueKind {lvalue,rvalue,path,error} value_kind;
        ExpressionType()=default;
        ExpressionType(std::shared_ptr<type::Prototype> prototype_,ValueKind kind){
            this->prototype = prototype_;
            this->value_kind = kind;
        }
        static ExpressionType *Error;
    };
}



#endif //EVOBASIC2_SEMANTIC_H

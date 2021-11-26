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
        static void collectSymbol(AST *ast,Context *context);
        static void collectDetail(AST *ast,Context *context);
        static void typeCheck(AST *ast,Context *context);
        static bool solveTypeInferenceDependencies(Context *context);
        static bool solveByteLengthDependencies(Context *context);
    };


    struct ExpressionType{
        type::Prototype *prototype;
        enum ValueKind {lvalue,rvalue,path,error} value_kind;
        ExpressionType()=default;
        ExpressionType(type::Prototype *prototype_,ValueKind kind){
            this->prototype = prototype_;
            this->value_kind = kind;
        }
        static ExpressionType *Error;
    };
}



#endif //EVOBASIC2_SEMANTIC_H

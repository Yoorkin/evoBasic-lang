//
// Created by yorkin on 7/20/21.
//

#ifndef EVOBASIC_SEMANTIC_H
#define EVOBASIC_SEMANTIC_H
#include"Type.h"
#include"AST.h"
namespace evoBasic{

    class Scanner{
        Domain *visitModule(Domain *parent);
        Domain *visitClass(Domain *parent);
        Domain *visitEnum(Domain *parent);
        Domain *visitType(Domain *parent);
        Domain *visitVariable(Domain *parent);
        Domain *visitFunction(Domain *parent);
        Domain *visitSub(Domain *parent);
    public:
        void scan();
    };

    class Analyzer{

    };
}


#endif //EVOBASIC_SEMANTIC_H

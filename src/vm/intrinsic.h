//
// Created by yorkin on 11/20/21.
//

#ifndef EVOBASIC_INTRINSIC_H
#define EVOBASIC_INTRINSIC_H
#include "data.h"
#include "stack.h"
namespace evoBasic::vm{
    enum class intrinsic : data::ptr{
        allocate = 0,dynamicCast
    }

    void *allocate_intrinsic(data::ptr size){

    }

    void dynamic_cast_intrinsic(Stack *stack){
        
    }

}


#endif //EVOBASIC_INTRINSIC_H

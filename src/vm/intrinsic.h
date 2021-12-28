//
// Created by yorkin on 11/20/21.
//

#ifndef EVOBASIC_INTRINSIC_H
#define EVOBASIC_INTRINSIC_H
#include "data.h"
#include "stack.h"
namespace evoBasic::vm{
    enum class intrinsic : data::ptr{
        putByte,getByte
    };

}


#endif //EVOBASIC_INTRINSIC_H

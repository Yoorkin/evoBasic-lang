//
// Created by yorkin on 11/20/21.
//

#ifndef EVOBASIC_INTRINSIC_H
#define EVOBASIC_INTRINSIC_H
#include "memory.h"
#include <functional>
#include "runtime.h"

namespace evoBasic::vm{

    enum class IntrinsicEnum {PutChar,GetChar,MemSet,ItNotInRange,DebugInt,DebugBool,MAlloc,Free};

    using IntrinsicHandler = std::function<void(Stack*)>;

    namespace intrinsic{
        void callHandler(IntrinsicEnum index,Stack *operand,RuntimeContext *context);
    }

}


#endif //EVOBASIC_INTRINSIC_H

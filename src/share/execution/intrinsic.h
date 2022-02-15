//
// Created by yorkin on 11/20/21.
//

#ifndef EVOBASIC_INTRINSIC_H
#define EVOBASIC_INTRINSIC_H
#include "memory.h"
#include <functional>

namespace evoBasic::vm{

    enum class IntrinsicEnum {PutChar,GetChar,MemSet,ItNotInRange};

    using IntrinsicHandler = std::function<void(Stack*)>;

    namespace intrinsic{

        IntrinsicHandler getHandler(IntrinsicEnum handler_enum);

        void putchar_(Stack *operand);

        void getchar_(Stack *operand);

        void memset_(Stack *operand);

        void isIteratorNotInRange(Stack *operand);

    }




}


#endif //EVOBASIC_INTRINSIC_H

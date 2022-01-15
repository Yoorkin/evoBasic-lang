//
// Created by yorkin on 11/20/21.
//

#ifndef EVOBASIC_INTRINSIC_H
#define EVOBASIC_INTRINSIC_H
#include "memory.h"
#include <functional>
namespace evoBasic::vm{

    using IntrinsicHandler = std::function<void(Stack*)>;

    std::vector<IntrinsicHandler> getIntrinsicHandlerList();

    void intrinsic_putchar(Stack *operand);

    void intrinsic_getchar(Stack *operand);

}


#endif //EVOBASIC_INTRINSIC_H

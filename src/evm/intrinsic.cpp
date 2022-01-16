//
// Created by yorkin on 12/7/21.
//

#include "intrinsic.h"

namespace evoBasic::vm{

    std::vector<IntrinsicHandler> vm::getIntrinsicHandlerList() {
        return {
            intrinsic_putchar,
            intrinsic_getchar
        };
    }

    void intrinsic_putchar(Stack *operand) {
        auto raw = operand->pop<data::u8>();
        putchar(raw);
    }

    void intrinsic_getchar(Stack *operand) {
        auto ret = (data::u8)getchar();
        operand->push<data::u8>(ret);
    }
}
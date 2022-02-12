//
// Created by yorkin on 12/7/21.
//

#include "intrinsic.h"

namespace evoBasic::vm{

    namespace intrinsic{

        IntrinsicHandler getHandler(IntrinsicEnum handler_enum){
            switch(handler_enum){
                case IntrinsicEnum::PutChar: return putchar_;
                case IntrinsicEnum::GetChar: return getchar_;
                case IntrinsicEnum::MemSet:  return memset_;
            }
        }

        void putchar_(Stack *operand) {
            auto raw = operand->pop<data::u8>();
            putchar(raw);
        }

        void getchar_(Stack *operand) {
            auto ret = (data::u8)getchar();
            operand->push<data::u8>(ret);
        }

        void memset_(Stack *operand) {
            auto size = operand->pop<data::u32>();
            auto value = operand->pop<data::u8>();
            auto ptr = operand->pop<data::u8*>();

            while(size--){
                *ptr = value;
                ptr++;
            }
        }
    }

}
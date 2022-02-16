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
                case IntrinsicEnum::ItNotInRange: return isIteratorNotInRange;
                case IntrinsicEnum::PutInt:       return putInt;
                case IntrinsicEnum::DebugBool:      return DebugBool;
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

        void isIteratorNotInRange(Stack *operand) {
            auto end = operand->pop<data::i32>();
            auto beg = operand->pop<data::i32>();
            auto iter = operand->pop<data::i32>();
            /*
            *  beg < end & (iter < beg | iter > end) ||
            *  beg > end & (iter < end | iter > beg) ||
            *  beg == end & iter != beg
            */
            bool result = (beg < end && (iter < beg || iter > end) ||
                beg > end && (iter < end || iter > beg) ||
                beg == end && iter != beg);

            operand->push((data::boolean)result);
        }

        void putInt(Stack *operand){
            auto value = operand->pop<data::i32>();
            std::cout<<'#'<<value<<std::endl;
        }

        void DebugBool(Stack *operand){
            auto value = operand->pop<data::boolean>();
            std::cout<<'#'<<(value==0?"false":"true")<<std::endl;
        }
    }

}
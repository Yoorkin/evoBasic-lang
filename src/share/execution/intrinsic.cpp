//
// Created by yorkin on 12/7/21.
//

#include "intrinsic.h"

namespace evoBasic::vm{

    namespace intrinsic{

        template<class Return,class ...Args>
        void handler_call_parameter_expand(Return(*handler)(Args...),Stack *operand){
            std::tuple<Args...> args{(operand->pop<Args>())...};
            Return ret = std::apply(*handler,std::move(args));
            operand->push<Return>(ret);
        }

        template<class ...Args>
        void handler_call_parameter_expand(void(*handler)(Args...),Stack *operand){
            std::tuple<Args...> args{(operand->pop<Args>())...};
            std::apply(*handler,std::move(args));
        }

        template<class Return,class ...Args>
        void handler_call_parameter_expand_with_context(Return(*handler)(Args...),Stack *operand,RuntimeContext *context){
            std::tuple<RuntimeContext,Args...> args{context,(operand->pop<Args>())...};
            Return ret = std::apply(*handler,std::move(args));
        }

        template<class ...Args>
        void handler_call_parameter_expand_with_context(void(*handler)(Args...),Stack *operand,RuntimeContext *context){
            std::tuple<RuntimeContext,Args...> args{context,(operand->pop<Args>())...};
            std::apply(*handler,std::move(args));
        }




        void putchar_(data::u8 c) {
            putchar(c);
            std::fflush(stdout);
        }

        data::u8 getchar_() {
            return (data::u8)getchar();
        }

        void memset_(data::u32 size,data::u8 value,data::u8 *ptr) {
            while(size--){
                *ptr = value;
                ptr++;
            }
        }

        auto isIteratorNotInRange(data::i32 end,data::i32 beg,data::i32 iter)->data::boolean {
            /*
            *  beg < end & (iter < beg | iter > end) ||
            *  beg > end & (iter < end | iter > beg) ||
            *  beg == end & iter != beg
            */
            bool result = (beg < end && (iter < beg || iter > end) ||
                            beg > end && (iter < end || iter > beg) ||
                            beg == end && iter != beg);

            return ((data::boolean)result);
        }

        void debugInt(data::i32 value){
            std::cout<<'#'<<value<<std::endl;
        }

        void debugBool(data::boolean value){
            std::cout<<'#'<<(value==0?"false":"true")<<std::endl;
        }

        auto malloc_(data::u32 size)->data::Byte*{
            return (data::Byte*)malloc(size);
        }

        void free_(data::Byte *ptr){
            free(ptr);
        }

//        void derefBool(Stack *operand);
//        void derefU8(Stack *operand);
//        void derefU16(Stack *operand);
//        void derefU32(Stack *operand);
//        void derefU64(Stack *operand);
//        void derefI8(Stack *operand);
//        void derefI16(Stack *operand);
//        void derefI32(Stack *operand);
//        void derefI64(Stack *operand);
//        void derefF32(Stack *operand);
//        void derefF64(Stack *operand);
//        void derefRef(Stack *operand);
//
//        auto getTypeID(Stack *operand)->data::Byte* {
//
//        }

        void callHandler(IntrinsicEnum index,Stack *operand,RuntimeContext *context){
            using enum IntrinsicEnum;
            switch(index){
                case PutChar:
                    handler_call_parameter_expand(putchar_,operand);
                    break;
                case GetChar:
                    handler_call_parameter_expand(getchar_,operand);
                    break;
                case ItNotInRange:
                    handler_call_parameter_expand(isIteratorNotInRange,operand);
                    break;
                case MAlloc:
                    handler_call_parameter_expand(malloc_,operand);
                    break;
                case Free:
                    handler_call_parameter_expand(free_,operand);
                    break;
                case DebugBool:
                    handler_call_parameter_expand(debugBool,operand);
                    break;
                case DebugInt:
                    handler_call_parameter_expand(debugInt,operand);
                    break;
                default: PANIC;
            }
        }

    }

}
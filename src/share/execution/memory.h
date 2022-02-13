//
// Created by yorkin on 11/7/21.
//

#ifndef EVOBASIC2_STACK_H
#define EVOBASIC2_STACK_H
#include <stdlib.h>
#include "utils/data.h"
#include "utils/logger.h"
#include "utils/format.h"
namespace evoBasic::vm{

    class Memory{
        data::Byte *mem = nullptr;
        bool is_borrowed = false;
        Memory()=default;
    public:
//        Memory(Memory& x){
//            this->mem = x.mem;
//            x.mem = nullptr;
//        }

        explicit Memory(size_t size){
            mem = (data::Byte*)malloc(size);
        }

        void copyFrom(data::u16 length, data::u64 dst_offset, data::Byte *src){
            auto beg = mem + dst_offset;
            auto end = beg + length;
            while(beg<end){
                *beg = *src;
                beg++;
                src++;
            }
        }

        void copyTo(data::u16 length, data::u64 src_offset, data::Byte *dst){
            auto beg = mem + src_offset;
            auto end = beg + length;
            while(beg<end){
                *dst = *beg;
                dst++;
                beg++;
            }
        }

        template<class T>
        void write(data::u64 offset,T t){
            copyFrom(sizeof(T),offset,(data::Byte*)&t);
        }

        data::Byte *address(data::u64 offset){
            return mem + offset;
        }

        Memory borrow(data::u64 offset){
            Memory ret;
            ret.mem = mem + offset;
            ret.is_borrowed = true;
            return ret;
        }

        data::Byte *getRawPtrAt(data::u64 offset){
            return address(offset);
        }

        ~Memory(){
            if(!is_borrowed)
                free(mem);
        }
    };

    class Stack {
        Memory memory;
        data::u64 top_idx = 0;
    public:
        Stack(const Stack&)=delete;

        explicit Stack(size_t size)
            :memory(size){}

        template<typename T>
        T top(){
            return *((T*)memory.address(top_idx-sizeof(T)));
        }

        template<typename T>
        void push(T t){
            pushFrom(sizeof(T),(data::Byte*)&t);
        }

        template<typename T>
        T pop(){
            T t;
            popTo(sizeof(T),(data::Byte*)&t);
            return t;
        }

        void pushFrom(data::u16 length, data::Byte *src){
            memory.copyFrom(length,top_idx,src);
            top_idx += length;
        }

        void popTo(data::u16 length, data::Byte *dst){
            top_idx -= length;
            memory.copyTo(length,top_idx,dst);
        }

        template<typename T>
        void dup(){
            push<T>(top<T>());
        }
    };

}



#endif //EVOBASIC2_STACK_H

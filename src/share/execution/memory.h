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
    class Stack {
        void *mem = nullptr;
        char *ptr = nullptr;
    public:
        Stack(const Stack&)=delete;
        explicit Stack(size_t size){
            mem = ptr = (char*)malloc(size);
        }

        template<typename T>
        void push(T t){
            (*((T*)ptr))=t;
            ptr+=sizeof(T);
        }

        template<typename T>
        T top(){
            return *(((T*)ptr)-1);
        }

        template<typename T>
        T pop(){
            auto tmp = top<T>();
            ptr-=sizeof(T);
            return tmp;
        }

        void pushFrom(data::u16 length, data::Byte *src){
            while(length--){
                *ptr = *src;
                ptr++;
                src++;
            }
        }

        void popTo(data::u16 length, data::Byte *dst){
            while(length--){
                *(dst + length) = *ptr;
                ptr--;
            }
        }

        template<typename T>
        void dup(){
            push<T>(top<T>());
        }

        void *getPtr(){
            return ptr;
        }

        ~Stack(){
            if(mem!=NULL){
                //free(mem);
            }
        }
    };

    class Memory{
        char *mem = nullptr;
        bool is_borrowed = false;
        Memory()=default;
    public:
        Memory(size_t size){
            mem = (char*)malloc(size);
        }

        template<class T>
        T read(data::u64 offset){
            return *((T*)(mem + offset));
        }

        template<class T>
        void write(data::u64 offset,T value){
            *((T*)(mem + offset)) = value;
        }

        template<class T>
        T *address(data::u64 offset){
            return ((T*)(mem + offset));
        }

        Memory borrow(data::u64 offset){
            Memory ret;
            ret.mem = mem + offset;
            ret.is_borrowed = true;
            return ret;
        }

        data::Byte *getRawPtrAt(data::u64 offset){
            return (data::Byte*)(mem + offset);
        }

        ~Memory(){
            if(!is_borrowed)delete mem;
        }
    };
}



#endif //EVOBASIC2_STACK_H

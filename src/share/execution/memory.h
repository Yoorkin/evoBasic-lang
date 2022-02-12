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

        void push(data::u16 length, char *src){
            while(length--){
                *ptr = *src;
                ptr++;
                src++;
            }
        }

        void pop(data::u16 length, char *dst){
            while(length--){
                *(dst + length) = *ptr;
                ptr--;
            }
        }

        void copy(data::u16 length,data::Byte *dst){
            ptr-=length;
            for(int i =0;i<length;i++){
                dst[i]=ptr[i];
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
            delete mem;
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

        ~Memory(){
            if(!is_borrowed)delete mem;
        }
    };
}



#endif //EVOBASIC2_STACK_H

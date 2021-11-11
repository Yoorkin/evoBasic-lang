//
// Created by yorkin on 11/7/21.
//

#ifndef EVOBASIC2_STACK_H
#define EVOBASIC2_STACK_H
#include<stdlib.h>
namespace evoBasic::vm{
    class Stack {
        void *mem = nullptr;
        void *ptr = nullptr;
    public:
        explicit Stack(size_t size){
            ptr = mem = (char*)malloc(size);
        }

        template<typename T>
        void push(T t){
            (*((T*)ptr))=t;
            ((T*)ptr)++;
        }

        template<typename T>
        void pop(){
            ((T*)ptr)--;
        }

        template<typename T>
        T top(){
            return *(((T*)ptr)-1);
        }

        template<typename T>
        void dup(){
            push<T>(top<T>());
        }

        void *getPtr(){
            return ptr;
        }
    };
}



#endif //EVOBASIC2_STACK_H

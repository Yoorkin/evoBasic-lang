//
// Created by yorkin on 11/7/21.
//

#ifndef EVOBASIC2_STACK_H
#define EVOBASIC2_STACK_H
#include<stdlib.h>
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

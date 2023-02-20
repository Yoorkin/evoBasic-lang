#ifndef EVM_UTILS
#define EVM_UTILS
#include <cstdint>
#include <cstdlib>
#include <codecvt>
#include <locale>
#include "bytecode.h"

class MemoryStack{
    uint8_t *stack = nullptr,
            *top = nullptr;
public:

    template<class T>
    inline void push(T t){
        write(top,t);
        top += sizeof(T);
    }

    inline void pushFromPtr(uint8_t *src, int size){
        memcpy(top,src,size);
        top += size;
    }

    inline void popToPtr(uint8_t *dst, int size){
        top -= size;
        memcpy(dst,top,size);
    }

    inline void moveFromPtr(uint8_t *src, int size){
        //use memmove because there is an overlaping region
        memmove(top,src,size);
        top += size;
    }

    template<class T>
    inline T pop(){
        top -= sizeof(T);
        auto r = read<T>(top);
        return r;
    }

    template<class T>
    inline T peek(){
        return read<T>(top - sizeof(T));
    }

    template<class T>
    inline T *ptrToPeek(){
        return (T*)(top - sizeof(T));
    }

    inline uint8_t *popAndGetTop(int count){ 
        top -= count;
        return top;
    }

    inline void pop(int count){
        top -= count;
    }

    inline uint8_t *borrow(int count){
        auto r = top;
        top += count;
        return r;
    }

    inline MemoryStack(uint32_t size){
        top = stack = (uint8_t*)malloc(size);
    }
    inline ~MemoryStack(){
        free(stack);
        stack = nullptr;
    }

    inline uint8_t *base(){
        return stack;
    }
};


template<class T>
inline std::string genericTypeToString(){
    T t;
    return "<Unexpected>"; 
}

#define GENERIC_TYPE_DEBUG(X) template<> inline std::string genericTypeToString<X>(){ return #X; }
GENERIC_TYPE_DEBUG(uint8_t)
GENERIC_TYPE_DEBUG(uint16_t)
GENERIC_TYPE_DEBUG(uint32_t)
GENERIC_TYPE_DEBUG(uint64_t)
GENERIC_TYPE_DEBUG(int8_t)
GENERIC_TYPE_DEBUG(int16_t)
GENERIC_TYPE_DEBUG(int32_t)
GENERIC_TYPE_DEBUG(int64_t)
GENERIC_TYPE_DEBUG(float)
GENERIC_TYPE_DEBUG(double)
GENERIC_TYPE_DEBUG(bool)
namespace interop { 
    struct Instance; 
    struct InteriorPointer;
    struct RecordOpaque;
}
GENERIC_TYPE_DEBUG(interop::Instance*)
GENERIC_TYPE_DEBUG(interop::InteriorPointer)
GENERIC_TYPE_DEBUG(interop::RecordOpaque)
#undef GENERIC_TYPE_DEBUG

#define DEBUG
#ifdef DEBUG
    #define LOG(tag,x) std::clog<<std::left<<std::setw(20)<<std::setfill('.')<<std::string("["#tag) + "]"<<x;std::clog.flush();
#else
    #define LOG(tag,x)
#endif

#endif
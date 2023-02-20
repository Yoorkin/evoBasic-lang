#ifndef EVM_GC
#define EVM_GC
#include "interop.h"
#include "runtime.h"
#include <cstdint>
#include <exception>
#include <queue>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <iomanip>

class Reference{
    enum class ReferenceKind{InstanceRef,InteriorRef,PinedIns};
    using Ins = interop::Instance;
    Ins **ins_ptr = nullptr;
    Ins *pined_ins = nullptr;
    interop::InteriorPointer *interior = nullptr;
    
    ReferenceKind kind;
    explicit inline Reference(Ins **ptr) : ins_ptr(ptr), kind(ReferenceKind::InstanceRef) {}
    explicit inline Reference(interop::InteriorPointer *ptr) : interior(ptr), kind(ReferenceKind::InteriorRef) {}
    explicit inline Reference(Ins *pined) : pined_ins(pined), kind(ReferenceKind::PinedIns) {}

public:

    static inline Reference fromPined(Ins *pined){ return Reference(pined); }
    static inline Reference fromInterior(interop::InteriorPointer *ptr){ return Reference(ptr); }
    static inline Reference fromRefPtr(Ins **ptr){ return Reference(ptr); }

    inline void updateForward(){
        switch(kind) {
            case ReferenceKind::InstanceRef:
                if((*ins_ptr)->forward!=nullptr){
                    *ins_ptr = (*ins_ptr)->forward;
                }
                break;
            case ReferenceKind::InteriorRef: {
                auto ins = interop::getInteriorPointerInstance(*interior);
                if(ins!=nullptr){
                    if(ins->forward != nullptr){
                        interior->ptr = (uint8_t*)ins->forward + interior->offset; 
                    }
                }
                break;
            }
            case ReferenceKind::PinedIns:
                throw std::invalid_argument("cannot move pined object");
                break;
        }
    }

    inline Ins* get() {
        switch (kind) {
            case ReferenceKind::InstanceRef:
                return *ins_ptr;
            case ReferenceKind::InteriorRef:
                return interop::getInteriorPointerInstance(*interior);
            case ReferenceKind::PinedIns:
                return pined_ins;
        }
    }

    inline uintptr_t getID(){
        switch (kind){
            case ReferenceKind::InstanceRef:
                return (uintptr_t)ins_ptr;
            case ReferenceKind::InteriorRef:
                return (uintptr_t)interior;
            case ReferenceKind::PinedIns:
                return (uintptr_t)pined_ins;
        }
    }

    inline std::list<Reference> getReferences(){
        std::list<Reference> ls;
        Ins *ins = get();
        if(ins==nullptr)return ls;
        char *ins_beg_ptr = (char*)ins;  
        if(auto spec_ary = dynamic_cast<runtime::SpecializedArray*>(ins->klass)){
            auto ary_ins = (interop::ArrayInstance*)ins;
            // 获取数组实例存储的引用
            if(runtime::instancesOf<runtime::Class>(spec_ary->getElementType())){
                for(int i = 0; i<ary_ins->length; i++){
                    auto offset = sizeof(interop::ArrayInstance) + i*sizeof(interop::Instance*);
                    ls.push_back(fromRefPtr((Ins**)(ins_beg_ptr + offset)));
                }
            }
        }
  
        // 获取普通实例存储的引用
        for(auto offset : ins->klass->getInstanceRefFieldOffsets()){
            ls.push_back(fromRefPtr((Ins**)(ins_beg_ptr + offset)));
        }
        
        return ls;
    }
};


class GarbageCollector{
    char *space_begin,*space_end;
    char *from_semi_space,*to_semi_space;
    char *free_semi;
    int semi_space_size;

    std::map<uintptr_t,Reference> root_set;

public:
    inline int remainSemiSpace() const{
        return semi_space_size - (free_semi - from_semi_space);
    }

    inline GarbageCollector(int semi_space_size) : semi_space_size(semi_space_size){
        space_begin = (char*)malloc(semi_space_size * 2);
        space_end = space_begin + semi_space_size * 2;

        free_semi = from_semi_space = space_begin;
        to_semi_space = space_begin + semi_space_size;
    }

    inline void addRoot(uintptr_t id, Reference reference){
        if(id!=reference.getID())throw ""; //debug
        if(reference.get()!=nullptr && reference.get()->klass==nullptr)throw "";//debug
        root_set.insert({id, reference});
    }

    inline void removeRoot(uintptr_t id){
        root_set.erase(id);
    }

    inline void addRoot(interop::Instance **ptr){
        root_set.insert({(uintptr_t)ptr, Reference::fromRefPtr(ptr)});
    }

    inline void removeRoot(interop::Instance** ptr) {
        root_set.erase((uintptr_t)ptr);
    }

    inline bool isYoung(Reference &ref){ 
        auto pos = (char*)ref.get();
        return pos >= space_begin && pos < space_end;    
    }

    inline interop::Instance *allocate(runtime::Class *klass, uint32_t size){
        if(remainSemiSpace() < size) {
            minorGC();
        }

        if(remainSemiSpace() < size) {
            throw std::invalid_argument("out of heap memory");
            //return nullptr;
        }
        LOG(MinorGC, "allocate:"<<size<<" at "<<std::hex<<(uintptr_t)free_semi <<std::dec<<" remain:"<< remainSemiSpace() - size<<std::endl);
        memset(free_semi, 0, size);
        auto ret = (interop::Instance*)free_semi;
        ret->klass = klass;
        free_semi += size;
        return ret;
    }

    inline interop::Instance *allocate(runtime::Class *klass){
        //return (interop::Instance*)malloc(klass->getInstanceMemorySize());
        auto ins = allocate(klass, klass->getInstanceMemorySize());
        ins->klass = klass;
        return ins;
    }

    inline unicode::string debugRunesString(interop::Instance *ins){
        unicode::string ret;
        //if(ins->klass->name == "Rune[]"_utf32){
        //    auto arr = (interop::ArrayInstance*)ins;
        //    uint32_t *ptr = (uint32_t*)((uint8_t*)ins + sizeof(interop::ArrayInstance));
        //    for(int i=0;i<arr->length;i++){
        //        ret.push_back(ptr[i]);
        //    }
        //}
        //else if(ins->klass->name == "String"_utf32){
        //    auto runeArray = ((interop::StringInstance*)ins)->runes;
        //    unicode::codepoint *ptr = (unicode::codepoint*)(((uint8_t*)runeArray) + sizeof(interop::ArrayInstance));
        //    return unicode::string(ptr,runeArray->length);
        //}
        return ret;
    }

    inline void copyAndUpdateRef(Reference &ref){
        if(isYoung(ref)){
            if(ref.get()->forward == 0){ //未被移动的对象
                LOG(MinorGC, "@ move survivor "<<std::hex<<ref.get()<<std::dec<<"("<<ref.get()->klass->name
                    <<debugRunesString(ref.get())
                    <<", size "<< interop::getInstanceSize(ref.get()) <<", age "<< (int)ref.get()->age <<") to "
                    <<std::hex<< (void*)free_semi <<std::dec<<std::endl);
                
                auto ins_size = interop::getInstanceSize(ref.get());
                //set forward address
                ref.get()->forward = (interop::Instance*)free_semi;

                //copy to new space
                memcpy(free_semi, ref.get(), ins_size);

                //update this reference to new place inside root instance
                ref.updateForward();
                //increase age
                ref.get()->age++;

                free_semi += ins_size;
            }
            else{ //已经移动的对象
                ref.updateForward();
            }
        }
    }

    inline void minorGC(){
        LOG(MinorGC, "@ trigger minorGC. usage " << semi_space_size << "/" << free_semi - from_semi_space << std::endl);

        //reset forward field in all young object
        char *unreseted = from_semi_space;
        while(unreseted < free_semi){
            auto ins = (interop::Instance*)unreseted;
            ins->forward = 0;
            unreseted += interop::getInstanceSize(ins);
        }

        free_semi = to_semi_space;
        char *unscanned = to_semi_space;

        for(auto [_,root] : root_set){
            //root.get()->klass->name == "OutOfRangeException"
            copyAndUpdateRef(root);
        }

        while(unscanned < free_semi){
            Reference obj = Reference::fromRefPtr((interop::Instance**)&unscanned);

            // copy objects and update references inside survivor
            for(auto ref : obj.getReferences()){
                copyAndUpdateRef(ref);
            }

            unscanned += interop::getInstanceSize(obj.get());
        }

#ifdef DEBUG
        auto discard = from_semi_space;
        while(true){
            auto ins = (interop::Instance*)discard;
            if(ins->klass==0)break;
            if(ins->forward==0){
                LOG(MinorGC,"discard object "<<std::hex<<(void*)ins<<std::dec<<" "<<ins->klass->qualifiedName()<<debugRunesString(ins)<<std::endl)
            }
            discard += interop::getInstanceSize(ins);
        }
        //memset for debug purpose.
        memset(from_semi_space, 0, semi_space_size);
#endif

        // swap two semi space
        auto tmp = from_semi_space;
        from_semi_space = to_semi_space;
        to_semi_space = tmp;

        LOG(MinorGC, "@ MinorGC finished. usage "<< semi_space_size <<"/"<< free_semi - from_semi_space<<std::endl<<std::endl);
    }

    inline void pin(interop::Instance* ins) {
        if(ins->pined == 1)return;
        char age = ins->age + 1;
        auto new_place = (interop::Instance*)malloc(interop::getInstanceSize(ins));
        ins->pined = 1; // mark as pined object
        ins->forward = new_place;
        ins->age++;
        minorGC();
        new_place->age = age;
        new_place->forward = 0;
        LOG(MinorGC, "pin at " << std::hex << (uintptr_t)new_place << std::dec << std::endl);
    }

    inline void unpin(interop::Instance* ins) {
        if(ins->pined==0)return;
        auto size = interop::getInstanceSize(ins);

        if (remainSemiSpace() < size) {
            minorGC();
        }

        if (remainSemiSpace() < size) {
            throw std::invalid_argument("out of heap memory");
        }

        LOG(MinorGC, "unpin:" << size << " at " << std::hex << (uintptr_t)free_semi << std::dec << " remain:" << remainSemiSpace() - size << std::endl);
        
        ins->pined = 0;
        ins->age++;
        memcpy(free_semi, ins, size);
        free_semi += size;
        free(ins);
    }

    inline interop::ProtectedCell makeProtectedCell(interop::Instance *ins){
        return interop::ProtectedCell(this,ins);
    }

    //void incrementalGC();
};



// class Container{
//     interop::Instance *instance;
// public:

    
// };

// class Car{
//     Car *predecessor = nullptr,
//         *successor = nullptr;
//     int number,size;
//     Container containers[3];
//     std::list<Reference> remember_set;
// };

// class Train{
//     int number;
//     Car *car;
//     std::list<Reference> remember_set;
// };

#endif
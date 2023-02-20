#ifndef EVO_INTEROP
#define EVO_INTEROP
#include "runtime.h"
#include "unicode.h"
#include "utils.h"
#include <cstdint>
#include <stdexcept>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

class Processor;
class Loader;
class CallEnv;
class GarbageCollector;


namespace interop {

    PACK(struct Instance {
        runtime::Class *klass;
        struct Instance *forward;
        uint8_t pined;
        uint8_t age;
    });

    PACK(struct ArrayInstance{
        Instance base;
        int32_t length;
    }); 

    PACK(struct StringInstance {
        Instance base;
        ArrayInstance *runes;
    }); 

    PACK(struct ExceptionInstance {
        Instance base;
        StringInstance *message;
        StringInstance *name;
        StringInstance *trace;
    });

    PACK(struct InteriorPointer {
        uint8_t *ptr;
        uint32_t offset;
    });

    struct RecordOpaque{};

    enum class DelegateKind{Ctor,Ftn,VFtn,SFtn,Foreign};

    PACK(struct Delegate{
        DelegateKind kind;
        runtime::Symbol *function = nullptr;
        interop::Instance *instance = nullptr;
    });
    
    inline Instance* getInteriorPointerInstance(InteriorPointer p) {
        if(p.offset == UINT32_MAX)return nullptr;
        return (Instance*)(p.ptr - p.offset);
    }

    inline InteriorPointer makeInteriorPointer(uint8_t *ptr) {
        InteriorPointer p;
        p.ptr = ptr;
        p.offset = UINT32_MAX;
        return p;
    }

    inline InteriorPointer makeInteriorPointer(Instance* ins, uint32_t offset) {
        InteriorPointer p;
        p.ptr = ((uint8_t*)ins) + offset;
        p.offset = offset;
        return p;
    }

    uint32_t getInstanceSize(interop::Instance *ins);

    class ProtectedCell {
        GarbageCollector* gc;
        struct SharedObj{
            interop::Instance *ins;
            int shared_count;
        };
        SharedObj *obj;
    public:
        explicit ProtectedCell(GarbageCollector* gc, interop::Instance* ins);
        ProtectedCell(ProtectedCell &&that);
        ProtectedCell(const ProtectedCell &that);
        ~ProtectedCell();
        template<class T = interop::Instance*>
        inline T get() { return (T)(obj->ins); }
    };


    class Value{
        enum Kind{Boolean,I8,I16,I32,I64,U8,U16,U32,U64,F32,F64,Ref}kind;
        union{
            bool boolean_val;
            int8_t i8_val;
            int16_t i16_val;
            int32_t i32_val;
            int64_t i64_val;
            uint8_t u8_val;
            uint16_t u16_val;
            uint32_t u32_val;
            uint64_t u64_val;
            float f32_val;
            double f64_val;
        }val;
        std::optional<ProtectedCell> ref_val{};
    public:
        inline static Value fromBoolean(bool value){
            Value ret;
            ret.kind = Boolean;
            ret.val.boolean_val = value;
            return ret;
        }
        inline static Value fromI8(int8_t value){
            Value ret;
            ret.kind = I8;
            ret.val.i8_val = value;
            return ret;
        }
        inline static Value fromI16(int16_t value){
            Value ret;
            ret.kind = I16;
            ret.val.i16_val = value;
            return ret;
        }
        inline static Value fromI32(int32_t value){
            Value ret;
            ret.kind = I32;
            ret.val.i32_val = value;
            return ret;
        }
        inline static Value fromI64(int64_t value){
            Value ret;
            ret.kind = I64;
            ret.val.i64_val = value;
            return ret;
        }
        inline static Value fromU8(uint8_t value){
            Value ret;
            ret.kind = U8;
            ret.val.u8_val = value;
            return ret;
        }
        inline static Value fromU16(uint16_t value){
            Value ret;
            ret.kind = U16;
            ret.val.u16_val = value;
            return ret;
        }
        inline static Value fromU32(uint32_t value){
            Value ret;
            ret.kind = U32;
            ret.val.u32_val = value;
            return ret;
        }
        inline static Value fromU64(uint64_t value){
            Value ret;
            ret.kind = U64;
            ret.val.u64_val = value;
            return ret;
        }
        inline static Value fromF32(float value){
            Value ret;
            ret.kind = F32;
            ret.val.f32_val = value;
            return ret;
        }
        inline static Value fromF64(double value){
            Value ret;
            ret.kind = F64;
            ret.val.f64_val = value;
            return ret;
        }
        inline static Value fromRef(ProtectedCell cell){
            Value ret;
            ret.kind = Ref;
            ret.ref_val.emplace(std::move(cell));
            return ret;
        }

        void pushToStack(Processor *processor) const;
    };

    enum class Intrinsic {
        NotFound,
        PutChar,
        GetChar,
        MemSet,
        ItNotInRange,
        DebugInt,
        DebugBool,
        GetSelfPointer,
        Len,
        DebugLong,
        PutRune,
        BooleanToString,
        ByteToString,
        ShortToString,
        IntegerToString,
        LongToString,
        SingleToString,
        DoubleToString,
        UShortToString,
        UIntegerToString,
        ULongToString,
        GetCallStackTrace,
        ArrayToString,
        DebugObjAddr,
        Trap,
        StringToCStr,
        Pin,
        Unpin,
        DisableGC,
        EnableGC,
        AryPtr,
        ObjPtr,
        RefPtr
    };

    class Agent{
        Processor *processor;
    public:
        ProtectedCell createInstance(runtime::Class *klass, std::list<Value> parameters);
        ProtectedCell createArray(runtime::SpecializedArray *array, int count);
        ProtectedCell createString(unicode::string string);

        interop::Instance *createUnprotectedInstance(runtime::Class *klass, std::list<Value> parameters);
        interop::ArrayInstance *createUnprotectedArray(runtime::SpecializedArray *array, int count);
        interop::StringInstance *createUnprotectedString(unicode::string string);


        unicode::string fetchStringFromInstance(StringInstance *instance);
        void callIntrinsic(Intrinsic intrinsic, Processor *processor);
        Intrinsic getInstrinsicByName(unicode::string name);

        explicit Agent(Loader *loader);
    };

}

inline std::ostream& operator<<(std::ostream& os, const interop::InteriorPointer& p){
    return os << "InteriorPointer{ ptr:"<<p.ptr<<", "<<p.offset<<"}";
}

#endif
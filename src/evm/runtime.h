//
// Created by yorkin on 1/12/22.
//

#ifndef EVOBASIC_RUNTIME_H
#define EVOBASIC_RUNTIME_H
#include <vector>
#include <map>
#include <string>
#include "memory.h"
#include "intrinsic.h"
#include <loader/il.h>
namespace evoBasic::vm{

    class RuntimeContext;
    class TokenTable;

    enum class RuntimeKind{
        Context,Class,Enum,Module,Record,Interface,Function,
        VirtualFtnSlot,FieldSlot,StaticFieldSlot,Intrinsic,
        ForeignFunction,BuiltIn
    };

    class Runtime{
        friend RuntimeContext;
    protected:
        TokenTable *table = nullptr;
    public:
        explicit Runtime(TokenTable *table);
        virtual RuntimeKind getKind()=0;
        TokenTable *getTokenTable();
    };

    class Sizeable{
        friend evoBasic::vm::RuntimeContext;
        data::u64 size = 0;
    public:
        data::u64 getByteLength();
    };


    enum class BuiltInKind {
        boolean,u8,u16,u32,u64,i8,i16,i32,i64,f32,f64
    };

    class BuiltIn : public Runtime{
        BuiltInKind kind;
    public:
        explicit BuiltIn(BuiltInKind kind);
        RuntimeKind getKind()override{ return RuntimeKind::BuiltIn; }
        BuiltInKind getBuiltInKind();
    };

    class NameSpace : public Runtime{
        friend CompileTimeContext;
    protected:
        std::map<std::string,Runtime*> childs;
    public:
        explicit NameSpace(TokenTable *table);
        virtual Runtime *find(std::string name);
    };

    class TokenTable{
        evoBasic::vm::RuntimeContext *context = nullptr;
        std::vector<il::TokenDef*> tokens;
        std::vector<Runtime*> cache;
        void loadCache(data::u64 idx);
    public:
        TokenTable(RuntimeContext *context, std::vector<il::TokenDef*> tokens);
        template<typename T>
        T *getRuntime(il::TokenDef::ID id){
            if(!cache[id]) loadCache(id);
            return (T*)cache[id];
        }
    };

    class Function : public Runtime{
        friend evoBasic::vm::RuntimeContext;
        il::FunctionDefine *il_info = nullptr;
        std::vector<Runtime*> params;
        std::vector<Runtime*> locals;
        std::vector<data::u64> params_offset;
        std::vector<data::u64> locals_offset;
        std::vector<data::u16> params_length;
        data::u64 params_frame_size = -1;
        data::u64 locals_frame_size = -1;
    public:
        Function(TokenTable *table, il::FunctionDefine *info);
        RuntimeKind getKind()override{ return RuntimeKind::Function; }
        data::Byte *getBlock();
        const std::vector<Runtime*> &getParams();
        const std::vector<Runtime*> &getLocals();
        const std::vector<data::u16> &getParamsLength();
        data::u64 getParamOffset(data::u16 index);
        data::u64 getLocalOffset(data::u16 index);
        data::u64 getParamsStackFrameLength();
        data::u64 getLocalsStackFrameLength();
        data::u64 getStackFrameLength();
    };

    class ForeignFunction : public Runtime{
        friend evoBasic::vm::RuntimeContext;
        std::string library,name;
        il::Ext *il_info = nullptr;
    public:
        ForeignFunction(std::string library,std::string name,il::Ext *info);
        RuntimeKind getKind()override{ return RuntimeKind::ForeignFunction; }
    };

    class Intrinsic : public Runtime{
    private:
        il::Ext *il_info = nullptr;
        IntrinsicHandler handler;
    public:
        Intrinsic(IntrinsicHandler handler,il::Ext *info);
        RuntimeKind getKind()override{ return RuntimeKind::Intrinsic; }
        void invoke(vm::Stack *operand);
    };

    class VirtualFtnSlot : public Runtime{
        friend evoBasic::vm::RuntimeContext;
        il::VFtn *il_info = nullptr;
        data::u64 offset = -1;
    public:
        VirtualFtnSlot(data::u64 offset,il::VFtn *info);
        RuntimeKind getKind()override{ return RuntimeKind::VirtualFtnSlot; }
        data::u64 getOffset();
    };

    class FieldSlot : public Runtime{
        friend evoBasic::vm::RuntimeContext;
        data::u64 offset = -1;
        il::Fld *il_info = nullptr;
    public:
        FieldSlot(data::u64 offset,il::Fld *info);
        RuntimeKind getKind()override{ return RuntimeKind::FieldSlot; }
    };

    class StaticFieldSlot : public Runtime{
        friend evoBasic::vm::RuntimeContext;
        data::u64 offset = -1;
        il::SFld *il_info = nullptr;
    public:
        StaticFieldSlot(data::u64 offset,il::SFld *info);
        RuntimeKind getKind()override{ return RuntimeKind::StaticFieldSlot; }
    };

    class Module : public NameSpace,public Sizeable{
        friend evoBasic::vm::RuntimeContext;
        il::Module *il_info = nullptr;
        char *static_field_memory = nullptr;
    public:
        Module(TokenTable *table,il::Module *info);
        RuntimeKind getKind()override{ return RuntimeKind::Module; }
    };

    class Record : public NameSpace,public Sizeable{
        friend evoBasic::vm::RuntimeContext;
        il::Record *il_info = nullptr;
        data::u64 byte_length = 0;
    public:
        Record(TokenTable *table,il::Record *info);
        RuntimeKind getKind()override{ return RuntimeKind::Record; }
    };

    class Class : public NameSpace,public Sizeable{
        friend evoBasic::vm::RuntimeContext;
        char *static_field_memory = nullptr;
        Class *base_class = nullptr;
        std::vector<Function*> vtable;
        il::Class *il_info = nullptr;
    public:
        Class(TokenTable *table,il::Class *info);
        RuntimeKind getKind()override{ return RuntimeKind::Class; }
        Function *virtualFunctionDispatch(VirtualFtnSlot slot);
        template<typename T>
        T read(StaticFieldSlot *slot){

        }
        template<typename T>
        void write(StaticFieldSlot *slot, T value){

        }
        template<typename T>
        T address(StaticFieldSlot *slot){

        }
        Runtime *find(std::string name)override;
    };

    class Enum : public NameSpace{
        friend evoBasic::vm::RuntimeContext;
        il::Enum *il_info = nullptr;
    public:
        Enum(TokenTable *table,il::Enum *info);
        RuntimeKind getKind()override{ return RuntimeKind::Enum; }
        il::Enum *late_binding = nullptr;
    };

    class RecordInstance{
        Record *record = nullptr;
        char *field_memory = nullptr;
    public:
        template<typename T>
        T *getField(FieldSlot slot){

        }
    };

    struct ClassInstance{
        Class *klass = nullptr;
        char *field_memory = nullptr;
    public:
        template<typename T>
        T read(FieldSlot *slot){

        }
        template<typename T>
        void write(FieldSlot *slot,T value){

        }
        template<typename T>
        T address(FieldSlot *slot){

        }
        Class *getClass();
    };

    class RuntimeContext : public NameSpace{
        std::list<TokenTable*> token_tables;
        std::vector<IntrinsicHandler> intrinsic_handler_table;
        using NameRuntimePair = std::pair<std::string,Runtime*>;
        std::optional<NameRuntimePair> collectSymbolRecursively(TokenTable *table,il::Member *member);
        void collectDependencies(Runtime *parent, Runtime *current, Dependencies<Class*> &inherit, Dependencies<Record*> &include);
        void recordFieldsResolution(Record *record);
        void classFieldsAndVTableResolution(Class *cls);
        RuntimeKind getKind()override{ return RuntimeKind::Context; }
        void collectDetailRecursively(Runtime *runtime);
    public:
        explicit RuntimeContext(std::list<il::Document*> &documents);

    };



}

#endif //EVOBASIC_RUNTIME_H

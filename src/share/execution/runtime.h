//
// Created by yorkin on 1/12/22.
//

#ifndef EVOBASIC_RUNTIME_H
#define EVOBASIC_RUNTIME_H
#include <vector>
#include <map>
#include <utils/unicode.h>
#include "memory.h"

#include "loader/il.h"
namespace evoBasic::vm{

    class RuntimeContext;
    class TokenTable;
    class Class;
    class Module;
    class Global;

    enum class RuntimeKind{
        Global,Class,Enum,Module,Record,Interface,Function,
        VirtualFtnSlot,FieldSlot,StaticFieldSlot,
        ForeignFunction,BuiltIn,Array
    };

    class Runtime{
        friend RuntimeContext;
    protected:
        TokenTable *table = nullptr;
    public:
        explicit Runtime(TokenTable *table);
        virtual RuntimeKind getKind()=0;
        TokenTable *getTokenTable();
        virtual DebugInfo *toStructuredInfo()=0;
    };

    class Sizeable{
    protected:
        friend evoBasic::vm::RuntimeContext;
        data::u64 size = 0;
    public:
        data::u64 getByteLength();
    };


    enum class BuiltInKind {
        boolean,u8,u16,u32,u64,i8,i16,i32,i64,f32,f64
    };

    class Array : public Runtime, public Sizeable{
        Runtime *element_type = nullptr;
        data::u64 count;
    public:
        Array(Runtime *element,data::u64 count);
        Runtime *getElementRuntime();
        data::u64 getElementCount();
        RuntimeKind getKind()override{ return RuntimeKind::Array; }
        DebugInfo *toStructuredInfo()override;
    };

    class BuiltIn : public Runtime, public Sizeable{
        BuiltInKind kind;
    public:
        explicit BuiltIn(BuiltInKind kind);
        RuntimeKind getKind()override{ return RuntimeKind::BuiltIn; }
        BuiltInKind getBuiltInKind();
        DebugInfo *toStructuredInfo()override;
    };

    class NameSpace : public Runtime{
        friend CompileTimeContext;
    protected:
        std::map<unicode::Utf8String,Runtime*> childs;
    public:
        explicit NameSpace(TokenTable *table);
        virtual Runtime *find(unicode::Utf8String name);
        const std::map<unicode::Utf8String,Runtime*> &getChilds();
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
            return dynamic_cast<T*>(cache[id]);
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
        DebugInfo *toStructuredInfo()override;
    };

    class ForeignFunction : public Runtime{
        friend evoBasic::vm::RuntimeContext;
        unicode::Utf8String library,name;
        il::Ext *il_info = nullptr;
    public:
        ForeignFunction(unicode::Utf8String library,unicode::Utf8String name,il::Ext *info);
        RuntimeKind getKind()override{ return RuntimeKind::ForeignFunction; }
        DebugInfo *toStructuredInfo()override;
    };

    class VirtualFtnSlot : public Runtime{
        friend evoBasic::vm::RuntimeContext;
        il::VFtn *il_info = nullptr;
        data::u64 offset = -1;
    public:
        VirtualFtnSlot(data::u64 offset,il::VFtn *info);
        RuntimeKind getKind()override{ return RuntimeKind::VirtualFtnSlot; }
        data::u64 getOffset();
        DebugInfo *toStructuredInfo()override;
    };

    class FieldSlot : public Runtime{
        friend evoBasic::vm::RuntimeContext;
        data::u64 offset = -1;
        il::Fld *il_info = nullptr;
    public:
        FieldSlot(data::u64 offset,il::Fld *info);
        data::u64 getOffset();
        RuntimeKind getKind()override{ return RuntimeKind::FieldSlot; }
        DebugInfo *toStructuredInfo()override;
    };

    class StaticFieldSlot : public Runtime{
        friend evoBasic::vm::RuntimeContext;
        friend Class;
        friend Module;
        friend RuntimeContext;
        data::u64 offset = -1;
        il::SFld *il_info = nullptr;
        Runtime *owner = nullptr;
    public:
        StaticFieldSlot(Module *owner,data::u64 offset,il::SFld *info);
        StaticFieldSlot(Class *owner,data::u64 offset,il::SFld *info);
        StaticFieldSlot(Global *owner,data::u64 offset,il::SFld *info);
        RuntimeKind getKind()override{ return RuntimeKind::StaticFieldSlot; }
        data::Byte *getAddress();
        DebugInfo *toStructuredInfo()override;
    };

    class Module : public NameSpace,public Sizeable{
        friend evoBasic::vm::RuntimeContext;
        il::Module *il_info = nullptr;
        data::Byte *static_field_memory = nullptr;
    public:
        Module(TokenTable *table,il::Module *info);
        RuntimeKind getKind()override{ return RuntimeKind::Module; }
        data::Byte *getStaticFieldPtr(data::u64 offset);
        DebugInfo *toStructuredInfo()override;
        ~Module();
    };

    class Record : public NameSpace,public Sizeable{
        friend evoBasic::vm::RuntimeContext;
        il::Record *il_info = nullptr;
        data::u64 byte_length = 0;
    public:
        Record(TokenTable *table,il::Record *info);
        RuntimeKind getKind()override{ return RuntimeKind::Record; }
        DebugInfo *toStructuredInfo()override;
    };

    class Class : public NameSpace,public Sizeable{
        friend evoBasic::vm::RuntimeContext;
        data::Byte *static_field_memory = nullptr;
        Class *base_class = nullptr;
        std::vector<Function*> vtable;
        il::Class *il_info = nullptr;
    public:
        Class(TokenTable *table,il::Class *info);
        RuntimeKind getKind()override{ return RuntimeKind::Class; }
        Function *virtualFunctionDispatch(VirtualFtnSlot slot);

        data::Byte *getStaticFieldPtr(data::u64 offset);
        Runtime *find(unicode::Utf8String name)override;
        DebugInfo *toStructuredInfo()override;
        ~Class();
    };

    class Global : public NameSpace{
        friend evoBasic::vm::RuntimeContext;
        data::Byte *global_static_field_memory = nullptr;
    public:
        explicit Global(): NameSpace(nullptr){}
        RuntimeKind getKind()override{ return RuntimeKind::Global; }
        data::Byte *getStaticFieldPtr(data::u64 offset);
        DebugInfo *toStructuredInfo()override;
        ~Global();
    };


    class Enum : public NameSpace{
        friend evoBasic::vm::RuntimeContext;
        il::Enum *il_info = nullptr;
    public:
        Enum(TokenTable *table,il::Enum *info);
        RuntimeKind getKind()override{ return RuntimeKind::Enum; }
        il::Enum *late_binding = nullptr;
        DebugInfo *toStructuredInfo()override;
    };


    class RuntimeContext{
        Global *global = nullptr;
        std::list<TokenTable*> token_tables;
        using NameRuntimePair = std::pair<unicode::Utf8String,Runtime*>;
        std::optional<NameRuntimePair> collectSymbolRecursively(TokenTable *table,il::Member *member);
        void collectDependencies(Runtime *parent, Runtime *current, Dependencies<Class*> &inherit, Dependencies<Record*> &include);
        void recordFieldsResolution(Record *record);
        void classFieldsAndVTableResolution(Class *cls);
        void collectDetailRecursively(Runtime *runtime);
        void fillModuleStaticField(Runtime *runtime);
        void fillGlobalStaticFields(std::list<std::pair<TokenTable*,il::Document*>> &document_list);
    public:
        Function *getEntrance();
        Global *getGlobalRuntime();
        explicit RuntimeContext(std::list<il::Document*> &documents);
        unicode::Utf8String debug();
        ~RuntimeContext();
    };



}

#endif //EVOBASIC_RUNTIME_H

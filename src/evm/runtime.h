//
// Created by yorkin on 1/12/22.
//

#ifndef EVOBASIC_RUNTIME_H
#define EVOBASIC_RUNTIME_H
#include <vector>
#include <map>
#include <string>
#include "stack.h"
#include <loader/il.h>
namespace evoBasic::runtime{

    class RuntimeContext;
    class TokenTable;

    enum class RuntimeKind{
        Context,Class,Enum,Module,Record,Interface,ExecutableEnv,
        VirtualFtnSlot,FieldSlot,StaticFieldSlot,Intrinsic,
        ForeignFunction
    };

    class Runtime{
        friend RuntimeContext;
        TokenTable *table = nullptr;
    public:
        explicit Runtime(TokenTable *table);
        virtual RuntimeKind getKind()=0;
    };

    class Sizeable{
        friend evoBasic::runtime::RuntimeContext;
        data::u64 size = 0;
    public:
        data::u64 getByteLength();
    };

    class NameSpace : public Runtime{
        friend CompileTimeContext;
    protected:
        std::map<std::string,Runtime*> childs;
    public:
        explicit NameSpace(TokenTable *table);
        virtual Runtime *find(std::string &name);
    };

    class TokenTable{
        evoBasic::runtime::RuntimeContext *context = nullptr;
        std::vector<il::TokenDef*> tokens;
        std::vector<Runtime*> cache;
        void loadCache(data::u64 idx);
    public:
        TokenTable(evoBasic::runtime::RuntimeContext *context, std::vector<il::TokenDef*> tokens);
        template<typename T>
        T *getRuntime(il::TokenDef::ID id){
            if(!cache[id]) loadCache(id);
            return (T*)cache[id];
        }
    };

    class ExecutableEnv : public Runtime{
        friend evoBasic::runtime::RuntimeContext;
        il::FunctionDefine *il_info = nullptr;
        data::Byte *block = nullptr;
    public:
        ExecutableEnv(TokenTable *table,il::FunctionDefine *info);
        RuntimeKind getKind()override{ return RuntimeKind::ExecutableEnv; }
    };

    class ForeignFunction : public Runtime{
        friend evoBasic::runtime::RuntimeContext;
        std::string library,name;
        il::Ext *il_info = nullptr;
    public:
        ForeignFunction(std::string library,std::string name,il::Ext *info);
        RuntimeKind getKind()override{ return RuntimeKind::ForeignFunction; }
    };

    class Intrinsic : public Runtime{
    public:
        using Handler = std::function<void(vm::Stack*)>;
    private:
        il::Ext *il_info = nullptr;
        Handler handler;
    public:
        Intrinsic(Handler handler,il::Ext *info);
        RuntimeKind getKind()override{ return RuntimeKind::Intrinsic; }
        void invoke(vm::Stack *operand);
    };

    class VirtualFtnSlot : public Runtime{
        friend evoBasic::runtime::RuntimeContext;
        il::VFtn *il_info = nullptr;
        data::u64 offset = -1;
    public:
        VirtualFtnSlot(data::u64 offset,il::VFtn *info);
        RuntimeKind getKind()override{ return RuntimeKind::VirtualFtnSlot; }
    };

    class FieldSlot : public Runtime{
        friend evoBasic::runtime::RuntimeContext;
        data::u64 offset = -1;
        il::Fld *il_info = nullptr;
    public:
        FieldSlot(data::u64 offset,il::Fld *info);
        RuntimeKind getKind()override{ return RuntimeKind::FieldSlot; }
    };

    class StaticFieldSlot : public Runtime{
        friend evoBasic::runtime::RuntimeContext;
        data::u64 offset = -1;
        il::SFld *il_info = nullptr;
    public:
        StaticFieldSlot(data::u64 offset,il::SFld *info);
        RuntimeKind getKind()override{ return RuntimeKind::StaticFieldSlot; }
    };

    class Module : public NameSpace,public Sizeable{
        friend evoBasic::runtime::RuntimeContext;
        il::Module *il_info = nullptr;
        char *static_field_memory = nullptr;
    public:
        Module(TokenTable *table,il::Module *info);
        RuntimeKind getKind()override{ return RuntimeKind::Module; }
    };

    class Record : public NameSpace,public Sizeable{
        friend evoBasic::runtime::RuntimeContext;
        il::Record *il_info = nullptr;
        data::u64 byte_length = 0;
    public:
        Record(TokenTable *table,il::Record *info);
        RuntimeKind getKind()override{ return RuntimeKind::Record; }
    };

    class Class : public NameSpace,public Sizeable{
        friend evoBasic::runtime::RuntimeContext;
        char *static_field_memory = nullptr;
        Class *base_class = nullptr;
        std::vector<ExecutableEnv*> vtable;
        il::Class *il_info = nullptr;
    public:
        Class(TokenTable *table,il::Class *info);
        RuntimeKind getKind()override{ return RuntimeKind::Class; }
        ExecutableEnv *virtualFunctionDispatch(VirtualFtnSlot slot);
        template<typename T>
        T *getStaticField(StaticFieldSlot slot){

        }
        Runtime *find(std::string &name)override;
    };

    class Enum : public NameSpace{
        friend evoBasic::runtime::RuntimeContext;
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
        T *getField(FieldSlot slot){

        }
    };

    class RuntimeContext : public NameSpace{
        std::list<TokenTable*> token_tables;
        std::vector<Intrinsic::Handler> intrinsic_handler_table;
    public:
        using NameRuntimePair = std::pair<std::string,Runtime*>;
        explicit RuntimeContext(std::list<il::Document*> &documents);
        std::optional<NameRuntimePair> collectSymbolRecursively(TokenTable *table,il::Member *member);
        void collectDependencies(Runtime *parent, Runtime *current, Dependencies<Class*> &inherit,
                                 Dependencies<Record*> &include);

        void recordFieldsResolution(Record *record);

        void classFieldsAndVTableResolution(Class *cls);
        RuntimeKind getKind()override{ return RuntimeKind::Context; }

        void collectDetailRecursively(Runtime *runtime);
    };

}

#endif //EVOBASIC_RUNTIME_H

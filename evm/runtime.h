#ifndef EVM_RUNTIME
#define EVM_RUNTIME
#include <algorithm>
#include <cctype>
#include <google/protobuf/repeated_ptr_field.h>
#include <map>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <list>
#include <algorithm>
#include "bytecode.h"
#include "backage.pb.h"
#include "unicode.h"

class FFIEntry;
class TokenTable;

namespace runtime{

    class Method;
    class Ctor;
    class Scope;
    class Symbol;

    template<class T,class I>
    bool instancesOf(I *i){
        return dynamic_cast<T*>(i) != nullptr;
    }

    class Symbol{
    public:
        std::string debug_name;
        unicode::string name;
        Scope *parent = nullptr;
        virtual void complete(){}
        unicode::string qualifiedName();
        Symbol(unicode::string name) : name(name),debug_name(unicode::toPlatform(name)){}
    };

    inline bool isNamedObject(const Symbol *symbol){
        return unicode::toLower(symbol->name) == "object"_utf32;
    }

    inline bool isNamedArray(const Symbol *symbol){
        return unicode::toLower(symbol->name) == "array"_utf32;
    }

    inline bool isNamedMain(const Symbol *symbol){
        return unicode::toLower(symbol->name) == "main"_utf32;
    }

    uint32_t getRuntimeSize(Symbol *target);

    class Scope : public Symbol{
    protected:
        std::map<std::string,Symbol*> debug_childern;
        std::map<unicode::string,Symbol*> childern;
        inline Scope(unicode::string name) : Symbol(name){}
    public:
        const std::map<unicode::string,Symbol*> &getChildern(){return childern;}

        Symbol *find(unicode::string name){
            if(childern.contains(name)) return childern[name];
            else return nullptr;
        }

        Scope(unicode::string name, std::list<Symbol*> childern) : Symbol(name){
            for(auto child : childern){
                child->parent = this;
                this->childern.insert({child->name,child});
                this->debug_childern.insert({child->debug_name,child});
            }
        }
    };

    class Global : public Scope{
        uint8_t *static_memory = nullptr;
        uint32_t static_memory_size = -1;
        Method *main = nullptr;
    public:
        inline Method *getMainMethod(){ return main; }
        inline Symbol *searchByQualifiedName(std::initializer_list<unicode::string> qualified_name){
            Symbol *sym = this;
            for(auto name : qualified_name){
                if(auto scope = dynamic_cast<Scope*>(sym)){
                    sym = scope->find(name);
                }
                else{
                    return nullptr;
                }
            }
            return sym;
        }
        void add(Symbol *symbol);
        uint8_t *getStaticMemory();
        uint32_t getStaticMemorySize();
        void complete()override;
        Global() : Scope("global"_utf32,{}) {}
    };

    class Module : public Scope{
        // Backage::ModuleDecl &decl;
        TokenTable &table;
        uint32_t static_memory_size = -1;
        uint8_t *static_memory = nullptr;
    public:
        void complete()override;
        Module(unicode::string name, TokenTable &table, std::list<Symbol*> childern)
            : Scope(name,childern) , table(table){}
    };

    class Class : public Scope{
        uint32_t flag;
        Class *base_class = nullptr;

        uint32_t instance_memory_size = -1;

        uint32_t static_memory_size = -1;
        uint8_t *static_memory = nullptr;

        std::map<unicode::string,int> virtual_table_map;
        std::vector<Method*> virtual_table;

    protected:
        std::vector<uint32_t> instance_ref_offsets;

    public:
        void completeFieldAndVFtn(Class *base);
        virtual std::list<Symbol*> getDependencies() = 0;

        Ctor *getCtor();
        uint32_t getInstanceMemorySize()const;
        uint32_t getStaticMemorySize()const;

        inline Class *getBaseClass(){ return base_class; }

        inline Method *dispatchMethod(int virtual_method_offset){
            return virtual_table[virtual_method_offset];
        }

        inline const std::vector<uint32_t> &getInstanceRefFieldOffsets(){ return instance_ref_offsets; }

        uint32_t getFlag(){ return flag; }

        Class(unicode::string name, const uint32_t flag, std::list<Symbol*> childern)
            : Scope(name,childern), flag(flag){}
    };


    class PackageDefinedClass : public Class{
        const Backage::ClassDecl decl;
        TokenTable &table;
    public:
        std::list<Symbol*> getDependencies()override;
        void complete()override;

        uint32_t getFlag(){return decl.flag();}

        PackageDefinedClass(unicode::string name, TokenTable &table, const Backage::ClassDecl decl, std::list<Symbol*> childern)
            : Class(name,decl.flag(),childern), decl(decl), table(table){}
    };


    class SpecializedArray : public Class{
        Symbol &element_type;
        Class *array_base_type;
    public:
        inline Symbol *getElementType(){ return &element_type; }
        std::list<Symbol*> getDependencies()override;
        void complete()override;

        SpecializedArray(Symbol &element_type, Class *array_base_type)
            : Class(element_type.name + "[]"_utf32, bytecode::flag_nothing, {}), 
            element_type(element_type), array_base_type(array_base_type){}
    };

    class Record : public Scope{
        const Backage::RecordDecl decl;
        TokenTable &table;
        uint32_t record_memory_size = -1;
    public:
        void complete()override;
        std::list<Record*> getDependencies();
        uint32_t getMemorySize()const;
        uint32_t getFlag(){return decl.flag();}

        Record(unicode::string name, TokenTable &table, const Backage::RecordDecl decl, std::list<Symbol*> childern)
            : Scope(name,childern), decl(decl), table(table) {}
    };

    class Variable : public Symbol{
        const Backage::VariableDecl decl;
        TokenTable &table;
        uint32_t memory_offset = -1;
        uint32_t memory_length = -1;
        uint8_t *static_memory_address = nullptr;
    public:
        uint32_t getOffset();
        void setOffset(uint32_t value);

        inline uint32_t getLength(){ return memory_length; }
        inline void setLength(uint32_t value){ memory_length = value; }

        inline uint8_t *getStaticAddress(){return static_memory_address;}
        inline void setStaticAddress(uint8_t *addr){
            static_memory_address = addr;
            memory_offset = -1;
        }

        uint32_t getFlag(){return decl.flag();}
        Symbol *getType();
        Variable(unicode::string name, TokenTable &table, const Backage::VariableDecl decl)
            : Symbol(name), decl(decl), table(table){}
    };

    //class OptionParameter : public Symbol{
    //    uint32_t flag_offset;
    //    uint32_t value_offset;
    //    uint32_t value_size;
    //    bool is_obj_ref;
    //public:
    //    inline uint32_t getFlagOffset(){ return flag_offset; }
    //    inline uint32_t getValueOffset(){ return value_offset; }
    //    inline uint32_t getValueSize(){ return value_size; }
    //    inline bool isObjRef(){ return is_obj_ref; }
    //    inline OptionParameter(unicode::string name, bool is_obj_ref, uint32_t flag_offset, uint32_t value_offset, uint32_t value_size)
    //        : Symbol(name), is_obj_ref(is_obj_ref), flag_offset(flag_offset), value_offset(value_offset), value_size(value_size){}
    //};

    struct LineNumber{
        int line,begin,end;
        LineNumber(int line,int begin,int end) : line(line),begin(begin),end(end){}
    };

    class LineNumberTable{
        std::vector<LineNumber> numbers;
        inline int determineLineR(int offset,int beg,int end){
            auto mid = (beg + end) / 2;
            auto target = numbers[mid];
            if(offset < target.begin) return determineLineR(offset, beg, mid);
            else if(offset > target.end) return determineLineR(offset, mid, end);
            else return target.line;
        }
    public:
        inline LineNumber &getLineNumber(int index){ return numbers[index]; }
        inline int getNumberCount() const { return numbers.size(); }
        inline int determineLine(int offset){ 
            if(offset<0)throw "";
            if(numbers.size()==0)return 0;
            return determineLineR(offset, 0, numbers.size()); 
        }
        explicit LineNumberTable(std::vector<LineNumber> num) : numbers(num){}
    };

    enum class ParameterKind{Normal,Optional,ParamArray};
    enum class EvaluationKind{Byval,Byref};

    struct Range{
        uint32_t begin = 0;
        uint32_t length = 0;
        Range() = default;
        Range(uint32_t begin, uint32_t length)
            : begin(begin), length(length) {}
    };

    class Parameter : public Symbol{
        ParameterKind kind;
        EvaluationKind eval_kind;
        Symbol *type = nullptr;
        uint16_t index;
        Range offset_range;
    public:
        inline Range getOffsetRange() const { return offset_range; }
        inline uint32_t getLength() const { return offset_range.length; }
        inline uint32_t getOffset() const { return offset_range.begin; }
        inline ParameterKind getKind() const { return kind; }
        inline Symbol *getType() const { return type; }
        inline EvaluationKind getEvalKind() const { return eval_kind; }
        inline explicit Parameter(unicode::string name, uint16_t index, ParameterKind kind, EvaluationKind eval, Symbol *type, Range offset_range, Scope *parent)
            : Symbol(name), index(index), kind(kind), eval_kind(eval), type(type), offset_range(offset_range) {
            this->parent = parent;
        }
    };

    class NormalParameter : public Parameter{
    public:
        inline explicit NormalParameter(unicode::string name, uint16_t index, EvaluationKind eval, Symbol *type, Range offset_range, Scope *parent)
            : Parameter(name, index, ParameterKind::Normal, eval, type, offset_range, parent){}
    };

    class OptionalParameter : public Parameter{
        uint32_t flag_offset;
    public:
        inline uint32_t getFlagOffset() const { return flag_offset; }
        inline explicit OptionalParameter(unicode::string name, uint16_t index, EvaluationKind eval, 
                                            Symbol *type, uint32_t flag_offset, Range offset_range, Scope *parent)
            : Parameter(name, index, ParameterKind::Optional, eval, type, offset_range, parent), flag_offset(flag_offset){}
    };

    class ParamArrayParameter : public Parameter{
    public:
        inline explicit ParamArrayParameter(unicode::string name, uint16_t index, EvaluationKind eval, Symbol *type, Range offset_range, Scope *parent)
            : Parameter(name, index, ParameterKind::ParamArray, eval, type, offset_range, parent){}
    };

    class Function : public Scope{
        const google::protobuf::RepeatedPtrField<Backage::ParameterDecl> params;
        uint32_t return_type_token;

        Parameter *implicit_self = nullptr;
        std::vector<Parameter*> normal_parameters;
        // 按值传递的可选参数内存布局：
        //      +--------------+--------------+ 
        //      | Flag (1byte) | value (size) |
        //      +--------------+--------------+
        std::vector<Parameter*> optional_parameters;
        Parameter *param_array = nullptr;

        std::vector<uint32_t> stackframe_interior_pointer_offsets;
        uint32_t param_memory_size;


        Symbol *return_type = nullptr;
    protected:
        uint32_t flag;
        TokenTable &table;
        std::vector<uint32_t> stackframe_ref_offsets;

    public:

        void complete()override;

        inline TokenTable &getTable() const {return table;}

        inline const std::vector<uint32_t> &getStackFrameRefOffsets(){ return stackframe_ref_offsets; }
        inline const std::vector<uint32_t> &getStackFrameInteriorPointerOffsets(){ return stackframe_interior_pointer_offsets; }

        inline int32_t getParamMemorySize() const { return param_memory_size; }
        inline const Parameter* getImplicitSelf() const { return implicit_self; }
        inline const Parameter* getParamArray() const { return param_array; }
        inline const std::vector<Parameter*> &getNormalParameters() const { return normal_parameters; }
        inline const std::vector<Parameter*> &getOptionalParameters() const { return optional_parameters; }
        inline const Parameter* getParameterByIndex(int index){
            if(index==0){
                if(implicit_self == nullptr)throw "";
                return implicit_self;
            }
            else if(index < normal_parameters.size() + 1){
                return normal_parameters[index-1];
            }
            else if(index < normal_parameters.size() + optional_parameters.size() + 1){
                return optional_parameters[index - (normal_parameters.size() + 1)];
            }
            else if(index == normal_parameters.size() + optional_parameters.size() + 1){
                if(param_array == nullptr)throw "";
                return param_array;
            }
            throw "";
        }
        inline Symbol *getReturnType(){ return return_type; }

        inline explicit Function(unicode::string name, TokenTable &table, uint32_t flag, const google::protobuf::RepeatedPtrField<Backage::ParameterDecl> &params, uint32_t return_type_token)
            : Scope(name,{}), table(table), flag(flag), params(params), return_type_token(return_type_token){}
    };

    class HostedFunction : public Function{
        const google::protobuf::RepeatedPtrField<Backage::LocalIndex> locals;
        uint32_t local_memory_size = 0;

        std::vector<uint32_t> local_offsets;
        LineNumberTable *lineNumberTable = nullptr;
        std::string block;

        void generateLineTable(const google::protobuf::RepeatedPtrField<Backage::LineNumber> &lineNumbers);

    public:
        inline uint32_t getLocalMemorySize(){ return local_memory_size; }

        // localindex 下标从1开始。0为无效
        inline uint32_t getLocalOffset(uint16_t index){ return local_offsets[index-1]; }

        inline virtual LineNumberTable *getLineNumberTable(){ return lineNumberTable; }
        inline virtual uint8_t *getBlock(){ return (uint8_t*)block.data(); }

        HostedFunction(unicode::string name,TokenTable &table,const std::string &block,
                    const google::protobuf::RepeatedPtrField<Backage::ParameterDecl> &params,
                    const google::protobuf::RepeatedPtrField<Backage::LocalIndex> &locals,
                    const google::protobuf::RepeatedPtrField<Backage::LineNumber> &lineNumbers,
                    uint32_t return_type_token,
                    uint32_t flag);

        void complete()override;
    };

    class Method : public HostedFunction{
        const Backage::MethodDecl decl;
    public:
        uint32_t getFlag(){return decl.flag();}
        Method(unicode::string name, TokenTable &table, const Backage::MethodDecl decl)
            : HostedFunction(name,table,decl.block(),decl.params(),decl.localindex(),decl.linenumbers(),decl.rettypetoken(),decl.flag()), decl(decl){}
    };

    class Ctor : public HostedFunction{
        const Backage::CtorDecl decl;
        Class *klass = nullptr;
    public:
        inline Class *getClass(){ return klass; }
        inline void setClass(Class *klass){ this->klass = klass; }
        uint32_t getFlag(){return decl.flag();}
        Ctor(TokenTable &table, const Backage::CtorDecl decl)
            : HostedFunction("#ctor"_utf32,table,decl.block(),decl.params(),decl.localindex(),decl.linenumbers(),0,decl.flag()), decl(decl){}
    };

    class VirtualMethod : public Symbol{
        int vtable_offset = -1;
        Method *self_implemention = nullptr;
    public:
        inline int getVTableOffset(){
            return vtable_offset;
        }

        inline Method *getSelfImpl(){
            return self_implemention;
        }

        VirtualMethod(unicode::string name, int vtable_offset, Method *self_implemention, Class *parent)
            : Symbol(name),vtable_offset(vtable_offset), self_implemention(self_implemention) {
            this->parent = parent;
        }
    };

    class ForeignEntry : public Function{
        const Backage::ForeignEntryDecl decl;
        std::optional<std::string> lib,alias;
        std::string ffi_name;
        FFIEntry *entry = nullptr;
    public:
        inline uint32_t getFlag(){return decl.flag();}
        inline std::optional<std::string> getLib(){ return lib; }
        inline std::optional<std::string> getAlias(){ return alias; }
        inline FFIEntry *getEntry(){ return entry; }
        inline void setEntry(FFIEntry *entry){ this->entry = entry; }
        inline ForeignEntry(unicode::string name, TokenTable &table, const Backage::ForeignEntryDecl decl)
            : Function(name,table,decl.flag(),decl.params(),decl.rettypetoken()), decl(decl){
            alias = decl.has_alias() ? std::make_optional(decl.alias()) : std::nullopt;
            lib = decl.has_lib() ? std::make_optional(decl.lib()) : std::nullopt;
        }
    };


    enum class PrimitiveKind{Void,Boolean,Byte,Short,UShort,Rune,Integer,UInteger,Long,ULong,Single,Double};

    unicode::string getPrimitiveKindString(PrimitiveKind kind);

    class Primitive : public Symbol{
        PrimitiveKind kind;
    public:
        uint32_t getSize()const;
        inline PrimitiveKind getKind() const { return kind; }
        Primitive(PrimitiveKind kind) : Symbol(getPrimitiveKindString(kind)), kind(kind){}
    };


    class Enumeration : public Scope{
        const Backage::EnumDecl decl;
        TokenTable &table;
    public:
        uint32_t getFlag(){return decl.flag();}
        Enumeration(unicode::string name, TokenTable &table, const Backage::EnumDecl decl, std::list<Symbol*> childern)
            : Scope(name,childern), decl(decl), table(table) {}
    };


    class EnumConstant : public Symbol{
        const Backage::EnumConstant decl;
        TokenTable &table;
    public:
        int getValue(){return decl.value();}
        EnumConstant(unicode::string name, TokenTable &table, const Backage::EnumConstant decl)
            : Symbol(name), table(table), decl(decl) {}
    };
} 


#endif
//
// Created by yorkin on 11/2/21.
//

#ifndef EVOBASIC2_IR_H
#define EVOBASIC2_IR_H
#include <string>
#include <vector>
#include <list>
#include <map>
#include <ostream>
#include <memory>
#include <variant>
#include "data.h"
#include "bytecode.h"
namespace evoBasic::ir{

    class Segment;
    class IR;
    class Function;
    class Meta;
    class Block;
    class Pair;

    class IRBase{
        data::u8 address_ = 0;
    public:
        virtual void toString(std::ostream &stream)=0;
        virtual void toHex(std::ostream &stream)=0;
        virtual data::ptr getAddress(){return address_;}
        virtual void setAddress(data::ptr address){address_ = address;}
        virtual data::ptr getByteLength(){return 0;}
    };

    class ConstBase : public IRBase{};

    template<typename T>
    class Const : public ConstBase{
        T t_;
    public:
        explicit Const(T t) : t_(t){}
        data::ptr getByteLength()override{
            return sizeof(T);
        }
        void toString(std::ostream &stream)override{
            stream<<t_;
        }
        void toHex(std::ostream &stream)override{
            stream.write((const char*)&t_,sizeof(T));
        }
    };

    class Instruction : public IRBase{
        friend class IR;
    public:
        struct TypeProp{
            vm::Data data = vm::Data::void_;
        };

        struct CastProp{
            vm::Data src = vm::Data::void_,
                    dst = vm::Data::void_;
        };

        struct InvokeProp{
            Block *target = nullptr;
            std::string signature;
        };

        struct PushProp{
            vm::Data data = vm::Data::void_;
            ConstBase *const_value = nullptr;
        };

        struct JumpProp{
            Block *target = nullptr;
        };

        struct MemProp{
            data::u32 size;
        };

        struct PlmProp{
            data::u32 size;
            const char *memory = nullptr;
        };

        using Prop = std::variant<bool,TypeProp,CastProp,InvokeProp,PushProp,JumpProp,MemProp,PlmProp>;

        Instruction(vm::Bytecode bytecode,Prop prop){
            this->prop = prop;
            op = bytecode;
        }

        Instruction(vm::Bytecode bytecode){
            op = bytecode;
        }
    private:
        vm::Bytecode op = vm::Bytecode::Nop;
        Prop prop{};
    public:
        void toHex(std::ostream &stream)override;
        void toString(std::ostream &stream)override;
        data::ptr getByteLength()override;
    };

    class IR{
        std::map<std::string,Block*> function_block;
        std::map<std::string,int> label_name_count;

        std::vector<Meta*> meta;
        std::vector<Block*> blocks;
    public:

        void addMeta(Meta *meta);
        void addBlock(Block *block);

        void toString(std::ostream &stream);
        void toHex(std::ostream &stream);
    };

    class Block : public IRBase{
        std::string label_;
        std::vector<Instruction*> instructons;
    public:
        explicit Block(std::string label);
        void toString(std::ostream &stream)override;
        void toHex(std::ostream &stream)override;
        data::ptr getByteLength()override;
        data::ptr getAddress()override;
        std::vector<Instruction*> getInstructions();
        std::string getLabel();
        void setLabel(std::string label);
        Block &Jmp(Block *block);
        Block &Jif(Block *block);
        Block &EQ(vm::Data data);
        Block &NE(vm::Data data);
        Block &LT(vm::Data data);
        Block &GT(vm::Data data);
        Block &LE(vm::Data data);
        Block &GE(vm::Data data);
        Block &Add(vm::Data data);
        Block &Sub(vm::Data data);
        Block &Mul(vm::Data data);
        Block &Div(vm::Data data);
        Block &FDiv(vm::Data data);
        Block &Neg(vm::Data data);
        Block &And();
        Block &Or();
        Block &Xor();
        Block &Not();
        Block &Load(vm::Data data);
        Block &Store(vm::Data data);
        Block &StoreR(vm::Data data);
        Block &Invoke(std::string signature);
        Block &Push(vm::Data data,ConstBase *const_value);
        Block &Pop(vm::Data data);
        Block &Ret();
        Block &Cast(vm::Data src,vm::Data dst);
        Block &Dup(vm::Data data);
        Block &Stm(data::u32 size);
        Block &StmR(data::u32 size);
        Block &Ldm(data::u32 size);
        Block &Psm(data::u32 size,const char *memory);
        Block &Dpm(data::u32 size);
        Block &PushFrameBase();
        Block &PushGlobalBase();
    };





    class Meta : public IRBase{
    public:
        enum Enum{unknown,function,pair,mark,record,enum_meta,depend}meta_kind = Enum::unknown;
        void toHex(std::ostream &stream)override;
    };

    class Type : public Meta{};

    //i32 i64 void
    class Mark : public Type{
        bool is_ref = false;
        bool is_array = false;
        std::string name_;
    public:
        Mark(std::string name,bool isRef,bool isArray);
        void toString(std::ostream &stream)override;
    };

    //name : type
    class Pair : public Meta{
        std::string name_;
        Type *type_;
    public:
        std::string getName(){return name_;}
        Type *getType(){return type_;}
        Pair(std::string name,Type *type);
        void toString(std::ostream &stream)override;
    };

    //sampleFunc : Function(param1:i32,param2:boolean)->void = u32_segment_address
    //sampleExt : External(param1:i32)->i32 = library_name
    class Function : public Type{
        std::list<Pair*> params;
        Type *ret_type = nullptr;
        bool external = false;
        std::string library_name;
        Block *block = nullptr;
    public:
        Function(std::list<Pair*> params,Type *ret,std::string library);
        Function(std::list<Pair*> params,Type *ret,Block *block);
        void toString(std::ostream &stream)override;
        data::ptr getAddress()override;
        Block *getBlock();
    };

    //sampleStruct : Record{x:i32,y:i32}
    class Record : public Type{
        std::list<Pair*> members_;
    public:
        explicit Record(std::list<Pair*> members);
        void toString(std::ostream &stream)override;
    };

    class Enum : public Type{
        std::list<std::pair<std::string,int>> members_;
    public:
        explicit Enum(std::list<std::pair<std::string,int>> members);
        void toString(std::ostream &stream)override;
    };

    class Depend : public Type{
        std::string file_;
    public:
        explicit Depend(std::string file);
        void toString(std::ostream &stream)override;
    };




}

#endif //EVOBASIC2_IR_H

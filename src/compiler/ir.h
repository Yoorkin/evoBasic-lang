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
#include <sstream>
#include <any>
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
        data::ptr address_ = 0;
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

        struct IntrinsicProp{
            data::ptr id;
        };

        struct ExternalProp{
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
            data::ptr size;
        };

        struct PsmProp{
            data::ptr size;
            const char *memory = nullptr;
        };

        using Prop = std::variant<bool,TypeProp,CastProp,InvokeProp,PushProp,JumpProp,MemProp,PsmProp,IntrinsicProp,ExternalProp>;

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

    class Constant : public IRBase{
        data::ptr address;
        vm::Data data;
        std::any value;
        friend IR;
        Constant(vm::Data kind,std::any value);
    public:
        data::ptr getAddress();
        void setAddress(data::ptr address);
        bool equal(Constant *rhs);
    };


    class IR{
        std::map<std::string,Block*> function_block;
        std::map<std::string,int> label_name_count;

        std::stringstream meta_stream;
        std::vector<Block*> blocks;
        std::vector<Constant*> constant_pool;
    public:

        Block* findFunctionBlock(std::string name){
            auto target = function_block.find(name);
            if(target == function_block.end())return nullptr;
            return target->second;
        }

        std::stringstream &getMetadataStream();
        void addBlock(Block *block);

        Constant *createConst(vm::Data kind,std::any value);

        void toString(std::ostream &stream);
        void toHex(std::ostream &stream);
    };

    class Block : public IRBase{
        std::string label_;
        std::vector<Instruction*> instructions;
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
        Block &Intrinsic(data::ptr id);
        Block &External(std::string signature);
        Block &Push(Constant *constant);
        Block &Pop(vm::Data data);
        Block &Ret();
        Block &Cast(vm::Data src,vm::Data dst);
        Block &Dup(vm::Data data);
        Block &Stm(data::size size);
        Block &StmR(data::size size);
        Block &Ldm(data::size size);
        Block &Psm(Constant *constant);
        Block &Dpm(data::size size);
        Block &PushFrameBase();
        Block &PushGlobalBase();
        Block &PushConstBase();
        Block &RcInc();
        Block &RcDec();
    };
}

#endif //EVOBASIC2_IR_H

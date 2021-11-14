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
#include "data.h"
#include "bytecode.h"
namespace evoBasic::ir{
    class Segment;
    class IR;

    class IRBase{
        data::u8 address_ = 0;
    public:
        virtual void toString(std::ostream &stream)=0;
        virtual void toHex(std::ostream &stream)=0;
        virtual data::u32 getAddress(){return address_;}
        virtual void setAddress(data::u32 address){address_ = address;}
        virtual data::u32 getByteLength(){return 0;}
    };

    class Instruction : public IRBase{
        friend class IR;
        vm::Bytecode op = vm::Bytecode::Nop;
        vm::Data data1 = vm::Data::void_,data2 = vm::Data::void_;
        std::string label;
        Segment *segment = nullptr;
        IRBase *const_value = nullptr;
        enum InstType{with_type,without_type,cast,invoke,push,jmp,jif}inst_type;
    public:
        void toHex(std::ostream &stream)override;
        void toString(std::ostream &stream)override;
        data::u32 getByteLength()override;

        static Instruction *WithType(vm::Bytecode op,vm::Data type);
        static Instruction *Cast(vm::Data src,vm::Data dst);
        static Instruction *WithoutType(vm::Bytecode op);
        static Instruction *Invoke(std::string label);
        static Instruction *Push(vm::Data type,IRBase *const_value);
        static Instruction *StoreMemory(IRBase *const_value);
        static Instruction *LoadMemory(IRBase *const_value);
        static Instruction *PushMemory(IRBase *const_value,std::string memory);
        static Instruction *Jmp(Segment *segment);
        static Instruction *Jif(Segment *segment);
    };
    /*
 *  Jmp,Jif,EQ,NE,LT,GT,LE,GE,Add,Sub,Mul,Div,FDiv,Neg,And,Or,Xor,Not,Load,Store,pop,dup
 *  pushframebase,pushglobalbase,ret
 *  invoke
 *  push
 *  cast
 */



    class Segment : public IRBase{
        std::string label;
        std::vector<IRBase*> codes;
        friend class IR;
        explicit Segment(std::string label_name);
    public:
        Segment *add(IRBase *code);
        void toHex(std::ostream &stream)override;
        void toString(std::ostream &stream)override;
        std::string getLabel();
    };

    class IR{
        std::vector<Segment*> code_segments;
        std::map<std::string,Segment*> function_segment;
        std::map<std::string,int> label_name_count;
        Segment *meta = new Segment("meta");
        Segment *data = new Segment("data");
    public:
        static const vm::Data ptr;

        IR(){
            code_segments.push_back(data);
        }

        Segment *getMetaSegment(){
            return meta;
        }
        Segment *getDataSegment(){
            return data;
        }

        Segment *createSegment(const std::string& label){
            auto ret = new Segment(label);
            auto conflict = label_name_count[label];
            if(conflict > 0){
                ret->label=ret->label + "_" + std::to_string(conflict);
            }
            label_name_count[label]++;
            return ret;
        }

        void add(Segment *segment){
            code_segments.push_back(segment);
            function_segment.insert({segment->getLabel(),segment});
        }

        void toString(std::ostream &stream);
        void toHex(std::ostream &stream);
    };

    class Meta : public IRBase{
    public:
        enum Enum{function,pair,primitive,record};
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
        Segment *segment = nullptr;
    public:
        Function(std::list<Pair*> params,Type *ret,std::string library);
        Function(std::list<Pair*> params,Type *ret,Segment *segment);
        void toString(std::ostream &stream)override;
        data::u32 getAddress()override;
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

    template<typename T>
    class Const : public IRBase{
        T t_;
    public:
        explicit Const(T t) : t_(t){}
        data::u32 getByteLength()override{
            return sizeof(T);
        }
        void toString(std::ostream &stream)override{
            stream<<t_;
        }
        void toHex(std::ostream &stream)override{
            stream.write((const char*)&t_,sizeof(T));
        }
    };


}

#endif //EVOBASIC2_IR_H

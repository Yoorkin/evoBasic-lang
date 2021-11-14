//
// Created by yorkin on 11/2/21.
//

#include "ir.h"
#include "nullSafe.h"
#include <utility>
#include <iomanip>
using namespace std;
namespace evoBasic::ir{

    Mark::Mark(std::string name, bool isRef, bool isArray)
            : name_(std::move(name)),is_ref(isRef),is_array(isArray){}

    void Mark::toString(ostream &stream) {
        if(is_ref)stream<<"ref ";
        if(is_array)stream<<"mem ";
        stream<<name_;
    }


    Pair::Pair(std::string name, Type *type)
            : name_(std::move(name)),type_(type) {}

    void Pair::toString(ostream &stream) {
        stream<<name_<<':';
        type_->toString(stream);
    }


    Function::Function(std::list<Pair *> params, Type *ret, std::string library)
            : params(std::move(params)),ret_type(ret),library_name(std::move(library)),external(true){}

    Function::Function(std::list<Pair*> params,Type *ret,Segment *segment)
            : params(std::move(params)),ret_type(ret),segment(segment),external(false){}

    data::u32 Function::getAddress() {
        ASSERT(external,"invalid operation");
        return segment->getAddress();
    }

    void Function::toString(ostream &stream) {
        stream<<"Function(";
        for(auto& p:params){
            p->toString(stream);
            if(&p != &params.back())stream << ',';
        }
        stream<<")->";
        if(ret_type)ret_type->toString(stream);
        else stream<<"void";
    }



    Record::Record(std::list<Pair*> members) : members_(std::move(members)){}

    void Record::toString(ostream &stream) {
        stream<<"Record{";
        for(auto &m:members_){
            m->toString(stream);
            if(m != members_.back())stream << ',';
        }
        stream<<"}";
    }


    Enum::Enum(std::list<std::pair<std::string,int>> members) : members_(std::move(members)){}

    void Enum::toString(ostream &stream) {
        stream<<"Enum{";
        for(const auto& m:members_){
            stream<<m.first<<'='<<to_string(m.second);
            if(&m != &members_.back())stream<<',';
        }
        stream<<'}';
    }


    Depend::Depend(std::string file) : file_(std::move(file)){}

    void Depend::toString(ostream &stream) {
        stream<<"Include("<<file_<<")";
    }


    void Meta::toHex(ostream &stream) {
        toString(stream);
    }


    Segment::Segment(std::string label_name) : label(move(label_name)) {}

    Segment *Segment::add(IRBase *code) {
        codes.push_back(code);
        return this;
    }

    void Segment::toHex(std::ostream &stream) {
        for(auto &c:codes) {
            c->toHex(stream);
        }
    }

    void Segment::toString(ostream &stream) {
        stream<<label<<":\n";
        for(auto &c:codes) {
            stream<< ' '<< setw(8) << setfill(' ') << std::left;
            stream<<c->getAddress();
            stream<<"│ ";
            c->toString(stream);
            stream<<"\n";
        }
    }

    std::string Segment::getLabel() {
        return label;
    }


    Instruction *Instruction::WithType(vm::Bytecode op, vm::Data type){
        auto ret = new Instruction;
        ret->op = op;
        ret->data1 = type;
        ret->inst_type = with_type;
        return ret;
    }

    Instruction *Instruction::WithoutType(vm::Bytecode op) {
        auto ret = new Instruction;
        ret->op = op;
        ret->inst_type = without_type;
        return ret;
    }

    Instruction *Instruction::Cast(vm::Data src, vm::Data dst) {
        auto ret = new Instruction;
        ret->op = vm::Bytecode::Cast;
        ret->data2 = src;
        ret->data1 = dst;
        ret->inst_type = without_type;
        return ret;
    }

    Instruction *Instruction::Invoke(std::string label) {
        auto ret = new Instruction;
        ret->op = vm::Bytecode::Invoke;
        ret->label = move(label);
        ret->inst_type = invoke;
        return ret;
    }

    Instruction *Instruction::Push(vm::Data type, IRBase *const_value) {
        auto ret = new Instruction;
        ret->op = vm::Bytecode::Push;
        ret->data1 = type;
        ret->const_value = const_value;
        ret->inst_type = push;
        return ret;
    }

    Instruction *Instruction::Jmp(Segment *segment) {
        auto ret = new Instruction;
        ret->op = vm::Bytecode::Jmp;
        ret->segment = segment;
        ret->inst_type = jmp;
        return ret;
    }

    Instruction *Instruction::Jif(Segment *segment) {
        auto ret = new Instruction;
        ret->op = vm::Bytecode::Jif;
        ret->segment = segment;
        ret->inst_type = jif;
        return ret;
    }

    Instruction *Instruction::StoreMemory(IRBase *const_value) {
        auto ret = new Instruction;
        ret->op = vm::Bytecode::Stm;
        ret->const_value = const_value;
        return ret;
    }

    Instruction *Instruction::LoadMemory(IRBase *const_value) {
        auto ret = new Instruction;
        ret->op = vm::Bytecode::Ldm;
        ret->const_value = const_value;
        return ret;
    }

    Instruction *Instruction::PushMemory(IRBase *const_value, std::string memory) {
        auto ret = new Instruction;
        ret->op = vm::Bytecode::Psm;
        ret->const_value = const_value;
        ret->label = move(memory);
        return ret;
    }



    data::u32 Instruction::getByteLength() {
        switch (inst_type) {
            case InstType::with_type:       return 2;
            case InstType::without_type:    return 1;
            case InstType::push:            return 2 + data1.getSize();
            case InstType::cast:            return 3;
            case InstType::invoke:          
            case InstType::jif:             
            case InstType::jmp:             return 1 + vm::Data(vm::Data::u32).getSize();
        }
    }

    void Instruction::toString(std::ostream &stream) {
        switch (op.getValue()) {
            case vm::Bytecode::Jmp:
                stream<<"Jmp "<<segment->getLabel();
                break;
            case vm::Bytecode::Jif:
                stream<<"Jif "<<segment->getLabel();
                break;
            case vm::Bytecode::EQ:
                stream<<"EQ."<<data1.toString();
                break;
            case vm::Bytecode::NE:
                stream<<"NE."<<data1.toString();
                break;
            case vm::Bytecode::GT:
                stream<<"GT."<<data1.toString();
                break;
            case vm::Bytecode::LT:
                stream<<"LT."<<data1.toString();
                break;
            case vm::Bytecode::GE:
                stream<<"GE."<<data1.toString();
                break;
            case vm::Bytecode::LE:
                stream<<"LE."<<data1.toString();
                break;
            case vm::Bytecode::Add:
                stream<<"Add."<<data1.toString();
                break;
            case vm::Bytecode::Sub:
                stream<<"Sub."<<data1.toString();
                break;
            case vm::Bytecode::Load:
                stream<<"Load."<<data1.toString();
                break;
            case vm::Bytecode::Store:
                stream<<"Store."<<data1.toString();
                break;
            case vm::Bytecode::Invoke:
                stream<<"Invoke "<<label;
                break;
            case vm::Bytecode::Push:
                stream<<"Push."<<data1.toString()<<' ';
                const_value->toString(stream);
                break;
            case vm::Bytecode::Pop:
                stream<<"Pop."<<data1.toString();
                break;
            case vm::Bytecode::Cast:
                stream<<"Cast."<<data1.toString()<<' '<<data2.toString();
                break;
            case vm::Bytecode::Ret:
                stream<<"Ret";
                break;
            case vm::Bytecode::PushFrameBase:
                stream<<"PushFrameBase";
                break;
            case vm::Bytecode::PushGlobalBase:
                stream<<"PushGlobalBase";
                break;
            case vm::Bytecode::Mul:
                stream<<"Mul."<<data1.toString();
                break;
            case vm::Bytecode::Div:
                stream<<"Div."<<data1.toString();
                break;
            case vm::Bytecode::FDiv:
                stream<<"FDiv."<<data1.toString();
                break;
            case vm::Bytecode::And:
                stream<<"And";
                break;
            case vm::Bytecode::Or:
                stream<<"Or";
                break;
            case vm::Bytecode::Xor:
                stream<<"Xor";
                break;
            case vm::Bytecode::Not:
                stream<<"Not";
                break;
            case vm::Bytecode::Dup:
                stream<<"Dup."<<data1.toString();
                break;
            case vm::Bytecode::Neg:
                stream<<"Neg."<<data1.toString();
                break;
        }
    }

    void Instruction::toHex(ostream &stream) {
        data::u32 address;
        stream<<op.toHex();
        switch (op.getValue()) {
            case vm::Bytecode::Jmp:
            case vm::Bytecode::Jif:
            case vm::Bytecode::Invoke:
                address = segment->getAddress();
                stream.write((const char*)&address,sizeof(address));
                break;
            case vm::Bytecode::EQ:
            case vm::Bytecode::NE:
            case vm::Bytecode::GT:
            case vm::Bytecode::LT:
            case vm::Bytecode::GE:
            case vm::Bytecode::LE:
            case vm::Bytecode::Add:
            case vm::Bytecode::Sub:
            case vm::Bytecode::Load:
            case vm::Bytecode::Store:
            case vm::Bytecode::Pop:
            case vm::Bytecode::Mul:
            case vm::Bytecode::Div:
            case vm::Bytecode::FDiv:
            case vm::Bytecode::Dup:
            case vm::Bytecode::Neg:
                stream<<data1.toHex();
                break;
            case vm::Bytecode::Push:
                stream<<data1.toHex();
                const_value->toHex(stream);
                break;
            case vm::Bytecode::Cast:
                stream<<data1.toHex()<<data2.toHex();
                break;
            case vm::Bytecode::Ret:
            case vm::Bytecode::PushFrameBase:
            case vm::Bytecode::PushGlobalBase:
            case vm::Bytecode::And:
            case vm::Bytecode::Or:
            case vm::Bytecode::Xor:
            case vm::Bytecode::Not:
                // do nothing
                break;
        }
    }

    const vm::Data IR::ptr = vm::Data::u32;

    void IR::toHex(ostream &stream) {
        //compute ir address
        data::u8 address = 0;
        for(auto &seg : code_segments){
            for(auto &inst : seg->codes){
                inst->setAddress(address);
                auto instuction = (Instruction*)inst;
                if(instuction->inst_type==Instruction::InstType::invoke){
                    instuction->segment = function_segment.find(instuction->label)->second;
                }
                address += inst->getByteLength();
            }
        }

        stream<<vm::Bytecode(vm::Bytecode::MetaSegment).toHex();
        meta->toHex(stream);
        bool is_data = true;
        stream<<vm::Bytecode(vm::Bytecode::ConstSegment).toHex();
        for(auto &seg : code_segments){
            seg->toHex(stream);
            if(is_data){
                is_data = false;
                stream<<vm::Bytecode(vm::Bytecode::CodeSegment).toHex();
            }
        }
    }

    void IR::toString(ostream &stream) {
        data::u8 address = 0;
        for(auto &seg : code_segments){
            for(auto &inst : seg->codes){
                inst->setAddress(address);
                auto instuction = (Instruction*)inst;
                if(instuction->inst_type==Instruction::InstType::invoke){
                    instuction->segment = function_segment.find(instuction->label)->second;
                }
                address += inst->getByteLength();
            }
        }

        meta->toString(stream);
        for(auto &seg : code_segments){
            seg->toString(stream);
        }
    }
}

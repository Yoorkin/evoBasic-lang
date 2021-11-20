//
// Created by yorkin on 11/2/21.
//

#include "ir.h"
#include "nullSafe.h"
#include "utils.h"
#include <utility>
#include <iomanip>
using namespace std;
namespace evoBasic::ir{

    void IR::addMeta(Meta *meta_) {
        this->meta.push_back(meta_);
        if(meta_->meta_kind == Meta::pair){
            auto pair = (Pair*)meta_;
            if(pair->getType()->meta_kind == Meta::function)
                function_block.insert({pair->getName(),((Function*)pair->getType())->getBlock()});
        }
    }

    void IR::addBlock(Block *block) {
        auto label = block->getLabel();
        auto conflict = label_name_count[label];
        if(conflict > 0){
            block->setLabel(label + '_' + std::to_string(conflict));
        }
        label_name_count[label]++;
        blocks.push_back(block);
    }



    void IR::toHex(ostream &stream) {
        //compute ir address
        data::u8 address = 0;
        for(auto &block : blocks){
            for(auto &inst : block->getInstructions()){
                inst->setAddress(address);
                if(inst->op == vm::Bytecode::Invoke){
                    auto &prop = get<Instruction::InvokeProp>(inst->prop);
                    auto target = function_block.find(prop.signature);
                    ASSERT(target == function_block.end(),"invalid");
                    prop.target = target->second;
                }
                auto size = inst->getByteLength();
                ASSERT(size<=0,"invalid size");
                address += size;
            }
        }

        auto entrance = function_block.find("main");
        if(entrance != function_block.end()){
            stream<<vm::Bytecode(vm::Bytecode::Entrance).toHex();
            auto address = entrance->second->getAddress();
            stream.write((const char *)&address,sizeof(address));
        }

        stream<<vm::Bytecode(vm::Bytecode::MetaSegment).toHex();
        for(auto &m : meta){
            m->toHex(stream);
        }

        stream<<vm::Bytecode(vm::Bytecode::CodeSegment).toHex();
        for(auto &block : blocks){
            block->toHex(stream);
        }
    }

    void IR::toString(ostream &stream) {
        data::u8 address = 0;
        for(auto &block : blocks){
            for(auto &inst : block->getInstructions()){
                inst->setAddress(address);
                if(inst->op == vm::Bytecode::Invoke){
                    auto &prop = get<Instruction::InvokeProp>(inst->prop);
                    prop.target = function_block.find(prop.signature)->second;
                }
                auto size = inst->getByteLength();
                ASSERT(size<=0,"invalid size");
                address += size;
            }
        }

        stream<<"entrance: ";
        auto entrance = function_block.find("main");
        if(entrance == function_block.end()) stream<<"?\n";
        else{
            stream<<entrance->second->getAddress();
        }
        stream<<"\n";

        stream<<"meta: \n";
        for(auto &m : meta){
            stream<<"\t";
            m->toHex(stream);
            stream<<"\n";
        }

        stream<<"code: \n";
        for(auto &block : blocks){
            block->toString(stream);
        }
    }


    Mark::Mark(std::string name, bool isRef, bool isArray)
            : name_(std::move(name)),is_ref(isRef),is_array(isArray){
        meta_kind = mark;
    }

    void Mark::toString(ostream &stream) {
        if(is_ref)stream<<"ref ";
        if(is_array)stream<<"mem ";
        stream<<name_;
    }


    Pair::Pair(std::string name, Type *type)
            : name_(std::move(name)),type_(type) {
        meta_kind = pair;
    }

    void Pair::toString(ostream &stream) {
        stream<<name_<<':';
        type_->toString(stream);
    }


    Function::Function(std::list<Pair *> params, Type *ret, std::string library)
            : params(std::move(params)),ret_type(ret),library_name(std::move(library)),external(true){
        meta_kind = function;
    }

    Function::Function(std::list<Pair*> params,Type *ret,Block *block)
            : params(std::move(params)),ret_type(ret),block(block),external(false){
        meta_kind = function;
    }

    data::ptr Function::getAddress() {
        ASSERT(external,"invalid operation");
        return block->getAddress();
    }

    void Function::toString(ostream &stream) {
        stream<<"Function(";
        for(auto& p:params){
            p->toString(stream);
            if(&p != &params.back())stream << ',';
        }
        stream<<")->";
        if(ret_type)
            ret_type->toString(stream);
        else stream<<"void";
    }

    Block *Function::getBlock() {
        NotNull(block);
        return block;
    }


    Record::Record(std::list<Pair*> members) : members_(std::move(members)){
        meta_kind = record;
    }

    void Record::toString(ostream &stream) {
        stream<<"Record{";
        for(auto &m:members_){
            m->toString(stream);
            if(m != members_.back())stream << ',';
        }
        stream<<"}";
    }


    Enum::Enum(std::list<std::pair<std::string,int>> members) : members_(std::move(members)){
        meta_kind = enum_meta;
    }

    void Enum::toString(ostream &stream) {
        stream<<"Enum{";
        for(const auto& m:members_){
            stream<<m.first<<'='<<to_string(m.second);
            if(&m != &members_.back())stream<<',';
        }
        stream<<'}';
    }


    Depend::Depend(std::string file) : file_(std::move(file)){
        meta_kind = depend;
    }

    void Depend::toString(ostream &stream) {
        stream<<"Include("<<file_<<")";
    }


    void Meta::toHex(ostream &stream) {
        toString(stream);
    }




    data::ptr Instruction::getByteLength() {
//        return std::visit(overloaded {
//                [](auto){return 0;},
//                [](TypeProp){return 2;},
//                [](CastProp){return 3;},
//                [](InvokeProp){return 1 + vm::Data(vm::Data::u32).getSize();},
//                //[](PushProp &p){return 2 + p.const_value->getByteLength();},
//                [](JumpProp){return 1 + vm::Data(vm::Data::u32).getSize();},
//                [](MemProp){return sizeof(data::u32);},
//        }, prop);
        switch (prop.index()) {
            case 0:       return 1;
            case 1:       return 2;
            case 4:       return 2 + get<PushProp>(prop).const_value->getByteLength();
            case 2:       return 3;
            case 3:
            case 5:       return 1 + vm::Data(vm::Data::u32).getSize();
            case 6:       return sizeof(data::u32);
            case 7:       return sizeof(data::u32) + get<PlmProp>(prop).size;
            default:      ASSERT(true,"invalid");
        }
    }

    void Instruction::toString(std::ostream &stream) {
        stream<<std::setfill(' ')<<std::setw(5)<<std::left<<getAddress();
        stream<<"│ ";
        stream<<op.toString();
        switch (op.getValue()) {
            case vm::Bytecode::Jmp:
            case vm::Bytecode::Jif:
                stream<<" "<<get<JumpProp>(prop).target->getLabel();
                break;
            case vm::Bytecode::Invoke:
                stream<<" "<<get<InvokeProp>(prop).signature;
                break;
            case vm::Bytecode::Ldm:
            case vm::Bytecode::Stm:
            case vm::Bytecode::StmR:
                stream <<" "<< get<MemProp>(prop).size;
                break;
            case vm::Bytecode::EQ:
            case vm::Bytecode::NE:
            case vm::Bytecode::GT:
            case vm::Bytecode::LT:
            case vm::Bytecode::GE:
            case vm::Bytecode::LE:
            case vm::Bytecode::Add:
            case vm::Bytecode::Sub:
            case vm::Bytecode::Mul:
            case vm::Bytecode::Div:
            case vm::Bytecode::FDiv:
            case vm::Bytecode::Load:
            case vm::Bytecode::Pop:
            case vm::Bytecode::Store:
            case vm::Bytecode::StoreR:
            case vm::Bytecode::Dup:
            case vm::Bytecode::Neg:
                stream<<"."<<get<TypeProp>(prop).data.toString();
                break;
            case vm::Bytecode::Push:
                stream<<"."<<get<PushProp>(prop).data.toString()<<' ';
                get<PushProp>(prop).const_value->toString(stream);
                break;
            case vm::Bytecode::Cast:
                stream<<"."<<get<CastProp>(prop).src.toString()<<' '
                           <<get<CastProp>(prop).dst.toString();
                break;
            case vm::Bytecode::Ret:
            case vm::Bytecode::PushFrameBase:
            case vm::Bytecode::PushGlobalBase:
            case vm::Bytecode::And:
            case vm::Bytecode::Or:
            case vm::Bytecode::Xor:
            case vm::Bytecode::Not:
                //do nothing
                break;
        }
    }

    void Instruction::toHex(ostream &stream) {
        data::u32 address;
        stream<<op.toHex();
        switch (op.getValue()) {
            case vm::Bytecode::Jmp:
            case vm::Bytecode::Jif:
                address = get<JumpProp>(prop).target->getAddress();
                stream.write((const char*)&address,sizeof(address));
                break;
            case vm::Bytecode::Invoke: {
                NotNull(get<InvokeProp>(prop).target);
                auto tmp = get<InvokeProp>(prop);
                address = tmp.target->getAddress();
                stream.write((const char *) &address, sizeof(address));
                break;
            }
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
            case vm::Bytecode::StoreR:
            case vm::Bytecode::Pop:
            case vm::Bytecode::Mul:
            case vm::Bytecode::Div:
            case vm::Bytecode::FDiv:
            case vm::Bytecode::Dup:
            case vm::Bytecode::Neg:
                stream<<get<TypeProp>(prop).data.toHex();
                break;
            case vm::Bytecode::Push:
                stream<<get<PushProp>(prop).data.toHex();
                get<PushProp>(prop).const_value->toHex(stream);
                break;
            case vm::Bytecode::Cast:
                stream << get<CastProp>(prop).src.toHex()
                       << get<CastProp>(prop).dst.toHex();
                break;
            case vm::Bytecode::Ldm:
            case vm::Bytecode::Stm:
            case vm::Bytecode::StmR:
                //TODO
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


    std::vector<Instruction *> Block::getInstructions() {
        return instructons;
    }

    std::string Block::getLabel() {
        return label_;
    }

    void Block::setLabel(std::string label) {
        label_ = move(label);
    }

    Block &Block::Jmp(Block *block) {
        instructons.push_back(new Instruction(vm::Bytecode::Jmp,Instruction::JumpProp{block}));
        return *this;
    }

    Block &Block::Jif(Block *block) {
        instructons.push_back(new Instruction(vm::Bytecode::Jif,Instruction::JumpProp{block}));
        return *this;
    }

    Block &Block::EQ(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::EQ,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::NE(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::NE,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::LT(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::LT,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::GT(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::GT,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::LE(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::LE,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::GE(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::GE,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::Add(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::Add,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::Sub(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::Sub,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::Mul(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::Mul,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::Div(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::Div,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::FDiv(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::FDiv,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::Neg(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::Neg,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::And() {
        instructons.push_back(new Instruction(vm::Bytecode::And));
        return *this;
    }

    Block &Block::Or() {
        instructons.push_back(new Instruction(vm::Bytecode::Or));
        return *this;
    }

    Block &Block::Xor() {
        instructons.push_back(new Instruction(vm::Bytecode::Xor));
        return *this;
    }

    Block &Block::Not() {
        instructons.push_back(new Instruction(vm::Bytecode::Not));
        return *this;
    }

    Block &Block::Load(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::Load,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::Store(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::Store,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::StoreR(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::StoreR,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::Invoke(std::string signature) {
        instructons.push_back(new Instruction(vm::Bytecode::Invoke,Instruction::InvokeProp{nullptr,signature}));
        return *this;
    }

    Block &Block::Push(vm::Data data, ConstBase *const_value) {
        instructons.push_back(new Instruction(vm::Bytecode::Push,Instruction::PushProp{data,const_value}));
        return *this;
    }

    Block &Block::Pop(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::Pop,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::Ret() {
        instructons.push_back(new Instruction(vm::Bytecode::Ret));
        return *this;
    }

    Block &Block::Cast(vm::Data src, vm::Data dst) {
        instructons.push_back(new Instruction(vm::Bytecode::Cast,Instruction::CastProp{src,dst}));
        return *this;
    }

    Block &Block::Dup(vm::Data data) {
        instructons.push_back(new Instruction(vm::Bytecode::Dup,Instruction::TypeProp{data}));
        return *this;
    }

    Block &Block::Stm(data::u32 size) {
        instructons.push_back(new Instruction(vm::Bytecode::Stm,Instruction::MemProp{size}));
        return *this;
    }

    Block &Block::StmR(data::u32 size) {
        instructons.push_back(new Instruction(vm::Bytecode::StmR,Instruction::MemProp{size}));
        return *this;
    }


    Block &Block::Ldm(data::u32 size) {
        instructons.push_back(new Instruction(vm::Bytecode::Ldm,Instruction::MemProp{size}));
        return *this;
    }

    Block &Block::Psm(data::u32 size,const char *memory) {
        instructons.push_back(new Instruction(vm::Bytecode::Psm,Instruction::PlmProp{size,memory}));
        return *this;
    }

    Block &Block::PushFrameBase() {
        instructons.push_back(new Instruction(vm::Bytecode::PushFrameBase));
        return *this;
    }

    Block &Block::PushGlobalBase() {
        instructons.push_back(new Instruction(vm::Bytecode::PushGlobalBase));
        return *this;
    }

    Block::Block(std::string label){
        label_ = std::move(label);
    }

    void Block::toString(ostream &stream) {
        stream<<"<"<<label_<<">\n";
        for(auto &inst:instructons) {
            inst->toString(stream);
            stream<<"\n";
        }
    }

    void Block::toHex(ostream &stream) {
        for(auto &inst:instructons)
            inst->toHex(stream);
    }

    data::ptr Block::getByteLength() {
        int ans = 0;
        for(auto &inst : instructons)
            ans += inst->getByteLength();
        return ans;
    }

    data::ptr Block::getAddress() {
        return this->instructons[0]->getAddress();
    }


}

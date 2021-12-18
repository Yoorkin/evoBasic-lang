////
//// Created by yorkin on 11/2/21.
////
//
//#include "ir.h"
//#include "nullSafe.h"
//#include "utils.h"
//#include <utility>
//#include <iomanip>
//using namespace std;
//namespace evoBasic::ir{
//
//    void IR::addBlock(Block *block) {
//        auto label = block->getLabel();
//        auto conflict = label_name_count[label];
//        if(conflict > 0){
//            block->setLabel(label + '_' + std::to_string(conflict));
//        }
//        label_name_count[label]++;
//        blocks.push_back(block);
//    }
//
//
//
//    void IR::toHex(ostream &stream) {
//        //compute ir address
//        data::ptr address = 0;
//        for(auto &block : blocks){
//            for(auto &inst : block->getInstructions()){
//                inst->setAddress(address);
//                if(inst->op == vm::Bytecode::Invoke){
//                    auto &prop = get<Instruction::InvokeProp>(inst->prop);
//                    auto target = function_block.find(prop.signature);
//                    ASSERT(target == function_block.end(),"invalid");
//                    prop.target = target->second;
//                }
//                auto size = inst->getByteLength();
//                ASSERT(size<=0,"invalid size");
//                address += size;
//            }
//        }
//
//        auto entrance = function_block.find("main");
//        if(entrance != function_block.end()){
//            stream<<vm::Bytecode(vm::Bytecode::Entrance).toHex();
//            auto address = entrance->second->getAddress();
//            stream.write((const char *)&address,sizeof(address));
//        }
//
//        stream<<vm::Bytecode(vm::Bytecode::MetaSegment).toHex();
//        for(auto &m : meta){
//            m->toHex(stream);
//        }
//
//        stream<<vm::Bytecode(vm::Bytecode::CodeSegment).toHex();
//        for(auto &block : blocks){
//            block->toHex(stream);
//        }
//    }
//
//    void IR::toString(ostream &stream) {
//        data::ptr address = 0;
//        for(auto &block : blocks){
//            for(auto &inst : block->getInstructions()){
//                inst->setAddress(address);
//                if(inst->op == vm::Bytecode::Invoke){
//                    auto &prop = get<Instruction::InvokeProp>(inst->prop);
//                    prop.target = function_block.find(prop.signature)->second;
//                }
//                auto size = inst->getByteLength();
//                ASSERT(size<=0,"invalid size");
//                address += size;
//            }
//        }
//
//        stream<<"entrance: ";
//        auto entrance = function_block.find("main");
//        if(entrance == function_block.end()) stream<<"?\n";
//        else{
//            stream<<entrance->second->getAddress();
//        }
//        stream<<"\n";
//
//        stream<<"meta: \n";
//        for(auto &m : meta){
//            stream<<"\t";
//            m->toHex(stream);
//            stream<<"\n";
//        }
//
//        stream<<"code: \n";
//        for(auto &block : blocks){
//            block->toString(stream);
//        }
//    }
//
//    std::stringstream &IR::getMetadataStream() {
//        return meta_stream;
//    }
//
//    Constant *IR::getRawStrConstant(string str) {
//
//    }
//
//
//    data::ptr Instruction::getByteLength() {
//        switch (prop.index()) {
//            case 0:/* empty */          return 1;
//            case 1:/* TypeProp */       return 2;
//            case 2:/* CastProp */       return 3;
//            case 3:/* InvokeProp */     return 1 + sizeof(data::ptr);
//            case 4:/* PushProp */       return 2 + get<PushProp>(prop).const_value->getByteLength();
//            case 5:/* JumpProp */       return 1 + sizeof(data::ptr);
//            case 6:/* MemProp */        return 1 + sizeof(data::ptr);
//            case 7:/* PsmProp */        return 1 + sizeof(data::ptr) + get<PsmProp>(prop).size;
//            case 8:/* IntrinsicProp */  return 1 + sizeof(data::ptr);
//            case 9:/* ExternalProp */   return 1 + get<ExternalProp>(prop).signature.size();
//            default: ASSERT(true,"invalid");
//        }
//    }
//
//    void Instruction::toString(std::ostream &stream) {
//        stream<<std::setfill(' ')<<std::setw(5)<<std::left<<getAddress();
//        stream<<"│ ";
//        stream<<op.toString();
//        switch (op.getValue()) {
//            case vm::Bytecode::Jmp:
//            case vm::Bytecode::Jif:
//                stream<<" "<<get<JumpProp>(prop).target->getLabel();
//                break;
//            case vm::Bytecode::Invoke:
//                stream<<" "<<get<InvokeProp>(prop).signature;
//                break;
//            case vm::Bytecode::Ldm:
//            case vm::Bytecode::Stm:
//            case vm::Bytecode::StmR:
//                stream <<" "<< get<MemProp>(prop).size;
//                break;
//            case vm::Bytecode::Psm:
//                stream <<" "<< get<PsmProp>(prop).size << get<PsmProp>(prop).memory;
//                break;
//            case vm::Bytecode::EQ:
//            case vm::Bytecode::NE:
//            case vm::Bytecode::GT:
//            case vm::Bytecode::LT:
//            case vm::Bytecode::GE:
//            case vm::Bytecode::LE:
//            case vm::Bytecode::Add:
//            case vm::Bytecode::Sub:
//            case vm::Bytecode::Mul:
//            case vm::Bytecode::Div:
//            case vm::Bytecode::FDiv:
//            case vm::Bytecode::Load:
//            case vm::Bytecode::Pop:
//            case vm::Bytecode::Store:
//            case vm::Bytecode::StoreR:
//            case vm::Bytecode::Dup:
//            case vm::Bytecode::Neg:
//                stream<<"."<<get<TypeProp>(prop).data.toString();
//                break;
//            case vm::Bytecode::Push:
//                stream<<"."<<get<PushProp>(prop).data.toString()<<' ';
//                get<PushProp>(prop).const_value->toString(stream);
//                break;
//            case vm::Bytecode::Cast:
//                stream<<"."<<get<CastProp>(prop).src.toString()<<' '
//                           <<get<CastProp>(prop).dst.toString();
//                break;
//            case vm::Bytecode::Ret:
//            case vm::Bytecode::PushFrameBase:
//            case vm::Bytecode::PushGlobalBase:
//            case vm::Bytecode::And:
//            case vm::Bytecode::Or:
//            case vm::Bytecode::Xor:
//            case vm::Bytecode::Not:
//                //do nothing
//                break;
//        }
//    }
//
//    void Instruction::toHex(ostream &stream) {
//        data::u32 address;
//        stream<<op.toHex();
//        switch (op.getValue()) {
//            case vm::Bytecode::Jmp:
//            case vm::Bytecode::Jif:
//                address = get<JumpProp>(prop).target->getAddress();
//                stream.write((const char*)&address,sizeof(address));
//                break;
//            case vm::Bytecode::Invoke: {
//                NotNull(get<InvokeProp>(prop).target);
//                auto tmp = get<InvokeProp>(prop);
//                address = tmp.target->getAddress();
//                stream.write((const char *) &address, sizeof(address));
//                break;
//            }
//            case vm::Bytecode::EQ:
//            case vm::Bytecode::NE:
//            case vm::Bytecode::GT:
//            case vm::Bytecode::LT:
//            case vm::Bytecode::GE:
//            case vm::Bytecode::LE:
//            case vm::Bytecode::Add:
//            case vm::Bytecode::Sub:
//            case vm::Bytecode::Load:
//            case vm::Bytecode::Store:
//            case vm::Bytecode::StoreR:
//            case vm::Bytecode::Pop:
//            case vm::Bytecode::Mul:
//            case vm::Bytecode::Div:
//            case vm::Bytecode::FDiv:
//            case vm::Bytecode::Dup:
//            case vm::Bytecode::Neg:
//                stream<<get<TypeProp>(prop).data.toHex();
//                break;
//            case vm::Bytecode::Push:
//                stream<<get<PushProp>(prop).data.toHex();
//                get<PushProp>(prop).const_value->toHex(stream);
//                break;
//            case vm::Bytecode::Cast:
//                stream << get<CastProp>(prop).src.toHex()
//                       << get<CastProp>(prop).dst.toHex();
//                break;
//            case vm::Bytecode::Ldm:
//            case vm::Bytecode::Stm:
//            case vm::Bytecode::StmR:
//                stream.write((const char*)&get<MemProp>(prop).size,sizeof(data::ptr));
//                break;
//            case vm::Bytecode::Psm:
//                stream.write((const char*)&get<PsmProp>(prop).size,sizeof(data::ptr));
//                stream.write(get<PsmProp>(prop).memory,get<PsmProp>(prop).size);
//                break;
//            case vm::Bytecode::Ret:
//            case vm::Bytecode::PushFrameBase:
//            case vm::Bytecode::PushGlobalBase:
//            case vm::Bytecode::And:
//            case vm::Bytecode::Or:
//            case vm::Bytecode::Xor:
//            case vm::Bytecode::Not:
//                // do nothing
//                break;
//        }
//    }
//
//
//    std::vector<Instruction *> Block::getInstructions() {
//        return instructions;
//    }
//
//    std::string Block::getLabel() {
//        return label_;
//    }
//
//    void Block::setLabel(std::string label) {
//        label_ = move(label);
//    }
//
//    Block &Block::Jmp(Block *block) {
//        instructions.push_back(new Instruction(vm::Bytecode::Jmp, Instruction::JumpProp{block}));
//        return *this;
//    }
//
//    Block &Block::Jif(Block *block) {
//        instructions.push_back(new Instruction(vm::Bytecode::Jif, Instruction::JumpProp{block}));
//        return *this;
//    }
//
//    Block &Block::EQ(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::EQ, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::NE(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::NE, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::LT(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::LT, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::GT(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::GT, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::LE(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::LE, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::GE(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::GE, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::Add(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::Add, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::Sub(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::Sub, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::Mul(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::Mul, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::Div(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::Div, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::FDiv(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::FDiv, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::Neg(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::Neg, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::And() {
//        instructions.push_back(new Instruction(vm::Bytecode::And));
//        return *this;
//    }
//
//    Block &Block::Or() {
//        instructions.push_back(new Instruction(vm::Bytecode::Or));
//        return *this;
//    }
//
//    Block &Block::Xor() {
//        instructions.push_back(new Instruction(vm::Bytecode::Xor));
//        return *this;
//    }
//
//    Block &Block::Not() {
//        instructions.push_back(new Instruction(vm::Bytecode::Not));
//        return *this;
//    }
//
//    Block &Block::Load(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::Load, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::Store(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::Store, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::StoreR(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::StoreR, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::Invoke(std::string signature) {
//        instructions.push_back(new Instruction(vm::Bytecode::Invoke, Instruction::InvokeProp{nullptr, signature}));
//        return *this;
//    }
//
//    Block &Block::Intrinsic(data::ptr id) {
//        instructions.push_back(new Instruction(vm::Bytecode::Intrinsic, Instruction::IntrinsicProp{id}));
//        return *this;
//    }
//
//    Block &Block::External(std::string signature) {
//        instructions.push_back(new Instruction(vm::Bytecode::External, Instruction::ExternalProp{signature}));
//        return *this;
//    }
//
//    Block &Block::Push(vm::Data data, ConstBase *const_value) {
//        instructions.push_back(new Instruction(vm::Bytecode::Push, Instruction::PushProp{data, const_value}));
//        return *this;
//    }
//
//    Block &Block::Pop(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::Pop, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::Ret() {
//        instructions.push_back(new Instruction(vm::Bytecode::Ret));
//        return *this;
//    }
//
//    Block &Block::Cast(vm::Data src, vm::Data dst) {
//        instructions.push_back(new Instruction(vm::Bytecode::Cast, Instruction::CastProp{src, dst}));
//        return *this;
//    }
//
//    Block &Block::Dup(vm::Data data) {
//        instructions.push_back(new Instruction(vm::Bytecode::Dup, Instruction::TypeProp{data}));
//        return *this;
//    }
//
//    Block &Block::Stm(data::u32 size) {
//        instructions.push_back(new Instruction(vm::Bytecode::Stm, Instruction::MemProp{size}));
//        return *this;
//    }
//
//    Block &Block::StmR(data::u32 size) {
//        instructions.push_back(new Instruction(vm::Bytecode::StmR, Instruction::MemProp{size}));
//        return *this;
//    }
//
//
//    Block &Block::Ldm(data::u32 size) {
//        instructions.push_back(new Instruction(vm::Bytecode::Ldm, Instruction::MemProp{size}));
//        return *this;
//    }
//
//    Block &Block::Psm(data::u32 size,const char *memory) {
//        instructions.push_back(new Instruction(vm::Bytecode::Psm, Instruction::PsmProp{size, memory}));
//        return *this;
//    }
//
//    Block &Block::PushFrameBase() {
//        instructions.push_back(new Instruction(vm::Bytecode::PushFrameBase));
//        return *this;
//    }
//
//    Block &Block::PushGlobalBase() {
//        instructions.push_back(new Instruction(vm::Bytecode::PushGlobalBase));
//        return *this;
//    }
//
//    Block &Block::RcInc() {
//        instructions.push_back(new Instruction(vm::Bytecode::RcInc));
//        return *this;
//    }
//
//    Block &Block::RcDec() {
//        instructions.push_back(new Instruction(vm::Bytecode::RcDec));
//        return *this;
//    }
//
//    Block::Block(std::string label){
//        label_ = std::move(label);
//    }
//
//    void Block::toString(ostream &stream) {
//        stream<<"<"<<label_<<">\n";
//        for(auto &inst:instructions) {
//            inst->toString(stream);
//            stream<<"\n";
//        }
//    }
//
//    void Block::toHex(ostream &stream) {
//        for(auto &inst:instructions)
//            inst->toHex(stream);
//    }
//
//    data::ptr Block::getByteLength() {
//        int ans = 0;
//        for(auto &inst : instructions)
//            ans += inst->getByteLength();
//        return ans;
//    }
//
//    data::ptr Block::getAddress() {
//        return this->instructions[0]->getAddress();
//    }
//
//
//
//}

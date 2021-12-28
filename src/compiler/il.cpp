//
// Created by yorkin on 12/11/21.
//

#include "il.h"
#include "nullSafe.h"

namespace evoBasic::il{
    using namespace std;

    void read(std::istream &stream,Bytecode code) {
        if((data::u8)stream.get()!=(data::u8)code)
            PANIC;
    }

    bool predict(std::istream &stream,Bytecode code){
        data::u8 byte;
        byte = stream.peek();
        return byte == (data::u8)code;
    }

    Result *Document::createResult(type::Prototype *prototype) {
        auto ret = new Result;
        ret->type = getTokenRef(prototype->getFullName());
        return ret;
    }

    TokenRef Document::getTokenRef(std::string text) {
        auto target = token_pool_map.find(text);
        if(target == token_pool_map.end()){
            token_pool.push_back(new TextTokenDef(this,token_pool.size(),text));
            token_pool_map.insert({text,token_pool.size()-1});
            return TokenRef(this, token_pool.size() - 1);
        }
        else return TokenRef(this, target->second);
    }

    TokenRef Document::getTokenRef(std::list<string> full_name_list) {
        Format fullname;
        for(auto &name : full_name_list){
            if(&name != &full_name_list.front())fullname<<'.';
            fullname << name;
        }
        auto target = token_pool_map.find(fullname);
        if(target == token_pool_map.end()){
            list<data::u64> ls;
            for(auto &str : full_name_list){
                ls.push_back(getTokenRef(str).getID());
            }
            token_pool_map.insert({fullname,token_pool.size()});
            token_pool.emplace_back(new ConstructedTokenDef(this,token_pool.size(),ls));
            return TokenRef(this, token_pool.size() - 1);
        }
        else return TokenRef(this, target->second);
    }

    Module *Document::createModule(std::string name, AccessFlag access, std::vector<Member*> members) {
        auto mod = new il::Module;
        mod->access = Access(access);
        mod->name = getTokenRef(name);
        mod->members = std::move(members);
        return mod;
    }

    Class *Document::createClass(std::string name, AccessFlag access, Extend extend, std::vector<Impl> impls,
                                  std::vector<Member*> members) {
        auto cls = new il::Class;
        cls->name = getTokenRef(name);
        cls->access = Access(access);
        cls->extend = extend;
        cls->impls = std::move(impls);
        cls->members = std::move(members);
        return cls;
    }

    Interface *Document::createInterface(std::string name, AccessFlag access, std::vector<FtnBase*> ftns) {
        auto ret = new il::Interface;
        ret->name = getTokenRef(name);
        ret->access = Access(access);
        ret->ftns = std::move(ftns);
        return ret;
    }

    Enum *Document::createEnum(std::string name, AccessFlag access, std::vector<Pair> pairs) {
        auto ret = new il::Enum;
        ret->name = getTokenRef(name);
        ret->access = Access(access);
        ret->pairs = std::move(pairs);
        return ret;
    }

    Record *Document::createRecord(std::string name, AccessFlag access, std::vector<Fld*> fields) {
        auto ret = new il::Record;
        ret->name = getTokenRef(name);
        ret->access = Access(access);
        ret->fields = std::move(fields);
        return ret;
    }

    Fld *Document::createField(std::string name, AccessFlag access, type::Prototype *prototype) {
        auto ret = new il::Fld;
        ret->name = getTokenRef(name);
        ret->access = Access(access);
        ret->type = getTokenRef(prototype->getFullName());
        return ret;
    }

    SFld *Document::createStaticField(std::string name, AccessFlag access, type::Prototype *prototype) {
        auto ret = new il::SFld;
        ret->name = getTokenRef(name);
        ret->access = Access(access);
        ret->type = getTokenRef(prototype->getFullName());
        return ret;
    }

    Ftn *Document::createFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result, vector<Local*> locals,
                                  std::list<Block*> blocks) {
        auto ret = new il::Ftn;
        ret->name = getTokenRef(name);
        ret->access = Access(access);
        ret->blocks = blocks;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        ret->result = result;
        return ret;
    }

    Ctor *Document::createConstructor(AccessFlag access, std::vector<Param*> params, std::vector<Local*> locals, std::list<Block*> blocks) {
        auto ret = new il::Ctor;
        ret->access = Access(access);
        ret->blocks = blocks;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        return ret;
    }

    VFtn *
    Document::createVirtualFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result, vector<Local*> locals,
                                    std::list<Block*> blocks) {
        auto ret = new il::VFtn;
        ret->name = getTokenRef(name);
        ret->access = Access(access);
        ret->blocks = blocks;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        ret->result = result;
        return ret;
    }

    SFtn *
    Document::createStaticFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result, vector<Local*> locals,
                                   std::list<Block*> blocks) {
        auto ret = new il::SFtn;
        ret->name = getTokenRef(name);
        ret->access = Access(access);
        ret->blocks = blocks;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        ret->result = result;
        return ret;
    }


    Ext *
    Document::createExternalFunction(std::string name, std::string lib, ExtAlias alias, AccessFlag access,
                                     std::vector<Param*> params, Result *result) {
        auto ret = new il::Ext;
        ret->name = getTokenRef(name);
        ret->lib = Lib(getTokenRef(lib));
        ret->access = Access(access);
        ret->params = std::move(params);
        ret->result = result;
        ret->alias = alias;
        return ret;
    }

    ExtAlias
    Document::createExtAlias(std::string text){
        return ExtAlias(getTokenRef(text));
    }

    Regular *Document::createParam(std::string name, type::Prototype *prototype, bool byref) {
        auto ret = new Regular;
        ret->name = getTokenRef(name);
        ret->type = getTokenRef(prototype->getFullName());
        ret->is_ref = byref;
        return ret;
    }

    Opt *Document::createOption(std::string name, type::Prototype *prototype, bool byref, Block *initial) {
        auto ret = new Opt;
        ret->name = getTokenRef(name);
        ret->type = getTokenRef(prototype->getFullName());
        ret->is_ref = byref;
        ret->initial = initial;
        return ret;
    }

    Inf *Document::createParamArray(std::string name, type::Prototype *prototype, bool byref) {
        auto ret = new Inf;
        ret->name = getTokenRef(name);
        ret->type = getTokenRef(prototype->getFullName());
        ret->is_ref = byref;
        return ret;
    }

    Pair Document::createPair(std::string name, data::u32 value) {
        il::Pair ret;
        ret.name = getTokenRef(name);
        ret.value = value;
        return ret;
    }

    Extend Document::createExtend(type::Class *cls) {
        il::Extend ret;
        ret.target = getTokenRef(cls->getFullName());
        return ret;
    }

    Impl Document::createImplements(type::Interface *interface) {
        il::Impl ret;
        ret.target = getTokenRef(interface->getFullName());
        return ret;
    }

    Local *Document::createLocal(std::string name, type::Prototype *prototype, data::u16 address) {
        auto ret = new il::Local;
        ret->name = getTokenRef(name);
        ret->type = getTokenRef(prototype->getFullName());
        ret->address = address;
        return ret;
    }

    FtnBase *Document::createInterfaceFunction(std::string name, AccessFlag access, std::vector<Param*> params,
                                                Result *result) {
        auto ret = new il::FtnBase;
        ret->name = getTokenRef(name);
        ret->access = Access(access);
        ret->params = std::move(params);
        ret->result = result;
        return ret;
    }
    


    std::string Local::toString() {
        return Format() << "(local " << name.toString() << "@" << to_string(address) << " " << type.toString() << ")";
    }


    Block &Block::Ldarg(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Ldarg,data));
        return *this;
    }

    std::string Block::toString() {
        Format fmt;
        fmt << '\n' << to_string(getAddress()) << ":";
        for(auto i : insts){
            fmt << "\n    " << i->toString();
        }
        return fmt;
    }

    Block::Block(std::initializer_list<Inst *> inst_init) {

    }

    Block &Block::Br(Block *block) {
        auto br = new InstBr;
        br->target = block;
        insts.push_back(br);
        return *this;
    }

    Block &Block::Jif(Block *block) {
        auto jif = new InstJif;
        jif->target = block;
        insts.push_back(jif);
        return *this;
    }

    Block &Block::EQ(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::EQ,data));
        return *this;
    }

    Block &Block::NE(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::NE,data));
        return *this;
    }

    Block &Block::LT(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::LT,data));
        return *this;
    }

    Block &Block::GT(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::GT,data));
        return *this;
    }

    Block &Block::LE(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::LE,data));
        return *this;
    }

    Block &Block::GE(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::GE,data));
        return *this;
    }

    Block &Block::Add(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Add,data));
        return *this;
    }

    Block &Block::Sub(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Sub,data));
        return *this;
    }

    Block &Block::Mul(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Mul,data));
        return *this;
    }

    Block &Block::Div(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Div,data));
        return *this;
    }

    Block &Block::FDiv(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::FDiv,data));
        return *this;
    }

    Block &Block::Neg(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Neg,data));
        return *this;
    }

    Block &Block::And() {
        insts.push_back(new InstWithOp(InstWithOp::Op::And));
        return *this;
    }

    Block &Block::Or() {
        insts.push_back(new InstWithOp(InstWithOp::Op::Or));
        return *this;
    }

    Block &Block::Xor() {
        insts.push_back(new InstWithOp(InstWithOp::Op::Xor));
        return *this;
    }

    Block &Block::Not() {
        insts.push_back(new InstWithOp(InstWithOp::Op::Not));
        return *this;
    }

    Block &Block::Nop() {
        insts.push_back(new InstWithOp(InstWithOp::Op::Nop));
        return *this;
    }

    Block &Block::Pop(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Pop,data));
        return *this;
    }

    Block &Block::Dup(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Dup,data));
        return *this;
    }

    Block &Block::Ret() {
        insts.push_back(new InstWithOp(InstWithOp::Op::Ret));
        return *this;
    }

    Block &Block::Push(DataType data, std::any value) {
        auto push = new InstPush;
        push->value = value;
        push->type = data;
        insts.push_back(push);
        return *this;
    }

    Block &Block::Ldc(TokenRef token) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldc,token));
        return *this;
    }

    Block &Block::Ldftn(TokenRef ftn) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldftn,ftn));
        return *this;
    }

    Block &Block::Ldsftn(TokenRef sftn) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldsftn,sftn));
        return *this;
    }

    Block &Block::Ldvftn(TokenRef vftn) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldvftn,vftn));
        return *this;
    }

    Block &Block::Starg(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Starg,data));
        return *this;
    }

    Block &Block::Ldarga() {
        insts.push_back(new InstWithOp(InstWithOp::Op::Ldarga));
        return *this;
    }

    Block &Block::Ldloc(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Ldloc,data));
        return *this;
    }

    Block &Block::Ldloca() {
        insts.push_back(new InstWithOp(InstWithOp::Op::Ldloca));
        return *this;
    }

    Block &Block::Stloc(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Stloc,data));
        return *this;
    }

    Block &Block::Ldfld(DataType data, TokenRef fld) {
        insts.push_back(new InstWithDataToken(InstWithDataToken::Op::Ldfld,data,fld));
        return *this;
    }

    Block &Block::Ldsfld(DataType data, TokenRef sfld) {
        insts.push_back(new InstWithDataToken(InstWithDataToken::Op::Ldsfld,data,sfld));
        return *this;
    }

    Block &Block::Ldflda(TokenRef fld) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldflda,fld));
        return *this;
    }

    Block &Block::Ldsflda(TokenRef sfld) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldflda,sfld));
        return *this;
    }

    Block &Block::Stfld(DataType data, TokenRef fld) {
        insts.push_back(new InstWithDataToken(InstWithDataToken::Op::Stfld,data,fld));
        return *this;
    }

    Block &Block::Stsfld(DataType data, TokenRef sfld) {
        insts.push_back(new InstWithDataToken(InstWithDataToken::Op::Stsfld,data,sfld));
        return *this;
    }

    Block &Block::Ldelem(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Stloc,data));
        return *this;
    }

    Block &Block::Ldelema() {
        insts.push_back(new InstWithOp(InstWithOp::Op::Ldelema));
        return *this;
    }

    Block &Block::Stelem(DataType data) {
        insts.push_back(new InstWithData(InstWithData::Op::Stelem,data));
        return *this;
    }

    Block &Block::Ldnull() {
        insts.push_back(new InstWithOp(InstWithOp::Op::Ldnull));
        return *this;
    }

    Block &Block::Newobj(TokenRef cls) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Newobj,cls));
        return *this;
    }

    Block &Block::Castcls(TokenRef src, TokenRef dst) {
        auto cast = new InstCastcls;
        cast->src_class = src;
        cast->dst_class = dst;
        insts.push_back(cast);
        return *this;
    }

    Block &Block::Conv(DataType src, DataType dst) {
        auto conv = new InstConv;
        conv->src = src;
        conv->dst = dst;
        insts.push_back(conv);
        return *this;
    }

    Block &Block::Callvirt() {
        insts.push_back(new InstWithOp(InstWithOp::CallVirt));
        return *this;
    }

    Block &Block::Invoke(TokenRef external) {
        insts.push_back(new InstWithToken(InstWithToken::Invoke,external));
        return *this;
    }

    Block &Block::Callstatic() {
        insts.push_back(new InstWithOp(InstWithOp::Callstatic));
        return *this;
    }

    Block &Block::Call() {
        insts.push_back(new InstWithOp(InstWithOp::Call));
        return *this;
    }

    data::u32 Block::getAddress() {
        return address;
    }

    void Block::setAddress(data::u32 value) {
        address = value;
    }

    data::u32 Block::getByteSize(){
        if(size == -1){
            size = 0;
            for(auto inst : insts){
                size += inst->getByteSize();
            }
        }
        return size;
    }

    std::string TokenRef::toString() {
        return getDocument()->getTokenDef(id)->getName();
    }

    data::u64 TokenRef::getID() {
        return id;
    }

    std::string Access::toString() {
        vector<string> str = {"Public","Private","Friend","Protected"};
        return Format() << "(Access " << str[(int)flag] << ")";
    }

    std::string Extend::toString() {
        return Format() << "(extend " << target.toString() << ")";
    }


    std::string Impl::toString() {
        return Format() << "(impl " << target.toString() << ")";
    }

    std::string Lib::toString() {
        return Format() << "(lib " << target.toString() << ")";
    }




    std::string Class::toString() {
        Format fmt;
        fmt << "(cls " << access.toString() << ' ' << name.toString() << ' ' << extend.toString();
        for(auto impl : impls) fmt << ' ' << impl.toString();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }


    std::string Module::toString() {
        Format fmt;
        fmt << "(mod " << access.toString() << ' ' << name.toString();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }


    std::string Interface::toString() {
        Format fmt;
        fmt << "(interface " << access.toString() << ' ' << name.toString();
        for(auto ftn : ftns) fmt << '\n' << ftn->toString();
        fmt << ")";
        return fmt;
    }


    std::string Pair::toString() {
        return Format() << "(pair " << name.toString() << ' ' << to_string(value);
    }

    std::string Enum::toString() {
        Format fmt;
        fmt << "(Enum " << access.toString() << ' ' << name.toString();
        for(auto p : pairs) fmt << '\n' << p.toString();
        fmt << ")";
        return fmt;
    }

    std::string Fld::toString() {
        return Format() << "(fld " << access.toString() << ' ' << name.toString() << ' ' << type.toString() <<")";
    }


    std::string SFld::toString() {
        return Format() << "(sfld " << access.toString() << ' ' << name.toString() << ' ' << type.toString() <<")";
    }

    std::string Record::toString() {
        Format fmt;
        fmt << "(record " << access.toString() << ' ' << name.toString();
        for(auto fld : fields) fmt << '\n' << fld->toString();
        fmt << ")";
        return fmt;
    }


    std::string Regular::toString() {
        return Format() << "(param " << name.toString() << " " << type.toString() << ")";
    }

    std::string Opt::toString() {
        return Format() << "(opt " << name.toString()
                        << " " << type.toString()
                        << initial->toString() << ")";
    }

    std::string Inf::toString() {
        return Format() << "(inf " << name.toString() << " " << type.toString() << ")";
    }


    std::string Result::toString() {
        return Format() << "(result " << type.toString() << ")";
    }


    std::string Ctor::toString() {
        return Format() << "(ctor " << FtnWithDefinition::toString() << ")";
    }


    std::string FtnBase::toString() {
        Format fmt;
        fmt << access.toString() << ' ';
        if(!name.isEmpty()) fmt << name.toString();
        for(auto p : params) fmt << ' ' << p->toString();
        if(result) fmt << ' ' << result->toString();
        return fmt;
    }

    void generateAddressInfo(list<Block*> blocks){
        Inst::ByteSize address = 0;
        for(auto block : blocks){
            block->setAddress(address);
            address += block->getByteSize();
        }
    }

    std::string FtnWithDefinition::toString() {
        generateAddressInfo(blocks);
        Format fmt;
        fmt << FtnBase::toString();
        for(auto l : locals) fmt << '\n' << l->toString();
        for(auto b : blocks) fmt << b->toString();
        return fmt;
    }

    std::string Ftn::toString() {
        return Format() << "(ftn " << FtnWithDefinition::toString() << ")";
    }


    std::string VFtn::toString() {
        return Format() << "(vftn " << FtnWithDefinition::toString() << ")";
    }

    std::string SFtn::toString() {
        return Format() << "(sftn " << FtnWithDefinition::toString() << ")";
    }

    std::string Ext::toString() {
        Format fmt;
        fmt << "(ext " << lib.toString();
        fmt << ' ' << alias.toString();
        fmt << ' ' << FtnBase::toString() << ")";
        return fmt;
    }

    std::string InstWithOp::toString() {
        vector<string> str = {"Nop","Ret","CallVirt","CallExt","Callstatic","Call",
                           "Ldnull","And","Or","Xor","Ldloca","Ldarga","Ldelema","Not"};
        Format fmt;
        fmt << str[(int)op];
        return fmt;
    }

    std::string InstWithToken::toString() {
        vector<string> str = {"Ldftn","Ldsftn","Ldvftn","Ldc","Newobj","Callext"};
        Format fmt;
        fmt << str[(int)op] << ' ' << token.toString();
        return fmt;
    }

    std::string InstJif::toString() {
        return Format() << "Jif " << target->getAddress();
    }

    std::string InstBr::toString() {
        return Format() << "Br " << target->getAddress();
    }

    std::string InstPush::toString() {
        vector<string> str = {
            "empty","i8","i16","i32","i64","u8","u16","u32","u64","f32","f64",
            "ref","ftn","vftn","sftn","record","array","boolean","character","delegate"
        };
        Format fmt;
        fmt << "Push." << str[(int)type] << ' ';
        switch(type){
            case DataType::i8:
                fmt << to_string(any_cast<data::i8>(value));
                break;
            case DataType::i16:
                fmt << to_string(any_cast<data::i16>(value));
                break;
            case DataType::i32:
                fmt << to_string(any_cast<data::i32>(value));
                break;
            case DataType::i64:
                fmt << to_string(any_cast<data::i64>(value));
                break;
            case DataType::u8:
                fmt << to_string(any_cast<data::u8>(value));
                break;
            case DataType::u16:
                fmt << to_string(any_cast<data::u16>(value));
                break;
            case DataType::u32:
                fmt << to_string(any_cast<data::u32>(value));
                break;
            case DataType::u64:
                fmt << to_string(any_cast<data::u64>(value));
                break;
            case DataType::f32:
                fmt << to_string(any_cast<data::f32>(value));
                break;
            case DataType::f64:
                fmt << to_string(any_cast<data::f64>(value));
                break;
            case DataType::boolean:
                fmt << to_string(any_cast<data::boolean>(value));
                break;
            default: PANIC;
        }
        return fmt;
    }

    std::string InstWithData::toString() {
        vector<string> inst = {"Ldelem","Stelem","Stelema","Ldarg","Starg","Ldloc","Stloc",
                           "Add","Sub","Mul","Div","FDiv","EQ","NE","LT","GT","LE","GE","Neg","Pop","Dup"};
        vector<string> ty = {
                "empty","i8","i16","i32","i64","u8","u16","u32","u64","f32","f64",
                "ref","ftn","vftn","sftn","record","array","boolean","character","delegate"
        };
        return Format() << inst[(int)op] << '.' << ty[(int)type];
    }

    std::string InstWithDataToken::toString() {
        vector<string> inst = {"Ldfld","Ldsfld","Ldflda","Ldsflda","Stfld","Stsfld"};
        vector<string> ty = {
                "empty","i8","i16","i32","i64","u8","u16","u32","u64","f32","f64",
                "ref","ftn","vftn","sftn","record","array","boolean","character","delegate"
        };
        return Format() << inst[(int)op] << '.' << ty[(int)type] << token.toString();
    }



    std::string InstCastcls::toString() {
        return Format() << "cast." << src_class.toString() << " " << dst_class.toString();
    }

    std::string InstConv::toString() {
        vector<string> ty = {
                "empty","i8","i16","i32","i64","u8","u16","u32","u64","f32","f64",
                "ref","ftn","vftn","sftn","record","array","boolean","character","delegate"
        };
        return Format() << "conv." << ty[(int)src] << " " << ty[(int)dst];
    }

    std::string Document::toString() {
        Format fmt;
        fmt << "(Document ";
        for(auto library : dependencies) fmt << '\n' << "(depend " << library.toString() << ")";
        for(auto token : token_pool) fmt << '\n' << token->toString();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }

    void Document::add(Member *member) {
        members.push_back(member);
    }

    std::string ExtAlias::toString() {
        return Format() << "(alias " << token.toString() <<")";
    }



    
    
    Inst::ByteSize InstWithOp::getByteSize() {
        return 1;
    }

    Inst::ByteSize InstWithToken::getByteSize() {
        return 1 + sizeof(TokenRef::ID);
    }

    Inst::ByteSize InstJif::getByteSize() {
        return 1 + sizeof(ByteSize);
    }

    Inst::ByteSize InstBr::getByteSize() {
        return 1 + sizeof(ByteSize);
    }
    
    Inst::ByteSize InstPush::getByteSize() {
        using enum DataType;
        switch(type){
            case i8:  return 2 + sizeof(data::i8);
            case i16: return 2 + sizeof(data::i16);
            case i32: return 2 + sizeof(data::i32);
            case i64: return 2 + sizeof(data::i64);
            case u8:  return 2 + sizeof(data::u8);
            case u16: return 2 + sizeof(data::u16);
            case u32: return 2 + sizeof(data::u32);
            case u64: return 2 + sizeof(data::u64);
            case f32: return 2 + sizeof(data::f32);
            case f64: return 2 + sizeof(data::f64);
            case boolean: return 2 + sizeof(data::boolean);
            default: PANIC;
        }
    }
    
    Inst::ByteSize InstWithData::getByteSize() {
        return 2;
    }

    Inst::ByteSize InstWithDataToken::getByteSize() {
        return 2 + sizeof(ByteSize);
    }
    
    Inst::ByteSize InstCastcls::getByteSize() {
        return 1 + 2 * sizeof(ByteSize);
    }

    Inst::ByteSize InstConv::getByteSize() {
        return 3;
    }







    void Block::toHex(std::ostream &stream) {
        for(auto inst : insts)inst->toHex(stream);
    }

    void TokenRef::toHex(std::ostream &stream) {
        write(stream,Bytecode::TokenRef);
        write(stream,id);
    }

    void TokenRef::fromHex(istream &stream) {
        read(stream,Bytecode::TokenRef);
        read(stream,id);
    }


    TokenRef::TokenRef(Document *document, data::u64 id) : id(id){
        setDocument(document);
    }

    TokenRef::TokenRef(Document *document) : TokenRef(document, -1){}

    bool TokenRef::isEmpty() {
        return id == -1 || getDocument() == nullptr;
    }


    void Access::toHex(std::ostream &stream) {
        switch (flag) {
            case AccessFlag::Private:
                write(stream,Bytecode::PriAcsDef);
                break;
            case AccessFlag::Public:
                write(stream,Bytecode::PubAcsDef);
                break;
            case AccessFlag::Protected:
                write(stream,Bytecode::PtdAcsDef);
                break;
        }
    }

    void Access::fromHex(istream &stream) {
        auto hex = (data::u8)stream.get();
        if(hex == (data::u8)Bytecode::PriAcsDef){
            flag = AccessFlag::Private;
        }
        else if(hex == (data::u8)Bytecode::PubAcsDef){
            flag = AccessFlag::Public;
        }
        else if(hex == (data::u8)Bytecode::PtdAcsDef){
            flag = AccessFlag::Protected;
        }
    }

    void Extend::toHex(std::ostream &stream) {
        write(stream,Bytecode::ExtendDef);
        write(stream,Bytecode::TokenRef);
        target.toHex(stream);
    }

    void Extend::fromHex(istream &stream) {
        read(stream,Bytecode::ExtendDef);
        read(stream,Bytecode::TokenRef);
        target.fromHex(stream);
    }

    void Impl::toHex(std::ostream &stream) {
        write(stream,Bytecode::ImplDef);
        target.toHex(stream);
    }

    void Impl::fromHex(istream &stream) {
        read(stream,Bytecode::ImplDef);
        target.fromHex(stream);
    }

    void Lib::toHex(std::ostream &stream) {
        write(stream,Bytecode::LibDef);
        target.toHex(stream);
    }

    void Lib::fromHex(istream &stream) {
        read(stream,Bytecode::LibDef);
        target.fromHex(stream);
    }

    void Member::toHex(ostream &stream) {
        write(stream,Bytecode::TokenRef);
        name.toHex(stream);
        access.toHex(stream);
    }

    void Member::fromHex(istream &stream) {
        read(stream,Bytecode::TokenRef);
        name.fromHex(stream);
        access.fromHex(stream);
    }

    void Class::toHex(std::ostream &stream) {
        write(stream,Bytecode::ClassDef);
        Member::toHex(stream);
        extend.toHex(stream);
        for(auto &impl : impls)impl.toHex(stream);
        for(auto member : members)member->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    void Class::fromHex(std::istream &stream){
        read(stream,Bytecode::ClassDef);
        Member::fromHex(stream);
        extend.fromHex(stream);
        while(predict(stream,Bytecode::ImplDef)){
            Impl impl;
            impl.fromHex(stream);
            impls.push_back(impl);
        }
        read(stream, members);
        read(stream,Bytecode::EndMark);
    }

    void Module::toHex(std::ostream &stream) {
        write(stream,Bytecode::ModuleDef);
        Member::toHex(stream);
        for(auto member : members)member->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    void Module::fromHex(istream &stream) {
        read(stream,Bytecode::ModuleDef);
        Member::fromHex(stream);
        read(stream, members);
        read(stream,Bytecode::EndMark);
    }

    void Interface::toHex(std::ostream &stream) {
        write(stream,Bytecode::InterfaceDef);
        Member::toHex(stream);
        for(auto member : ftns)member->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    void Interface::fromHex(istream &stream) {
        read(stream,Bytecode::InterfaceDef);
        Member::fromHex(stream);
        read(stream, ftns);
        read(stream,Bytecode::EndMark);
    }

    void Pair::toHex(std::ostream &stream) {
        write(stream,Bytecode::InterfaceDef);
        name.toHex(stream);
        write(stream,value);//u32
    }

    void Pair::fromHex(istream &stream) {
        read(stream,Bytecode::InterfaceDef);
        name.fromHex(stream);
        read(stream,value);
    }

    void Enum::toHex(std::ostream &stream) {
        write(stream,Bytecode::EnumDef);
        Member::toHex(stream);
        for(auto &p : pairs) p.toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    void Enum::fromHex(istream &stream) {
        read(stream,Bytecode::EnumDef);
        Member::fromHex(stream);
        while(predict(stream,Bytecode::PairDef)){
            Pair p;
            p.fromHex(stream);
            pairs.push_back(p);
        }
        read(stream,Bytecode::EndMark);
    }

    void Fld::toHex(std::ostream &stream) {
        write(stream,Bytecode::FldDef);
        Member::toHex(stream);
        type.toHex(stream);
    }

    void Fld::fromHex(istream &stream) {
        read(stream,Bytecode::FldDef);
        Member::fromHex(stream);
        type.fromHex(stream);
    }

    void SFld::toHex(std::ostream &stream) {
        write(stream,Bytecode::SFldDef);
        Member::toHex(stream);
        type.toHex(stream);
    }

    void SFld::fromHex(istream &stream) {
        read(stream,Bytecode::SFldDef);
        Member::fromHex(stream);
        type.fromHex(stream);
    }

    void Record::toHex(std::ostream &stream) {
        write(stream,Bytecode::RecordDef);
        Member::toHex(stream);
        for(auto member : fields)member->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    void Record::fromHex(istream &stream) {
        read(stream,Bytecode::RecordDef);
        Member::fromHex(stream);
        while(!predict(stream,Bytecode::EndMark)){
            auto fld = new Fld;
            fld->fromHex(stream);
            fields.push_back(fld);
        }
        read(stream,Bytecode::EndMark);
    }

    void Param::toHex(ostream &stream) {
        name.toHex(stream);
        type.toHex(stream);
        write(stream,(is_ref ? Bytecode::Byref : Bytecode::Byval));
    }

    void Param::fromHex(istream &stream) {
        name.fromHex(stream);
        type.fromHex(stream);
        if(predict(stream,Bytecode::Byref)){
            read(stream,Bytecode::Byref);
            is_ref = true;
        }
        else{
            read(stream,Bytecode::Byval);
            is_ref = false;
        }
    }

    void Regular::toHex(std::ostream &stream) {
        write(stream,Bytecode::RegDef);
        Param::toHex(stream);
    }

    void Regular::fromHex(istream &stream) {
        read(stream,Bytecode::RegDef);
        Param::fromHex(stream);
    }

    void Opt::toHex(std::ostream &stream) {
        write(stream,Bytecode::OptDef);
        Param::toHex(stream);
        write(stream,Bytecode::InstBeg);
        initial->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    void Opt::fromHex(istream &stream) {
        read(stream,Bytecode::OptDef);
        Param::fromHex(stream);
        //todo
//        read(stream,Bytecode::InstBeg);
//        initial->toHex(stream);
//        write(stream,Bytecode::EndMark);
    }

    void Inf::toHex(std::ostream &stream) {
        write(stream,Bytecode::InfDef);
        Param::toHex(stream);
    }

    void Inf::fromHex(istream &stream) {
        read(stream,Bytecode::InfDef);
        Param::fromHex(stream);
    }

    void Local::toHex(std::ostream &stream) {
        write(stream,Bytecode::LocalDef);
        name.toHex(stream);
        type.toHex(stream);
    }

    void Local::fromHex(istream &stream) {
        read(stream,Bytecode::LocalDef);
        name.fromHex(stream);
        type.fromHex(stream);
    }

    void Result::toHex(std::ostream &stream) {
        write(stream,Bytecode::ResultDef);
        type.toHex(stream);
    }

    void Result::fromHex(istream &stream) {
        read(stream,Bytecode::ResultDef);
        type.fromHex(stream);
    }

    void FtnBase::toHex(ostream &stream) {
        Member::toHex(stream);
        for(auto p : params) p->toHex(stream);
        if(result)result->toHex(stream);
    }

    void FtnBase::fromHex(istream &stream) {
        Member::fromHex(stream);
        while(true){
            Param *param = nullptr;
            if(predict(stream,Bytecode::RegDef)){
                param = new Regular;
            }
            else if(predict(stream,Bytecode::OptDef)){
                param = new Opt;
            }
            else if(predict(stream,Bytecode::InfDef)){
                param = new Inf;
            }
            else break;
            param->fromHex(stream);
            params.push_back(param);
        }
        if(predict(stream,Bytecode::ResultDef)){
            result = new Result;
            result->fromHex(stream);
        }
    }

    void FtnWithDefinition::toHex(ostream &stream) {
        generateAddressInfo(blocks);

        FtnBase::toHex(stream);
        for(auto local : locals) local->toHex(stream);
        write(stream,(data::u8)Bytecode::InstBeg);
        for(auto block : blocks) block->toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void FtnWithDefinition::fromHex(istream &stream) {
        FtnBase::fromHex(stream);
        while(predict(stream,Bytecode::LocalDef)){
            auto local = new Local;
            local->fromHex(stream);
            locals.push_back(local);
        }
        read(stream,Bytecode::InstBeg);
        for(auto block : blocks) block->fromHex(stream);
        read(stream,Bytecode::EndMark);
    }

    void Ctor::toHex(std::ostream &stream) {
        write(stream,Bytecode::CtorDef);
        FtnWithDefinition::toHex(stream);
    }

    void Ctor::fromHex(istream &stream) {
        read(stream,Bytecode::CtorDef);
        FtnWithDefinition::fromHex(stream);
    }

    void Ftn::toHex(std::ostream &stream) {
        write(stream,Bytecode::FtnDef);
        FtnWithDefinition::toHex(stream);
    }

    void Ftn::fromHex(istream &stream) {
        read(stream,Bytecode::FtnDef);
        FtnWithDefinition::fromHex(stream);
    }

    void VFtn::toHex(std::ostream &stream) {
        write(stream,Bytecode::VFtnDef);
        FtnWithDefinition::toHex(stream);
    }

    void VFtn::fromHex(istream &stream) {
        read(stream,Bytecode::VFtnDef);
        FtnWithDefinition::fromHex(stream);
    }

    void SFtn::toHex(std::ostream &stream) {
        write(stream,Bytecode::SFtnDef);
        FtnWithDefinition::toHex(stream);
    }

    void SFtn::fromHex(istream &stream) {
        read(stream,Bytecode::SFtnDef);
        FtnWithDefinition::fromHex(stream);
    }

    void ExtAlias::toHex(ostream &stream) {
        write(stream,Bytecode::ExtAliasDef);
        token.toHex(stream);
    }

    void ExtAlias::fromHex(istream &stream) {
        read(stream,Bytecode::ExtAliasDef);
        token.fromHex(stream);
    }

    void Ext::toHex(std::ostream &stream) {
        write(stream,Bytecode::ExtDef);
        FtnBase::toHex(stream);
        lib.toHex(stream);
        alias.toHex(stream);
    }

    void Ext::fromHex(istream &stream) {
        read(stream,Bytecode::ExtDef);
        FtnBase::fromHex(stream);
        lib.fromHex(stream);
        alias.fromHex(stream);
    }


    void Document::toHex(ostream &stream) {
        write(stream,Bytecode::DocumentDef);
        for(auto library : dependencies){
            write(stream,Bytecode::Depend);
            library.toHex(stream);
        }

        for(auto token : token_pool){
            token->toHex(stream);
        }

        for(auto member : members)member->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    void Document::fromHex(istream &stream) {
        read(stream,Bytecode::DocumentDef);
        while(predict(stream,Bytecode::Depend)){
            read(stream,Bytecode::Depend);
            TokenRef library;
            library.fromHex(stream);
            dependencies.push_back(library);
        }


        while(true){
            TokenDef *token = nullptr;
            if(predict(stream,Bytecode::TokenDef)){
                token = new TextTokenDef(this,token_pool.size());
            }
            else if(predict(stream,Bytecode::ConstructedDef)){
                token = new ConstructedTokenDef(this,token_pool.size());
            }
            token->fromHex(stream);
            token_pool.push_back(token);
        }

        read(stream,members);
        read(stream,Bytecode::EndMark);
    }



    void Document::addDependenceLibrary(std::string name) {
        dependencies.push_back(getTokenRef(name));
    }


    Document *Document::loadFromHex(std::istream &stream) {
        auto ret = new Document;

    }

    TokenRef Document::getTokenRef(data::u64 id) {
        ASSERT(id > token_pool.size(),"invalid token id");
        return TokenRef(this, id);
    }

    TokenDef *Document::getTokenDef(data::u64 id) {
        ASSERT(id > token_pool.size(),"invalid token id");
        return token_pool[id];
    }


    void read(istream &stream, std::vector<Member*> &members){
        while(true){
            Member *member = nullptr;
            if(predict(stream,Bytecode::ClassDef)){
                read(stream,Bytecode::ClassDef);
                member = new Class;
            }
            else if(predict(stream,Bytecode::ModuleDef)){
                read(stream,Bytecode::ModuleDef);
                member = new Module;
            }
            else if(predict(stream,Bytecode::RecordDef)){
                read(stream,Bytecode::RecordDef);
                member = new Record;
            }
            else if(predict(stream,Bytecode::EnumDef)){
                read(stream,Bytecode::EnumDef);
                member = new Enum;
            }
            else if(predict(stream,Bytecode::FldDef)){
                read(stream,Bytecode::FldDef);
                member = new Fld;
            }
            else if(predict(stream,Bytecode::SFldDef)){
                read(stream,Bytecode::SFldDef);
                member = new SFld;
            }
            else if(predict(stream,Bytecode::InterfaceDef)){
                read(stream,Bytecode::InterfaceDef);
                member = new Interface;
            }
            else if(predict(stream,Bytecode::ExtDef)){
                read(stream,Bytecode::ExtDef);
                member = new Ext;
            }
            else if(predict(stream,Bytecode::CtorDef)){
                read(stream,Bytecode::CtorDef);
                member = new Ctor;
            }
            else if(predict(stream,Bytecode::VFtnDef)){
                read(stream,Bytecode::VFtnDef);
                member = new VFtn;
            }
            else if(predict(stream,Bytecode::FtnDef)){
                read(stream,Bytecode::FtnDef);
                member = new Ftn;
            }
            else if(predict(stream,Bytecode::SFtnDef)){
                read(stream,Bytecode::SFtnDef);
                member = new SFtn;
            }
            else{
                break;
            }
            member->fromHex(stream);
            members.push_back(member);
        }
    }





    void InstWithOp::toHex(std::ostream &stream) {
        Bytecode code;
        switch(op){
            case Nop:           code = Bytecode::Nop;           break;
            case Ret:           code = Bytecode::Nop;           break;
            case CallVirt:      code = Bytecode::Nop;           break;
            case CallExt:       code = Bytecode::Nop;           break;
            case Callstatic:    code = Bytecode::Callstatic;    break;
            case Call:          code = Bytecode::Call;          break;
            case Ldnull:        code = Bytecode::Ldnull;        break;
            case And:           code = Bytecode::And;           break;
            case Or:            code = Bytecode::Or;            break;
            case Xor:           code = Bytecode::Xor;           break;
            case Ldloca:        code = Bytecode::Ldloca;        break;
            case Ldarga:        code = Bytecode::Ldarga;        break;
            case Ldelema:       code = Bytecode::Ldelema;       break;
            case Not:           code = Bytecode::Not;           break;
        }
        write(stream,(data::u8)code);
    }


    void InstWithToken::toHex(std::ostream &stream) {
        Bytecode code;
        switch(op){
            case Ldftn:   code = Bytecode::Ldftn;  break;
            case Ldsftn:  code = Bytecode::Ldsftn; break;
            case Ldvftn:  code = Bytecode::Ldvftn; break;
            case Ldc:     code = Bytecode::Ldc;    break;
            case Newobj:  code = Bytecode::Newobj; break;
            case Invoke:  code = Bytecode::Invoke; break;
            case Ldflda:  code = Bytecode::Ldflda; break;
            case Ldsflda: code = Bytecode::Ldsflda;break;
        }
        write(stream,(data::u8)code);
        write(stream,token.getID());
    }

    void InstJif::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::Jif);
        write(stream,target->getAddress());
    }


    void InstBr::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::Br);
        write(stream,target->getAddress());
    }


    Bytecode ILDataTypeToByteCode(DataType type){
        using enum DataType;
        switch(type){
            case i8:     return Bytecode::i8;
            case i16:    return Bytecode::i16;
            case i32:    return Bytecode::i32;
            case i64:    return Bytecode::i64;
            case u8:     return Bytecode::u8; 
            case u16:    return Bytecode::u16;
            case u32:    return Bytecode::u32;
            case u64:    return Bytecode::u64;
            case f32:    return Bytecode::f32;
            case f64:    return Bytecode::f64;
            case empty: 
            case ref:   
            case boolean:    
            case character:  
            case delegate:   
            case record:
            case array:  
            case ftn:  
            case vftn:  
            case sftn:   
                PANIC;
        }
    }

    void InstPush::toHex(std::ostream &stream) {
        using enum DataType;
        write(stream,(data::u8)Bytecode::Push);
        write(stream,(data::u8)ILDataTypeToByteCode(type));
        switch(type){
            case i8:
                write(stream,any_cast<data::i8>(value));
                break;
            case i16:
                write(stream,any_cast<data::i16>(value));
                break;
            case i32:
                write(stream,any_cast<data::i32>(value));
                break;
            case i64:
                write(stream,any_cast<data::i64>(value));
                break;
            case u8:
                write(stream,any_cast<data::u8>(value));
                break;
            case u16:
                write(stream,any_cast<data::u16>(value));
                break;
            case u32:
                write(stream,any_cast<data::u32>(value));
                break;
            case u64:
                write(stream,any_cast<data::u64>(value));
                break;
            case f32:
                write(stream,any_cast<data::f32>(value));
                break;
            case f64:
                write(stream,any_cast<data::f64>(value));
                break;
            default:
                PANIC;
        }
    }


    void InstWithData::toHex(std::ostream &stream) {
        Bytecode code;
        switch(op){
            case Ldelem:    code = Bytecode::Ldelem;    break;
            case Stelem:    code = Bytecode::Stelem;    break;
            case Stelema:   code = Bytecode::Stelema;   break;
            case Ldarg:     code = Bytecode::Ldarg;     break;
            case Starg:     code = Bytecode::Starg;     break;
            case Ldloc:     code = Bytecode::Ldloc;     break;
            case Stloc:     code = Bytecode::Stloc;     break;
            case Add:       code = Bytecode::Add;       break;
            case Sub:       code = Bytecode::Sub;       break;
            case Mul:       code = Bytecode::Mul;       break;
            case Div:       code = Bytecode::Div;       break;
            case FDiv:      code = Bytecode::FDiv;      break;
            case EQ:        code = Bytecode::EQ;        break;
            case NE:        code = Bytecode::NE;        break;
            case LT:        code = Bytecode::LT;        break;
            case GT:        code = Bytecode::GT;        break;
            case LE:        code = Bytecode::LE;        break;
            case GE:        code = Bytecode::GE;        break;
            case Neg:       code = Bytecode::Neg;       break;
            case Pop:       code = Bytecode::Pop;       break;
            case Dup:       code = Bytecode::Dup;       break;
        }
        write(stream,(data::u8)code);
        write(stream,(data::u8)ILDataTypeToByteCode(type));
    }

    void InstWithDataToken::toHex(std::ostream &stream) {
        Bytecode code;
        switch(op) {
            case Ldfld:     code = Bytecode::Ldfld;     break;
            case Ldsfld:    code = Bytecode::Ldsfld;    break;
            case Stfld:     code = Bytecode::Stfld;     break;
            case Stsfld:    code = Bytecode::Stsfld;    break;
        }
        write(stream,(data::u8)code);
        write(stream,(data::u8)ILDataTypeToByteCode(type));
        write(stream,token.getID());
    }
    
    void InstCastcls::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::CastCls);
        write(stream,src_class.getID());
        write(stream,dst_class.getID());
    }

    void InstConv::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::CastCls);
        write(stream,(data::u8)ILDataTypeToByteCode(src));
        write(stream,(data::u8)ILDataTypeToByteCode(dst));
    }




    Document *Node::getDocument(){
        return document;
    }

    void Node::setDocument(Document *document) {
        this->document = document;
    }


    std::string TextTokenDef::toString() {
        return Format() << "(token "<< to_string(id) << ' ' << text << ")";
    }

    std::string TextTokenDef::getName() {
        return text;
    }

    void TextTokenDef::toHex(ostream &stream) {
        write(stream,Bytecode::TokenDef);
        for(auto c : text){
            write(stream,c);
        }
        write(stream,Bytecode::EndMark);
    }

    void TextTokenDef::fromHex(istream &stream) {
        text.clear();
        read(stream,Bytecode::TokenDef);
        while(!predict(stream,Bytecode::EndMark)){
            char c;
            read(stream,c);
            text.push_back(c);
        }
        read(stream,Bytecode::EndMark);
    }

    std::string ConstructedTokenDef::toString() {
        Format fmt;
        fmt << "(Constructed " << to_string(id);
        for(auto sub_id : sub_token_list){
            fmt  << ' ' << to_string(sub_id);
        }
        fmt << ")";
        return fmt;
    }

    std::string ConstructedTokenDef::getName() {
        stringstream fmt;
        for(auto sub_id : sub_token_list){
            if(&sub_id == &sub_token_list.front()) fmt << '.';
            fmt << getDocument()->getTokenRef(sub_id).toString();
        }
        return fmt.str();
    }

    void ConstructedTokenDef::toHex(ostream &stream) {
        write(stream,Bytecode::ConstructedDef);
        for(auto sub_id : sub_token_list){
            write(stream,Bytecode::TokenRef);
            write(stream,sub_id);
        }
        write(stream,Bytecode::EndMark);
    }

    void ConstructedTokenDef::fromHex(istream &stream) {
        read(stream,Bytecode::ConstructedDef);
        while(predict(stream,Bytecode::TokenRef)){
            data::u64 sub_id;
            read(stream,Bytecode::TokenRef);
            read(stream,sub_id);
            sub_token_list.push_back(sub_id);
        }
        read(stream,Bytecode::EndMark);
    }
}
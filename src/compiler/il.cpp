//
// Created by yorkin on 12/11/21.
//

#include "il.h"

namespace evoBasic::il{
    using namespace std;


    Result *Document::createResult(type::Prototype *prototype) {
        auto ret = new Result;
        ret->type = createConstructedToken(prototype->getFullName());
        return ret;
    }

    Token *Document::createToken(std::string text) {
        auto target = token_pool_map.find(text);
        if(target == token_pool_map.end()){
            auto token = new il::Token(text);
            token_pool.push_back(token);
            token->setID(token_pool.size()-1);
            token_pool_map.insert({text,token_pool.size()-1});
            return token;
        }
        else return token_pool[target->second];
    }

    Token *Document::createConstructedToken(std::list<string> full_name_list) {
        Format fullname;
        for(auto &name : full_name_list){
            if(&name != &full_name_list.front())fullname<<'.';
            fullname << name;
        }
        auto target = token_pool_map.find(fullname);
        if(target == token_pool_map.end()){
            auto ret = new ConstructedToken;
            for(auto &str : full_name_list){
                ret->tokens.push_back(createToken(str));
            }
            ret->setID(token_pool.size());
            token_pool_map.insert({fullname,token_pool.size()});
            token_pool.push_back(ret);
            return ret;
        }
        else return token_pool[target->second];
    }

    Module *Document::createModule(std::string name, AccessFlag access, std::vector<Member*> members) {
        auto mod = new il::Module;
        mod->access = new Access(access);
        mod->name = createToken(name);
        mod->members = std::move(members);
        return mod;
    }

    Class *Document::createClass(std::string name, AccessFlag access, Extend *extend, std::vector<Impl*> impls,
                                  std::vector<Member*> members) {
        auto cls = new il::Class;
        cls->name = createToken(name);
        cls->access = new Access(access);
        cls->extend = extend;
        cls->impls = std::move(impls);
        cls->members = std::move(members);
        return cls;
    }

    Interface *Document::createInterface(std::string name, AccessFlag access, std::vector<FtnBase*> ftns) {
        auto ret = new il::Interface;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->ftns = std::move(ftns);
        return ret;
    }

    Enum *Document::createEnum(std::string name, AccessFlag access, std::vector<Pair*> pairs) {
        auto ret = new il::Enum;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->pairs = std::move(pairs);
        return ret;
    }

    Record *Document::createRecord(std::string name, AccessFlag access, std::vector<Fld*> fields) {
        auto ret = new il::Record;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->fields = std::move(fields);
        return ret;
    }

    Fld *Document::createField(std::string name, AccessFlag access, type::Prototype *prototype) {
        auto ret = new il::Fld;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->type = createConstructedToken(prototype->getFullName());
        return ret;
    }

    SFld *Document::createStaticField(std::string name, AccessFlag access, type::Prototype *prototype) {
        auto ret = new il::SFld;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->type = createConstructedToken(prototype->getFullName());
        return ret;
    }

    Ftn *Document::createFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result, vector<Local*> locals,
                                  std::list<Block*> blocks) {
        auto ret = new il::Ftn;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->blocks = blocks;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        ret->result = result;
        return ret;
    }

    Ctor *Document::createConstructor(AccessFlag access, std::vector<Param*> params, vector<Local*> locals, std::list<Block*> blocks) {
        auto ret = new il::Ctor;
        ret->access = new Access(access);
        ret->blocks = blocks;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        return ret;
    }

    VFtn *
    Document::createVirtualFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result, vector<Local*> locals,
                                    std::list<Block*> blocks) {
        auto ret = new il::VFtn;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->blocks = blocks;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        ret->result = result;
        return ret;
    }

    SFtn *
    Document::createStaticFunction(std::string name, AccessFlag access, std::vector<Param *> params, Result *result, vector<Local*> locals,
                                   std::list<Block*> blocks) {
        auto ret = new il::SFtn;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->blocks = blocks;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        ret->result = result;
        return ret;
    }


    Ext *
    Document::createExternalFunction(std::string name, std::string lib, ExtAlias *alias, AccessFlag access,
                                          std::vector<Param *> params, Result *result) {
        auto ret = new il::Ext;
        ret->name = createToken(name);
        ret->lib = new Lib(createToken(lib));
        ret->access = new Access(access);
        ret->params = std::move(params);
        ret->result = result;
        ret->alias = alias;
        return ret;
    }

    ExtAlias *
    Document::createExtAlias(std::string text){
        return new ExtAlias(createToken(text));
    }

    Param *Document::createParam(std::string name, type::Prototype *prototype,bool byref) {
        auto ret = new Param;
        ret->name = createToken(name);
        ret->type = createConstructedToken(prototype->getFullName());
        ret->is_ref = byref;
        return ret;
    }

    Opt *Document::createOption(std::string name, type::Prototype *prototype, bool byref, Block *initial) {
        auto ret = new Opt;
        ret->name = createToken(name);
        ret->type = createConstructedToken(prototype->getFullName());
        ret->is_ref = byref;
        ret->initial = initial;
        return ret;
    }

    Inf *Document::createParamArray(std::string name, type::Prototype *prototype, bool byref) {
        auto ret = new Inf;
        ret->name = createToken(name);
        ret->type = createConstructedToken(prototype->getFullName());
        ret->is_ref = byref;
        return ret;
    }

    Pair *Document::createPair(std::string name, data::u32 value) {
        auto ret = new il::Pair;
        ret->name = createToken(name);
        ret->value = value;
        return ret;
    }

    Extend *Document::createExtend(type::Class *cls) {
        auto ret = new il::Extend;
        ret->target = createConstructedToken(cls->getFullName());
        return ret;
    }

    Impl *Document::createImplements(type::Interface *interface) {
        auto ret = new il::Impl;
        ret->target = createConstructedToken(interface->getFullName());
        return ret;
    }

    Local *Document::createLocal(std::string name, type::Prototype *prototype, data::u16 address) {
        auto ret = new il::Local;
        ret->name = createToken(name);
        ret->type = createConstructedToken(prototype->getFullName());
        ret->address = address;
        return ret;
    }

    FtnBase *Document::createInterfaceFunction(std::string name, AccessFlag access, std::vector<Param *> params,
                                                Result *result) {
        auto ret = new il::FtnBase;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->params = std::move(params);
        ret->result = result;
        return ret;
    }
    


    std::string Local::toString() {
        return Format() << "(local " << name->getName() << "@" << to_string(address) << " " << type->getName() << ")";
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

    Block &Block::Ldc(Token *token) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldc,token));
        return *this;
    }

    Block &Block::Ldftn(Token *ftn) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldftn,ftn));
        return *this;
    }

    Block &Block::Ldsftn(Token *sftn) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldsftn,sftn));
        return *this;
    }

    Block &Block::Ldvftn(Token *vftn) {
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

    Block &Block::Ldfld(DataType data, Token *fld) {
        insts.push_back(new InstWithDataToken(InstWithDataToken::Op::Ldfld,data,fld));
        return *this;
    }

    Block &Block::Ldsfld(DataType data, Token *sfld) {
        insts.push_back(new InstWithDataToken(InstWithDataToken::Op::Ldsfld,data,sfld));
        return *this;
    }

    Block &Block::Ldflda(Token *fld) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldflda,fld));
        return *this;
    }

    Block &Block::Ldsflda(Token *sfld) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Ldflda,sfld));
        return *this;
    }

    Block &Block::Stfld(DataType data, Token *fld) {
        insts.push_back(new InstWithDataToken(InstWithDataToken::Op::Stfld,data,fld));
        return *this;
    }

    Block &Block::Stsfld(DataType data, Token *sfld) {
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

    Block &Block::Newobj(Token *cls) {
        insts.push_back(new InstWithToken(InstWithToken::Op::Newobj,cls));
        return *this;
    }

    Block &Block::Castcls(Token *src, Token *dst) {
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

    Block &Block::Invoke(Token *external) {
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

    std::string Token::toString() {
        return Format() << "(token " << to_string(id) << " " << text << ")";
    }


    std::string Token::getName() {
        return Format() << text;
    }

    void Token::setID(data::u64 value) {
        id = value;
    }

    data::u64 Token::getID() {
        return id;
    }

    std::string ConstructedToken::toString() {
        Format fmt;
        fmt << "(constructed " << to_string(id);
        for(auto sub_token : tokens){
            fmt << ' ' << to_string(sub_token->getID());
        }
        fmt << ')';
        return fmt;
    }


    std::string ConstructedToken::getName(){
        Format fmt;
        for(auto token : tokens){
            if(token != tokens.front()) fmt << '.';
            fmt << token->getName();
        }
        return fmt;
    }

    std::string Access::toString() {
        vector<string> str = {"Public","Private","Friend","Protected"};
        return Format() << "(Access " << str[(int)flag] << ")";
    }

    std::string Extend::toString() {
        return Format() << "(extend " << target->getName() << ")";
    }


    std::string Impl::toString() {
        return Format() << "(impl " << target->getName() << ")";
    }

    std::string Lib::toString() {
        return Format() << "(lib " << target->getName() << ")";
    }


    ConstructedToken *Member::getConstructedToken() {

    }

    std::string Class::toString() {
        Format fmt;
        fmt << "(cls " << access->toString() << ' ' << name->getName() << ' ' << extend->toString();
        for(auto impl : impls) fmt << ' ' << impl->toString();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }


    std::string Module::toString() {
        Format fmt;
        fmt << "(mod " << access->toString() << ' ' << name->getName();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }


    std::string Interface::toString() {
        Format fmt;
        fmt << "(interface " << access->toString() << ' ' << name->getName();
        for(auto ftn : ftns) fmt << '\n' << ftn->toString();
        fmt << ")";
        return fmt;
    }


    std::string Pair::toString() {
        return Format() << "(pair " << name->getName() << ' ' << to_string(value);
    }

    std::string Enum::toString() {
        Format fmt;
        fmt << "(Enum " << access->toString() << ' ' << name->getName();
        for(auto p : pairs) fmt << '\n' << p->toString();
        fmt << ")";
        return fmt;
    }

    std::string Fld::toString() {
        return Format() << "(fld " << access->toString() << ' ' << name->getName() << ' ' << type->getName() <<")";
    }


    std::string SFld::toString() {
        return Format() << "(sfld " << access->toString() << ' ' << name->getName() << ' ' << type->getName() <<")";
    }

    std::string Record::toString() {
        Format fmt;
        fmt << "(record " << access->toString() << ' ' << name->getName();
        for(auto fld : fields) fmt << '\n' << fld->toString();
        fmt << ")";
        return fmt;
    }


    std::string Param::toString() {
        return Format() << "(param " << name->getName() << " " << type->getName() << ")";
    }

    std::string Opt::toString() {
        return Format() << "(opt " << name->getName()
                        << " " << type->getName()
                        << initial->toString() << ")";
    }

    std::string Inf::toString() {
        return Format() << "(inf " << name->getName() << " " << type->getName() << ")";
    }


    std::string Result::toString() {
        return Format() << "(result " << type->getName() << ")";
    }


    std::string Ctor::toString() {
        return Format() << "(ctor " << FtnWithDefinition::toString() << ")";
    }


    std::string FtnBase::toString() {
        Format fmt;
        fmt << access->toString() << ' ';
        if(name) fmt << name->getName();
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
        fmt << "(ext " << lib->toString();
        if(alias)fmt << ' ' << alias->toString();
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
        fmt << str[(int)op] << ' ' << token->getName();
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
        return Format() << inst[(int)op] << '.' << ty[(int)type] << token->getName();
    }



    std::string InstCastcls::toString() {
        return Format() << "cast." << src_class->getName() << " " << dst_class->getName();
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
        for(auto token : token_pool) fmt << '\n' << token->toString();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }

    void Document::add(Member *member) {
        members.push_back(member);
    }

    std::string ExtAlias::toString() {
        return Format() << "(alias " << token->getName() <<")";
    }



    
    
    Inst::ByteSize InstWithOp::getByteSize() {
        return 2;
    }

    Inst::ByteSize InstWithToken::getByteSize() {
        return 1 + sizeof(Token::ID);
    }

    Inst::ByteSize InstJif::getByteSize() {
        return 1 + sizeof(ByteSize);
    }

    Inst::ByteSize InstBr::getByteSize() {
        return 1 + sizeof(ByteSize);
    }
    
    Inst::ByteSize InstPush::getByteSize() {
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






    void Local::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::LocalDef);
        write(stream,(data::u8)Bytecode::TokenRef);
        write(stream,type->getID());
    }

    void Block::toHex(std::ostream &stream) {
        for(auto inst : insts)inst->toHex(stream);
    }

    void Token::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::TokenDef);
        for(char c : text) write(stream,c);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void ConstructedToken::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::ConstructedDef);
        for(auto t : tokens) write(stream,t->getID());
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Access::toHex(std::ostream &stream) {
        switch (flag) {
            case AccessFlag::Private:
                write(stream,(data::u8)Bytecode::PriAcsDef);
                break;
            case AccessFlag::Public:
                write(stream,(data::u8)Bytecode::PubAcsDef);
                break;
            case AccessFlag::Protected:
                write(stream,(data::u8)Bytecode::PtdAcsDef);
                break;
        }
    }

    void Extend::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::ExtendDef);
        write(stream,target->getID());
    }

    void Impl::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::ImplDef);
        write(stream,target->getID());
    }

    void Lib::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::LibDef);
        write(stream,target->getID());
    }

    void Document::toHex(ostream &stream) {
        write(stream,(data::u8)Bytecode::DocumentDef);
        for(auto token : token_pool)token->toHex(stream);
        for(auto member : members)member->toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Class::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::ClassDef);
        name->toHex(stream);
        access->toHex(stream);
        extend->toHex(stream);
        for(auto impl : impls)impl->toHex(stream);
        for(auto member : members)member->toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Module::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::ModuleDef);
        name->toHex(stream);
        access->toHex(stream);
        for(auto member : members)member->toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Interface::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::InterfaceDef);
        name->toHex(stream);
        access->toHex(stream);
        for(auto member : ftns)member->toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Pair::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::InterfaceDef);
        write(stream,name->getID());
        write(stream,value);//u32
    }

    void Enum::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::EnumDef);
        write(stream,name->getID());
        access->toHex(stream);
        for(auto p : pairs) p->toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Fld::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::FldDef);
        write(stream,name->getID());
        write(stream,type->getID());
    }

    void SFld::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::SFldDef);
        write(stream,name->getID());
        write(stream,type->getID());
    }

    void Record::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::RecordDef);
        name->toHex(stream);
        access->toHex(stream);
        for(auto member : fields)member->toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Param::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::ParamDef);
        write(stream,name->getID());
        write(stream,type->getID());
        write(stream,(data::u8)(is_ref ? Bytecode::Byref : Bytecode::Byval));
    }

    void Opt::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::OptDef);
        write(stream,name->getID());
        write(stream,type->getID());
        write(stream,(data::u8)(is_ref ? Bytecode::Byref : Bytecode::Byval));
        write(stream,(data::u8)Bytecode::InstBeg);
        initial->toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Inf::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::InfDef);
        write(stream,name->getID());
        write(stream,type->getID());
        write(stream,(data::u8)(is_ref ? Bytecode::Byref : Bytecode::Byval));
    }

    void Result::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::ResultDef);
        write(stream,type->getID());
    }

    void Ctor::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::CtorDef);
        access->toHex(stream);
        FtnWithDefinition::toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void FtnBase::toHex(ostream &stream) {
        for(auto p : params) p->toHex(stream);
        if(result)result->toHex(stream);
    }


    void FtnWithDefinition::toHex(ostream &stream) {
        generateAddressInfo(blocks);

        FtnBase::toHex(stream);
        for(auto local : locals) local->toHex(stream);
        write(stream,(data::u8)Bytecode::InstBeg);
        for(auto block : blocks) block->toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Ftn::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::FtnDef);
        write(stream,name->getID());
        access->toHex(stream);
        FtnWithDefinition::toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void VFtn::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::VFtnDef);
        write(stream,name->getID());
        access->toHex(stream);
        FtnWithDefinition::toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void SFtn::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::SFtnDef);
        write(stream,name->getID());
        access->toHex(stream);
        FtnWithDefinition::toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
    }

    void Ext::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::ExtDef);
        write(stream,name->getID());
        access->toHex(stream);
        lib->toHex(stream);
        alias->toHex(stream);
        FtnBase::toHex(stream);
        write(stream,(data::u8)Bytecode::EndMark);
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
        write(stream,token->getID());
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
        write(stream,token->getID());
    }
    
    void InstCastcls::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::CastCls);
        write(stream,src_class->getID());
        write(stream,dst_class->getID());
    }

    void InstConv::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::CastCls);
        write(stream,(data::u8)ILDataTypeToByteCode(src));
        write(stream,(data::u8)ILDataTypeToByteCode(dst));
    }

    void ExtAlias::toHex(ostream &stream) {
        write(stream,(data::u8)Bytecode::ExtAliasDef);
        write(stream,token->getID());
    }

}
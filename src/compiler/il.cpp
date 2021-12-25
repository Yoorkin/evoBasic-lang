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
                                   Block *entry) {
        auto ret = new il::Ftn;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->entry = entry;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        ret->result = result;
        return ret;
    }

    Ctor *Document::createConstructor(AccessFlag access, std::vector<Param*> params, vector<Local*> locals, Block *entry) {
        auto ret = new il::Ctor;
        ret->access = new Access(access);
        ret->entry = entry;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        return ret;
    }

    VFtn *
    Document::createVirtualFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result, vector<Local*> locals,
                                     Block *entry) {
        auto ret = new il::VFtn;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->entry = entry;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        ret->result = result;
        return ret;
    }

    SFtn *
    Document::createStaticFunction(std::string name, AccessFlag access, std::vector<Param *> params, Result *result, vector<Local*> locals,
                                    Block *entry) {
        auto ret = new il::SFtn;
        ret->name = createToken(name);
        ret->access = new Access(access);
        ret->entry = entry;
        ret->params = std::move(params);
        ret->locals = std::move(locals);
        ret->result = result;
        return ret;
    }

    Ext *
    Document::createExternalFunction(std::string name, std::string lib, AccessFlag access, std::vector<Param*> params,
                                      Result *result) {
        auto ret = new il::Ext;
        ret->name = createToken(name);
        ret->lib = new Lib(createToken(lib));
        ret->access = new Access(access);
        ret->params = std::move(params);
        ret->result = result;
        return ret;
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

    void Local::toHex(std::ostream &stream) {

    }

    Block &Block::Ldarg(DataType data) {

    }

    std::string Block::toString() {
        Format fmt;
        for(auto i : insts){
            fmt << '\n' << i->toString();
        }
        return fmt;
    }

    void Block::toHex(std::ostream &stream) {

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
        return 0;
    }

    std::string Token::toString() {
        return Format() << "(token " << to_string(id) << " " << text << ")";
    }

    void Token::toHex(std::ostream &stream) {

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

    void ConstructedToken::toHex(std::ostream &stream) {

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

    void Access::toHex(std::ostream &stream) {

    }

    std::string Extend::toString() {
        return Format() << "(extend " << target->getName() << ")";
    }

    void Extend::toHex(std::ostream &stream) {

    }

    std::string Impl::toString() {
        return Format() << "(impl " << target->getName() << ")";
    }

    void Impl::toHex(std::ostream &stream) {

    }

    std::string Lib::toString() {
        return Format() << "(lib " << target->getName() << ")";
    }

    void Lib::toHex(std::ostream &stream) {

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

    void Class::toHex(std::ostream &stream) {

    }

    std::string Module::toString() {
        Format fmt;
        fmt << "(mod " << access->toString() << ' ' << name->getName();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }

    void Module::toHex(std::ostream &stream) {

    }

    std::string Interface::toString() {
        Format fmt;
        fmt << "(interface " << access->toString() << ' ' << name->getName();
        for(auto ftn : ftns) fmt << '\n' << ftn->toString();
        fmt << ")";
        return fmt;
    }

    void Interface::toHex(std::ostream &stream) {

    }

    std::string Pair::toString() {
        return Format() << "(pair " << name->getName() << ' ' << to_string(value);
    }

    void Pair::toHex(std::ostream &stream) {

    }

    std::string Enum::toString() {
        Format fmt;
        fmt << "(Enum " << access->toString() << ' ' << name->getName();
        for(auto p : pairs) fmt << '\n' << p->toString();
        fmt << ")";
        return fmt;
    }

    void Enum::toHex(std::ostream &stream) {

    }

    std::string Fld::toString() {
        return Format() << "(fld " << access->toString() << ' ' << name->getName() << ' ' << type->getName() <<")";
    }

    void Fld::toHex(std::ostream &stream) {

    }

    std::string SFld::toString() {
        return Format() << "(sfld " << access->toString() << ' ' << name->getName() << ' ' << type->getName() <<")";
    }

    void SFld::toHex(std::ostream &stream) {
        Fld::toHex(stream);
    }

    std::string Record::toString() {
        Format fmt;
        fmt << "(record " << access->toString() << ' ' << name->getName();
        for(auto fld : fields) fmt << '\n' << fld->toString();
        fmt << ")";
        return fmt;
    }

    void Record::toHex(std::ostream &stream) {

    }

    std::string Param::toString() {
        return Format() << "(param " << name->getName() << " " << type->getName() << ")";
    }

    void Param::toHex(std::ostream &stream) {

    }

    std::string Opt::toString() {
        return Format() << "(opt " << name->getName()
                        << " " << type->getName()
                        << initial->toString() << ")";
    }

    void Opt::toHex(std::ostream &stream) {
        Param::toHex(stream);
    }

    std::string Inf::toString() {
        return Format() << "(inf " << name->getName() << " " << type->getName() << ")";
    }

    void Inf::toHex(std::ostream &stream) {
        Param::toHex(stream);
    }

    std::string Result::toString() {
        return Format() << "(result " << type->getName() << ")";
    }

    void Result::toHex(std::ostream &stream) {

    }

    std::string Ctor::toString() {
        return Format() << "(ctor " << FtnWithDefinition::toString() << ")";
    }

    void Ctor::toHex(std::ostream &stream) {

    }


    std::string FtnBase::toString() {
        Format fmt;
        fmt << access->toString() << ' ';
        if(name) fmt << name->getName();
        for(auto p : params) fmt << ' ' << p->toString();
        if(result) fmt << ' ' << result->toString();
        return fmt;
    }

    void FtnBase::toHex(ostream &stream) {

    }

    std::string FtnWithDefinition::toString() {
        Format fmt;
        fmt << FtnBase::toString();
        for(auto l : locals) fmt << '\n' << l->toString();
        fmt << entry->toString();
        return fmt;
    }

    std::string Ftn::toString() {
        return Format() << "(ftn " << FtnWithDefinition::toString() << ")";
    }

    void Ftn::toHex(std::ostream &stream) {

    }

    std::string VFtn::toString() {
        return Format() << "(vftn " << FtnWithDefinition::toString() << ")";
    }

    void VFtn::toHex(std::ostream &stream) {

    }

    std::string SFtn::toString() {
        return Format() << "(sftn " << FtnWithDefinition::toString() << ")";
    }

    void SFtn::toHex(std::ostream &stream) {

    }

    std::string Ext::toString() {
        return Format() << "(ext " << lib->toString() << ' ' << FtnBase::toString() << ")";
    }

    void Ext::toHex(std::ostream &stream) {

    }

    std::string InstWithOp::toString() {
        vector<string> str = {"Nop","Ret","CallVirt","CallExt","Callstatic","Call",
                           "Ldnull","And","Or","Xor","Ldloca","Ldarga","Ldelema","Not"};
        Format fmt;
        fmt << "\n" << str[(int)op]; 
        return fmt;
    }

    void InstWithOp::toHex(std::ostream &stream) {

    }

    std::string InstWithToken::toString() {
        vector<string> str = {"Ldftn","Ldsftn","Ldvftn","Ldc","Newobj","Callext"};
        Format fmt;
        fmt << "\n" << str[(int)op] << ' ' << token->getName();
        return fmt;
    }

    void InstWithToken::toHex(std::ostream &stream) {

    }

    std::string InstJif::toString() {
        return Format() << "Jif " << target->getAddress();
    }

    void InstJif::toHex(std::ostream &stream) {

    }

    std::string InstBr::toString() {
        return Format() << "Br " << target->getAddress();
    }

    void InstBr::toHex(std::ostream &stream) {

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

    void InstPush::toHex(std::ostream &stream) {

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

    void InstWithData::toHex(std::ostream &stream) {

    }

    std::string InstWithDataToken::toString() {
        vector<string> inst = {"Ldfld","Ldsfld","Ldflda","Ldsflda","Stfld","Stsfld"};
        vector<string> ty = {
                "empty","i8","i16","i32","i64","u8","u16","u32","u64","f32","f64",
                "ref","ftn","vftn","sftn","record","array","boolean","character","delegate"
        };
        return Format() << inst[(int)op] << '.' << ty[(int)type] << token->getName();
    }

    void InstWithDataToken::toHex(std::ostream &stream) {

    }


    std::string InstCastcls::toString() {
        return Format() << "cast." << src_class->getName() << " " << dst_class->getName();
    }

    void InstCastcls::toHex(std::ostream &stream) {

    }

    std::string InstConv::toString() {
        vector<string> ty = {
                "empty","i8","i16","i32","i64","u8","u16","u32","u64","f32","f64",
                "ref","ftn","vftn","sftn","record","array","boolean","character","delegate"
        };
        return Format() << "conv." << ty[(int)src] << " " << ty[(int)dst];
    }

    void InstConv::toHex(std::ostream &stream) {

    }

    std::string Document::toString() {
        Format fmt;
        fmt << "(Document ";
        for(auto token : token_pool) fmt << '\n' << token->toString();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }

    void Document::toHex(ostream &stream) {

    }

    void Document::add(Member *member) {
        members.push_back(member);
    }
}
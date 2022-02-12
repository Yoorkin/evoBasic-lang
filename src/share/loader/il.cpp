//
// Created by yorkin on 12/11/21.
//

#include "il.h"
#include <utils/nullSafe.h>
#include "bytecode.h"
#include <utils/enums.h>

namespace evoBasic::il{
    using namespace std;

    void read(std::istream &stream,Bytecode code) {
        auto c = stream.get();
        if((data::u8)c!=(data::u8)code)
            PANICMSG(to_string(stream.tellg()));
    }

    bool predict(std::istream &stream,Bytecode code){
        data::u8 byte;
        byte = stream.peek();
        return byte == (data::u8)code;
    }

    void read(istream &stream, data::u8 *memory, Inst::ByteSize size){
        for(Inst::ByteSize i = 0;i<size;i++){
            stream.read(reinterpret_cast<char*>(memory+i),1);
        }
    }


    TokenRef *Document::getTokenRef(std::string text) {
        auto target = token_pool_map.find(text);
        if(target == token_pool_map.end()){
            token_pool.push_back(new TextTokenDef(this,token_pool.size(),text));
            token_pool_map.insert({text,token_pool.size()-1});
            return new TokenRef(this, token_pool.size() - 1);
        }
        else return new TokenRef(this, target->second);
    }

    TokenRef *Document::getTokenRef(std::list<string> full_name_list) {
        Format fullname;
        for(auto &name : full_name_list){
            if(&name != &full_name_list.front())fullname<<'.';
            fullname << name;
        }
        auto target = token_pool_map.find(fullname);
        if(target == token_pool_map.end()){
            list<TokenRef*> ls;
            for(auto &str : full_name_list){
                ls.push_back(getTokenRef(str));
            }
            token_pool_map.insert({fullname,token_pool.size()});
            token_pool.emplace_back(new ConstructedTokenDef(this,token_pool.size(),ls));
            return new TokenRef(this, token_pool.size() - 1);
        }
        else return new TokenRef(this, target->second);
    }


    std::string Local::toString() {
        return Format() << name->toString() << "@" << to_string(address) << " " << type->toString();
    }


    BasicBlock &BasicBlock::Ldarg(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Ldarg,data));
        return *this;
    }

    BasicBlock &BasicBlock::Br(BasicBlock *block) {
        insts.push_back(new InstBr(getDocument(),block));
        return *this;
    }

    BasicBlock &BasicBlock::Jif(BasicBlock *block) {
        insts.push_back(new InstJif(getDocument(),block));
        return *this;
    }

    BasicBlock &BasicBlock::EQ(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::EQ,data));
        return *this;
    }

    BasicBlock &BasicBlock::NE(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::NE,data));
        return *this;
    }

    BasicBlock &BasicBlock::LT(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::LT,data));
        return *this;
    }

    BasicBlock &BasicBlock::GT(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::GT,data));
        return *this;
    }

    BasicBlock &BasicBlock::LE(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::LE,data));
        return *this;
    }

    BasicBlock &BasicBlock::GE(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::GE,data));
        return *this;
    }

    BasicBlock &BasicBlock::Add(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Add,data));
        return *this;
    }

    BasicBlock &BasicBlock::Sub(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Sub,data));
        return *this;
    }

    BasicBlock &BasicBlock::Mul(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Mul,data));
        return *this;
    }

    BasicBlock &BasicBlock::Div(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Div,data));
        return *this;
    }

    BasicBlock &BasicBlock::FDiv(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::FDiv,data));
        return *this;
    }

    BasicBlock &BasicBlock::Neg(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Neg,data));
        return *this;
    }

    BasicBlock &BasicBlock::And() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Op::And));
        return *this;
    }

    BasicBlock &BasicBlock::Or() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Op::Or));
        return *this;
    }

    BasicBlock &BasicBlock::Xor() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Op::Xor));
        return *this;
    }

    BasicBlock &BasicBlock::Not() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Op::Not));
        return *this;
    }

    BasicBlock &BasicBlock::Nop() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Op::Nop));
        return *this;
    }

    BasicBlock &BasicBlock::Pop(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Pop,data));
        return *this;
    }

    BasicBlock &BasicBlock::Dup(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Dup,data));
        return *this;
    }

    BasicBlock &BasicBlock::Ret() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Op::Ret));
        return *this;
    }

    BasicBlock &BasicBlock::Push(DataType data, std::any value) {
        insts.push_back(new InstPush(getDocument(),data,value));
        return *this;
    }

    BasicBlock &BasicBlock::Ldc(TokenRef *token) {
        insts.push_back(new InstWithToken(getDocument(),InstWithToken::Op::Ldc,token));
        return *this;
    }

    BasicBlock &BasicBlock::Ldftn(TokenRef *ftn) {
        insts.push_back(new InstWithToken(getDocument(),InstWithToken::Op::Ldftn,ftn));
        return *this;
    }

    BasicBlock &BasicBlock::Ldsftn(TokenRef *sftn) {
        insts.push_back(new InstWithToken(getDocument(),InstWithToken::Op::Ldsftn,sftn));
        return *this;
    }

    BasicBlock &BasicBlock::Ldvftn(TokenRef *vftn) {
        insts.push_back(new InstWithToken(getDocument(),InstWithToken::Op::Ldvftn,vftn));
        return *this;
    }

    BasicBlock &BasicBlock::Starg(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Starg,data));
        return *this;
    }

    BasicBlock &BasicBlock::Store(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Store,data));
        return *this;
    }

    BasicBlock &BasicBlock::Load(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Load,data));
        return *this;
    }

    BasicBlock &BasicBlock::Ldarga() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Op::Ldarga));
        return *this;
    }

    BasicBlock &BasicBlock::Ldloc(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Ldloc,data));
        return *this;
    }

    BasicBlock &BasicBlock::Ldloca() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Op::Ldloca));
        return *this;
    }

    BasicBlock &BasicBlock::Stloc(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Stloc,data));
        return *this;
    }

    BasicBlock &BasicBlock::Ldfld(DataType data, TokenRef *fld) {
        insts.push_back(new InstWithDataToken(getDocument(),InstWithDataToken::Op::Ldfld,data,fld));
        return *this;
    }

    BasicBlock &BasicBlock::Ldsfld(DataType data, TokenRef *sfld) {
        insts.push_back(new InstWithDataToken(getDocument(),InstWithDataToken::Op::Ldsfld,data,sfld));
        return *this;
    }

    BasicBlock &BasicBlock::Ldflda(TokenRef *fld) {
        insts.push_back(new InstWithToken(getDocument(),InstWithToken::Op::Ldflda,fld));
        return *this;
    }

    BasicBlock &BasicBlock::Ldsflda(TokenRef *sfld) {
        insts.push_back(new InstWithToken(getDocument(),InstWithToken::Op::Ldflda,sfld));
        return *this;
    }

    BasicBlock &BasicBlock::Stfld(DataType data, TokenRef *fld) {
        insts.push_back(new InstWithDataToken(getDocument(),InstWithDataToken::Op::Stfld,data,fld));
        return *this;
    }

    BasicBlock &BasicBlock::Stsfld(DataType data, TokenRef *sfld) {
        insts.push_back(new InstWithDataToken(getDocument(),InstWithDataToken::Op::Stsfld,data,sfld));
        return *this;
    }

    BasicBlock &BasicBlock::Ldelem(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Ldelem,data));
        return *this;
    }

    BasicBlock &BasicBlock::Ldelema(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Ldelema,data));
        return *this;
    }

    BasicBlock &BasicBlock::Stelem(DataType data) {
        insts.push_back(new InstWithData(getDocument(),InstWithData::Op::Stelem,data));
        return *this;
    }

    BasicBlock &BasicBlock::Ldnull() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Op::Ldnull));
        return *this;
    }

    BasicBlock &BasicBlock::Newobj(TokenRef *cls) {
        insts.push_back(new InstWithToken(getDocument(),InstWithToken::Op::Newobj,cls));
        return *this;
    }

    BasicBlock &BasicBlock::Castcls(TokenRef *src, TokenRef *dst) {
        insts.push_back(new InstCastcls(getDocument(),src,dst));
        return *this;
    }

    BasicBlock &BasicBlock::Conv(DataType src, DataType dst) {
        insts.push_back(new InstConv(getDocument(),src,dst));
        return *this;
    }

    BasicBlock &BasicBlock::Callvirt() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::CallVirt));
        return *this;
    }

    BasicBlock &BasicBlock::Invoke(TokenRef *external) {
        insts.push_back(new InstWithToken(getDocument(),InstWithToken::Invoke,external));
        return *this;
    }

    BasicBlock &BasicBlock::Intrinsic(vm::IntrinsicEnum index) {
        insts.push_back(new InstIntrinsic(getDocument(),index));
        return *this;
    }

    BasicBlock &BasicBlock::Callstatic() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Callstatic));
        return *this;
    }

    BasicBlock &BasicBlock::Call() {
        insts.push_back(new InstWithOp(getDocument(),InstWithOp::Call));
        return *this;
    }

    Inst::ByteSize BasicBlock::getAddress() {
        return address;
    }

    void BasicBlock::setAddress(Inst::ByteSize value) {
        address = value;
    }

    Inst::ByteSize BasicBlock::getByteSize(){
        if(size == -1){
            size = 0;
            for(auto inst : insts){
                size += inst->getByteSize();
            }
        }
        return size;
    }

    std::string Opt::toString() {
        return Format() << "optional " << Param::toString();
    }

    std::string Inf::toString() {
        return Format() << "ParamArray " << Param::toString();
    }

    Inf::Inf(Document *document, TokenRef *name, TokenRef *type, bool ref)
        : Param(document,name,type,ref,Bytecode::InfDef){}

    Inf::Inf(Document *document, istream &stream)
        : Param(document,Bytecode::InfDef,stream){}


    std::string Result::toString() {
        return Format() << "result " << type->toString();
    }



    void generateAddressInfo(list<BasicBlock*> blocks){
        Inst::ByteSize address = 0;
        for(auto block : blocks){
            block->setAddress(address);
            address += block->getByteSize();
        }
    }

    std::string Ftn::toString() {
        return FunctionDefine::toString() + " ftn";
    }


    std::string VFtn::toString() {
        return FunctionDefine::toString() + " vftn";
    }

    VFtn::VFtn(Document *document,AccessFlag access,TokenRef *name,std::list<Param*> params,Result *result,std::list<Local*> locals,std::list<BasicBlock*> blocks)
           : FunctionDefine(document,Bytecode::VFtnDef,access,name,std::move(params),result,std::move(locals),std::move(blocks)){}

    VFtn::VFtn(Document *document, istream &stream) : FunctionDefine(document,Bytecode::VFtnDef,stream){}

    type::Symbol *VFtn::prepareSymbol() {
        if(!symbol){
            symbol = new type::UserFunction(FunctionFlag::Virtual);
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
        }
        return symbol;
    }

    void VFtn::fillSymbolDetail(CompileTimeContext *context) {
        fillParameterList(context,symbol);
    }

    std::string SFtn::toString() {
        return FunctionDefine::toString() + " sftn";
    }

    SFtn::SFtn(Document *document,AccessFlag access,TokenRef *name,std::list<Param*> params,Result *result,std::list<Local*> locals,std::list<BasicBlock*> blocks)
        : FunctionDefine(document,Bytecode::SFtnDef,access,name,std::move(params),result,std::move(locals),std::move(blocks)){}

    SFtn::SFtn(Document *document, istream &stream) : FunctionDefine(document,Bytecode::SFtnDef,stream){}

    type::Symbol *SFtn::prepareSymbol() {
        if(!symbol){
            symbol = new type::UserFunction(FunctionFlag::Static);
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
        }
        return symbol;
    }

    void SFtn::fillSymbolDetail(CompileTimeContext *context) {
        fillParameterList(context,symbol);
    }

    std::string Ext::toString() {
        return FunctionDeclare::toString() + " ext";
    }

    std::string InstWithOp::toString() {
        vector<string> str = {"Nop","Ret","CallVirt","CallExt","Callstatic","Call",
                              "Ldnull","And","Or","Xor","Ldloca","Ldarga","Not"};
        Format fmt;
        fmt << str[(int)op];
        return fmt;
    }

    std::string InstWithToken::toString() {
        vector<string> str = {"Ldftn","Ldsftn","Ldvftn","Ldc","Newobj","Invoke","Ldflda","Ldsflda"};
        Format fmt;
        fmt << str[(int)op] << ' ' << token->toString();
        return fmt;
    }

    std::string InstJif::toString() {
        return Format() << "Jif " << target->getAddress();
    }

    std::string InstBr::toString() {
        return Format() << "Br " << target->getAddress();
    }


    vector<string> ty = {
            "empty","i8","i16","i32","i64","u8","u16","u32","u64","f32","f64","array","record",
            "ref","ftn","vftn","sftn","boolean","character","delegate"
    };

    std::string InstPush::toString() {
        Format fmt;
        fmt << "Push." << ty[(int)type.getKind()] << ' ';
        switch(type.getKind()){
            case DataTypeEnum::i8:
                fmt << to_string(any_cast<data::i8>(value));
                break;
            case DataTypeEnum::i16:
                fmt << to_string(any_cast<data::i16>(value));
                break;
            case DataTypeEnum::i32:
                fmt << to_string(any_cast<data::i32>(value));
                break;
            case DataTypeEnum::i64:
                fmt << to_string(any_cast<data::i64>(value));
                break;
            case DataTypeEnum::u8:
                fmt << to_string(any_cast<data::u8>(value));
                break;
            case DataTypeEnum::u16:
                fmt << to_string(any_cast<data::u16>(value));
                break;
            case DataTypeEnum::u32:
                fmt << to_string(any_cast<data::u32>(value));
                break;
            case DataTypeEnum::u64:
                fmt << to_string(any_cast<data::u64>(value));
                break;
            case DataTypeEnum::f32:
                fmt << to_string(any_cast<data::f32>(value));
                break;
            case DataTypeEnum::f64:
                fmt << to_string(any_cast<data::f64>(value));
                break;
            case DataTypeEnum::boolean:
                fmt << to_string(any_cast<data::boolean>(value));
                break;
            default: PANIC;
        }
        return fmt;
    }

    std::string InstWithData::toString() {
        vector<string> inst = {"Ldarg","Starg","Load","Store","Ldloc","Stloc",
                               "Add","Sub","Mul","Div","FDiv","EQ","NE","LT","GT","LE","GE","Neg","Pop","Dup",
                               "Ldelem","Stelem","Ldelema"};
        Format fmt;
        fmt << inst[(int)op] << '.' << ty[(int)type.getKind()];
        switch (type.getKind()) {
            case DataTypeEnum::record:
            case DataTypeEnum::array:
                fmt << ' ' << type.getToken()->toString();
                break;
        }
        return fmt;
    }

    std::string InstWithDataToken::toString() {
        vector<string> inst = {"Ldfld","Ldsfld","Stfld","Stsfld"};
        Format fmt;
        fmt << inst[(int)op] << '.' << ty[(int)type.getKind()] << ' ' << token->toString();
        switch (type.getKind()) {
            case DataTypeEnum::record:
            case DataTypeEnum::array:
                fmt << ' ' << type.getToken()->toString();
                break;
        }
        return fmt;
    }

    std::string InstCastcls::toString() {
        return Format() << "cast." << src_class->toString() << " " << dst_class->toString();
    }

    std::string InstConv::toString() {
        return Format() << "conv." << ty[(int)src.getKind()] << " " << ty[(int)dst.getKind()];
    }

    void printILInfo(DebugInfo *info,string indent,ostream &stream){
        stream << indent << info->text;
        if(!info->childs.empty()){
            stream << "{\n";
            for(auto child : info->childs){
                printILInfo(child,indent + "    ",stream);
                stream << '\n';
            }
            stream << indent << '}';
        }
    }

    std::string Document::toString() {
        Format fmt;
        fmt << "\nToken:\n";
        for(auto token : token_pool){
            fmt << '\t' << token->toString() << '\n';
        }
        fmt << "\nimport:\n";
        for(auto pkg : dependencies_token){
            fmt << '\t' << pkg->toString() << '\n';
        }
        fmt << "\ncode:\n";
        for(auto member : members){
            auto tmp = member->toStructuredInfo();
            printILInfo(tmp,"\t",fmt.stream);
            fmt << '\n';
            delete tmp;
        }
        return fmt;
    }

    void Document::add(Member *member) {
        members.push_back(member);
    }

    Inst::ByteSize InstWithOp::getByteSize() {
        return 1;
    }

    Inst::ByteSize InstWithToken::getByteSize() {
        return 1 + 1 + sizeof(TokenRef::ID);
    }

    Inst::ByteSize InstJif::getByteSize() {
        return 1 + sizeof(ByteSize);
    }

    Inst::ByteSize InstBr::getByteSize() {
        return 1 + sizeof(ByteSize);
    }

    Inst::ByteSize InstPush::getByteSize() {
        using enum DataTypeEnum;
        switch(type.getKind()){
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
        switch (type.getKind()) {
            case DataTypeEnum::record:
            case DataTypeEnum::array:
                return sizeof(TokenRef*) + 2;
            default:
                return 2;
        }
    }

    Inst::ByteSize InstWithDataToken::getByteSize() {
        return 2 + (1 + sizeof(TokenRef::ID));
    }

    Inst::ByteSize InstCastcls::getByteSize() {
        return 1 + 2 * (1 + sizeof(TokenRef::ID));
    }

    Inst::ByteSize InstConv::getByteSize() {
        return 3;
    }







    void BasicBlock::toHex(std::ostream &stream) {
        for(auto inst : insts)inst->toHex(stream);
    }

    BasicBlock::BasicBlock(Document *document) : Node(document){}

    DebugInfo *BasicBlock::toStructuredInfo() {
        auto ret = Node::toStructuredInfo();
        for(auto inst : insts){
            ret->add(inst->toString());
        }
        return ret;
    }

    std::string BasicBlock::toString() {
        return to_string(getAddress());
    }

    TokenRef::TokenRef(Document *document, data::u64 id) : Node(document,Bytecode::TokenRef),id(id){}

    TokenRef::TokenRef(Document *document, istream &stream) : Node(document,Bytecode::TokenRef,stream) {
        read(stream,id);
    }

    void TokenRef::toHex(std::ostream &stream) {
        Node::toHex(stream);
        write(stream,id);
    }

    bool TokenRef::isEmpty() {
        return id == -1 || getDocument() == nullptr;
    }

    std::string TokenRef::toString() {
        return getDocument()->findTokenDef(id)->getName();
    }

    data::u64 TokenRef::getID() {
        return id;
    }

    TokenDef *TokenRef::getDef() {
        return getDocument()->findTokenDef(id);
    }


    Member::Member(Document *document,Bytecode begin_mark,AccessFlag access,TokenRef *name)
        : Node(document,begin_mark),access(access),name(name){}

    Member::Member(Document *document,Bytecode begin_mark,std::istream &stream)
        : Node(document,begin_mark,stream){
        if(predict(stream,Bytecode::PubAcsDef)){
            read(stream,Bytecode::PubAcsDef);
            access = AccessFlag::Public;
        }
        else if(predict(stream,Bytecode::PriAcsDef)){
            read(stream,Bytecode::PriAcsDef);
            access = AccessFlag::Private;
        }
        else if(predict(stream,Bytecode::PtdAcsDef)){
            read(stream,Bytecode::PtdAcsDef);
            access = AccessFlag::Protected;
        }
        else PANIC;

        name = new TokenRef(document,stream);
    }

    void Member::toHex(ostream &stream) {
        Node::toHex(stream);
        switch(access){
            case AccessFlag::Public:
                write(stream,Bytecode::PubAcsDef);
                break;
            case AccessFlag::Private:
                write(stream,Bytecode::PriAcsDef);
                break;
            case AccessFlag::Protected:
                write(stream,Bytecode::PtdAcsDef);
                break;
        }
        name->toHex(stream);
    }

    std::string Member::toString() {
        Format fmt;
        fmt<<name->toString()<<" : ";
        switch(access){
            case AccessFlag::Public:
                fmt<<"public";
                break;
            case AccessFlag::Private:
                fmt<<"private";
                break;
            case AccessFlag::Protected:
                fmt<<"protected";
                break;
        }
        return fmt;
    }

    AccessFlag Member::getAccessFlag() {
        return access;
    }

    TokenRef *Member::getNameToken() {
        return name;
    }

    Member::~Member() {
        delete name;
    }

    Class::Class(Document *document, AccessFlag access, TokenRef *name, TokenRef *extend, std::list<TokenRef*> impl, std::list<Member*> members)
        : Scope(document,Bytecode::ClassDef,access,name,std::move(members)),extend_class(extend),impl_interface_list(std::move(impl)){}

    Class::Class(Document *document, istream &stream) : Scope(document,Bytecode::ClassDef,stream) {
        extend_class = new TokenRef(document,stream);
        while(predict(stream,Bytecode::ImplDef)){
            impl_interface_list.push_back(new TokenRef(document,stream));
        }
        Scope::loadMembersFromStream(document,stream);
        read(stream,Bytecode::EndMark);
    }

    void Class::toHex(ostream &stream) {
        Member::toHex(stream);
        extend_class->toHex(stream);
        for(auto impl:impl_interface_list){
            write(stream,Bytecode::ImplDef);
            impl->toHex(stream);
        }
        for(auto member:getMembers()){
            member->toHex(stream);
        }
        write(stream,Bytecode::EndMark);
    }

    DebugInfo *Class::toStructuredInfo() {
        list<DebugInfo*> info;
        info.push_back(new DebugInfo{Format()<<"extend "<<extend_class->toString()});
        if(!impl_interface_list.empty()){
            auto impl_info = new DebugInfo{"impl"};
            for(auto interface:impl_interface_list){
                impl_info->childs.push_back(new DebugInfo{interface->toString()});
            }
        }
        for(auto member : getMembers())info.push_back(member->toStructuredInfo());
        return new DebugInfo{toString(),info};
    }

    std::string Class::toString() {
        return Member::toString() + " class";
    }

    type::Symbol *Class::prepareSymbol() {
        if(!symbol) {
            symbol = new type::Class;
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
        }
        return symbol;
    }

    void Class::fillSymbolDetail(CompileTimeContext *context) {
        auto extend_symbol = context->findSymbol(extend_class->getDef()->getFullName());
        symbol->setExtend(extend_symbol->as<type::Class*>());
        for(auto impl : impl_interface_list){
            auto impl_symbol = context->findSymbol(impl->getDef()->getFullName());
            symbol->addImpl(impl_symbol->as<type::Interface*>());
        }
        for(auto member : getMembers()){
            auto member_symbol = member->prepareSymbol();
            member->fillSymbolDetail(nullptr);
            symbol->add(member_symbol);
        }
    }

    Class::~Class() {
        delete extend_class;
        for(auto impl : impl_interface_list) delete impl;
        for(auto member : getMembers()) delete member;
    }

    TokenRef *Class::getExtendClassToken() {
        return extend_class;
    }

    void Module::toHex(std::ostream &stream) {
        Member::toHex(stream);
        for(auto member : getMembers())member->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    Module::Module(Document *document, AccessFlag access, TokenRef *name, std::list<Member*> members)
        : Scope(document,Bytecode::ModuleDef,access,name,std::move(members)){}

    Module::Module(Document *document, std::istream &stream) : Scope(document,Bytecode::ModuleDef,stream){
        loadMembersFromStream(document,stream);
        read(stream,Bytecode::EndMark);
    }

    DebugInfo *Module::toStructuredInfo() {
        list<DebugInfo*> info;
        for(auto member : getMembers())info.push_back(member->toStructuredInfo());
        return new DebugInfo{toString(),info};
    }

    std::string Module::toString() {
        return Member::toString() + " module";
    }

    Module::~Module() {
        for(auto member : getMembers()) delete member;
    }

    type::Symbol *Module::prepareSymbol() {
        if(!symbol){
            symbol = new type::Module;
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
        }
        return symbol;
    }

    void Module::fillSymbolDetail(CompileTimeContext *context) {
        for(auto member : getMembers()){
            auto member_symbol = member->prepareSymbol();
            member->fillSymbolDetail(nullptr);
            symbol->add(member_symbol);
        }
    }

    void Interface::toHex(std::ostream &stream) {
        write(stream,Bytecode::InterfaceDef);
        Member::toHex(stream);
        for(auto member : getMembers())member->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    Interface::Interface(Document *document, AccessFlag access, TokenRef *name, std::list<InterfaceFunction*> functions)
        : Scope(document,Bytecode::InterfaceDef,access,name, convertToMemberList(functions)){}

    DebugInfo *Interface::toStructuredInfo() {
        list<DebugInfo*> info;
        for(auto ftn : getMembers())info.push_back(ftn->toStructuredInfo());
        return new DebugInfo{toString(),info};
    }

    Interface::Interface(Document *document, istream &stream) : Scope(document,Bytecode::InterfaceDef,stream){
        loadMembersFromStream(document,stream);
        read(stream,Bytecode::EndMark);
    }

    std::string Interface::toString() {
        return Member::toString() + " interface";
    }

    type::Symbol *Interface::prepareSymbol() {
        if(!symbol){
            symbol = new type::Interface;
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
        }
        return symbol;
    }

    void Interface::fillSymbolDetail(CompileTimeContext *context) {
        for(auto ftn : getMembers()){
            auto ftn_symbol = ftn->prepareSymbol();
            ftn->fillSymbolDetail(nullptr);
            symbol->add(ftn_symbol);
        }
    }

    Interface::~Interface() {
        for(auto ftn : getMembers()) delete ftn;
    }

    std::list<Member *> Interface::convertToMemberList(std::list<InterfaceFunction*> &ls) {
        list<Member*> ret;
        for(auto ele : ls)ret.push_back((Member*)ele);
        return ret;
    }


    void Enum::toHex(std::ostream &stream) {
        write(stream,Bytecode::EnumDef);
        Member::toHex(stream);
        for(auto &p : enums){
            p.first->toHex(stream);
            write(stream,p.second);
        }
        write(stream,Bytecode::EndMark);
    }

    Enum::Enum(Document *document, AccessFlag access, TokenRef *name, list<Pair> enums)
        : Member(document,Bytecode::EnumDef,access,name),enums(std::move(enums)){}

    Enum::Enum(Document *document, istream &stream)
        : Member(document,Bytecode::EnumDef,stream){
        while(predict(stream,Bytecode::PairDef)){
            read(stream,Bytecode::PairDef);
            Pair p;
            p.first = new TokenRef(document,stream);
            read(stream,p.second);
            enums.push_back(p);
        }
        read(stream,Bytecode::EndMark);
    }

    DebugInfo *Enum::toStructuredInfo() {
        list<DebugInfo*> info;
        for(auto pair : enums){
            info.push_back(new DebugInfo{pair.first->toString() + '=' + to_string(pair.second)});
        }
        return new DebugInfo{toString(),info};
    }

    std::string Enum::toString() {
        return Member::toString() + " enum";
    }

    type::Symbol *Enum::prepareSymbol() {
        if(!symbol){
            symbol = new type::Enumeration;
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
        }
        return symbol;
    }

    void Enum::fillSymbolDetail(CompileTimeContext *context) {
        for(auto pair : enums){
            auto enum_member = new type::EnumMember(pair.second);
            enum_member->setName(pair.first->toString());
            symbol->add(enum_member);
        }
    }

    Enum::~Enum() {
        for(auto pair : enums) delete pair.first;
    }

    Fld::Fld(Document *document, AccessFlag access, TokenRef *name, TokenRef *type)
        : Member(document,Bytecode::FldDef,access,name),type(type){}

    Fld::Fld(Document *document, istream &stream)
        : Member(document,Bytecode::FldDef,stream){
        type = new TokenRef(document,stream);
    }

    void Fld::toHex(std::ostream &stream) {
        Member::toHex(stream);
        type->toHex(stream);
    }

    DebugInfo *Fld::toStructuredInfo() {
        return new DebugInfo{toString()};
    }

    std::string Fld::toString() {
        return Member::toString() + " fld(" + type->toString() + ")";
    }

    type::Symbol *Fld::prepareSymbol() {
        if(!symbol){
            symbol = new type::Variable;
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
            symbol->setStatic(false);
        }
        return symbol;
    }

    void Fld::fillSymbolDetail(CompileTimeContext *context) {
        auto type_symbol = context->findSymbol(type->getDef()->getFullName());
        symbol->setPrototype(type_symbol->as<type::Prototype*>());
    }

    Fld::~Fld() {
        delete type;
    }


    SFld::SFld(Document *document, AccessFlag access, TokenRef *name, TokenRef *type)
        : Member(document,Bytecode::SFldDef,access,name),type(type){}

    SFld::SFld(Document *document, istream &stream)
        : Member(document,Bytecode::SFldDef,stream){
        type = new TokenRef(document,stream);
    }

    void SFld::toHex(std::ostream &stream) {
        Member::toHex(stream);
        type->toHex(stream);
    }

    DebugInfo *SFld::toStructuredInfo() {
        return new DebugInfo{toString()};
    }

    std::string SFld::toString() {
        return Member::toString() + " sfld(" + type->toString() + ")";
    }

    type::Symbol *SFld::prepareSymbol() {
        if(!symbol){
            symbol = new type::Variable;
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
            symbol->setStatic(true);
        }
        return symbol;
    }

    void SFld::fillSymbolDetail(CompileTimeContext *context) {
        auto type_symbol = context->findSymbol(type->getDef()->getFullName());
        symbol->setPrototype(type_symbol->as<type::Prototype*>());
    }

    SFld::~SFld() {
        delete type;
    }
	
    void Record::toHex(std::ostream &stream) {
        write(stream,Bytecode::RecordDef);
        Member::toHex(stream);
        for(auto member : getMembers())member->toHex(stream);
        write(stream,Bytecode::EndMark);
    }
	
	list<Member*> Record::convertFldToMembers(list<Fld*> &fields){
		list<Member*> ret;
		for(auto ele : fields)ret.push_back(ele);
		return ret;	
	}

    Record::Record(Document *document, AccessFlag access, TokenRef *name, std::list<Fld*> fields)
        : Scope(document,Bytecode::RecordDef,access,name,convertFldToMembers(fields)){}

    Record::Record(Document *document, istream &stream)
        : Scope(document,Bytecode::RecordDef,stream){
		loadMembersFromStream(document,stream);
		read(stream,Bytecode::EndMark);
    }

    DebugInfo *Record::toStructuredInfo() {
        list<DebugInfo*> info;
        for(auto field : getMembers()){
            info.push_back(field->toStructuredInfo());
        }
        return new DebugInfo{toString(),info};
    }

    std::string Record::toString() {
        return Member::toString() + " record";
    }

    type::Symbol *Record::prepareSymbol() {
        if(!symbol){
            symbol = new type::Record;
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
        }
        return symbol;
    }

    void Record::fillSymbolDetail(CompileTimeContext *context) {
        for(auto field : getMembers()){
            auto field_symbol = field->prepareSymbol();
            field->fillSymbolDetail(nullptr);
            symbol->add(field_symbol);
        }
    }

    Record::~Record() {
        for(auto field : getMembers()) delete field;
    }

    Param::Param(Document *document, TokenRef *name, TokenRef *type, bool ref, Bytecode begin_mark)
        : Node(document,begin_mark),name(name),type(type),is_ref(ref){}

    Param::Param(Document *document, Bytecode begin_mark, istream &stream)
        : Node(document, begin_mark, stream) {
        name = new TokenRef(document,stream);
        type = new TokenRef(document,stream);
        if(predict(stream,Bytecode::Byref)){
            read(stream,Bytecode::Byref);
            is_ref = true;
        }
        else{
            read(stream,Bytecode::Byval);
            is_ref = false;
        }
    }

    void Param::toHex(ostream &stream) {
        Node::toHex(stream);
        name->toHex(stream);
        type->toHex(stream);
        write(stream,(is_ref ? Bytecode::Byref : Bytecode::Byval));
    }

    std::string Param::toString() {
        return name->toString() + ':' + type->toString();
    }

    TokenRef *Param::getTypeToken() {
        return type;
    }

    bool Param::isRef() {
        return is_ref;
    }

    TokenRef *Param::getNameToken() {
        return name;
    }

    Regular::Regular(Document *document, TokenRef *name, TokenRef *type, bool ref)
        : Param(document,name,type,ref,Bytecode::RegDef) {}

    Regular::Regular(Document *document, istream &stream)
        : Param(document,Bytecode::RegDef,stream){}



    void Opt::toHex(std::ostream &stream) {
        Param::toHex(stream);
        write(stream,Bytecode::InstBeg);
        write(stream,initial_memory_size);
        initial->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    Opt::Opt(Document *document, TokenRef *name, TokenRef *type, bool ref, BasicBlock *initial)
        : Param(document,name,type,ref,Bytecode::OptDef),initial(initial){
        initial->setAddress(0);
        initial_memory_size = initial->getByteSize();
    }

    Opt::Opt(Document *document, istream &stream)
        : Param(document,Bytecode::OptDef,stream){
        read(stream,Bytecode::InstBeg);
        read(stream,initial_memory_size);
        initial_memory = (data::u8*)malloc(initial_memory_size);
        read(stream,initial_memory,initial_memory_size);
        read(stream,Bytecode::EndMark);
    }

    DebugInfo *Opt::toStructuredInfo() {
        auto ret = Param::toStructuredInfo();
        if(initial)ret->add(initial->toStructuredInfo());
        return ret;
    }

    Opt::~Opt() {
        delete initial_memory;
    }

    Local::Local(Document *document, TokenRef *name, TokenRef *type, Local::ID address)
        : Node(document,Bytecode::LocalDef),name(name),type(type),address(address){}

    Local::Local(Document *document, istream &stream, ID address)
            : Node(document,Bytecode::LocalDef,stream),address(address) {
        name = new TokenRef(document,stream);
        type = new TokenRef(document,stream);
    }

    void Local::toHex(std::ostream &stream) {
        Node::toHex(stream);
        name->toHex(stream);
        type->toHex(stream);
    }

    TokenRef *Local::getTypeToken() {
        return type;
    }

    Result::Result(Document *document, TokenRef *type)
        : Node(document,Bytecode::ResultDef),type(type){}

    Result::Result(Document *document, istream &stream)
        : Node(document,Bytecode::ResultDef,stream){
        type = new TokenRef(document,stream);
    }

    void Result::toHex(std::ostream &stream) {
        Node::toHex(stream);
        type->toHex(stream);
    }

    TokenRef *Result::getTypeToken() {
        return type;
    }


    FunctionDeclare::FunctionDeclare(Document *document,Bytecode begin_mark,AccessFlag access,TokenRef *name,std::list<Param*> params,Result *result)
        : Member(document,begin_mark,access,name),params(std::move(params)),result(result){}

    FunctionDeclare::FunctionDeclare(Document *document, Bytecode begin_mark, istream &stream)
            : Member(document, begin_mark, stream) {
        while(true){
            if(predict(stream,Bytecode::RegDef)){
                params.push_back(new Regular(document,stream));
            }
            else if(predict(stream,Bytecode::OptDef)){
                params.push_back(new Opt(document,stream));
            }
            else if(predict(stream,Bytecode::InfDef)){
                params.push_back(new Inf(document,stream));
            }
            else{
                break;
            }
        }
        if(predict(stream,Bytecode::ResultDef)){
            result = new Result(document,stream);
        }
    }

    void FunctionDeclare::toHex(ostream &stream) {
        Member::toHex(stream);
        for(auto p : params) p->toHex(stream);
        if(result)result->toHex(stream);
    }

    DebugInfo *FunctionDeclare::toStructuredInfo() {
        auto ret = new DebugInfo{toString()};
        auto param_info = new DebugInfo{"Parameters"};
        for(auto param : params){
            param_info->add(param->toStructuredInfo());
        }
        ret->add(param_info);
        if(result){
            ret->add(result->toString());
        }
        return ret;
    }

    void FunctionDeclare::fillParameterList(CompileTimeContext *context, type::Function *symbol) {
        for(auto param : params){
            auto name = param->getNameToken()->toString();
            auto type_symbol = context->findSymbol(param->getTypeToken()->getDef()->getFullName());
            auto prototype = type_symbol->as<type::Prototype*>();
            type::Parameter *parameter;
            switch (param->getKind()) {
                case ParamKind::Regular:
                    parameter = new type::Parameter(name,prototype,!param->isRef(),false,false);
                    break;
                case ParamKind::Opt:
                    parameter = new type::Parameter(name,prototype,!param->isRef(),true,false);
                    break;
                case ParamKind::Inf:
                    parameter = new type::Parameter(name,prototype,!param->isRef(),false,true);
                    break;
            }
            symbol->add(parameter);
        }

        if(result){
            auto ret_symbol = context->findSymbol(result->getTypeToken()->getDef()->getFullName());
            symbol->setRetSignature(ret_symbol->as<type::Prototype*>());
        }
    }

    const std::list<Param *> &FunctionDeclare::getParams() {
        return params;
    }

    FunctionDefine::FunctionDefine(Document *document, Bytecode begin_mark, AccessFlag access, TokenRef *name,
                                   std::list<Param*> params, Result *result, std::list<Local*> locals, std::list<BasicBlock*> blocks)
        : FunctionDeclare(document, begin_mark, access, name, std::move(params), result), locals(locals), blocks(std::move(blocks)){
        for(auto block : this->blocks){
            block->setAddress(block_byte_length);
            block_byte_length += block->getByteSize();
        }
    }

    FunctionDefine::FunctionDefine(Document *document, Bytecode begin_mark, istream &stream)
        : FunctionDeclare(document, begin_mark, stream) {
        Local::ID id = 0;
        while(predict(stream,Bytecode::LocalDef)){
            locals.push_back(new Local(document,stream,id));
            id++;
        }
        read(stream,Bytecode::InstBeg);
        read(stream,block_byte_length);
        blocks_memory = (data::Byte*)malloc(block_byte_length);
        read(stream,blocks_memory,block_byte_length);
        read(stream,Bytecode::EndMark);
    }

    void FunctionDefine::toHex(ostream &stream) {
        FunctionDeclare::toHex(stream);
        for(auto local : locals) local->toHex(stream);
        write(stream,Bytecode::InstBeg);
        write(stream,block_byte_length);
        for(auto block : blocks) block->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    data::Byte *FunctionDefine::getBlocksMemory() {
        return blocks_memory;
    }

    FunctionDefine::~FunctionDefine() {
        delete blocks_memory;
    }

    DebugInfo *FunctionDefine::toStructuredInfo() {
        auto ret = FunctionDeclare::toStructuredInfo();
        auto local_info = new DebugInfo{"locals"};
        for(auto local : locals){
            local_info->add(local->toString());
        }
        ret->add(local_info);
        auto block_info = new DebugInfo{"blocks"};
        for(auto block : blocks){
            block_info->add(block->toStructuredInfo());
        }
        ret->add(block_info);
        return ret;
    }

    std::list<Local*> &FunctionDefine::getLocals() {
        return locals;
    }


    Ctor::Ctor(Document *document,std::list<Param*> params, std::list<Local*> locals,std::list<BasicBlock*> blocks)
            : FunctionDefine(document,Bytecode::CtorDef,AccessFlag::Public,document->getTokenRef("#ctor"),std::move(params),nullptr,std::move(locals),std::move(blocks)){}

    Ctor::Ctor(Document *document, istream &stream) : FunctionDefine(document,Bytecode::CtorDef,stream){}

    std::string Ctor::toString() {
        return Member::toString() + " ctor";
    }

    type::Symbol *Ctor::prepareSymbol() {
        if(!symbol){
            symbol = new type::Constructor;
        }
        return symbol;
    }

    void Ctor::fillSymbolDetail(CompileTimeContext *context) {
        fillParameterList(context,symbol);
    }

    Ftn::Ftn(Document *document,AccessFlag access,TokenRef *name,std::list<Param*> params,Result *result,std::list<Local*> locals,std::list<BasicBlock*> blocks)
        : FunctionDefine(document,Bytecode::FtnDef,access,name,std::move(params),result,std::move(locals),std::move(blocks)){}

    Ftn::Ftn(Document *document, istream &stream) : FunctionDefine(document,Bytecode::FtnDef,stream) {}

    type::Symbol *Ftn::prepareSymbol() {
        if(!symbol){
            symbol = new type::UserFunction(FunctionFlag::Method);
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
        }
        return symbol;
    }

    void Ftn::fillSymbolDetail(CompileTimeContext *context) {
        fillParameterList(context,symbol);
    }

    Ext::Ext(Document *document,AccessFlag access,TokenRef *name,
             TokenRef *library,TokenRef *alias,std::list<Param *> params,Result *result)
         : FunctionDeclare(document,Bytecode::ExtDef,access,name,std::move(params),result),lib(library),alias(alias){}

    Ext::Ext(Document *document, istream &stream) : FunctionDeclare(document,Bytecode::ExtDef,stream){
        lib = new TokenRef(document,stream);
        alias = new TokenRef(document,stream);
    }

    void Ext::toHex(std::ostream &stream) {
        FunctionDeclare::toHex(stream);
        lib->toHex(stream);
        alias->toHex(stream);
    }

    type::Symbol *Ext::prepareSymbol() {
        if(!symbol){
            symbol = new type::ExternalFunction(lib->toString(),alias->toString());
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(getAccessFlag());
        }
        return symbol;
    }

    void Ext::fillSymbolDetail(CompileTimeContext *context) {
        fillParameterList(context,symbol);
    }

    Ext::~Ext() {
        delete lib;
        delete alias;
    }

    TokenRef *Ext::getLibraryToken() {
        return lib;
    }

    TokenRef *Ext::getAliasToken() {
        return alias;
    }


    TokenRef *Document::getTokenRef(data::u64 id) {
        ASSERT(id > token_pool.size(),"invalid token id");
        return new TokenRef(this, id);
    }

    TokenDef *Document::findTokenDef(data::u64 id) {
        ASSERT(id > token_pool.size(),"invalid token id");
        return token_pool[id];
    }

    void Document::addResource(Node* node) {
        resources.push_back(node);
    }

    TokenDef *Document::findTokenDef(std::string text) {
        auto target = token_pool_map.find(text);
        if(target == token_pool_map.end())PANIC;
        return token_pool[target->second];
    }

    Document::Document(std::string package_name)
        : Node(nullptr,Bytecode::DocumentDef),name(package_name){}

    Document::Document(std::string package_name,istream &stream)
        : Node(nullptr,Bytecode::DocumentDef,stream),name(package_name){
        while(true){
            TokenDef *token;
            if(predict(stream,Bytecode::TextTokenDef)){
                token = new TextTokenDef(this,stream);
                token_pool.push_back(token);
            }
            else if(predict(stream,Bytecode::ConstructedDef)){
                token = new ConstructedTokenDef(this,stream);
                token_pool.push_back(token);
            }
            else{
                break;
            }
        }

        for(int i=0;i<token_pool.size();i++){
            token_pool_map.insert({token_pool[i]->getName(),i});
        }
        
        while(predict(stream,Bytecode::DependDef)){
            read(stream,Bytecode::DependDef);
            dependencies_token.push_back(new TokenRef(this, stream));
        }

        this->members = Scope::loadMembersFromStream(this,stream);
        read(stream,Bytecode::EndMark);
    }

    void Document::toHex(ostream &stream) {
        write(stream,Bytecode::DocumentDef);

        for(auto token : token_pool){
            token->toHex(stream);
        }

        for(auto library : dependencies_token){
            write(stream,Bytecode::DependDef);
            library->toHex(stream);
        }

        for(auto member : members)member->toHex(stream);
        write(stream,Bytecode::EndMark);
    }

    void Document::addDependenceLibrary(std::string name) {
        dependencies_token.push_back(getTokenRef(name));
    }

    void Document::pushSymbolsInto(CompileTimeContext *context) {
        for(auto member : members) {
            member->prepareSymbol();
        }

        for(auto member : members){
            context->getGlobal()->add(member->prepareSymbol());
        }
    }

    void Document::fillSymbolsDetail(CompileTimeContext *context) {
        for(auto member : members){
            member->fillSymbolDetail(context);
        }
    }

    std::list<std::string> Document::getDependenciesPath() {
        list<string> ret;
        for(auto token : dependencies_token){
            ret.push_back(token->getDef()->getName());
        }
        return ret;
    }

    std::string Document::getPackageName() {
        return name;
    }

    std::list<Member*> &Document::getMembers() {
        return members;
    }

    std::vector<TokenDef*> &Document::getTokens() {
        return token_pool;
    }

    SFtn *Document::getEntrance() {
        //todo
        PANIC;
    }


    Bytecode InstWithOp::opToBytecode(InstWithOp::Op op) {
        switch(op){
            case Nop:           return Bytecode::Nop;
            case Ret:           return Bytecode::Ret;
            case CallVirt:      return Bytecode::CallVirt;
            case CallExt:       return Bytecode::Invoke;
            case Callstatic:    return Bytecode::Callstatic;
            case Call:          return Bytecode::Call;
            case Ldnull:        return Bytecode::Ldnull;
            case And:           return Bytecode::And;
            case Or:            return Bytecode::Or;
            case Xor:           return Bytecode::Xor;
            case Ldloca:        return Bytecode::Ldloca;
            case Ldarga:        return Bytecode::Ldarga;
            case Not:           return Bytecode::Not;
        }
    }

    InstWithOp::InstWithOp(Document *document, InstWithOp::Op op)
        : Inst(document,opToBytecode(op)),op(op){}

    void InstWithOp::toHex(std::ostream &stream) {
        Bytecode code = opToBytecode(op);
        write(stream,code);
    }

    Bytecode InstWithToken::opToBytecode(Op op) {
        switch(op){
            case Ldftn:   return Bytecode::Ldftn;
            case Ldsftn:  return Bytecode::Ldsftn;
            case Ldvftn:  return Bytecode::Ldvftn;
            case Ldc:     return Bytecode::Ldc;
            case Newobj:  return Bytecode::Newobj;
            case Invoke:  return Bytecode::Invoke;
            case Ldflda:  return Bytecode::Ldflda;
            case Ldsflda: return Bytecode::Ldsflda;
        }
    }

    InstWithToken::InstWithToken(Document *document,InstWithToken::Op op, TokenRef *token)
            : Inst(document,opToBytecode(op)),op(op),token(token){}

    void InstWithToken::toHex(std::ostream &stream) {
        Bytecode code = opToBytecode(op);
        write(stream,code);
        token->toHex(stream);
    }


    void InstJif::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::Jif);
        write(stream,target->getAddress());
    }

    InstJif::InstJif(Document *document, BasicBlock *target)
        : Inst(document,Bytecode::Jif),target(target){}


    void InstBr::toHex(std::ostream &stream) {
        write(stream,(data::u8)Bytecode::Br);
        write(stream,target->getAddress());
    }

    InstBr::InstBr(Document *document, BasicBlock *target)
        : Inst(document,Bytecode::Br),target(target){}


    Bytecode ILDataTypeToByteCode(DataTypeEnum type){
        using enum DataTypeEnum;
        switch(type){
            case i8:      return Bytecode::i8;
            case i16:     return Bytecode::i16;
            case i32:     return Bytecode::i32;
            case i64:     return Bytecode::i64;
            case u8:      return Bytecode::u8;
            case u16:     return Bytecode::u16;
            case u32:     return Bytecode::u32;
            case u64:     return Bytecode::u64;
            case f32:     return Bytecode::f32;
            case f64:     return Bytecode::f64;
            case ref:     return Bytecode::u64;
            case boolean: return Bytecode::boolean;
            case array:   return Bytecode::array;
            case record:  return Bytecode::record;
            case empty:
            case character:
            case delegate:
            case ftn:
            case vftn:
            case sftn:
                PANIC;
        }
    }

    void InstPush::toHex(std::ostream &stream) {
        using enum DataTypeEnum;
        write(stream,Bytecode::Push);
        write(stream,ILDataTypeToByteCode(type.getKind()));

        switch (type.getKind()) {
            case DataTypeEnum::array:
            case DataTypeEnum::record:
                PANIC;
                break;
        }

        switch(type.getKind()){
            case boolean:
                write(stream,any_cast<data::boolean>(value));
                break;
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

    InstPush::InstPush(Document *document, DataType type, std::any value)
        : Inst(document,Bytecode::Push),type(type),value(value){}


    void InstWithData::toHex(std::ostream &stream) {
        Bytecode code = opToBytecode(op);
        write(stream,code);
        write(stream,ILDataTypeToByteCode(type.getKind()));
        switch (type.getKind()) {
            case DataTypeEnum::array:
            case DataTypeEnum::record:
                type.getToken()->toHex(stream);
                break;
        }
    }

    InstWithData::InstWithData(Document *document, InstWithData::Op op, DataType type)
        : Inst(document,opToBytecode(op)),type(type),op(op){}

    Bytecode InstWithData::opToBytecode(InstWithData::Op op) {
        switch(op){
            case Ldarg:     return Bytecode::Ldarg;
            case Load:      return Bytecode::Load;
            case Starg:     return Bytecode::Starg;
            case Store:     return Bytecode::Store;
            case Ldloc:     return Bytecode::Ldloc;     
            case Stloc:     return Bytecode::Stloc;     
            case Add:       return Bytecode::Add;       
            case Sub:       return Bytecode::Sub;       
            case Mul:       return Bytecode::Mul;       
            case Div:       return Bytecode::Div;       
            case FDiv:      return Bytecode::FDiv;      
            case EQ:        return Bytecode::EQ;        
            case NE:        return Bytecode::NE;        
            case LT:        return Bytecode::LT;        
            case GT:        return Bytecode::GT;        
            case LE:        return Bytecode::LE;        
            case GE:        return Bytecode::GE;        
            case Neg:       return Bytecode::Neg;       
            case Pop:       return Bytecode::Pop;       
            case Dup:       return Bytecode::Dup;
            case Ldelema:   return Bytecode::Ldelema;
            case Ldelem:    return Bytecode::Ldelem;
            case Stelem:    return Bytecode::Stelem;
            default: PANIC;
        }
    }


    Bytecode InstWithDataToken::opToBytecode(InstWithDataToken::Op op) {
        switch(op) {
            case Ldfld:     return Bytecode::Ldfld;
            case Ldsfld:    return Bytecode::Ldsfld;
            case Stfld:     return Bytecode::Stfld;
            case Stsfld:    return Bytecode::Stsfld;
        }
    }

    InstWithDataToken::InstWithDataToken(Document *document, InstWithDataToken::Op op, DataType type, TokenRef *token)
        : Inst(document,opToBytecode(op)),op(op),type(type),token(token){}

    void InstWithDataToken::toHex(std::ostream &stream) {
        Bytecode code = opToBytecode(op);
        write(stream,code);
        write(stream,ILDataTypeToByteCode(type.getKind()));
        switch (type.getKind()) {
            case DataTypeEnum::array:
            case DataTypeEnum::record:
                type.getToken()->toHex(stream);
                break;
        }
        token->toHex(stream);
    }

    void InstCastcls::toHex(std::ostream &stream) {
        write(stream,Bytecode::CastCls);
        src_class->toHex(stream);
        dst_class->toHex(stream);
    }

    InstCastcls::InstCastcls(Document *document, TokenRef *src_class, TokenRef *dst_class)
        : Inst(document,Bytecode::CastCls),src_class(src_class),dst_class(dst_class){}


    void InstConv::toHex(std::ostream &stream) {
        write(stream,Bytecode::Conv);
        write(stream,ILDataTypeToByteCode(src.getKind()));
        write(stream,ILDataTypeToByteCode(dst.getKind()));
    }

    InstConv::InstConv(Document *document, DataType src, DataType dst)
        : Inst(document,Bytecode::Conv),src(src),dst(dst){}


    Document *Node::getDocument(){
        return document;
    }

    Node::Node(Document *document,Bytecode begin_mark) : document(document),begin_mark(begin_mark){
        if(document)document->addResource(this);
    }

    Node::Node(Document *document,Bytecode begin_mark,istream &stream) : document(document),begin_mark(begin_mark){
        read(stream,begin_mark);
    }

    Node::Node(Document *document) : document(document){}

    void Node::toHex(ostream &stream) {
        write(stream,begin_mark);
    }

    DebugInfo *Node::toStructuredInfo() {
        return new DebugInfo{toString()};
    }

    std::string TextTokenDef::getName() {
        return text;
    }

    void TextTokenDef::toHex(ostream &stream) {
        TokenDef::toHex(stream);
        for(auto c : text){
            write(stream,c);
        }
        write(stream,Bytecode::EndMark);
    }

    TextTokenDef::TextTokenDef(Document *document, ID id, std::string text)
        : TokenDef(document,Bytecode::TextTokenDef,id),text(std::move(text)){}

    TextTokenDef::TextTokenDef(Document *document, istream &stream)
        : TokenDef(document,Bytecode::TextTokenDef,stream) {

        while(!predict(stream,Bytecode::EndMark)){
            char c;
            read(stream,c);
            text.push_back(c);
        }
        read(stream,Bytecode::EndMark);
    }

    string TextTokenDef::toString(){
        return Format()<<'#'<<to_string(id)<<' '<<text;
    }

    std::list<std::string> TextTokenDef::getFullName() {
        return {getName()};
    }

    std::string ConstructedTokenDef::getName() {
        stringstream fmt;
        for(auto &token : sub_token_list){
            if(&token != &sub_token_list.front()) fmt << '.';
            fmt << token->toString();
        }
        return fmt.str();
    }

    ConstructedTokenDef::ConstructedTokenDef(Document *document, TokenDef::ID id, std::list<TokenRef*> sub_tokens)
        : TokenDef(document,Bytecode::ConstructedDef,id), sub_token_list(std::move(sub_tokens)){}

    ConstructedTokenDef::ConstructedTokenDef(Document *document, istream &stream)
        : TokenDef(document,Bytecode::ConstructedDef,stream) {
        while(predict(stream,Bytecode::TokenRef)){
            sub_token_list.push_back(new TokenRef(document,stream));
        }
        read(stream,Bytecode::EndMark);
    }

    void ConstructedTokenDef::toHex(ostream &stream) {
        TokenDef::toHex(stream);
        for(auto sub_id : sub_token_list){
            write(stream,Bytecode::TokenRef);
            write(stream,sub_id->getID());
        }
        write(stream,Bytecode::EndMark);
    }

    std::string ConstructedTokenDef::toString(){
        return Format()<<'#'<<to_string(id)<<' '<<getName();
    }

    std::list<std::string> ConstructedTokenDef::getFullName() {
        list<string> ret;
        for(auto token : sub_token_list){
            ret.push_back(token->toString());
        }
        return ret;
    }

    TokenDef::TokenDef(Document *document,Bytecode begin_mark,ID id) : Node(document,begin_mark),id(id) {}

    TokenDef::TokenDef(Document *document,Bytecode begin_mark,std::istream &stream) : Node(document,begin_mark,stream) {
        read(stream,id);
    }

    void TokenDef::toHex(ostream &stream) {
        Node::toHex(stream);
        write(stream,id);
    }

    Member *TokenDef::getTarget() {
        return target;
    }

    void TokenDef::setTarget(Member *member) {
        target = member;
    }

    TokenDef::ID TokenDef::getID() {
        return id;
    }


    Inst::Inst(Document *document,Bytecode begin_mark) : Node(document,begin_mark) {}

    Inst::Inst(Document *document, Bytecode begin_mark, istream &stream) : Node(document, begin_mark, stream) {}


    InterfaceFunction::InterfaceFunction(Document *document,TokenRef *name,std::list<Param *> params,Result *result)
        : FunctionDeclare(document,Bytecode::ItfFtnDef,AccessFlag::Public,name,std::move(params),result){}

    InterfaceFunction::InterfaceFunction(Document *document, istream &stream)
        : FunctionDeclare(document,Bytecode::ItfFtnDef,stream){}

    std::string InterfaceFunction::toString() {
        return FunctionDeclare::toString();
    }

    type::Symbol *InterfaceFunction::prepareSymbol() {
        if(!symbol){
            symbol = new type::UserFunction(FunctionFlag::Virtual);
            symbol->setName(getNameToken()->toString());
            symbol->setAccessFlag(AccessFlag::Public);
        }
        return symbol;
    }

    void InterfaceFunction::fillSymbolDetail(CompileTimeContext *context) {
        fillParameterList(context,symbol);
    }
    
    
    Scope::Scope(Document *document,Bytecode begin_mark,AccessFlag access,TokenRef *name,std::list<Member*> members)
        : Member(document,begin_mark,access,name),members(std::move(members)){}

    Scope::Scope(Document *document, Bytecode begin_mark, istream &stream)
        : Member(document, begin_mark, stream){}

    std::list<Member*> Scope::loadMembersFromStream(Document *document,std::istream &stream) {
        list<Member*> members;
        while(true){
            Member *member = nullptr;
            if(predict(stream,Bytecode::ClassDef)){
                member = new Class(document,stream);
            }
            else if(predict(stream,Bytecode::ModuleDef)){
                member = new Module(document,stream);
            }
            else if(predict(stream,Bytecode::RecordDef)){
                member = new Record(document,stream);
            }
            else if(predict(stream,Bytecode::EnumDef)){
                member = new Enum(document,stream);
            }
            else if(predict(stream,Bytecode::FldDef)){
                member = new Fld(document,stream);
            }
            else if(predict(stream,Bytecode::SFldDef)){
                member = new SFld(document,stream);
            }
            else if(predict(stream,Bytecode::InterfaceDef)){
                member = new Interface(document,stream);
            }
            else if(predict(stream,Bytecode::ExtDef)){
                member = new Ext(document,stream);
            }
            else if(predict(stream,Bytecode::CtorDef)){
                member = new Ctor(document,stream);
            }
            else if(predict(stream,Bytecode::VFtnDef)){
                member = new VFtn(document,stream);
            }
            else if(predict(stream,Bytecode::FtnDef)){
                member = new Ftn(document,stream);
            }
            else if(predict(stream,Bytecode::SFtnDef)){
                member = new SFtn(document,stream);
            }
            else{
                break;
            }
            members.push_back(member);
        }
        return members;
    }

    const std::list<Member*> &Scope::getMembers() {
        return members;
    }

    InstIntrinsic::InstIntrinsic(Document *document, vm::IntrinsicEnum id)
        : Inst(document,Bytecode::Intrinsic), index((data::u8)id){}

    std::string InstIntrinsic::toString() {
        return Format()<<"Intrinsic "<< to_string(index);
    }

    Inst::ByteSize InstIntrinsic::getByteSize() {
        return 2;
    }

    void InstIntrinsic::toHex(std::ostream &stream) {
        Inst::toHex(stream);
        write(stream,index);
    }
}

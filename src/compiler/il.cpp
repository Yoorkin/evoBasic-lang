//
// Created by yorkin on 12/11/21.
//

#include "il.h"

namespace evoBasic::il{
    using namespace std;


    InstBr *Br(Block *block){
        auto ret = new InstBr;
        ret->target = block;
        return ret;
    }
    InstJif *Jif(Block *block){
        auto ret = new InstJif;
        ret->target = block;
        return ret;
    }

#define FillOp(OP) auto ret = new InstWithOp; \
                    ret->op = InstWithOp::OP;   \
                    return ret;

#define FillOpData(OP) auto ret = new InstWithData; \
                        ret->op = InstWithData::OP; \
                        ret->type = data;           \
                        return ret;

#define FillOpToken(OP,TOKEN) auto ret = new InstWithToken; \
                                        ret->op = InstWithToken::OP;  \
                                        ret->token = TOKEN;              \
                                        return ret;

#define FillOpDataToken(OP,DATA,TOKEN) auto ret = new InstWithDataToken; \
                                        ret->op = InstWithDataToken::OP;  \
                                        ret->type = DATA;                \
                                        ret->token = TOKEN;              \
                                        return ret;


    InstWithData *EQ(DataType data){
        FillOpData(EQ)
    }
    InstWithData *NE(DataType data){
        FillOpData(NE);
    }
    InstWithData *LT(DataType data){
        FillOpData(NE);
    }
    InstWithData *GT(DataType data){
        FillOpData(GT);
    }
    InstWithData *LE(DataType data){
        FillOpData(LE);
    }
    InstWithData *GE(DataType data){
        FillOpData(GE);
    }
    InstWithData *Add(DataType data){
        FillOpData(Add);
    }
    InstWithData *Sub(DataType data){
        FillOpData(Sub);
    }
    InstWithData *Mul(DataType data){
        FillOpData(Mul);
    }
    InstWithData *Div(DataType data){
        FillOpData(Div);
    }
    InstWithData *FDiv(DataType data){
        FillOpData(FDiv);
    }
    InstWithData *Neg(DataType data){
        FillOpData(NE);
    }
    InstWithOp *And(){
        FillOp(And);
    }
    InstWithOp *Or(){
        FillOp(Or);
    }
    InstWithOp *Xor(){
        FillOp(Xor);
    }
    InstWithOp *Not(){
        FillOp(Not);
    }
    InstWithOp *Nop(){
        FillOp(Nop);
    }
    InstWithData *Pop(DataType data){
        FillOpData(Pop);
    }
    InstWithData *Dup(DataType data){
        FillOpData(Dup);
    }
    InstWithOp *Ret(){
        FillOp(Ret);
    }
    InstPush *Push(DataType data, std::any value){
        auto ret = new InstPush;
        ret->type = data;
        ret->value = value;
        return ret;
    }
    InstWithToken *Ldc(ConstructedToken *token){
        FillOpToken(Ldc,token);
    }
    InstWithToken *Ldftn(Ftn *ftn){
        FillOpToken(Ldftn,ftn->getConstructedToken());
    }
    InstWithToken *Ldsftn(SFtn *sftn){
        FillOpToken(Ldsftn,sftn->getConstructedToken());
    }
    InstWithToken *Ldvftn(VFtn *vftn){
        FillOpToken(Ldvftn,vftn->getConstructedToken());
    }
    InstWithData *Ldarg(DataType data){
        FillOpData(Ldarg);
    }
    InstWithData *Starg(DataType data){
        FillOpData(Starg);
    }
    InstWithOp *Ldarga(){
        FillOp(Ldarga);
    }
    InstWithData *Ldloc(DataType data){
        FillOpData(Ldloc);
    }
    InstWithOp *Ldloca(){
        FillOp(Ldloca);
    }
    InstWithData *Stloc(DataType data){
        FillOpData(Stloc);
    }
    InstWithDataToken *Ldfld(DataType data,Fld *field){
        FillOpDataToken(Ldfld,data,field->getConstructedToken());
    }
    InstWithDataToken *Ldsfld(DataType data,SFld *field){
        FillOpDataToken(Ldsfld,data,field->getConstructedToken());
    }
    InstWithDataToken *Ldflda(DataType data,Fld *field){
        FillOpDataToken(Ldflda,data,field->getConstructedToken());
    }
    InstWithDataToken *Ldsflda(DataType data,SFld *field){
        FillOpDataToken(Ldsflda,data,field->getConstructedToken());
    }
    InstWithDataToken *Stfld(DataType data,Fld *field){
        FillOpDataToken(Stfld,data,field->getConstructedToken());
    }
    InstWithDataToken *Stsfld(DataType data,SFld *field){
        FillOpDataToken(Stsfld,data,field->getConstructedToken());
    }
    InstWithData *Ldelem(DataType data){
        FillOpData(Ldelem);
    }
    InstWithOp *Ldelema(){
        FillOp(Ldelema);
    }
    InstWithData *Stelem(DataType data){
        FillOpData(Stelem);
    }
    InstWithOp *Ldnull(){
        FillOp(Ldnull);
    }
    InstWithToken *Newobj(Class *cls){
        FillOpToken(Newobj,cls->getConstructedToken());
    }
    InstCastcls *Castcls(Class *src, Class *dst){
        auto ret = new InstCastcls;
        ret->srcClass = src->getConstructedToken();
        ret->dstClass = dst->getConstructedToken();
        return ret;
    }
    InstConv *Conv(DataType src, DataType dst){
        auto ret = new InstConv;
        ret->src = src;
        ret->dst = dst;
        return ret;
    }
    InstWithOp *Callvirt(){
        FillOp(CallVirt);
    }
    InstWithToken *Callext(Ext *external){
        FillOpToken(Callext,external->getConstructedToken());
    }
    InstWithOp *Callstatic(){
        FillOp(Calls);
    }
    InstWithOp *Call(){
        FillOp(Call);
    }

    Result *ILFactory::createResult(type::Prototype *prototype) {
        auto ret = new Result;
        //todo token query
        return ret;
    }

    Token *ILFactory::createToken(std::string text) {
        return nullptr;
    }

    ConstructedToken *ILFactory::createConstructedToken(std::vector<Token *> token_list) {
        return nullptr;
    }

    Module *ILFactory::createModule(std::string name, AccessFlag access, std::vector<Member *> members) {
        return nullptr;
    }

    Class *ILFactory::createClass(std::string name, AccessFlag access, Extend *extend, std::vector<Impl *> impls,
                                  std::vector<Member *> members) {
        return nullptr;
    }

    Interface *ILFactory::createInterface(std::string name, AccessFlag access, std::vector<FtnBase*> ftns) {
        return nullptr;
    }

    Enum *ILFactory::createEnum(std::string name, AccessFlag access, std::vector<Pair *> pairs) {
        return nullptr;
    }

    Record *ILFactory::createRecord(std::string name, AccessFlag access, std::vector<Fld *> fields) {
        return nullptr;
    }

    Fld *ILFactory::createField(std::string name, AccessFlag access, type::Prototype *prototype) {
        return nullptr;
    }

    SFld *ILFactory::createStaticField(std::string name, AccessFlag access, type::Prototype *prototype) {
        return nullptr;
    }

    Ftn *ILFactory::createFunction(std::string name, AccessFlag access, std::vector<Param *> params, Result *result,
                                   Block *entry) {
        return nullptr;
    }

    Ctor *ILFactory::createConstructor(AccessFlag access, std::vector<Param *> params, Block *entry) {
        return nullptr;
    }

    VFtn *
    ILFactory::createVirtualFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result,
                                     Block *entry) {
        return nullptr;
    }

    SFtn *
    ILFactory::createStaticFunction(std::string name, AccessFlag access, std::vector<Param *> params, Result *result,
                                    Block *entry) {
        return nullptr;
    }

    Ext *
    ILFactory::createExternalFunction(std::string name, std::string lib, AccessFlag access, std::vector<Param *> params,
                                      Result *result) {
        return nullptr;
    }

    Param *ILFactory::createParam(std::string name, type::Prototype *prototype,bool byref) {
        return nullptr;
    }

    Opt *ILFactory::createOption(std::string name, type::Prototype *prototype, bool byref, Block *initial) {
        return nullptr;
    }

    Inf *ILFactory::createParamArray(std::string name, type::Prototype *prototype, bool byref) {
        return nullptr;
    }

    Pair *ILFactory::createPair(std::string name, data::u32 value) {
        return nullptr;
    }

    Extend *ILFactory::createExtend(type::Class *cls) {
        return nullptr;
    }

    Impl *ILFactory::createImplements(type::Interface *interface) {
        return nullptr;
    }

    Local *ILFactory::createLocal(std::string name, type::Prototype *prototype) {
        return nullptr;
    }

    FtnBase *ILFactory::createInterfaceFunction(std::string name, AccessFlag access, std::vector<Param *> params,
                                                Result *result) {
        return nullptr;
    }

    Document *ILFactory::createDocument(std::vector<Member *> members) {
        return nullptr;
    }


    std::string Local::toString() {
        return std::string();
    }

    void Local::toHex(std::ostream &stream) {

    }

    Block &Block::Ldarg(DataType data) {

    }

    std::string Block::toString() {
        return std::string();
    }

    void Block::toHex(std::ostream &stream) {

    }

    Block::Block(std::initializer_list<Inst *> inst_init) {

    }

    Block &Block::Br(Block *block) {
        return *this;
    }

    Block &Block::Jif(Block *block) {
        return *this;
    }

    Block &Block::EQ(DataType data) {
        return *this;
    }

    Block &Block::NE(DataType data) {
        return *this;
    }

    Block &Block::LT(DataType data) {
        return *this;
    }

    Block &Block::GT(DataType data) {
        return *this;
    }

    Block &Block::LE(DataType data) {
        return *this;
    }

    Block &Block::GE(DataType data) {
        return *this;
    }

    Block &Block::Add(DataType data) {
        return *this;
    }

    Block &Block::Sub(DataType data) {
        return *this;
    }

    Block &Block::Mul(DataType data) {
        return *this;
    }

    Block &Block::Div(DataType data) {
        return *this;
    }

    Block &Block::FDiv(DataType data) {
        return *this;
    }

    Block &Block::Neg(DataType data) {
        return *this;
    }

    Block &Block::And() {
        return *this;
    }

    Block &Block::Or() {
        return *this;
    }

    Block &Block::Xor() {
        return *this;
    }

    Block &Block::Not() {
        return *this;
    }

    Block &Block::Nop() {
        return *this;
    }

    Block &Block::Pop(DataType data) {
        return *this;
    }

    Block &Block::Dup(DataType data) {
        return *this;
    }

    Block &Block::Ret() {
        return *this;
    }

    Block &Block::Push(DataType data, std::any value) {
        return *this;
    }

    Block &Block::Ldc(Token *token) {
        return *this;
    }

    Block &Block::Ldftn(Token *ftn) {
        return *this;
    }

    Block &Block::Ldsftn(Token *sftn) {
        return *this;
    }

    Block &Block::Ldvftn(Token *vftn) {
        return *this;
    }

    Block &Block::Starg(DataType data) {
        return *this;
    }

    Block &Block::Ldarga() {
        return *this;
    }

    Block &Block::Ldloc(DataType data) {
        return *this;
    }

    Block &Block::Ldloca() {
        return *this;
    }

    Block &Block::Stloc(DataType data) {
        return *this;
    }

    Block &Block::Ldfld(DataType data, Token *fld) {
        return *this;
    }

    Block &Block::Ldsfld(DataType data, Token *sfld) {
        return *this;
    }

    Block &Block::Ldflda(Token *fld) {
        return *this;
    }

    Block &Block::Ldsflda(Token *sfld) {
        return *this;
    }

    Block &Block::Stfld(DataType data, Token *fld) {
        return *this;
    }

    Block &Block::Stsfld(DataType data, Token *sfld) {
        return *this;
    }

    Block &Block::Ldelem(DataType data) {
        return *this;
    }

    Block &Block::Ldelema() {
        return *this;
    }

    Block &Block::Stelem(DataType data) {
        return *this;
    }

    Block &Block::Ldnull() {
        return *this;
    }

    Block &Block::Newobj(Token *cls) {
        return *this;
    }

    Block &Block::Castcls(Token *src, Token *dst) {
        return *this;
    }

    Block &Block::Conv(DataType src, DataType dst) {
        return *this;
    }

    Block &Block::Callvirt() {
        return *this;
    }

    Block &Block::Invoke(Token *external) {
        return *this;
    }

    Block &Block::Callstatic() {
        return *this;
    }

    Block &Block::Call() {
        return *this;
    }

    data::u32 Block::getAddress() {
        return 0;
    }

    std::string Token::toString() {
        return std::string();
    }

    void Token::toHex(std::ostream &stream) {

    }

    std::string ConstructedToken::toString() {

    }

    void ConstructedToken::toHex(std::ostream &stream) {

    }

    std::string Access::toString() {
        vector<string> str = {"Public","Private","Friend","Protected"};
        return Format() << "(Access " << str[(int)flag] << ")";
    }

    void Access::toHex(std::ostream &stream) {

    }

    std::string Extend::toString() {
        return Format() << "(extend " << target->toString() << ")";
    }

    void Extend::toHex(std::ostream &stream) {

    }

    std::string Impl::toString() {
        return Format() << "(impl " << target->toString() << ")";
    }

    void Impl::toHex(std::ostream &stream) {

    }

    std::string Lib::toString() {
        return Format() << "(lib " << target->toString() << ")";
    }

    void Lib::toHex(std::ostream &stream) {

    }

    ConstructedToken *Member::getConstructedToken() {

    }

    std::string Class::toString() {
        Format fmt;
        fmt << "(cls " << access->toString() << ' ' << name->toString() << ' ' << extend.toString();
        for(auto impl : impls) fmt << ' ' << impl.toString();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }

    void Class::toHex(std::ostream &stream) {

    }

    std::string Module::toString() {
        Format fmt;
        fmt << "(mod " << access->toString() << ' ' << name->toString();
        for(auto member : members) fmt << '\n' << member->toString();
        fmt << ")";
        return fmt;
    }

    void Module::toHex(std::ostream &stream) {

    }

    std::string Interface::toString() {
        Format fmt;
        fmt << "(interface " << access->toString() << ' ' << name->toString();
        for(auto ftn : ftns) fmt << '\n' << ftn->toString();
        fmt << ")";
        return fmt;
    }

    void Interface::toHex(std::ostream &stream) {

    }

    std::string Pair::toString() {
        return Format() << "(pair " << name->toString() << ' ' << to_string(value);
    }

    void Pair::toHex(std::ostream &stream) {

    }

    std::string Enum::toString() {
        Format fmt;
        fmt << "(Enum " << access->toString() << ' ' << name->toString();
        for(auto p : pairs) fmt << '\n' << p->toString();
        fmt << ")";
        return fmt;
    }

    void Enum::toHex(std::ostream &stream) {

    }

    std::string Fld::toString() {
        return Format() << "(fld " << access->toString() << ' ' << name->toString() << ' ' << type->toString() <<")";
    }

    void Fld::toHex(std::ostream &stream) {

    }

    std::string SFld::toString() {
        return Format() << "(sfld " << access->toString() << ' ' << name->toString() << ' ' << type->toString() <<")";
    }

    void SFld::toHex(std::ostream &stream) {
        Fld::toHex(stream);
    }

    std::string Record::toString() {
        Format fmt;
        fmt << "(record " << access->toString() << ' ' << name->toString();
        for(auto fld : fields) fmt << '\n' << fld->toString();
        fmt << ")";
        return fmt;
    }

    void Record::toHex(std::ostream &stream) {

    }

    std::string Param::toString() {
        return Format() << "(param " << name->toString() << " " << type->toString() << ")";
    }

    void Param::toHex(std::ostream &stream) {

    }

    std::string Opt::toString() {
        return Format() << "(opt " << name->toString() << " " << type->toString() << ")";
    }

    void Opt::toHex(std::ostream &stream) {
        Param::toHex(stream);
    }

    std::string Inf::toString() {
        return Format() << "(inf " << name->toString() << " " << type->toString() << ")";
    }

    void Inf::toHex(std::ostream &stream) {
        Param::toHex(stream);
    }

    std::string Result::toString() {
        return Format() << "(result " << type->toString() << ")";
    }

    void Result::toHex(std::ostream &stream) {

    }

    std::string Ctor::toString() {
        return std::string();
    }

    void Ctor::toHex(std::ostream &stream) {

    }


    std::string FtnBase::toString() {
        Format fmt;
        fmt << access->toString() << ' ' << name->toString();
        for(auto p : params) fmt << ' ' << p->toString();
        fmt << result->toString();
        return fmt;
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
        return Format() << "(ext " << lib.toString() << ' ' << FtnBase::toString() << ")";
    }

    void Ext::toHex(std::ostream &stream) {

    }

    std::string InstWithOp::toString() {
        vector<string> str = {"Nop","Ret","CallVirt","CallExt","Calls","Call",
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
        fmt << "\n" << str[(int)op] << ' ' << token->toString();
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
                fmt << any_cast<data::i8>(value);
                break;
            case DataType::i16:
                fmt << any_cast<data::i16>(value);
                break;
            case DataType::i32:
                fmt << any_cast<data::i32>(value);
                break;
            case DataType::i64:
                fmt << any_cast<data::i64>(value);
                break;
            case DataType::u8:
                fmt << any_cast<data::u8>(value);
                break;
            case DataType::u32:
                fmt << any_cast<data::u32>(value);
                break;
            case DataType::u64:
                fmt << any_cast<data::u64>(value);
                break;
            case DataType::f32:
                fmt << any_cast<data::f32>(value);
                break;
            case DataType::f64:
                fmt << any_cast<data::f64>(value);
                break;
            case DataType::boolean:
                fmt << any_cast<data::boolean>(value);
                break;
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
        return Format() << inst[(int)op] << '.' << ty[(int)type] << token->toString();
    }

    void InstWithDataToken::toHex(std::ostream &stream) {

    }

    std::string InstCastcls::toString() {
        return Format() << "cast." << srcClass->toString() << " " << dstClass->toString();
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

}
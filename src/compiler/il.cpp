//
// Created by yorkin on 12/11/21.
//

#include "il.h"

namespace evoBasic::il{



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

    Interface *ILFactory::createInterface(std::string name, AccessFlag access, std::vector<VFtn *> ftns) {
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

    Ftn *
    ILFactory::createVirtualFunction(std::string name, AccessFlag access, std::vector<Param *> params, Result *result,
                                     Block *entry) {
        return nullptr;
    }

    Ftn *
    ILFactory::createStaticFunction(std::string name, AccessFlag access, std::vector<Param *> params, Result *result,
                                    Block *entry) {
        return nullptr;
    }

    Ext *
    ILFactory::createExternalFunction(std::string name, std::string lib, AccessFlag access, std::vector<Param *> params,
                                      Result *result) {
        return nullptr;
    }

    Param *ILFactory::createParam(std::string name, type::Prototype *prototype) {
        return nullptr;
    }

    Opt *ILFactory::createOption(std::string name, type::Prototype *prototype, SFtn *initial) {
        return nullptr;
    }

    Inf *ILFactory::createParamArray(std::string name, type::Prototype *prototype) {
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

    }

    Block &Block::Jif(Block *block) {

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

    Block &Block::Ldftn(Ftn *ftn) {
        return *this;
    }

    Block &Block::Ldsftn(SFtn *sftn) {
        return *this;
    }

    Block &Block::Ldvftn(VFtn *vftn) {
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

    Block &Block::Ldflda(DataType data, Token *fld) {
        return *this;
    }

    Block &Block::Ldsflda(DataType data, Token *sfld) {
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

    Block &Block::Newobj(Class *cls) {
        return *this;
    }

    Block &Block::Castcls(Class *src, Class *dst) {
        return *this;
    }

    Block &Block::Conv(DataType src, DataType dst) {
        return *this;
    }

    Block &Block::Callvirt() {
        return *this;
    }

    Block &Block::Callext(Ext *external) {
        return *this;
    }

    Block &Block::Callstatic() {
        return *this;
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
        return std::string();
    }

    void Access::toHex(std::ostream &stream) {

    }

    std::string Extend::toString() {
        return std::string();
    }

    void Extend::toHex(std::ostream &stream) {

    }

    std::string Impl::toString() {
        return std::string();
    }

    void Impl::toHex(std::ostream &stream) {

    }

    std::string Lib::toString() {
        return std::string();
    }

    void Lib::toHex(std::ostream &stream) {

    }

    ConstructedToken *Member::getConstructedToken() {
        return nullptr;
    }

    std::string Class::toString() {
        return std::string();
    }

    void Class::toHex(std::ostream &stream) {

    }

    std::string Module::toString() {
        return std::string();
    }

    void Module::toHex(std::ostream &stream) {

    }

    std::string Interface::toString() {
        return std::string();
    }

    void Interface::toHex(std::ostream &stream) {

    }

    std::string Pair::toString() {
        return std::string();
    }

    void Pair::toHex(std::ostream &stream) {

    }

    std::string Enum::toString() {
        return std::string();
    }

    void Enum::toHex(std::ostream &stream) {

    }

    std::string Fld::toString() {
        return std::string();
    }

    void Fld::toHex(std::ostream &stream) {

    }

    std::string SFld::toString() {
        return Fld::toString();
    }

    void SFld::toHex(std::ostream &stream) {
        Fld::toHex(stream);
    }

    std::string Record::toString() {
        return std::string();
    }

    void Record::toHex(std::ostream &stream) {

    }

    std::string Param::toString() {
        return std::string();
    }

    void Param::toHex(std::ostream &stream) {

    }

    std::string Opt::toString() {
        return Param::toString();
    }

    void Opt::toHex(std::ostream &stream) {
        Param::toHex(stream);
    }

    std::string Inf::toString() {
        return Param::toString();
    }

    void Inf::toHex(std::ostream &stream) {
        Param::toHex(stream);
    }

    std::string Result::toString() {
        return std::string();
    }

    void Result::toHex(std::ostream &stream) {

    }

    std::string Ctor::toString() {
        return std::string();
    }

    void Ctor::toHex(std::ostream &stream) {

    }

    std::string Ftn::toString() {
        return std::string();
    }

    void Ftn::toHex(std::ostream &stream) {

    }

    std::string VFtn::toString() {
        return std::string();
    }

    void VFtn::toHex(std::ostream &stream) {

    }

    std::string SFtn::toString() {
        return std::string();
    }

    void SFtn::toHex(std::ostream &stream) {

    }

    std::string Ext::toString() {
        return std::string();
    }

    void Ext::toHex(std::ostream &stream) {

    }

    std::string InstWithOp::toString() {
        return std::string();
    }

    void InstWithOp::toHex(std::ostream &stream) {

    }

    std::string InstWithToken::toString() {
        return std::string();
    }

    void InstWithToken::toHex(std::ostream &stream) {

    }

    std::string InstJif::toString() {
        return std::string();
    }

    void InstJif::toHex(std::ostream &stream) {

    }

    std::string InstBr::toString() {
        return std::string();
    }

    void InstBr::toHex(std::ostream &stream) {

    }

    std::string InstPush::toString() {
        return std::string();
    }

    void InstPush::toHex(std::ostream &stream) {

    }

    std::string InstWithData::toString() {
        return std::string();
    }

    void InstWithData::toHex(std::ostream &stream) {

    }

    std::string InstWithDataToken::toString() {
        return std::string();
    }

    void InstWithDataToken::toHex(std::ostream &stream) {

    }

    std::string InstCastcls::toString() {
        return std::string();
    }

    void InstCastcls::toHex(std::ostream &stream) {

    }

    std::string InstConv::toString() {
        return std::string();
    }

    void InstConv::toHex(std::ostream &stream) {

    }
}
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
}
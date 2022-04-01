//
// Created by yorkin on 11/9/21.
//

#ifndef EVOBASIC2_BYTECODE_H
#define EVOBASIC2_BYTECODE_H
#include<string>
#include<vector>
#include<map>
#include"utils/data.h"
namespace evoBasic{

    enum class Bytecode : data::u8 {
        EndMark=0,DependDef,
        TextTokenDef,ConstructedDef,TokenRef,
        PtdAcsDef,PriAcsDef,PubAcsDef,ExtendDef,ImplDef,LibDef,ExtAliasDef,
        DocumentDef,ClassDef,ModuleDef,InterfaceDef,EnumDef,RecordDef,
        FtnDef,VFtnDef,SFtnDef,CtorDef,ExtDef,ItfFtnDef,FldDef,SFldDef,PairDef,
        RegDef,OptDef,InfDef,LocalDef,ResultDef,Byref,Byval,
        InstBeg,
        Nop,Ret,CallVirt,Callstatic,Call,Ldnull,And,Or,Xor,Ldloca,Ldarga,Ldelema,Not,
        Ldftn,Ldsftn,Ldvftn,Ldc,Newobj,Invoke,Intrinsic,Ldflda,Ldsflda,
        Ldelem,Stelem,Stelema,Ldarg,Load,Starg,Store,Ldloc,Stloc,
        Add,Sub,Mul,Div,FDiv,EQ,NE,LT,GT,LE,GE,Neg,Pop,Dup,
        Ldfld,Ldsfld,Stfld,Stsfld,
        Jif,Br,Push,CastCls,Conv,
        i8,i16,i32,i64,u8,u16,u32,u64,f32,f64,
        ref,ftn,vftn,sftn,record,array,boolean,character,delegate
    };

    // std::vector<data::u8> Data::to_hex = {
    //     0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD
    // };
}




#endif //EVOBASIC2_BYTECODE_H

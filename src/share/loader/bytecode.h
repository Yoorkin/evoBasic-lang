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
        Ldftn,Ldsftn,Ldvftn,Ldc,Newobj,Invoke,Ldflda,Ldsflda,
        Ldelem,Stelem,Stelema,Ldarg,Ldargr,Starg,Stargr,Ldloc,Stloc,
        Add,Sub,Mul,Div,FDiv,EQ,NE,LT,GT,LE,GE,Neg,Pop,Dup,
        Ldfld,Ldsfld,Stfld,Stsfld,
        Jif,Br,Push,CastCls,Conv,
        i8,i16,i32,i64,u8,u16,u32,u64,f32,f64,
        ref,ftn,vftn,sftn,record,array,boolean,character,delegate
    };

    namespace vm{

        class Data{
        public:
            enum Value{
                void_=0,boolean,i8,i16,i32,i64,f32,f64,u8,u16,u32,u64,raw_string,memory
            };
        private:
            Value value_;
            static std::vector<std::string> to_string;
            static std::vector<int> to_size;
            static std::vector<data::u8> to_hex;
        public:
            Data(Value value){value_ = value;}
            Value getValue(){return value_;}
            void setValue(Value value){value_ = value;}
            std::string toString();
            data::u8 toHex();
            int getSize();
            static Data fromHex(data::u8 hex);
            static Data ptr;
            bool operator==(const Data &data);
        };
    }
}




#endif //EVOBASIC2_BYTECODE_H

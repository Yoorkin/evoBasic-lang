//
// Created by yorkin on 12/25/21.
//

#ifndef EVOBASIC_ENUMS_H
#define EVOBASIC_ENUMS_H

namespace evoBasic{
    enum class UnaryOp{Empty,MINUS,ADD};
    enum class BinaryOp{
        Empty,And,Or,Xor,Not,EQ,NE,GE,LE,GT,LT,
        ADD,MINUS,MUL,DIV,FDIV
    };
    enum class AccessFlag {Public,Private,Friend,Protected};
    enum class MethodFlag {Virtual,Override,None};
    enum class VariableKind{Undefined,Local,Field,StaticField,Parameter};
    enum class FunctionFlag{Method=0,Static,Virtual,Override};
    enum class FunctionEnum{User,External,Intrinsic};
    enum class FunctionKind{InterfaceFunction,UserFunction,Operator,External,Constructor};
    enum DataType{
        empty,i8,i16,i32,i64,u8,u16,u32,u64,f32,f64,
        ref,ftn,vftn,sftn,record,array,boolean,character,delegate
    };

}

#endif //EVOBASIC_ENUMS_H

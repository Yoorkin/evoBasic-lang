//
// Created by yorkin on 11/9/21.
//

#ifndef EVOBASIC2_BYTECODE_H
#define EVOBASIC2_BYTECODE_H
#include<string>
#include<vector>
#include<map>
#include"data.h"
namespace evoBasic::vm{

    /*
     * int global_var = 99;
     * int max(int a,int b){
     *     if(a>b) return a; else return b;
     * }
     *
     * int main(){
     *     int x = 100,y = 20;
     *     char ret = max(x,y) - global_var;
     *     return 0;
     * }
     *
     * .meta
     *     max : Function(a:i32,b:i32)->i32
     *     main : Function()->i32
     * .data
     *     global_var : i32
     * .init
     *     PushGlobalBase
     *     Push.i32 99
     *     Store.i32
     * .main
     *     PushFrameBase
     *     Store.i32 100
     *
     *     PushFrameBase
     *     Push.i32 1
     *     Add.i32
     *
     *     Push.i32 20
     *     Store.i32
     *
     *     PushFrameBase
     *     Load.i32
     *
     *     PushFrameBase
     *     Push.i32 1
     *     Add.i32
     *     Load.i32
     *
     *     Invoke max
     *
     *     PushGlobalBase
     *     Load.i32
     *
     *     Sub.i32
     *
     *     Cast.i32 i8
     *     PushFrameBase
     *     Push.i32 16
     *     Add.i32
     *
     *     Store.i32
     *
     *     Push.i32 0
     *     ret
     * .max
     *     PushFrameBase
     *     Store.i32
     *
     *     PushFrameBase
     *     Push.i32 1
     *     Add.i32
     *     Store.i32
     *
     *     PushFrameBase
     *     Load.i32
     *
     *     PushFrameBase
     *     Push.i32 1
     *     Add.i32
     *
     *     Cmp.i32
     *     JIF .max.L1
     *     Jmp .max.L2
     * .max.L1
     *     PushFrameBase
     *     Load.i32
     *     Jmp .max.L3
     * .max.L2
     *     PushFrameBase
     *     Push.i32 1
     *     Add.i32
     *
     *     Load.i32
     *     Jmp .max.L3
     * .max.L3
     *     ret
     */

    class Bytecode{
    public:
        enum Value{
            CompilerVersion,
            MetaSegment,
            Class,Module,Enum,Type,Interface,StructEnd,
            Ext,EnumMember,Impl,
            Field,Function,Constructor,Decl,Lib,
            RegularParam,OptParam,ParamAry,Array,NonArray,
            Byval,ByRef,PubAcs,PriAcs,ProAcs,Alias,
            RefLocalTy,RefExtTy,RefPmtTy,Depend,
            ConstSegment,CodeSegment,
            PushFrameBase, /* push current stack frame address */
            PushGlobalBase, /* push global address */
            PushConstBase, /* push constant pool address */
            Invoke,Intrinsic,External,
            Jmp,Jif,EQ,NE,LT,GT,LE,GE,Add,Sub,Mul,Div,
            FDiv,Neg,And,Or,Xor,Not,Load,Store,
            Push,Pop,Ret,Cast,Dup,Stm,Ldm,Psm,
            Nop,StoreR,StmR,RcInc,RcDec
        };
    private:
        static std::vector<std::string> to_string;
        static std::vector<data::u8> to_hex;
        static std::map<std::string,Value> string_to_value;
        Value value_;
    public:
        Bytecode(Value value){ value_ = value;}
        Value getValue(){return value_;}
        void setValue(Value value){value_ = value;}
        std::string toString();
        data::u8 toHex();
        static Bytecode fromHex(data::u8 value);
        static Bytecode fromString(const std::string& str);
        bool operator==(const Bytecode &rhs);
    };

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



#endif //EVOBASIC2_BYTECODE_H

//
// Created by yorkin on 11/9/21.
//

#include "bytecode.h"
#include "nullSafe.h"
namespace evoBasic::vm{

//    Segment,Define,Jmp,Jif,EQ,NE,LT,GT,LE,GE,Add,Sub,Mul,Div,FDiv,Neg,And,Or,Xor,Not,Load,Store,Invoke,Push,Pop,Ret,Cast,Dup,
//    PushFrameBase, /* push current stack frame address */
//    PushGlobalBase /* push global address */

    std::vector<std::string> Bytecode::to_string = {
        "Entrance","MetaSegment","ConstSegment","CodeSegment",
        "Define","Jmp","Jif","EQ","NE","LT","GT","LE","GE","Add","Sub","Mul","Div","FDiv",
        "Neg","And","Or","Xor","Not","Load","Store","Invoke","Push","Pop","Ret","Cast","Dup",
        "Stm","Ldm","Psm",
        "PushFrameBase","PushGlobalBase","Nop","StoreR","StmR","Intrinsic","External",
        "RcInc","RcDec"
    };

    std::vector<data::u8> Bytecode::to_hex = {
        0x0F,0xA0,0xA1,0xA2,
        0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
        0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
        0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9
    };
    
    std::map<std::string,Bytecode::Value> Bytecode::string_to_value = {
        {"Entrance",Entrance},
        {"MetaSegment",MetaSegment},{"ConstSegment",ConstSegment},{"CodeSegment",CodeSegment},
        {"Define",Define},{"Jmp",Jmp},{"Jif",Jif},{"EQ",EQ},{"NE",NE},{"LT",LT},{"GT",GT},
        {"LE",LE},{"GE",GE},{"Add",Add},{"Sub",Sub},{"Mul",Mul},{"Div",Div},{"FDiv",FDiv},
        {"Neg",Neg},{"And",And},{"Or",Or},{"Xor",Xor},{"Not",Not},{"Load",Load},{"Store",Store},{"Invoke",Invoke},
        {"Push",Push},{"Pop",Pop},{"Ret",Ret},{"Cast",Cast},{"Dup",Dup},{"Stm",Stm},{"Ldm",Ldm},{"Psm",Psm},
        {"PushFrameBase",PushFrameBase},{"PushGlobalBase",PushGlobalBase},{"Nop",Nop},{"StoreR",StoreR},{"StmR",StmR},
        {"Intrinsic",Intrinsic},{"External",External},{"RcInc",RcInc},{"RcDec",RcDec}
    };
    
    std::string Bytecode::toString() {
        return to_string[value_];
    }

    data::u8 Bytecode::toHex() {
        return to_hex[value_];
    }

    Bytecode Bytecode::fromHex(data::u8 value) {
        auto em_value = (value - to_hex[0]);
        ASSERT(em_value < 0 || em_value >= to_hex.size(),"unknown hex value");
        return Bytecode((Value)em_value);
    }

    Bytecode Bytecode::fromString(const std::string& str) {
        auto target = string_to_value.find(str);
        ASSERT(target == string_to_value.end(),"unknown string value");
        return Bytecode(target->second);
    }

    bool Bytecode::operator==(const Bytecode &rhs) {
        return this->value_ == rhs.value_;
    }


    std::vector<std::string> Data::to_string = {
        "void","boolean","i8","i16","i32","i64","f32","f64","u8","u16","u32","u64","raw_string","memory"
    };

    std::vector<int> Data::to_size = {
        0,1,1,2,4,8,4,8,1,2,4,8,0,0
    };

    std::vector<data::u8> Data::to_hex = {
        0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD
    };

    Data Data::ptr(Data::u64);

    std::string Data::toString() {
        return to_string[value_];
    }

    int Data::getSize() {
        return to_size[value_];
    }

    data::u8 Data::toHex() {
        return to_hex[value_];
    }

    Data Data::fromHex(data::u8 hex) {
        auto em_value = hex - to_hex[0];
        ASSERT(em_value < 0 || em_value >= to_hex.size(),"unknown hex value");
        return Data((Value)em_value);
    }

    bool Data::operator==(const Data &data) {
        return data.value_ == value_;
    }
}


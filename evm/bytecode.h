#ifndef EVM_BYTECODE
#define EVM_BYTECODE
#include <cstdint>
#include <string>

using token_t = uint32_t;

namespace bytecode{
    const uint32_t
    flag_nothing = 0,
    flag_public = 1,
    flag_private = 1 << 1,
    flag_virtual = 1 << 2,
    flag_override = 1 << 3,
    flag_static = 1 << 4,
    flag_byval = 1 << 5,
    flag_byref = 1 << 6,
    flag_optional = 1 << 7,
    flag_paramArray = 1 << 8;

    const uint8_t 
    t_boolean = 80,
    t_i8 = 81,
    t_i16 = 82,
    t_i32 = 83,
    t_i64 = 84,
    t_u8 = 85,
    t_u16 = 86,
    t_u32 = 87,
    t_u64 = 88,
    t_f32 = 89,
    t_f64 = 90,
    t_ref = 91,
    t_ftn = 92,
    t_vftn = 93,
    t_sftn = 94,
    t_ctor = 95,
    t_record = 96,
    ldsftn = 97,
    ldvftn = 98,
    ldftn = 99,
    ldctor = 100,
    ldforeign = 101,
    callmethod = 102,
    callvirtual = 103,
    callstatic = 104,
    callforeign = 105,
    callintrinsic = 106,
    starg = 107,
    ldarg = 108,
    ldarga = 109,
    stloc = 110,
    ldloc = 111,
    ldloca = 112,
    stfld = 113,
    ldfld = 114,
    ldflda = 115,
    stsfld = 116,
    ldsfld = 117,
    ldsflda = 118,
    packopt = 119,
    stelem = 120,
    ldelem = 121,
    ldelema = 122,
    newarray = 123,
    arraylength = 124,
    jif = 125,
    br = 126,
    ret = 127,
    nop = 0,
    dup = 128,
    push = 129,
    store = 130,
    load = 131,
    ldnothing = 132,
    convert = 133,
    castClass = 134,
    instanceof = 145,
    throw_ = 146,
    enter = 147,
    leave = 148,
    add = 149,
    sub = 150,
    mul = 151,
    div = 152,
    and_ = 153,
    or_ = 154,
    xor_ = 155,
    eq = 156,
    ne = 157,
    lt = 158,
    gt = 159,
    le = 160,
    ge = 161,
    neg = 162,
    not_ = 163,
    callctor = 164,
    stelemr = 165,
    pop = 166,
    ldstr = 167,
    testopt = 168,
    ldoptinfo = 169,
    ldenumc = 170,
    t_emconst = 171,
    newobj = 172,
    mod = 173,
    t_hdl = 174,
    t_dlg = 175,
    wrapsftn = 176,
    wrapvftn = 177,
    wrapftn = 178,
    wrapctor = 179,
    wrapforeign = 180,
    calldlg = 181,
    t_ptr = 182;
}


inline int isBigEndian(){
    int i = 1;
    return *((uint8_t*)&i)==0;
}

inline void seqcpy(uint8_t *dst,uint8_t *src,int count){
    if(isBigEndian()){
        throw "";// todo: 这里有bug
                 // 这个函数原本是为了让大端序机器生成小端序的.bkg包
                 // 但是对于解释执行字节码时，从内存到内存的字节拷贝，不需要做大端到小端的转换。
        src += count - 1;
        while(count--){
            *dst++ = *src--;
        }
    }
    else{
        while(count--){
            *dst++ = *src++;
        }
    }
}

template<class T>
inline T read(uint8_t *ptr){
    T t;
    seqcpy((uint8_t*)&t,ptr,sizeof(T));
    return t;
}

template<class T>
inline void write(uint8_t *ptr,T t){
    seqcpy(ptr,(uint8_t*)&t,sizeof(T));
}


#endif
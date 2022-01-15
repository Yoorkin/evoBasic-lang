//
// Created by yorkin on 10/21/21.
//

#ifndef EVOBASIC_DATADEF_H
#define EVOBASIC_DATADEF_H
#include <vector>
#include <string>
namespace evoBasic::data{

    using boolean = bool;
    using i8 = char;
    using i16 = short;
    using i32 = int;
    using i64 = long long;
    using f32 = float;
    using f64 = double;

    using u8 = unsigned char;
    using Byte = u8;
    using u16 = unsigned char;
    using u32 = unsigned int;
    using u64 = unsigned long long;

    using ptr = u64;
    using address = u64;
    using size = u64;
}
#endif //EVOBASIC_DATADEF_H

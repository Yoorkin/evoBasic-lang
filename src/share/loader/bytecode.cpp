//
// Created by yorkin on 11/9/21.
//

#include "bytecode.h"
#include "utils/nullSafe.h"
namespace evoBasic::vm{

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


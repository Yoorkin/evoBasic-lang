#include <gtest/gtest.h>
#include "../src/frontend/Semantic.h"

using namespace evoBasic;
using namespace evoBasic::Type;

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 41);
}

TEST(Semantic, TypePromotion){

    auto& in = Semantics::Instance()->getBuiltIn();
    auto& rules = Semantics::Instance()->getConversionRules();

    enum Types {bin=0,i08,i16,i32,i64,f32,f64};
    vector<shared_ptr<Type::Prototype>> enumToPrototype = {
            in.getBooleanPrototype(),
            in.getBytePrototype(),
            in.getShortPrototype(),
            in.getIntegerPrototype(),
            in.getLongPrototype(),
            in.getSinglePrototype(),
            in.getDoublePrototype()
    };

    vector<vector<Types>> promotion_table = {
            /*        bin i08 i16 132 i64 f32 f64 */
            /* bin */{bin,i32,i32,i32,i64,f64,f64},
            /* i08 */{i32,i32,i32,i32,i64,f64,f64},
            /* i16 */{i32,i32,i32,i32,i64,f64,f64},
            /* i32 */{i32,i32,i32,i32,i64,f64,f64},
            /* i64 */{i64,i64,i64,i64,i64,f64,f64},
            /* f32 */{f64,f64,f64,f64,f64,f64,f64},
            /* f64 */{f64,f64,f64,f64,f64,f64,f64}
    };


    auto mock_node = make_node(Tag::Cast,{make_node(Tag::Empty), make_node(Tag::Empty)});

    for(int l=0;l<enumToPrototype.size();l++){
        for(int r=0;r<enumToPrototype.size();r++){
            auto lhs = enumToPrototype[l].get();
            auto rhs = enumToPrototype[r].get();
            auto r1 = rules.promotion(lhs,rhs);
            EXPECT_FALSE(rules.isEmpty(r1));
            auto p1 = r1->second.operator()(mock_node);
            auto real = promotion_table[l][r];
            EXPECT_EQ(p1->getName(),enumToPrototype[real]->getName())
                << enumToPrototype[l]->getName() << " op "<<enumToPrototype[r]->getName()
                <<" -> "<<enumToPrototype[real]->getName()<<", but result is "<<p1->getName()<<"\n";
        }
    }
}
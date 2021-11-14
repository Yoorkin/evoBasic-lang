#include <gtest/gtest.h>
#include <memory>
#include "semantic.h"
#include "type.h"

using namespace evoBasic;
using namespace evoBasic::type;
using namespace std;

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 41);
}

TEST(Semantic, TypePromotion){

//    auto& in = Semantics::Instance()->getBuiltIn();
//    auto& rules = Semantics::Instance()->getConversionRules();
//
//    enum Types {bin=0,i08,i16,i32,i64,f32,f64};
//    vector<shared_ptr<Type::Prototype>> enumToPrototype = {
//            in.getBooleanPrototype(),
//            in.getBytePrototype(),
//            in.getShortPrototype(),
//            in.getIntegerPrototype(),
//            in.getLongPrototype(),
//            in.getSinglePrototype(),
//            in.getDoublePrototype()
//    };
//
//    vector<vector<Types>> promotion_table = {
//            /*        bin i08 i16 132 i64 f32 f64 */
//            /* bin */{bin,i32,i32,i32,i64,f64,f64},
//            /* i08 */{i32,i32,i32,i32,i64,f64,f64},
//            /* i16 */{i32,i32,i32,i32,i64,f64,f64},
//            /* i32 */{i32,i32,i32,i32,i64,f64,f64},
//            /* i64 */{i64,i64,i64,i64,i64,f64,f64},
//            /* f32 */{f64,f64,f64,f64,f64,f64,f64},
//            /* f64 */{f64,f64,f64,f64,f64,f64,f64}
//    };
//
//
//    auto mock_node = make_node(Tag::Cast,{make_node(Tag::Empty), make_node(Tag::Empty)});
//
//    for(int l=0;l<enumToPrototype.size();l++){
//        for(int r=0;r<enumToPrototype.size();r++){
//            auto lhs = enumToPrototype[l].get();
//            auto rhs = enumToPrototype[r].get();
//            auto r1 = rules.promotion(lhs,rhs);
//            EXPECT_FALSE(rules.isEmpty(r1));
//            auto p1 = r1->second.operator()(mock_node);
//            auto real = promotion_table[l][r];
//            EXPECT_EQ(p1->getName(),enumToPrototype[real]->getName())
//                << enumToPrototype[l]->getName() << " op "<<enumToPrototype[r]->getName()
//                <<" -> "<<enumToPrototype[real]->getName()<<", but result is "<<p1->getName()<<"\n";
//        }
//    }
}

TEST(TypeInference,TopoSort_Succeed){
    Dependencies<char> inf;
    /*
     *  let e = a + b + s
     *  let a = b + s
     *  let b = s - 100
     *  let s = 50
     *  let i = 100
     */

    inf.addDependent('a','b');
    inf.addDependent('a','s');

    inf.addDependent('b','s');

    inf.addDependent('e','a');
    inf.addDependent('e','b');
    inf.addDependent('e','s');
    inf.addDependent('e','s');
    inf.addDependent('e','s');
    inf.addDependent('e','s');

    inf.addIsolate('i');

    auto succeed = inf.solve();
    const list<char> order = inf.getTopologicalOrder();

    ASSERT_TRUE(succeed);
    ASSERT_EQ(order.size(),5);

    auto iter = order.begin();
    ASSERT_EQ(*iter,'s');
    iter++;
    ASSERT_EQ(*iter,'b');
    iter++;
    ASSERT_EQ(*iter,'a');
    iter++;
    ASSERT_EQ(*iter,'e');
    iter++;
    ASSERT_EQ(*iter,'i');
}

TEST(TypeInference,TopoSort_Succeed_with_two_element){
    Dependencies<char> inf;
    /*
     *  let a = b + 100
     *  let b = 100
     */

    inf.addDependent('a','b');

    auto succeed = inf.solve();
    const list<char> order = inf.getTopologicalOrder();

    ASSERT_TRUE(succeed);
    ASSERT_EQ(order.size(),2);

    auto iter = order.begin();
    ASSERT_EQ(*iter,'b');
    iter++;
    ASSERT_EQ(*iter,'a');
}

TEST(TypeInference,TopoSort_Failed){
    Dependencies<char> inf;
    /*
     *  let f = a + b + s
     *  let a = b + s
     *  let b = s - 100
     *  let s = 50
     *  let x = y + 1
     *  let y = z + 1
     *  let z = x + 1
     */
    inf.addDependent('f','a');
    inf.addDependent('f','b');
    inf.addDependent('f','s');

    inf.addDependent('a','b');
    inf.addDependent('a','s');

    inf.addDependent('b','s');

    inf.addDependent('x','y');

    inf.addDependent('y','z');

    inf.addDependent('z','x');


    auto succeed = inf.solve();
    list<char> order = inf.getTopologicalOrder();

    ASSERT_FALSE(succeed);
    ASSERT_EQ(order.size(),4);

    auto iter = order.begin();
    ASSERT_EQ(*iter,'s');
    iter++;
    ASSERT_EQ(*iter,'b');
    iter++;
    ASSERT_EQ(*iter,'a');
    iter++;
    ASSERT_EQ(*iter,'f');

    auto circles = inf.getCircles();
    ASSERT_EQ(circles.size(),1);

    set<char> expected1 = {'x','y','z'};
    int i1=0;
    for(auto& iter:circles.front()){
        ASSERT_TRUE(expected1.contains(iter))<<iter;
        i1++;
    }
    ASSERT_EQ(i1,expected1.size()+1);
}
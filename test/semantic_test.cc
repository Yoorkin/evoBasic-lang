#include <gtest/gtest.h>
#include "semantic.h"
#include "type.h"

using namespace evoBasic;
using namespace evoBasic::type;

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
//    Dependencies<Field> inf;
//    /*
//     *  let final = a + b + begin
//     *  let a = b + begin
//     *  let b = begin - 100
//     *  let begin = 50
//     */
//    auto a = make_shared<Field>(false);
//    a->setName("a");
//    auto b = make_shared<Field>(false);
//    b->setName("b");
//    auto begin = make_shared<Field>(Semantics::Instance()->getBuiltIn().getIntegerPrototype(),false);
//    begin->setName("begin");
//    auto final = make_shared<Field>(false);
//    final->setName("final");
//
//    inf.addDependencies(a,{b,begin});
//    inf.addDependencies(b,{begin});
//    inf.addDependencies(final,{a,b,begin});
//
//    auto succeed = inf.solve();
//    list<shared_ptr<Field>> order = inf.getTopologicalOrder();
//
//    ASSERT_TRUE(succeed);
//    ASSERT_EQ(order.size(),3);
//
//    auto iter = order.begin();
//    ASSERT_EQ((*iter)->getName(),b->getName());
//    iter++;
//    ASSERT_EQ((*iter)->getName(),a->getName());
//    iter++;
//    ASSERT_EQ((*iter)->getName(),final->getName());
}

TEST(TypeInference,TopoSort_Failed){
//    Dependencies<type::Variable> inf;
//    /*
//     *  let final = a + b + begin
//     *  let a = b + begin
//     *  let b = begin - 100
//     *  let begin = 50
//     *  let x = y + 1
//     *  let y = z + 1
//     *  let z = x + 1
//     */
//    auto a = make_shared<Field>(false);
//    a->setName("a");
//    auto b = make_shared<Field>(false);
//    b->setName("b");
//    auto begin = make_shared<Field>(Semantics::Instance()->getBuiltIn().getIntegerPrototype(),false);
//    begin->setName("begin");
//    auto final = make_shared<Field>(false);
//    final->setName("final");
//
//    auto x = make_shared<Field>(false);
//    x->setName("x");
//    auto y = make_shared<Field>(false);
//    y->setName("y");
//    auto z = make_shared<Field>(false);
//    z->setName("z");
//
//    auto y1 = make_shared<Field>(false);
//    y1->setName("y1");
//    auto y2 = make_shared<Field>(false);
//    y2->setName("y2");
//
//    inf.addDependencies(a,{b,begin});
//    inf.addDependencies(b,{begin});
//    inf.addDependencies(final,{a,b,begin});
//
//    inf.addDependencies(x,{y});
//    inf.addDependencies(y,{z});
//    inf.addDependencies(z,{x});
//
//    inf.addDependencies(y,{y1});
//    inf.addDependencies(y1,{y2});
//    inf.addDependencies(y2,{y});
//
//    auto succeed = inf.solve();
//    list<shared_ptr<Field>> order = inf.getTopologicalOrder();
//
//    ASSERT_FALSE(succeed);
//    ASSERT_EQ(order.size(),3);
//
//    auto iter = order.begin();
//    ASSERT_EQ((*iter)->getName(),b->getName());
//    iter++;
//    ASSERT_EQ((*iter)->getName(),a->getName());
//    iter++;
//    ASSERT_EQ((*iter)->getName(),final->getName());
//
//    auto circles = inf.getCircles();
//    ASSERT_EQ(circles.size(),2);
//
//    set<string> expected1 = {"x","y","z"};
//    int i1=0;
//    for(auto& iter:circles.front()){
//        ASSERT_TRUE(expected1.contains(iter->getName()))<<iter->getName();
//        i1++;
//    }
//    ASSERT_EQ(i1,expected1.size()+1);
//
//    set<string> expected2 = {"y1","y","y2"};
//    int i2=0;
//    for(auto& iter:circles.back()){
//        ASSERT_TRUE(expected2.contains(iter->getName()))<<iter->getName();
//        i2++;
//    }
//    ASSERT_EQ(i2,expected1.size()+1);
}
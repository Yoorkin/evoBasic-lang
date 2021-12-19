#include <gtest/gtest.h>
#include <memory>
#include "semantic/semantic.h"
#include "type.h"
#include "lexer.h"
#include "parser.h"
#include "logger.h"
#include "dependencies.h"
#include "context.h"

using namespace evoBasic;
using namespace evoBasic::type;
using namespace std;


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

void IS_CODE_NO_ERROR(string code){
    Logger::errorCount = 0;
    stringstream stream;
    Logger::redirect(&stream);
    Logger::debugMode = false;
    Lexer lexer(new StringSource(code));
    Parser parser(&lexer);
    auto ast = parser.parseGlobal();
    Logger::debugMode = true;
    Context context;
    Semantic::collectSymbol(ast,&context);
    Semantic::collectDetail(ast,&context);
    Semantic::typeCheck(ast,&context);
    Logger::dev(debugParseTree(ast));
    Logger::dev(context.getGlobal()->debug(0));
    ASSERT_TRUE(Logger::errorCount == 0)<<stream.str();
}

TEST(SelfRefference,GetSelfImplicit){
    auto code = R"CODE(
Class MyClass
    Dim num as integer
    Function Get() as Integer
        return num
    End Function
End Class
)CODE";
    IS_CODE_NO_ERROR(code);
}
TEST(SelfRefference,GetSelfExplicit){
    auto code = R"CODE(
Class MyClass
    Dim num as integer
    Function Get() as Integer
        return Self.num
    End Function
End Class
)CODE";
    IS_CODE_NO_ERROR(code);
}


TEST(DotExpr,Case1){
    auto code = R"CODE(
Type MyType
    member as Integer
End Type
Enum MyEnum
    Member1
End Enum
Module MyModule
    Public Class MyClass
        Public Static Function MyStaticFunc(Byval a as MyEnum,b as Integer) As MyType

        End Function
    End Class
End Module
Sub Main()
    let typeVar as MyType
    let ans as integer
    ans = MyModule.MyClass.MyStaticFunc(MyEnum.Member1,typeVar.member).member
End Sub
)CODE";
    IS_CODE_NO_ERROR(code);
}

TEST(DotExpr,Case2){
    auto code = R"CODE(
Module MyModule
    Public Type MyType
        member as Integer
    End Type

    Public Enum MyEnum
        Member1
    End Enum

    Public Class MyClass
        Public Static StaticEnumVar as MyEnum
        Public Num as Integer
        Public Function GetNum() As Integer
            Return 233
        End Function
    End Class

    Public Function MyFunction() as MyClass

    End Function

    Public Sub Test(Byval clsArg as MyClass)
        let a as MyModule.MyType,
            b as MyModule.MyEnum = MyClass.StaticEnumVar,
            c as Integer = MyModule.MyFunction().GetNum() + clsArg.GetNum() + clsArg.Num
    End Sub
End Module
)CODE";
    IS_CODE_NO_ERROR(code);
}

TEST(DotExpr,Case3){
    auto code = R"CODE(
Module MyModule
    Interface MyInterface
        Sub Test()
    End Interface
End Module
Class Foo Impl MyModule.MyInterface
    Public Sub Test()

    End Sub
End Class
)CODE";
    IS_CODE_NO_ERROR(code);
}

TEST(DotExpr,Case4){
    auto code = R"CODE(
Module MyModule
    dim myClassVar as MyClass
End Module
Class MyClass
    Public num as Integer
    Public Function MyNonStaticFunc(Byval a as integer,b as integer) as MyClass[4]

    End Function
End Class
Sub Main()
    let a as MyClass,b as Integer[3],c as integer
    c = a.MyNonStaticFunc(b[1],a.num)[2].num
End Sub
)CODE";
    IS_CODE_NO_ERROR(code);
}

TEST(DotExpr,Case5){
    auto code = R"CODE(
Class MyClass
    Public array as Integer[4],id as integer
    Public Function getID() as integer
        return id
    End Function
    Public Function GetNum() As Integer
        Return 233
    End Function
    Public Function MyFunc(Byval a as integer) as Integer
        return a
    End Function
    Public Sub InClassSub(Byval clsArgs as MyClass[10],Byref b as MyClass)
        Self.id = Self.MyFunc(clsArgs[5].getNum())
        let local = b.getID() + b.id + b.array[1]
    End Sub
End Class
)CODE";
    IS_CODE_NO_ERROR(code);
}
#include <gtest/gtest.h>
#include "compileHelper.h"
using namespace evoBasic;
using namespace evoBasic::vm;

//#define COMPARE \
//    auto gen_block = ir->findFunctionBlock("test");\
//    stringstream str,str2;\
//    gen_block->toString(str);\
//    block->toString(str2);\
//    ASSERT_TRUE(is_block_eq(gen_block,block))\
//    <<"Test:\n"<<str2.str()<<"\n"\
//    <<"Gen:\n"<<str.str();
//
//
//TEST(CodeGen,PassByValPrimitive){
//    GTEST_SKIP();
//    auto ir = compile(R"code(
//        Function Test(Byval a as integer,Byval b as byte,Byval c as long) as long
//
//        End Function
//    )code");
//    auto block = new BasicBlock("test");
//    block->PushFrameBase()
//        .Push(Data::ptr,new Const<data::ptr>(5))
//        .Add(Data::ptr)
//        .StoreR(Data::i64)
//
//        .PushFrameBase()
//        .Push(Data::ptr,new Const<data::ptr>(4))
//        .Add(Data::ptr)
//        .StoreR(Data::i8)
//
//        .PushFrameBase()
//        .Push(Data::ptr,new Const<data::ptr>(0))
//        .Add(Data::ptr)
//        .StoreR(Data::i32)
//
//        .Ret();
//
//    COMPARE
//}
//
//TEST(CodeGen,PassByValPrimitive_And_Plus_100_50){
//    GTEST_SKIP();
//    auto ir = compile(R"code(
//        Function Test(Byval a as integer,Byval b as integer) as long
//            b = 100 + 50
//        End Function
//    )code");
//    auto block = new BasicBlock("test");
//    block->PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(4))
//            .Add(Data::ptr)
//            .StoreR(Data::i32)
//
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(0))
//            .Add(Data::ptr)
//            .StoreR(Data::i32)
//
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(4))
//            .Add(Data::ptr)
//
//            .Push(Data::i32,new Const<data::i32>(100))
//            .Push(Data::i32,new Const<data::i32>(50))
//            .Add(Data::i32)
//            .Store(Data::i32)
//            .Ret();
//    COMPARE
//}
//
//
//TEST(CodeGen,PassByValPrimitive_And_Load){
//    GTEST_SKIP();
//    auto ir = compile(R"code(
//        Function Test(Byval a as integer,Byval b as integer) as long
//            b = a + 50
//            b = a
//        End Function
//    )code");
//    auto block = new BasicBlock("test");
//    block->PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(4))
//            .Add(Data::ptr)
//            .StoreR(Data::i32)
//
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(0))
//            .Add(Data::ptr)
//            .StoreR(Data::i32)
//
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(4))
//            .Add(Data::ptr)
//
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(0))
//            .Add(Data::ptr)
//            .Load(Data::i32)
//
//            .Push(Data::i32,new Const<data::i32>(50))
//            .Add(Data::i32)
//
//            .Store(Data::i32)
//
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(4))
//            .Add(Data::ptr)
//
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(0))
//            .Add(Data::ptr)
//            .Load(Data::i32)
//            .Store(Data::i32)
//            .Ret();
//    COMPARE
//}
//
//TEST(CodeGen,PassByValType){
//    GTEST_SKIP();
//    auto ir = compile(R"code(
//        Type MyType  //size 8
//            m1 as integer
//            m2 as integer
//        End Type
//
//        Type MyType2  //size 5
//            m3 as Byte
//            m4 as integer
//        End Type
//
//        Function Test(ByVal a as MyType2,ByVal b as MyType) as long
//
//        End Function
//    )code");
//    auto block = new BasicBlock("test");
//    block->PushFrameBase()
//        .Push(Data::ptr,new Const<data::ptr>(5))
//        .Add(Data::ptr)
//        .StmR(8)
//        .PushFrameBase()
//        .Push(Data::ptr,new Const<data::ptr>(0))
//        .Add(Data::ptr)
//        .StmR(5)
//        .Ret();
//
//    COMPARE
//}
//
//TEST(CodeGen,PassByValType_Assign_Value){
//    GTEST_SKIP();
//    auto ir = compile(R"code(
//        Type MyType  //size 8
//            m1 as integer
//            m2 as integer
//        End Type
//
//        Type MyType2  //size 5
//            m3 as Byte
//            m4 as integer
//        End Type
//
//        Function Test(ByVal a as MyType,ByVal b as MyType2) as long
//            a.m1 = 50
//            b.m4 = 100
//        End Function
//    )code");
//    auto block = new BasicBlock("test");
//    block->PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(8))
//            .Add(Data::ptr)
//            .StmR(5)
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(0))
//            .Add(Data::ptr)
//            .StmR(8)
//
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(0))
//            .Add(Data::ptr)
//            .Push(Data::ptr,new Const<data::ptr>(0))
//            .Add(Data::ptr)
//            .Push(Data::i32,new Const<data::ptr>(50))
//            .Store(Data::i32)
//
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(8))
//            .Add(Data::ptr)
//            .Push(Data::ptr,new Const<data::ptr>(1))
//            .Add(Data::ptr)
//            .Push(Data::i32,new Const<data::ptr>(100))
//            .Store(Data::i32)
//            .Ret();
//
//    COMPARE
//}
//
//TEST(CodeGen,PassByValTypeArray_Load_And_Store){
//    GTEST_SKIP();
//    auto ir = compile(R"code(
//        Type MyType  //size 8
//            m1 as integer
//            m2 as integer
//        End Type
//
//
//        Function Test(ByVal a as MyType,ByVal b as Integer[5]) as long
//            a.m2 = b[3] + 1
//            b[4] = a.m1 + 2
//        End Function
//    )code");
//    auto block = new BasicBlock("test");
//    block->PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(8))
//            .Add(Data::ptr)
//            .StmR(20)
//            .PushFrameBase()
//            .Push(Data::ptr,new Const<data::ptr>(0))
//            .Add(Data::ptr)
//            .StmR(8)
//            .Ret();
//
//    COMPARE
//}
//
//
//TEST(CodeGen,PassByValClass){
//
//}
//
//TEST(CodeGen,PassByRefPrimitive){
//
//}
//
//TEST(CodeGen,PassByRefTypeArray){
//
//}
//
//TEST(CodeGen,PassByRefClass){
//
//}
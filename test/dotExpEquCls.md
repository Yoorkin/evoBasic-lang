# 点运算符表达式测试

## 可参与点运算符的类型
1.按符号类型划分
- Module
- Class
- Type
- Interface
- Enum
- EnumMember
- Invoke
- Variable

2.按是否静态划分
- Static
- NonStatic

3.按是否引用划分
- Byval-Argument
- Byref-Argument

4.按是否解引用数组划分
- [Deref-Array]

5.按变量位置划分
- Local-Variable
- Module-Variable
- Instance-Variable

6.`Self`关键字

## 类型检查中的等价类划分

### 有效等价类

1. Module.Class 
2. Module.Type 
3. Module.Interface 
4. Module.Enum 
5. Module.Invoke 
6. Module.Variable 
7. Class.Variable 
8. Class.Invoke 
9. Enum.EnumMember 
10. Invoke.Invoke 
11. Invoke.Variable 
12. Invoke[Deref-Array]
13. Variable.Invoke
14. Variable.Variable 
15. Variable[Deref-Array]
16. Static.Static 
17. NonStatic.NonStatic 
18. Byval-Argument.Invoke
19. Byval-Argument.Variable
20. Byval-Argument[Deref-Array]
21. Byref-Argument.Invoke
22. Byref-Argument.Variable
23. Byref-Argument[Deref-Array]
24. Local-Variable.X
25. Module-Variable.X
26. Instance-Variable.X
27. Self.Variable
28. Self.Invoke
29. [Deref-Array].Variable
30. [Deref-Array].Invoke

|测试用例|伪代码|覆盖等价类|
|--|--|--|
|1|`MyModule.MyClass.MyStaticFunc(MyEnum.Member1,MyTypeVar.Field1).RetTypeField`|1,8,9,11,14,16|
|2|`Sub Test(Byval clsArg as MyClass) let a as MyModule.MyType,b as MyModule.MyEnum = MyClass.StaticEnumVar,c = MyModule.MyFunction().GetID() + clsArg.GetNum() + clsArg.num End Sub`|2,4,5,7,10,18,19|
|3|`Class Foo implement MyModule.MyInterface`|3|
|4|`MyModule.MyClassVar.MyNonStaticFunc(myLocalArrayVar[1],MyClassVar.Field1)[5].myField`|6,12,13,15,17,24,25,26,29|
|5|`Sub InClassSub(Byval clsArgs as MyClass[10],ByRef b as MyClass) Self.id = Self.MyFunc(MyClass[5].getNum()) let local = b.getID() + b.id + b.array[1] End Sub`|20,21,22,23,27,28,30|

### 无效等价类

1. Static.NonStatic-Variable
1. Static.NonStatic-Function
2. NonStatic.Static-Variable
3. NonStatic.Static-Function



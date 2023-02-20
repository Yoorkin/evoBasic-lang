# evoBasic 使用说明
欢迎把玩evoBasic！这是一个以学习和实践编译原理和虚拟机设计与实践的项目。该项目目前实现和未实现的功能如下：

1. 编译器
- [ ] 原始类型如：无符号整形(Byte)、有符号整型(Short,Integer,Long)、浮点(Single,Double)、布尔型(Boolean)、以及存储Unicode codepoint的Rune。 
- [ ] 原始类型的隐式转换与装箱
- [x] 用户定义Type类型(类似Struct)
- [ ] 枚举类型
- [ ] 数组
- [ ] 字符串
- [ ] 前向引用
- [ ] 控制流程语句： if..then..else, while..wend, for..next, return 等
- [ ] 函数：Function与Sub、参数按值(Byval)或地址(Byref)传递语义、可选参数(Optional)、变长参数(ParamArray)
- [ ] 运算符与表达式：+, -, *, /, mod, and, or, xor, not
- [x] 运算符重载
- [ ] 模块化：Module、二进制包生成、二进制包导入(import)、可见性控制(Public和Private)
- [ ] 面向对象特性：类、继承、虚函数、多态
- [x] 接口
- [ ] 错误处理：throw、try..catch语句
- [x] 函数作为参数传递
- [x] 可选参数默认值
- [x] 局部变量类型推导

编译器目前无任何代码优化。

2. 虚拟机
- [ ] 二进制包加载和执行
- [x] 二进制包兼容性检查
- [ ] 标记-清扫式GC
- [x] 分代GC
- [ ] 错误处理与部分安全(空指针、下标越界等)检查
- [x] 外部函数调用

## evoBasic原理概览

```
    +---------------------+
    | 源码和二进制包依赖     |
    +---------------------+
            |
            | 输入
            V
    +----------------+   编译生成字节码     +-------------+   
    |  编译器ebc      |  --------------->  |   .bkg文件  | 
    +----------------+                    +-------------+
                                               | 输入执行
                                               V
                                         +-------------+
                                         |  虚拟机 evm  |
                                         +-------------+
```

## 示例：选择排序
1. 创建文本文件test.eb，输入如下内容：

```
Sub Swap(a As Integer, b As Integer)
	Dim t As Integer
	t = a
	a = b
	b = t
End Sub

Sub SelectionSort(Byval arr As Integer[])
	Dim A As Integer
	For Dim i = 0 To arr.Length()-1
		Dim m As Integer
		m = i
		For Dim j = i+1 To arr.Length()-1
			If arr[m] > arr[j] then m = j
		Next
		Swap(arr[i], arr[m])
	Next
End Sub

Sub PrintArray(Byval ls As Integer[])
	For Dim i = 0 To ls.length()-1
		Print(ls[i])
	Next
	Println(";")
End Sub

Sub Main()
	Dim a As Integer[]
	a = [9,8,7,6,5,4,3,2,1]
	SelectionSort(a)
	PrintArray(a)
End Sub

```

2. 使用ebc编译test.eb。 

注意目前需要使用-p选项手动添加包依赖文件夹。请确保这个文件夹下包含builtin.bkg

```
    ebc test.eb -p C:/Path/To/Bkg/Folder
```

3. 使用evm运行output.bkg

```

```

下面是evoBasic的详细说明。









----
# 一般规则

- **evoBasic是大小写不敏感的**
- **evoBasic语法严格定义换行**
- 注释的语法是 `/* 注释 */` 和 `//注释`

# 1 语句

## 1.1 变量定义语句

```
    Dim a As Integer, b as Long
```

## 1.2 流程控制语句
### 1.2.1 选择结构

```
    if exp1 then 
       ...
    else if exp2 then
       ...
    else
       ...
    end if
```

```
    if exp then statement1() else statement2() 
```

```
    if exp then statement1() 
```

目前不支持Select语句。

### 1.2.2 循环结构

```
    while exp
        ...
    wend
```

```
    Dim i As Integer
    For i = 0 To 100 Step 2
        ...
    Next
```

```
    //for循环的迭代变量只能是Integer类型
    For Dim i = 100 To 0 
        ...
    Next
```

### 1.2.3 异常抛出与处理语句

```
    Sub Main()
        Try
            Test()
        Catch a As OutOfRangeException //下标越界异常
            ...
        Catch a As ConversionException //转换异常
            ...
        End Try
    End Sub

    Sub Test()
        Throw New OutOfRangeException(10,5)
    End Sub
```

### 1.2.4 其他流程控制语句

|语句|描述|
|-|-|
|`Continue`|跳过一次当前循环|
|`break`|退出当前循环|
|`Exit Sub`|从Sub中返回|
|`Exit For`|退出For...Next循环|
|`Exit Loop`|退出While...Wend循环|
|`Return 表达式`|返回值exp|


# 2 表达式

# 2.1 运算符优先级

|运算符|描述|优先级（越小的优先级越高）|
|-|-|-|
|`a(b,c,d...)`|函数调用|1|
|`a[b]`|下标索引|1|
|`a Is TypeName`|类型判断|1|
|`a As TypeName`|类型转换|1|
|`a.b`|成员寻找|1|
|`New TypeName(a,b,c...)`|类实例化|1|
|`Optional a,b,c...`|可选参数测试|1|
|`-a`|负数|2|
|`+a`|正数|2|
|`*`, `/`, `Mod`|乘除、取模|3|
|`+`, `-` |加减|4|
| `>`,`>=`,`==`,`<>`,`=<`,`<`|大于、大于等于、等于、不等于、小于等于、小于|5|
|`Not`|非运算|6|
|`And`, `Or`, `Xor`|和、或、异或|7|
|`a = b`|赋值|8|

evobasic并没有定义运算符和优先级的功能，且运算符功能与其他命令式语言相似，因此只对部分运算符的功能作详细介绍。

- 类型判断运算符

```
    a Is TypeA
```
判断实例a类型是否为TypeA，返回值为Boolean。

- 类型转换运算符

```
    a As TypeB
```
将实例a转换为TypeB并返回。如果无法进行转换，则产生`ConversionException`异常。

- 可选参数测试运算符

```
    Optional a
```
判断函数调用者是否提供了可选参数a的实参。返回值为Boolean类型。

```
    Optional a,b,c
```
判断函数调用者是否提供了可选参数a,b,c的实参。返回值为Boolean类型。

- 赋值运算符

```
    a = b
```
将b的值赋给a，然后返回a的值。

## 2.2 字面量

### 2.2.1 字符串字面量和字符字面量

evoBasic使用`""`定义字符串字面量,包含任意数量的字符，`''`定义字符字面量，包含一个unicode codepoint。

字符转义使用与c语言类似的方式，来在字符串字面量和字符字面量中表示特殊字符。

|转义字符|描述|
|-|-|
|`\n`|换行|
|`\r`|回车|
|`\t`|制表符|

有关字符处理的更多信息，见类型、内置库字符串章节。

### 2.2.2 数组字面量

使用`[exp1,exp2,exp3]`表示的任意个元素。返回数组类型为`Object[]`。


### 2.2.3 其他字面量

|字面量|可能的值|类型|
|-|-|-|
|布尔|True,false|Boolean|
|整型字面量|1, 2, 3 ....|Integer|
|浮点字面量|1.1, 3.14, ...|Double|
|空对象|Nothing||

# 3 类型

## 3.1 原始类型

|原始类型|描述|
|-|-|
|Byte|8位无符号整型|
|Short|16位有符号整型|
|Integer|32位有符号整型|
|Long|64位有符号整型|
|Single|32位单精度浮点|
|Double|64位双精度浮点|
|Boolean|布尔类型|
|Rune|32位无符号整型，表示Unicode的一个码点（codepoint）|

## 3.2 枚举类型
定义一系列的枚举，它可以有一个32位无符号整型的值。
```
    enum MyEnum
        MyValO   // 0
        MyVal1 = 100
        MyVal2 = 50
        MyVal3   // 51
    end enum
```
## 3.3 记录类型（未实现）
带名字的积类型。功能等同c语言的struct。
```
    Type MyType
        FieldA As Integer
        FieldB As Boolean
    End Type
```
不支持合类型以及匿名积类型(Tuple)。

## 3.4 类类型

```
    Class A Extend B
        ...
    End Class
```
evobasic支持继承。当省略Extend B时，默认继承Object。
### 3.4.2 静态成员和静态字段

该特性与c#相似。
```
    Class MyClass
        Public Static Sub MyStaticSub()

        End Sub

        Private Static A As Integer
    End Class
```

### 3.4.3 构造函数

```
    Class Layout
        Public New(Byval Width As Integer, Byval Height As Integer)
            ...
        End New
    End Class

    Class Derived Extend Base
        Public New(Byval Label As String, Byval Width As Integer, Byval Height As Ineger) Extend(Width,Height)
            ...
        End New
    End Class
```

构造函数是实例化用户定义类的窗口。编译器不会提供类的默认构造函数，当一个类没有声明构造函数时，它就无法实例化。
构造函数的参数列表后的Extend(a,b,c...)是构造函数委托，语法与函数调用相似，它将参数转发给基类的构造函数；基类的构造函数在子类的构造函数之前执行。当基类直接为Object时，构造函数委托可以省略。

evobasic不支持重载，一个类只能拥有一个构造函数。如果要表达“从不同的输入构造实例”的抽象，请使用可选参数。如果在不同的输入构造时，几个可选参数之间的选择存在组合和互斥关系，请将构造函数设为私有，额外包装几个静态函数去调用它。

例如，刻画一个文件对象，该文件对象能通过以下途径构造：
* 从输入Byte[]读取文本并储存
* 从路径Path指向的本地文件读取
* 从网络路径NetPath指向的空间下载
* 新建一个空文件

下面使用可选参数刻画文件对象：

```
    Class MyFile
        Public New(Optional Byval data As Byte[], Optional Byval Path As String, Optional NetPath As String, Optional Byval Empty As Boolen)
            If Optional data Then

            Else If Optional Path Then

            Else if Optional NetPath Then

            Else if Optional Empty Then

            Else
                // throw Error
            End if
        End New
    End Class
```
缺点是显而易见的，几个可选参数的输入具有互斥关系，即，一个文件既不可能又是从本地加载，又是新建未保存的空文件。而这个构造函数却能够同时接受所有可选的输入。

下面通过将构造函数隐藏，使用静态函数包装的方式实现这种互斥的输入关系。

```
    Class MyFile
        Private New(Optional Byval data As Byte[], Optional Byval Path As String, Optional NetPath As String, Optional Byval Empty As Boolen)
            ...
        End New

        Public Static Function FromPath(Byval Path As String) As MyFile
            Return New MyFile(Path: Path)
        End Function

        Public Static Function FromWeb(Byval NetPath As String) As MyFile
            Return New MyFile(NetPath: NetPath)
        End Function

        Public Static Function FromBytes(Byval data As Bytes) As MyFile
            Return New MyFile(data: data)
        End Function

        Public Static Function Create() As MyFile
            Return New MyFile(Empty: True)
        End Function
    End Class
```

### 3.4.4 虚函数与多态

```
    Class Animal
        Public Virtual Function Bark() As String
            Throw New NotImplementedException("Animal.Bark not implemented")
        End Function
    End Class

    Class Dog Extend Animal
        Public Override Function Bark() As String
            Return "汪"
        End Funtion
    End Class

    Class Cat Extend Animal
        Public Override Function Bark() As String
            Return "喵"
        End Function
    End Class
```

### 3.4.5 特殊处理的类

一些类虽然使用用户代码实现在builtin.bkg当中，但是会被编译器特殊对待。

- Object

Object类是evobasic的顶层类型（top type），也即，所有的类型都可以转换成Object。Object不继承任何类。

|公开成员||
|-|-|
|`Virtual Function ToString() As String`|序列化成字符串并返回|

- Array

Array继承了Object。所有数组都从Array派生。

|公开成员||
|-|-|
|`Function Length() As Integer`|返回数组长度，即元素数量|


## 3.5 数组类型

数组是一种特殊的类，它的大小一经创建就是不可更改的。

声明接受一个Integer数组

```
Dim a As Integer[]

Sub Foo(Byval b As Integer[])

End Sub
```
创建一个数组，指定大小

```
    Dim a As Integer[]
    a = New Integer[6]
    Print(a.Length()) //输出6
```
使用Array类型变量存储任意数组，并通过Is表达式判断类型。

```
	Dim a As Array
	a = New Integer[5]
	Print(a is Integer[]) //输出True
```

## 3.6 函数类型
- 无返回值函数
```
    Sub MyFunc(Byval Input As Integer, ParamArray Byval xs As Object[])
        ...
    End Sub
```
- 有返回值函数
```
    Function MyFunc(Byval Input As Integer, Optional Byval a As Long) As Integer
        ...
    End Function
```
不支持函数重载。


### 3.6.1 函数参数的求值策略

与vb相同, 函数参数有两种求值策略Byval和Byref，分别为按值传递和按址传递。
evobasic没有指针，通过Byref可以代替部分指针的功能，配合Class可以模拟二级指针的部分功能。

### 3.6.2 可选参数与变长参数列表

可选参数表示该参数是可缺省的。通过Optional表达式可以测试参数是否被调用方提供。与vb不同，在传递可选时必须指定参数名，如下

```
    Sub Foo(Byval n1 As Integer, Byval n2 as Integer, Optional Byval a As Integer, Optional b As Integer)
        if optional a,b then
            ...
        end if
    end sub

    Sub Main()
        // Foo(1,2,3,4)         错误
        Foo(1,2, a: 3, b: 4)  //正确
    End Sub
```

变长参数列表是类型为Object[]的数组, 求值策略必须为Byval，且只能声明在参数列表的最后。它允许函数调用时传递任意数量的参数；当然，直接传递一个Object[]数组作为整个变长参数列表的值也是允许的。

```
    Sub Foo(Byval a As String, ParamArray Byval arr As Object[])
        ...
    End Sub

    Sub Main()
        Foo("第一次输入:",1,2,3,4,5,6)

        Dim arr As Object[]
        arr = [1,2,3,4,5,6]
        Foo("等效的输入:", arr)
    End Sub
```

# 4 隐式转换与装箱

## 4.1 类型提升

类似于c语言的整型提升规则，evoBasic将此种规则从整型推广到了浮点类型。
两种类型A,B的值a,b参与二元运算时，将a,b隐式转换到类型C后再进行运算。
表格中行是类型A，列为类型B。表格中的内容为转换后的类型C。如果内容为空，则表示两种类型无法进行二元运算。

|B\A 二元运算|Boolean |Byte |Short |Rune |Integer |Long |Single |Double |
|-|-|-|-|-|-|-|-|-|
|Boolean|||||||||
|Byte|||Integer||Integer|Long|Double|Double||
|Short||Integer|||Integer|Long|Double|Double||
|Rune||||||||||
|Integer||Integer|Integer|||Long|Double|Double||
|Long||Long|Long||Long||Double|Double||
|Single||Double|Double||Double|Double|Double|||
|Double||Double|Double||Double|Double|Double||

## 4.2 截断

当表示范围更宽的类型A的值a，传递给范围更窄的类型B中存储时，将发生隐式转换。
行是类型B，列为类型A。

|A\B是否转换|Byte|Short|Integer|Long|
|-|-|-|-|-|
|Byte||||
|Short|是|||
|Integer|是|是|||
|Long|是|是|是||

## 4.3 变宽 (widening)
|B\A是否转换|Byte|Short|Rune|Integer|Long|
|-|-|-|-|-|-|
|Byte||是||是|是|
|Short||||是|是|
|Rune||||||
|Integer|||||是|
|Long||||||

## 4.4 浮点类型转换
```
        int[][] floatingPoint = {/*Boolean Byte Short Rune Integer Long Single Double Object*/
            /*Boolean*/new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  N,     N,     NA },
            /*Byte*/   new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  Y,     Y,     NA },
            /*Short*/  new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  Y,     Y,     NA },
            /*Rune*/   new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  N,     N,     NA },
            /*Integer*/new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  Y,     Y,     NA },
            /*Long*/   new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  Y,     Y,     NA },
            /*Single*/ new int[]{   N,      Y,    Y,   N,   Y,     Y,   NA,    Y,     NA },
            /*Double*/ new int[]{   N,      Y,    Y,   N,   Y,     Y,   Y,     NA,    NA },
            /*Object*/ new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  NA,    NA,    NA }
        };
```
## 4.5 原始类型装箱

关于装箱是什么，参考 https://docs.microsoft.com/zh-cn/dotnet/csharp/programming-guide/types/boxing-and-unboxing

以下情况将发生原始类型装箱：
1. 将原始类型传递给Object类型参数
2. 将原始类型赋值给Object类型变量
3. 将原始类型从返回值为Object的函数返回
4. 将原始类型显式地转换成Object

通过显式的As表达式转型，可以进行拆箱。

```
Function Foo(Byval A As Object) As Object
    Return 5 //装箱
End Function

Sub Main()
    Dim a As Object
    a = Foo(3.14) //装箱
    If a Is Integer Then
        Dim num As Integer
        num = a As Integer //拆箱
        Println(a) //装箱，因为Println函数的参数是Object类型
    End If
End Sub

```


# 5 内置库

## 5.1 字符串

字符串类型String是类的一种，定义在builtin包中。内部通过UTF32编码，使用Rune存储，一个Rune对应一个unicode codepoint。

```
    Dim str As String
    Dim a As Rune, b As Rune, c As Rune
    str = “你好世界”
    a = 'y'
    b = '1'
    c = '蛤'
```

在代码中，字符串字面量会在编译期间转换成String。因此这样也是合法的：

```
    Dim length As Integer
    length = "你好世界".length()  //length为4
```
String被设计为不可变的。所有修改操作都会返回一个新的String。

|方法|描述|
|-|-|
|`New String(Byval Sequence As Rune[])`|从Unicode码点数组构造String|
|`s.Length()`|返回String中Rune的数量|
|`s.IndexGet(Byval i As Integer)`|获取第i个Rune|
|`s.ToString()`|返回它自身|
|`s.Concat(Byval str As String)`|返回它与str拼接后的字符串|
|`s.Append(Byval r As Rune)`|返回它与r拼接后的字符串|
|`String.Fold(Byval ls As String[], Optional Byval Separator As Rune)`|将字符串数组拼接，以可选的Rune插入到每个元素之间，返回拼接后的字符串。|

## 5.2 输出函数

evoBasic目前没有输入函数。由于暂不支持外部函数调用，因此目前输出函数是通过虚拟机内置函数实现的。

```
    Println("你好","世界！")
    PrintFmt("第{}次输出:{}\n", 2, "不是字符串也可以直接打印")
    Print(1.1, 2.2, 3.3, 4.4, 5.5, 6.6)
```

|函数|描述|
|-|-|
|`Sub Print(ParamArray Byval xs As Object[])`|打印任意数量的对象，在每个元素之间插入空格|
|`Sub Println(ParamArray Byval xs As Object[])`|如上，并在尾部追加换行|
|`Sub PrintFmt(Byval fmt As String, ParamArray Byval args As Object[])`|格式化输出|

以上三个打印函数也是在builtin.bkg包中通过用户代码实现的，**而非在编译器中硬编码的特殊规则！尽可能少地硬编码是一种良好的设计取向，使得用户能够更多地调整软件的行为。不要说语言的设计方式与最终用户无关，只要最终用户依赖的的包中有任何一个因此获得更好的抽象和性能，那么最终用户也能间接因此受益。** 当然，目前evobasic中存在很多违反这种原则的设计。这并非我在对待这件事上双重标准，而是在实践这个原则的道路上，我的能力和时间不足导致的。关于实现细节，请看Object.ToString()、虚函数覆写、原始类型装箱和虚拟机内置函数。

## 5.3 Text模块

|函数|描述|
|-|-|
|`Function Format(Byval fmt As String,ParamArray Byval args As Object[]) As String`|类似PrintFmt，不同的是将结果字符串返回而非直接打印|

## 5.4 List类

与数组不同的是，List的空间是可变的。当List空间不够时，自动扩大List的容量。

|成员|描述|
|-|-|
|`New(ParamArray Byval xs As Object[])`|从Object[]数组构造列表|
|`Function IndexGet(Byval i As Integer) As Object`|获得第i个元素|
|`Sub IndexSet(Byval i As Integer,Byval value As Object)`|将对象放置在第i个空间|
|`Function Length() As Integer`|获得元素数量|
|`Function Capacity() As Integer`|获得列表当前的容量|
|`Sub Add(Byval Value As Object)`|加入一个元素到末端|
|`Sub Expand()`|扩大列表容量，每次扩大两倍|
|`Override Function ToString() As String`|序列化列表成字符串|

## 5.5 Stack类

栈，后进先出的数据结构。

|成员|描述|
|-|-|
|`New()`|实例化|
|`Function Pop() As Object`|弹出一个元素|
|`Sub Push(Byval Value As Object)`|推入一个元素|
|`Function Empty() As Boolean`|判断是否为空|

## 5.6 LinkedList & LinkedNode

链表和链表节点。
|LinkedNode成员|描述|
|-|-|
|`New()`||
|`Dim Previous As LinkedNode`||
|`Dim Succeed As LinkedNode`||
|`Dim Value As Object`||

|LinkedList成员|描述|
|-|-|
|`New()`||
|`Function Length() As Integer`||
|`Sub Add(Byval Value As Object)`||
|`Sub PushBack(Byval Value As Object)`||
|`Sub PushFront(Byval Value As Object)`||
|`Sub RemoveBack()`||
|`Sub RemoveFront()`||
|`Function Back() As Object`||
|`Function Front() As Object`||
|`Function FrontNode() As LinkedNode`||
|`Function BackNode() As LinkedNode`||

## 5.7 异常类
Exception类是所有异常类的基类，成员如下
|成员|描述|
|-|-|
|`New(Byval Name As String, Byval Msg As String)`|构造函数|
|`Function GetMessage() As String`|获得构造时输入的Msg|
|`Override Function ToString() As String`|序列化成字符串|
|`Function PrintTrace() As String`|打印堆栈信息|

其他异常类

|异常|描述|
|-|-|
|`NotImplementedException`||
|`OutOfRangeException`||
|`ConversionException`||
|`NullPointerException `||
|`OptionMissingException `||
|`EvmInternalException `||
# 啥是 evoBasic

[ENG](/README.md) / 中文

EvoBasic，即“基于VB6的进化”。它是跨平台、面向对象以及强类型的玩具编程语言，目标是拥有简单的语法和现代语言的特性。
EvoBasic的代码被编译成字节码（.bkg文件），然后运行在自制的简单虚拟机上。


# 主要特性

- 封装

- 多态

- 继承

- 函数是第一公民

# 语法示例

```vb
Function Fibonacci(Byval x as Integer) As Integer
	If x == 0 Then 
		Return 0
	ElseIf x == 1 Then
		Return 1
	Else
		Return Fibonacci(x - 1) + Fibonacci(x - 2)
	End if
End Function
```

```vb
Public Module UI
    Public Enum Message 
        Update = 1
        Resize
        Hide
    End Enum

    Public Interface MessageReciver
         Function OnMessage(Byval msg as Message,Optional ByRef sender as UIObject) As Boolean
    End Interface

    Private Type Position
        x As Integer
        Y as Integer
    End Type

    Public Class Button Impl MessageReciver
        Private pos As Position
        Public Override Function OnMessage(Byval msg as Message,Optional ByRef sender as UIObject) As Boolean
            // process message...
        End Function
    End Class
End Module

Sub Main()
    Let btn = New UI.Button, msg = UI.Message.Update
    btn.OnMessage(msg)
End Sub
```
所有的语法定义，见 [grammar.bnf](/doc/grammar.bnf)。


# 项目进展

- [x] 词法分析和语法分析器
- [x] 语义检查
- [x] 中间代码生成
- [ ] 包文件 (.bkg) 生成和加载
- [ ] evoBasic 虚拟机
- [ ] 运行时

目前还没法跑任何evobasic的代码，但是可以使用`--dev-info`选项，让evoBasic的编译器在编译时输出语法树、符号表和中间表示等信息，来确定编译器是否工作。

# 如何编译

环境要求:
- clang 或者 MSVC （需要 c++20 支持）
- cmake
- git

```
    git clone https://github.com/Yoorkin/evoBasic-lang.git
    cd evoBasic-lang
    cmake -B"build" .
    cd build
    make
```

~~之前尝试写英文readme的时候一度自闭。直到今天加了这段中文版的。。太棒了！还以为只是英文水平不行，原来是中文也不太行啊~~
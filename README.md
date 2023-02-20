# What is evoBasic
ENG / [中文](/README_CN.md)

EvoBasic is a "evolution version" of classic visual basic,which aims to offer simple syntax and modern features.

It is an cross-platforms,object-oriented,strongly typed programming language, compiled to bytecode and run on evoBasic virtual machine.

[Language manual](/doc/tutorial.md)

# Features

- Data encapsulation

- Polymorphism

- Inheritance

- First-class functions

# Syntax example

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
    Dim btn As UI.Button, msg As UI.Message
    btn = New UI.Button
    msg = UI.Message.Update
    btn.OnMessage(msg)
End Sub
```
For a complete definition about syntax, see [grammar.bnf](/doc/grammar.bnf).


# Project status

- [x] lexer and parser: generate parse tree
- [x] semantic check: type check,generate symbol table and translate parse tree to abstract tree
- [x] IL generator: translate abstract tree to intermediate language
- [x] package file (.bkg) generator and loader
- [ ] package manager
- [x] evoBasic vm



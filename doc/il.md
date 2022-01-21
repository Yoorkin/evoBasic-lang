# data types

- i8 (boolean)
- i16
- i32
- i64
- u8
- u16
- u32
- u64
- f32
- f64
- ref

    reference to class-object/array/field/record

- delegate

    reference to wrapped function

- vftn

    reference to virtual function

- sftn

    reference to static function

- array

- record


# calling convention

## virtual function

..., `arg2 value`, `arg1 value`, ref, vftn, -> ..., `result1 value`, `result2 value`

## method 

..., `arg2 value`, `arg1 value`, ref, ftn, -> ..., `result1 value`, `result2 value`

## static function

..., `arg2 value`, `arg1 value`, sftn -> ..., `result1 value`, `result2 value`

## delegate

..., `arg2 value`, `arg1 value`, delegate -> ..., `result1 value`, `result2 value`


# instructions

- nop

    format: `nop`

    transform: ... -> ...

- call

    format: `call`

    transform: ..., ftn, ref, ... -> ...

- callvirt

    format: `callvirt`

    transform: ..., vftn, ref, ... -> ...

- callstatic

    format: `callstatic`

    transform: ..., sftn, ... -> ... 

- calldlg

    format: `calldlg`

    transform: ..., delegate, ... -> ...

- invoke

    format: `invoke <ExtFtnDecl>`

    transform: ... -> ..., `function return`

- intrinsic

    format: `intrinsic <u8>`

    transform: ... -> ...

- ldftn

    format: `ldftn <MethodDecl>`

    transform: ... -> ..., ftn

- ldsftn

    format: `ldsftn <staticFunctionDecl>`

    transform: ... -> ..., sftn

- ldvirtftn

    format: `ldvftn <virtualFunctionDecl>`
    
    transform: ... -> ..., vftn

- wrapvftn

    format: `wrapvftn`

    transform: ...,ref,vftn -> ...,delegate

- wrapsftn

    format: `wrapsftn`

    transform: ..., sftn -> ..., delegate

- wrapext

    format: `wrapext <ExtFtnDecl>`

    transform: ... -> ..., delegate

- jif

    format: `jif <u64 address>`

    transform: ..., boolean -> ...

- br

    format: `br <u64 address>`

    transform: ... -> ...

- ret

    format: `ret`

    transform: ... -> ...

- push

    format: `push.<DataType> <Value>`

    transform: ... -> ..., `data type`


- ldc

    format: `ldc <constDecl>`

    transform: ... -> ..., `value of constDecl`

- ldarg

    format: `ldarg.<DataType>`

    transform: ..., u16 -> ..., `data type`

- ldarga

    format: `ldarga`

    transform: ..., u16 -> ..., ptr

- ldloc

    format: `ldloc.<DataType>`

    transform: ..., u16 -> ..., `data type value`

- ldloca

    format: `ldloca`

    transform: ..., u16 -> ..., ref

- store

    format : `store.<DataType>`

    transform: ..., address, `data type value` -> ...

- load 

    format : `load.<DataType>`

    transform : ..., address -> ..., `data type value`

- starg

    format: `starg.<DataType>`

    transform: ..., u16, `data type value` -> ...

- starga

    format: `starga.<DataType>`

    transform: ..., u16, `data type value` -> ...

- stloc

    format: `stloc.<DataType>`

    transform: .., u16, `data type` -> ...

- stelem

    format: `stelem.<DataType> <ElementTypeDecl>`

    transform: ..., ref, u16, `data type value` -> ...

- stfld

    format: `stfld.<DataType> <FieldDecl>`

    transform: ..., ref, `data type` -> ...

- stsfld

    format: `stsfld.<DataType> <StaticFieldDecl>`

    transform: ..., `data type` -> ...

- ldfld

    format: `ldfld.<DataType> <FieldDecl>`

    transform: ..., ref -> ..., `data type` 

- ldsfld

    format: `ldsfld.<DataType> <staticFieldDecl>`

    transform: ... -> ..., `data type`

- ldflda

    format: `ldflda <FieldDecl>`

    transform: ..., ref -> ..., ref

- ldsflda

    format: `ldsflda <StaticFieldDecl>`

    transform: ... -> ..., ref

- ldelem

    format: `ldelem.<DataType> <ElementTypeDecl>`

    transform: ..., ref, u16 -> ..., `data type` 

- ldelema

    format: `ldelema <ElementTypeDecl>`

    transform: ..., ref, u16 -> ..., ref

- ldnull

    format: `ldnull`

    transform: ... -> ..., `null ref or ptr`


- newobj

    format: `newobj <CtorDecl>`

    transform: ... -> ..., ref

- castCls

    format: `castCls <srcClassDecl> <dstClassDecl>`

    transform: ..., ref -> ..., ref

- Conv

    format: `conv <SrcDataType> <DstDataType>`

    transform: ..., `src data type` -> ..., `dst data type`

## Binary operation

transform: ..., `lhs`, `rhs` -> ..., `result`

- Add
- Sub
- Mul
- Div
- And
- Or
- Xor
- EQ
- NE
- LT
- GT
- LE
- GE

## Unary operation

transform: ..., `value` -> ..., `value`

- Neg
- Not

# example

```
Public Module UI

    Public Enum Message 
        Update = 1
        Draw
        Resize
        Hide
    End Enum

    Public Interface MessageReciver
         Function OnMessage(Byval msg as Message,Optional ByRef sender as UIObject) As Boolean
    End Interface

    Private Type Position
        x As Integer
        Y as Integer
        W as Integer
        H as Integer
    End Type

    Public Class UIObject Impl MessageReciver
        Protected Pos As Position
        Public Static Count As Integer
    End Class

    Public Class Button Extend UIObject
        Public Override Function OnMessage(Byval msg As Message,Optional ByRef sender As UIObject)As Boolean
            if msg == Message.Update then
                Return Self.pos.x - sender.pos.w + 233
            else
                Return OnMessage(Message.Hide,sender: self)
            End if
        End Function
    End Class

End Module
```
```
    (mod public UI
        (enum public Message (pair Update 1) (pair Draw 2) (pair Resize 3) (pair Hide 4))
        (interface public MessageReciver
            (vftn OnMessage 
                (param msg UI.Message) (param sender UI.UIObject)
                (result Boolean)
            )
        )
        (record private Position
            (fld x i32)
            (fld y i32)
            (fld w i32)
            (fld h i32)
        )
        (cls public UIObject (extend Object) (impl Mod.MessageReciver)
            (fld Pos UI.Position)
            (sfld Count i32)
        )
        (cls public Button (extend UI.UIObject) 
            (vftn public OnMessage 
                (param msg UI.Message) 
                (param sender UI.UIObject)
                (result Boolean)
                    
                    ldarg.u32 1
                    ldc.u32 UI.Message.Update
                    EQ
                    ldc.i16 .iftrue
                    InstJif
                    ldc.u32 Message.Hide
                    ldarg.ref 0
                    callvirt UI.Button.OnMessage
                    ret
                .iftrue:
                    ldarg 0 
                    ldfld.i32 UI.Position.x
                    ldarg 0
                    ldfld.i32 UI.Position.w
                    sub.i32
                    push.i32 233
                    add.i32
                    conv.i32 boolean
                    ret
            )
        )
    )
```
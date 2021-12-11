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

    reference to class object

- ptr

    pointer to any types except class object and function

- ftn

    reference to function

- vftn

    reference to virtual function

- sftn

    reference to static function



# instructions

- nop

    format: `nop`

    transform: ... -> ...

- callvirt

    format: `callvirt`

    transform: ..., ftn,ref -> ...

- callext

    format: `callext <ExtFtnDecl>`

    transform: ... -> ..., `function return`

- call

    format: `call`

    transform: ..., ftn,ref -> ...

- jif

    format: `jif`

    transform: ..., boolean -> ...

- br

    format: `br`

    transform: ... -> ...

- ret

    format: `ret`

    transform: ... -> ...

- ldc
    format: `ldc.<DataType> <Value>`

    transform: ... -> ..., `value`

- ldftn

    format: `ldftn <functionDecl>`

    transform: ... -> ..., ftn

- ldsftn

    format: `ldsftn <staticFunctionDecl>`

    transform: ... -> ..., sftn

- ldvirtftn

    format: `ldvftn <virtualFunctionDecl>`
    
    transform: ... -> ..., vftn

- ldarg

    format: `ldarg.<DataType>`

    transform: ..., u16 -> ..., `data type`

- starg

    format: `starg.<DataType>`

    transform: ..., u16, `data type` -> ...

- ldarga

    format: `ldarga`

    transform: ..., u16 -> ..., ptr

- ldloc

    format: `ldloc.<DataType>`

    transform: ..., u16 -> ..., `data type`

- ldloca

    format: `ldloca.<DataType>`

    transform: ..., u16 -> ..., ptr

- stloc

    format: `stloc.<DataType>`

    transform: .., u16, `data type` -> ...

- ldfld

    format: `ldfld.<DataType> <FieldDecl>`

    transform: ... -> `data type` 

- ldsfld

    format: `ldsfld.<DataType> <staticFieldDecl>`

    transform: ... -> `data type`

- ldflda

    format: `ldflda.<DataType> <FieldDecl>`

    transform: ... -> `data type`

- ldsflda

    format: `ldsflda.<DataType> <StaticFieldDecl>`

    transform: ... -> `data type`

- stfld

    format: `stfld.<DataType> <FieldDecl>`

    transform: ..., `data type` -> ...

- stsfld

    format: `stsfld.<DataType> <StaticFieldDecl>`

    transform: ..., `data type` -> ...

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

- Add
- Sub
- Mul
- Div
- And
- Or
- Xor
- Not
- EQ
- NE
- LT
- GT
- LE
- GE
- Neg

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
                    token.ldc.u32 UI.Message.Update
                    EQ
                    ldc.i16 .iftrue
                    Jif
                    token.ldc.u32 Message.Hide
                    ldarg.ref 0
                    token.callvirt UI.Button.OnMessage
                    ret
                .iftrue:
                    ldarg 0 
                    token.ldfld.i32 UI.Position.x
                    ldarg 0
                    token.ldfld.i32 UI.Position.w
                    sub.i32
                    ldc 233
                    add.i32
                    conv.i32 boolean
                    ret
            )
        )
    )
```
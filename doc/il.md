# data types
|Format|Description|
|-|-|
|`i8(boolean)`||
|`i16`||
|`i32`||
|`i64`||
|`u8`||
|`u16`||
|`u32`||
|`u64`||
|`f32`||
|`f64`||
|`ref`|reference to class-object/array/field/record|
|`delegate`|reference to wrapped function|
|`vftn`|reference to virtual function|
|`sftn`|reference to static function|
|`array<ArrayTypeToken>`|memory of whole array|
|`record<RecordTypeToken>`|memory of whole record|


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

|Format|Transform|
|-|-|
|`nop`                                  |... -> ...|
|`call`                                 |..., ftn, ref, ... -> ... |
|`callvirt`                             |..., vftn, ref, ... -> ...|
|`callstatic`                           |..., sftn, ... -> ... |
|`calldlg`                              |..., delegate, ... -> ...|
|`invoke <ExtFtnDecl>`                  |... -> ..., `function return`|
|`intrinsic <u8>`                       |... -> ...|
|`wrapvftn`                             |...,ref,vftn -> ...,delegate|
|`wrapsftn`                             |..., sftn -> ..., delegate|
|`wrapext <ExtFtnDecl>`                 |... -> ..., delegate|
|`jif <u64 address>`                    |..., boolean -> ...|
|`br <u64 address>`                     |... -> ...|
|`ret`                                  |... -> ...|
|`push.<DataType> <Value>`              |... -> ..., `data type`|
|`ldc <constDecl>`                      |... -> ..., `value of constDecl`|
|`starg.<DataType>`                     |..., `data type value`, u16 -> ...|
|`ldarg.<DataType>`                     |..., u16 -> ..., `data type`|
|`ldarga`                               |..., u16 -> ..., ptr|
|`stloc.<DataType>`                     |..., `data type value`, u16 -> ...|
|`ldloc.<DataType>`                     |..., u16 -> ..., `data type value`|
|`ldloca`                               | ..., u16 -> ..., ref|
|`stfld.<DataType> <FieldToken>`        |..., `data type`, ref -> ...|
|`ldfld.<DataType> <FieldToken>`        |..., ref -> ..., `data type` |
|`ldflda <FieldToken>`                  |..., ref -> ..., ref|
|`stsfld.<DataType> <StaticFieldToken>` |..., `data type` -> ...|
|`ldsfld.<DataType> <staticFieldToken>` |... -> ..., `data type`|
|`ldsflda <StaticFieldToken>`           |... -> ..., ref|
|`stelem.<DataType>`                    |..., `data type value`, ref, i32 -> ...|
|`ldelem.<DataType>`                    |..., ref, i32 -> ..., `data type`|
|`ldelema.<DataType>`                   |..., ref, i32 -> ..., ref|
|`store.<DataType>`                     |..., `data type value`, ref -> ...|
|`load.<DataType>`                      |..., ref -> ..., `data type value`|
|`ldnull`                               |... -> ..., `null ref or ptr`|
|`newobj <CtorToken>`                    |... -> ..., ref|
|`castCls <srcClassToken> <dstClassToken>`|..., ref -> ..., ref|
|`conv <SrcDataType> <DstDataType>`     |..., `src data type` -> ..., `dst data type`|

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


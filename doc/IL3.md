# operand types
|Format|Description|
|-|-|
|`boolean`||
|`i8`||
|`i16`||
|`i32`||
|`i64`||
|`u8`||
|`u16`||
|`u32`||
|`u64`||
|`f32`||
|`f64`||
|`ref`|reference to instance|
|`hld`|handle of /Fields/Argument/local variable/static field|
|`ftn`|reference to method slot|
|`vftn`|reference to virtual function slot|
|`sftn`|reference to static function|
|`ctor`|reference to ctor|
|`emconst`|enum constant|
|`record<Token>`|memory of whole record|

# instructions

## Function Loading and Invocation

`option count` is omitted in invocation when function declaration did not have any optional parameter.

- virtual function

    ..., ref, `arg2 value`, `arg1 value`, `option value2`, `option info2`, `option value1` , `option info1`, `u8 option count`, `paramArray`, vftn | ..., `result1 value`, `result2 value`

- method 

    ..., ref, `arg2 value`, `arg1 value`, `option value2`, `option info2`, `option value1` , `option info1`, `u8 option count`, `paramArray`, ftn | ..., `result1 value`, `result2 value`

- static function

    ..., `arg2 value`, `arg1 value`, `option value2`, `option info2`, `option value1` , `option info1`, `u8 option count`, `paramArray`, sftn | ..., `result1 value`, `result2 value`

- constructor

    ..., ref, `arg2 value`, `arg1 value`, `option value2`, `option info2`, `option value1` , `option info1`, `u8 option count`, `paramArray`, ctor | ...


|Format|Operand|Result|
|-|-|-|
|`ldsftn <Token>`                       |... | ..., sftn|
|`ldvftn <Token>`                       |... | ..., vftn|
|`ldftn <Token>`                        |... | ..., ftn|
|`ldctor <Token>`                       |... | ..., ctor|
|`ldforeign <Token>`                    |... | ..., foreign|
|`wrapsftn`                             |..., sftn| ..., dlg|
|`wrapvftn`                             |..., vftn, ref| ..., dlg|
|`wrapftn`                              |..., ftn, ref| ..., dlg|
|`wrapctor`                             |..., ctor| ..., dlg|
|`wrapforeign`                          |..., foreign|..., dlg|
|`callmethod`                           |..., ftn, ref, ... | ... |
|`callvirtual`                          |..., vftn, ref, ... | ...|
|`callstatic`                           |..., sftn, ... | ... |
|`callctor`                             |..., ctor | ...|
|`callforeign`                          |..., foreign | ...|
|`callintrinsic <Token>`                |... | ...|
|`calldlg`                              |..., dlg| ...|


## Fields/Parameters/Local Variables/Enum Constant Access

|Format|Operand|Result|
|-|-|-|
|`starg.<Type>`                     |..., `value`, u16 | ...|
|`ldarg.<Type>`                     |..., u16 | ..., `value`|
|`ldarga`                               |..., u16 | ..., hld|
|`stloc.<Type>`                     |..., `value`, u16 | ...|
|`ldloc.<Type>`                     |..., u16 | ..., `value`|
|`ldloca`                               | ..., u16 | ..., hld|
|`stfld.<Type> <Token>`        |..., `value`, `ref or hld` | ...|
|`ldfld.<Type> <Token>`        |..., `ref, hld or record` | ..., `value` |
|`ldflda <Token>`                  |..., `ref or hld` | ..., hld|
|`stsfld.<Type> <Token>` |..., `value` | ...|
|`ldsfld.<Type> <Token>` |... | ..., `value`|
|`ldsflda <Token>`           |... | ..., hld|
|`testopt`              |..., u16 * n, u8, | ..., `boolean`|
|`ldoptinfo <Token>`    |... | ..., `option info`|
|`ldenumc <Token>`      |... | ..., `enum constant`| 


## Array

数组操作指令中的Token为元素类型的Token。

|Format|Operand|Result|
|-|-|-|
|`stelemr.<Type> <Token>`                    |..., ref, i32, `value` | ...|
|`stelem.<Type> <Token>`                    |..., `value`, ref, i32 | ...|
|`ldelem.<Type> <Token>`                    |..., ref, i32 | ..., `value`|
|`ldelema <Token>`                   |..., ref, i32 | ..., hld|
|`newarray <Token>`                    |..., i32 | ..., ref|

## Control Flow
|Format|Operand|Result|
|-|-|-|
|`jif <u32 address>`                    |..., boolean | ...|
|`br <u32 address>`                     |... | ...|
|`ret`                                  |... | ...|

## Data Operation

|Format|Operand|Result|
|-|-|-|
|`nop`                              |... | ...|
|`dup`                              |..., value | ..., value, value|
|`push.<Type> <Value>`              |... | ..., `value`|
|`pop.<Type>`                       |... `value` | ... | 
|`store.<Type>`                     |..., `value`, hld | ...|
|`load.<Type>`                      |..., hld | ..., `value`|
|`ldnothing`                        |... | ..., `nothing`|
|`convert <Type> <Type>`            |..., `value` | ..., `value`|
|`castClass <Token> <Token>`        |..., ref | ref |
|`instanceof <classToken>`          |...,ref | ..., boolean|
|`ldstr <Token>`                    |... | ..., ref|
|`newobj <Token>`                   |... | ..., ref|
|`pin`                              |..., ref|...|
|`unpin`                            |..., ref|...|

## Exception Handling

|Format|Operand|Result|
|-|-|-|
|`throw`| ...,ref | ...|
|`enter <constructed Token of exception class> <u32 address>`| ... | ...|
|`leave <constructed Token of exeception class>`| 

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
- Mod

## Unary operation

transform: ..., `value` -> ..., `value`

- Neg
- Not

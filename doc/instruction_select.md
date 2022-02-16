## Unary
|AST|Instructions|
|-|-|
|`Unary({OP},Binary)`|{OP}|
|`Unary({OP},Cast)`|{OP}|
|`Unary({OP},Assign)`|load value of assignment,{OP}|
|`Unary({OP},Parentheses)`|{OP}|
|`Unary({OP},ArrayElement)`|Ldelem,{OP}|
|`Unary({OP},FtnCall)`|{OP}|
|`Unary({OP},VFtnCall)`|{OP}|
|`Unary({OP},SFtnCall)`|{OP}|
|`Unary({OP},ExtCall)`|{OP}|
|`Unary({OP},Local)`|Ldloc,{OP}|
|`Unary({OP},Arg(ByRef))`|Ldarg,Load,{OP}|
|`Unary({OP},Arg(ByVal))`|Ldarg,{OP}|
|`Unary({OP},Fld)`|Ldfld,{OP}|
|`Unary({OP},SFld)`|Ldsfld,{OP}|
|`Unary({OP},Digit)`|{OP}|
|`Unary({OP},Decimal)`|{OP}|
|`Unary({OP},Char)`|{OP}|

## Binary
|AST of lhs or rhs|Instructions|
|-|-|
|||
|`Unary`|{OP}|
|`Binary`|{OP}|
|`Cast`|{OP}|
|`Assign`|load value of assign,{OP}|
|`Parentheses`|{OP}|
|`ArrayElement`|Ldelem|
|`FtnCall`|{OP}|
|`VFtnCall`|{OP}|
|`SFtnCall`|{OP}|
|`ExtCall`|{OP}|
|`Local`|Ldloc,{OP}|
|`Arg(ByRef)`|Ldarg,Load,{OP}|
|`Arg(ByVal)`|Ldarg,{OP}|
|`Fld`|Ldfld,{OP}|
|`SFld`|Ldsfld,{OP}|
|`Digit`|{OP}|
|`Decimal`|{OP}|
|`String`|{OP}|
|`Char`|{OP}|
|`Boolean`|{OP}|

## Cast
|AST|Instructions|
|-|-|
|`Cast(Unary,Target)`|Cast|
|`Cast(Binary,Target)`|Cast|
|`Cast(Cast,Target)`|Cast|
|`Cast(Assign,Target)`|load value of assignment,Cast|
|`Cast(Parentheses,Target)`|Cast|
|`Cast(ArrayElement,Target)`|Ldelem,Cast|
|`Cast(FtnCall,Target)`|Cast|
|`Cast(VFtnCall,Target)`|Cast|
|`Cast(SFtnCall,Target)`|Cast|
|`Cast(ExtCall,Target)`|Cast|
|`Cast(Local,Target)`|Ldloc,Cast|
|`Cast(Arg(ByRef),Target)`|Ldarg,Load,Cast|
|`Cast(Arg(ByVal),Target)`|Ldarg,Cast|
|`Cast(Fld,Target)`|Ldfld,Cast|
|`Cast(SFld,Target)`|Ldsfld,Cast|
|`Cast(Digit,Target)`|Cast|
|`Cast(Decimal,Target)`|Cast|
|`Cast(String,Target)`|Cast|
|`Cast(Char,Target)`|Cast|
|`Cast(Boolean,Target)`|Cast|
|`Cast(EnumMember,Target)`|Cast|

## Assign
|AST|Instructions|
|-|-|
|`Assign(ArrayElement,Unary)`|Stelem|
|`Assign(ArrayElement,Binary)`|Stelem|
|`Assign(ArrayElement,Cast)`|Stelem|
|`Assign(ArrayElement,Assign)`|load value of assignment,Stelem|
|`Assign(ArrayElement,Parentheses)`|Stelem|
|`Assign(ArrayElement,ArrayElement)`|Ldelem(rhs),Stelem(lhs)|
|`Assign(ArrayElement,FtnCall)`|Stelem|
|`Assign(ArrayElement,VFtnCall)`|Stelem|
|`Assign(ArrayElement,SFtnCall)`|Stelem|
|`Assign(ArrayElement,ExtCall)`|Stelem|
|`Assign(ArrayElement,Local)`|Ldloc,Stelem|
|`Assign(ArrayElement,Arg(ByRef))`|Ldarg,Load,Stelem|
|`Assign(ArrayElement,Arg(ByVal))`|Ldarg,Stelem|
|`Assign(ArrayElement,Fld)`|Ldfld,Stelem|
|`Assign(ArrayElement,SFld)`|Ldsfld,Stelem|
|`Assign(ArrayElement,Digit)`|Stelem|
|`Assign(ArrayElement,Decimal)`|Stelem|
|`Assign(ArrayElement,String)`|Stelem|
|`Assign(ArrayElement,Char)`|Stelem|
|`Assign(ArrayElement,Boolean)`|Stelem|
|`Assign(ArrayElement,EnumMember)`|Stelem|
|`Assign(Local,Unary)`|Stloc|
|`Assign(Local,Binary)`|Stloc|
|`Assign(Local,Cast)`|Stloc|
|`Assign(Local,Assign)`|load value of assignment,Stloc|
|`Assign(Local,Parentheses)`|Stloc|
|`Assign(Local,ArrayElement)`|Ldelem,Stloc|
|`Assign(Local,FtnCall)`|Stloc|
|`Assign(Local,VFtnCall)`|Stloc|
|`Assign(Local,SFtnCall)`|Stloc|
|`Assign(Local,ExtCall)`|Stloc|
|`Assign(Local,Local)`|Ldloc(rhs),Stloc(lhs)|
|`Assign(Local,Arg(ByRef))`|Ldarg,Load,Stloc|
|`Assign(Local,Arg(ByVal))`|Ldarg,Stloc|
|`Assign(Local,Fld)`|Ldfld,Stloc|
|`Assign(Local,SFld)`|Ldsfld,Stloc|
|`Assign(Local,Digit)`|Stloc|
|`Assign(Local,Decimal)`|Stloc|
|`Assign(Local,String)`|Stloc|
|`Assign(Local,Char)`|Stloc|
|`Assign(Local,Boolean)`|Stloc|
|`Assign(Local,EnumMember)`|Stloc|
|`Assign(Arg(ByRef),Unary)`|Ldarg,Store|
|`Assign(Arg(ByRef),Binary)`|Ldarg,Store|
|`Assign(Arg(ByRef),Cast)`|Ldarg,Store|
|`Assign(Arg(ByRef),Assign)`|load value of assignment,Ldarg,Store|
|`Assign(Arg(ByRef),Parentheses)`|Ldarg,Store|
|`Assign(Arg(ByRef),ArrayElement)`|Ldelem,Ldarg,Store|
|`Assign(Arg(ByRef),FtnCall)`|Ldarg,Store|
|`Assign(Arg(ByRef),VFtnCall)`|Ldarg,Store|
|`Assign(Arg(ByRef),SFtnCall)`|Ldarg,Store|
|`Assign(Arg(ByRef),ExtCall)`|Ldarg,Store|
|`Assign(Arg(ByRef),Local)`|Ldloc,Ldarg,Store|
|`Assign(Arg(ByRef),Arg(ByRef))`|Ldarg(rhs),Load,Ldarg(lhs),Store|
|`Assign(Arg(ByRef),Arg(ByVal))`|Ldarg(rhs),Ldarg(lhs),Store|
|`Assign(Arg(ByRef),Fld)`|Ldfld,Ldarg,Store|
|`Assign(Arg(ByRef),SFld)`|Ldsfld,Ldarg,Store|
|`Assign(Arg(ByRef),Digit)`|Ldarg,Store|
|`Assign(Arg(ByRef),Decimal)`|Ldarg,Store|
|`Assign(Arg(ByRef),String)`|Ldarg,Store|
|`Assign(Arg(ByRef),Char)`|Ldarg,Store|
|`Assign(Arg(ByRef),Boolean)`|Ldarg,Store|
|`Assign(Arg(ByRef),EnumMember)`|Ldarg,Store|
|`Assign(Arg(ByVal),Unary)`|Starg|
|`Assign(Arg(ByVal),Binary)`|Starg|
|`Assign(Arg(ByVal),Cast)`|Starg|
|`Assign(Arg(ByVal),Assign)`|load value of assignment,Starg|
|`Assign(Arg(ByVal),Parentheses)`|Starg|
|`Assign(Arg(ByVal),ArrayElement)`|Ldelem,Starg|
|`Assign(Arg(ByVal),FtnCall)`|Starg|
|`Assign(Arg(ByVal),VFtnCall)`|Starg|
|`Assign(Arg(ByVal),SFtnCall)`|Starg|
|`Assign(Arg(ByVal),ExtCall)`|Starg|
|`Assign(Arg(ByVal),Local)`|Ldloc,Starg|
|`Assign(Arg(ByVal),Arg(ByRef))`|Ldarg(rhs),Load,Starg(lhs)|
|`Assign(Arg(ByVal),Arg(ByVal))`|Ldarg(rhs),Starg(lhs)|
|`Assign(Arg(ByVal),Fld)`|Ldfld,Starg|
|`Assign(Arg(ByVal),SFld)`|Ldsfld,Starg|
|`Assign(Arg(ByVal),Digit)`|Starg|
|`Assign(Arg(ByVal),Decimal)`|Starg|
|`Assign(Arg(ByVal),String)`|Starg|
|`Assign(Arg(ByVal),Char)`|Starg|
|`Assign(Arg(ByVal),Boolean)`|Starg|
|`Assign(Arg(ByVal),EnumMember)`|Starg|
|`Assign(Fld,Unary)`|Stfld|
|`Assign(Fld,Binary)`|Stfld|
|`Assign(Fld,Cast)`|Stfld|
|`Assign(Fld,Assign)`|load value of assignment,Stfld|
|`Assign(Fld,Parentheses)`|Stfld|
|`Assign(Fld,ArrayElement)`|Ldelem,Stfld|
|`Assign(Fld,FtnCall)`|Stfld|
|`Assign(Fld,VFtnCall)`|Stfld|
|`Assign(Fld,SFtnCall)`|Stfld|
|`Assign(Fld,ExtCall)`|Stfld|
|`Assign(Fld,Local)`|Ldloc,Stfld|
|`Assign(Fld,Arg(ByRef))`|Ldarg,Load,Stfld|
|`Assign(Fld,Arg(ByVal))`|Ldarg,Stfld|
|`Assign(Fld,Fld)`|Ldfld(rhs),Stfld(lhs)|
|`Assign(Fld,SFld)`|Ldsfld,Stfld|
|`Assign(Fld,Digit)`|Stfld|
|`Assign(Fld,Decimal)`|Stfld|
|`Assign(Fld,String)`|Stfld|
|`Assign(Fld,Char)`|Stfld|
|`Assign(Fld,Boolean)`|Stfld|
|`Assign(Fld,EnumMember)`|Stfld|
|`Assign(SFld,Unary)`|Stsfld|
|`Assign(SFld,Binary)`|Stsfld|
|`Assign(SFld,Cast)`|Stsfld|
|`Assign(SFld,Assign)`|load value of assignment,Stsfld|
|`Assign(SFld,Parentheses)`|Stsfld|
|`Assign(SFld,ArrayElement)`|Ldelem,Stsfld|
|`Assign(SFld,FtnCall)`|Stsfld|
|`Assign(SFld,VFtnCall)`|Stsfld|
|`Assign(SFld,SFtnCall)`|Stsfld|
|`Assign(SFld,ExtCall)`|Stsfld|
|`Assign(SFld,Local)`|Ldloc,Stsfld|
|`Assign(SFld,Arg(ByRef))`|Ldarg,Load,Stsfld|
|`Assign(SFld,Arg(ByVal))`|Ldarg,Stsfld|
|`Assign(SFld,Fld)`|Ldfld,Stsfld|
|`Assign(SFld,SFld)`|Ldsfld(rhs),Stsfld(lhs)|
|`Assign(SFld,Digit)`|Stsfld|
|`Assign(SFld,Decimal)`|Stsfld|
|`Assign(SFld,String)`|Stsfld|
|`Assign(SFld,Char)`|Stsfld|
|`Assign(SFld,Boolean)`|Stsfld|
|`Assign(SFld,EnumMember)`|Stsfld|

## Argument
|AST|Instructions|
|-|-|
|`Argument(ByRef,Assign)`|load address of assignment|
|`Argument(ByRef,ArrayElement)`|Ldelema|
|`Argument(ByRef,Local)`|Ldloca|
|`Argument(ByRef,Arg(ByRef))`|Ldarg|
|`Argument(ByRef,Arg(ByVal))`|Ldarga|
|`Argument(ByRef,Fld)`|Ldflda|
|`Argument(ByRef,SFld)`|Ldsflda|
|`Argument(ByVal,Unary)`|Nop|
|`Argument(ByVal,Binary)`|Nop|
|`Argument(ByVal,Cast)`|Nop|
|`Argument(ByVal,Assign)`|load value of assignment|
|`Argument(ByVal,Parentheses)`|Nop|
|`Argument(ByVal,ArrayElement)`|Ldelem|
|`Argument(ByVal,FtnCall)`|Nop|
|`Argument(ByVal,VFtnCall)`|Nop|
|`Argument(ByVal,SFtnCall)`|Nop|
|`Argument(ByVal,ExtCall)`|Nop|
|`Argument(ByVal,Local)`|Ldloc|
|`Argument(ByVal,Arg(ByRef))`|Ldarg,Load|
|`Argument(ByVal,Arg(ByVal))`|Ldarg|
|`Argument(ByVal,Fld)`|Ldfld|
|`Argument(ByVal,SFld)`|Ldsfld|
|`Argument(ByVal,Digit)`|Nop|
|`Argument(ByVal,Decimal)`|Nop|
|`Argument(ByVal,String)`|Nop|
|`Argument(ByVal,Char)`|Nop|
|`Argument(ByVal,Boolean)`|Nop|
|`Argument(ByVal,EnumMember)`|Nop|

## Parentheses

## ArrayElememt
|AST|Instructions|
|-|-|
|`ArrayElement(ArrayElement,Unary)`|Ldelema|
|`ArrayElement(ArrayElement,Binary)`|Ldelema|
|`ArrayElement(ArrayElement,Cast)`|Ldelema|
|`ArrayElement(ArrayElement,Assign)`|Ldelema,load value of assignment|
|`ArrayElement(ArrayElement,Parentheses)`|Ldelema|
|`ArrayElement(ArrayElement,ArrayElement)`|Ldelema,Ldelem|
|`ArrayElement(ArrayElement,FtnCall)`|Ldelema|
|`ArrayElement(ArrayElement,VFtnCall)`|Ldelema|
|`ArrayElement(ArrayElement,SFtnCall)`|Ldelema|
|`ArrayElement(ArrayElement,ExtCall)`|Ldelema|
|`ArrayElement(ArrayElement,Local)`|Ldelema,Ldloc|
|`ArrayElement(ArrayElement,Arg(ByRef))`|Ldelema,Ldarg,Load|
|`ArrayElement(ArrayElement,Arg(ByVal))`|Ldelema,Ldarg|
|`ArrayElement(ArrayElement,Fld)`|Ldelema,Ldfld|
|`ArrayElement(ArrayElement,SFld)`|Ldelema,Ldsfld|
|`ArrayElement(ArrayElement,Digit)`|Ldelema|
|`ArrayElement(Local,Unary)`|Ldloca|
|`ArrayElement(Local,Binary)`|Loloca|
|`ArrayElement(Local,Cast)`|Ldloca|
|`ArrayElement(Local,Assign)`|Ldloca,load valueo of assignment|
|`ArrayElement(Local,Parentheses)`|Ldloca|
|`ArrayElement(Local,ArrayElement)`|Ldloca,Ldelem|
|`ArrayElement(Local,FtnCall)`|Ldloca|
|`ArrayElement(Local,VFtnCall)`|Ldloca|
|`ArrayElement(Local,SFtnCall)`|Ldloca|
|`ArrayElement(Local,ExtCall)`|Ldloca|
|`ArrayElement(Local,Local)`|Ldloca(lhs),Ldloc(rhs)|
|`ArrayElement(Local,Arg(ByRef))`|Ldloca,Ldarg,Load|
|`ArrayElement(Local,Arg(ByVal))`|Ldloca,Ldarg|
|`ArrayElement(Local,Fld)`|Ldloca,Ldfld|
|`ArrayElement(Local,SFld)`|Ldloca,Ldsfld|
|`ArrayElement(Local,Digit)`|Ldloca|
|`ArrayElement(Arg(ByRef),Unary)`|Ldarg|
|`ArrayElement(Arg(ByRef),Binary)`|Ldarg|
|`ArrayElement(Arg(ByRef),Cast)`|Ldarg|
|`ArrayElement(Arg(ByRef),Assign)`|Ldarg,load value of assignment|
|`ArrayElement(Arg(ByRef),Parentheses)`|Ldarg|
|`ArrayElement(Arg(ByRef),ArrayElement)`|Ldarg,Ldelem|
|`ArrayElement(Arg(ByRef),FtnCall)`|Ldarg|
|`ArrayElement(Arg(ByRef),VFtnCall)`|Ldarg|
|`ArrayElement(Arg(ByRef),SFtnCall)`|Ldarg|
|`ArrayElement(Arg(ByRef),ExtCall)`|Ldarg|
|`ArrayElement(Arg(ByRef),Local)`|Ldarg,Ldloc|
|`ArrayElement(Arg(ByRef),Arg(ByRef))`|Ldarg(lhs),Ldarg(rhs),Load|
|`ArrayElement(Arg(ByRef),Arg(ByVal))`|Ldarg(lhs),Ldarg(rhs)|
|`ArrayElement(Arg(ByRef),Fld)`|Ldarg,Ldfld|
|`ArrayElement(Arg(ByRef),SFld)`|Ldarg,Ldsfld|
|`ArrayElement(Arg(ByRef),Digit)`|Ldarg|
|`ArrayElement(Arg(ByVal),Unary)`|Ldarga|
|`ArrayElement(Arg(ByVal),Binary)`|Ldarga|
|`ArrayElement(Arg(ByVal),Cast)`|Ldarga|
|`ArrayElement(Arg(ByVal),Assign)`|Ldarga,load value of assignment|
|`ArrayElement(Arg(ByVal),Parentheses)`|Ldarga|
|`ArrayElement(Arg(ByVal),ArrayElement)`|Ldarga,Ldelem|
|`ArrayElement(Arg(ByVal),FtnCall)`|Ldarga|
|`ArrayElement(Arg(ByVal),VFtnCall)`|Ldarga|
|`ArrayElement(Arg(ByVal),SFtnCall)`|Ldarga|
|`ArrayElement(Arg(ByVal),ExtCall)`|Ldarga|
|`ArrayElement(Arg(ByVal),Local)`|Ldarga,Ldloc|
|`ArrayElement(Arg(ByVal),Arg(ByRef))`|Ldarga,Ldarg,Load|
|`ArrayElement(Arg(ByVal),Arg(ByVal))`|Ldarga,Ldarg|
|`ArrayElement(Arg(ByVal),Fld)`|Ldarga,Ldfld|
|`ArrayElement(Arg(ByVal),SFld)`|Ldarga,Ldsfld|
|`ArrayElement(Arg(ByVal),Digit)`|Ldarga|
|`ArrayElement(Fld,Unary)`|Ldflda|
|`ArrayElement(Fld,Binary)`|Ldflda|
|`ArrayElement(Fld,Cast)`|Ldflda|
|`ArrayElement(Fld,Assign)`|Ldflda,load value of assignment|
|`ArrayElement(Fld,Parentheses)`|Ldflda|
|`ArrayElement(Fld,ArrayElement)`|Ldflda,Ldelem|
|`ArrayElement(Fld,FtnCall)`|Ldflda|
|`ArrayElement(Fld,VFtnCall)`|Ldflda|
|`ArrayElement(Fld,SFtnCall)`|Ldflda|
|`ArrayElement(Fld,ExtCall)`|Ldflda|
|`ArrayElement(Fld,Local)`|Ldflda,Ldloc|
|`ArrayElement(Fld,Arg(ByRef))`|Ldflda,Ldarg,Load|
|`ArrayElement(Fld,Arg(ByVal))`|Ldflda,Ldarg|
|`ArrayElement(Fld,Fld)`|Ldflda,Ldfld|
|`ArrayElement(Fld,SFld)`|Ldflda,Ldsfld|
|`ArrayElement(Fld,Digit)`|Ldflda|
|`ArrayElement(SFld,Unary)`|Ldsflda|
|`ArrayElement(SFld,Binary)`|Ldsflda|
|`ArrayElement(SFld,Cast)`|Ldsflda|
|`ArrayElement(SFld,Assign)`|Ldsflda,load value of assignment|
|`ArrayElement(SFld,Parentheses)`|Ldsflda|
|`ArrayElement(SFld,ArrayElement)`|Ldsflda,Ldelem|
|`ArrayElement(SFld,FtnCall)`|Ldsflda|
|`ArrayElement(SFld,VFtnCall)`|Ldsflda|
|`ArrayElement(SFld,SFtnCall)`|Ldsflda|
|`ArrayElement(SFld,ExtCall)`|Ldsflda|
|`ArrayElement(SFld,Local)`|Ldsflda,Ldloc|
|`ArrayElement(SFld,Arg(ByRef))`|Ldsflda,Ldarg,Load|
|`ArrayElement(SFld,Arg(ByVal))`|Ldsflda,Ldarg|
|`ArrayElement(SFld,Fld)`|Ldsflda,Ldfld|
|`ArrayElement(SFld,SFld)`|Ldsflda,Ldsfld|
|`ArrayElement(SFld,Digit)`|Ldsflda|

## Delegate

## New

## FtnCall
|AST|Instructions|
|-|-|
|`FtnCall(Cast)`|Call|
|`FtnCall(Parentheses)`|Call|
|`FtnCall(ArrayElement)`|Ldelem,Call|
|`FtnCall(FtnCall)`|Call|
|`FtnCall(VFtnCall)`|Call|
|`FtnCall(SFtnCall)`|Call|
|`FtnCall(ExtCall)`|Call|
|`FtnCall(Local)`|Ldloc,Call|
|`FtnCall(Arg(ByRef))`|Ldarg,Load,Call|
|`FtnCall(Arg(ByVal))`|Ldarg,Call|
|`FtnCall(Fld)`|Ldfld,Call|
|`FtnCall(SFld)`|Ldsfld,Call|

## VFtnCall
|AST|Instructions|
|-|-|
|`VFtnCall(Cast)`|Callvirt|
|`VFtnCall(Parentheses)`|Callvirt|
|`VFtnCall(ArrayElement)`|Ldelem,Callvirt|
|`VFtnCall(FtnCall)`|Callvirt|
|`VFtnCall(VFtnCall)`|Callvirt|
|`VFtnCall(SFtnCall)`|Callvirt|
|`VFtnCall(ExtCall)`|Callvirt|
|`VFtnCall(Local)`|Ldloc,Callvirt|
|`VFtnCall(Arg(ByRef))`|Ldarg,Load,Callvirt|
|`VFtnCall(Arg(ByVal))`|Ldarg,Callvirt|
|`VFtnCall(Fld)`|Ldfld,Callvirt|
|`VFtnCall(SFld)`|Ldsfld,Callvirt|

## SFtnCall
## ExtCall

## Local
|AST|Instructions|
|-|-|
|`Local`|Push offset|

## Arg
|AST|Instructions|
|-|-|
|`Arg(ByRef)`|Push offset|
|`Arg(ByVal)`|Push offset|

## Fld
|AST|Instructions|
|-|-|
|`Fld(Cast)`|Nop|
|`Fld(Parentheses)`|Nop|
|`Fld(ArrayElement)`|Ldelem|
|`Fld(Local)`|Ldloc|
|`Fld(Arg(ByRef))`|Ldarg,Load|
|`Fld(Arg(ByVal))`|Ldarg|
|`Fld(Fld)`|Ldfld|
|`Fld(SFld)`|Ldsfld|
|`Fld(String)`|Nop|

for record:
|AST|Instructions|
|-|-|
|`Fld(Cast)`|Nop|
|`Fld(Parentheses)`|Nop|
|`Fld(ArrayElement)`|Ldelema|
|`Fld(Local)`|Ldloca|
|`Fld(Arg(ByRef))`|Ldarg|
|`Fld(Arg(ByVal))`|Ldarga|
|`Fld(Fld)`|Ldflda|
|`Fld(SFld)`|Ldsflda|
|`Fld(String)`|Nop|


## SFld
|AST|Instructions|
|-|-|
|`SFld`|Nop|

## Digit
|AST|Instructions|
|-|-|
|`Digit`|Push|

## Decimal
|AST|Instructions|
|-|-|
|`Decimal`|Push|

## String
|AST|Instructions|
|-|-|
|`String`|load string object|

## Char
|AST|Instructions|
|-|-|
|`Char`|Push|

## Boolean
|AST|Instructions|
|-|-|
|`Boolean`|Push|



















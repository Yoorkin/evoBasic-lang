## ASTs
Unary
Binary
Assign
Convert
Is
FtnCall
VFtnCall
SFtnCall
ForeignCall
CtorCall
NewArray
LocalAccess
ArgumentAccess
ArgumentPass
OptInit
FldAccess
SFldAccess
EnumConst
Subscript
ArrayLiteral
IntegerLiteral
DoubleLiteral
StringLiteral
RuneLiteral
BooleanLiteral
NothingLiteral

## Unary
|AST|Instructions|
|-|-|
|`Unary({OP},Unary)`|{OP}|
|`Unary({OP},Binary)`|{OP}|
|`Unary({OP},Assign)`|load value of assignment,{OP}|
|`Unary({OP},Convert)`|{OP}|
|`Unary({OP},Is)`| N/A |
|`Unary({OP},FtnCall)`|{OP}|
|`Unary({OP},VFtnCall)`|{OP}|
|`Unary({OP},SFtnCall)`|{OP}|
|`Unary({OP},ForeignCall)`|{OP}|
|`Unary({OP},CtorCall)`|{OP}|
|`Unary({OP},NewArray)`| N/A |
|`Unary({OP},LocalAccess)`|Ldloc,{OP}|
|`Unary({OP},ArgumentAccess(ByRef))`|Ldarg,Load,{OP}|
|`Unary({OP},ArgumentAccess(ByVal))`|Ldarg,{OP}|
|`Unary({OP},OptInit)`| N/A |
|`Unary({OP},FldAccess)`|Ldfld,{OP}|
|`Unary({OP},SFldAccess)`|Ldsfld,{OP}|
|`Unary({OP},EnumConst)`| N/A |
|`Unary({OP},Subscript)`|Ldelem,{OP}|
|`Unary({OP},ArrayLiteral)`| N/A |
|`Unary({OP},IntegerLiteral)`|{OP}|
|`Unary({OP},DoubleLiteral)`|{OP}|
|`Unary({OP},StringLiteral)`| N/A |
|`Unary({OP},RuneLiteral)`| N/A |
|`Unary({OP},BooleanLiteral)`| N/A |
|`Unary({OP},NothingLiteral)`| N/A |

## Binary
|AST of lhs or rhs|Instructions|
|-|-|
|`Unary`|{OP}|
|`Binary`|{OP}|
|`Assign`|load value of assign,{OP}|
|`Convert`|{OP}|
|`Is`|{OP}|
|`FtnCall`|{OP}|
|`VFtnCall`|{OP}|
|`SFtnCall`|{OP}|
|`ForeignCall`|{OP}|
|`CtorCall`|{OP}|
|`NewArray`|{OP}|
|`LocalAccess`|Ldloc,{OP}|
|`ArgumentAccess(Byref)`|Ldarg,Load,{OP}|
|`ArgumentAccess(Byval)`|Ldarg,{OP}|
|`OptInit`| - |
|`FldAccess`|Ldfld,{OP}|
|`SFldAccess`|Ldsfld,{OP}|
|`EnumConst`| - |
|`Subscript`|{OP}|
|`ArrayLiteral`|{OP}|
|`IntegerLiteral`|{OP}|
|`DoubleLiteral`|{OP}|
|`StringLiteral`|{OP}|
|`RuneLiteral`|{OP}|
|`BooleanLiteral`|{OP}|
|`NothingLiteral`| - |


## Assign
|AST|Instructions|
|-|-|
|`Assign(LocalAccess,Unary)`|Stloc|
|`Assign(LocalAccess,Binary)`|Stloc|
|`Assign(LocalAccess,Assign)`|load value of assignment,Stloc|
|`Assign(LocalAccess,Convert)`|Stloc|
|`Assign(LocalAccess,Is)`|stloc|
|`Assign(LocalAccess,FtnCall)`|Stloc|
|`Assign(LocalAccess,VFtnCall)`|Stloc|
|`Assign(LocalAccess,SFtnCall)`|Stloc|
|`Assign(LocalAccess,ForeignCall)`|Stloc|
|`Assign(LocalAccess,CtorCall)`|stloc|
|`Assign(LocalAccess,NewArray)`|stloc|
|`Assign(LocalAccess,LocalAccess)`|ldloc(rhs),stloc(lhs)|
|`Assign(LocalAccess,ArgumentAccess(ByRef))`|ldarg,load,stloc|
|`Assign(LocalAccess,ArgumentAccess(ByVal))`|ldarg,stloc|
|`Assign(LocalAccess,OptInit)`| N/A |
|`Assign(LocalAccess,FldAccess)`|ldfld,stloc|
|`Assign(LocalAccess,SFldAccess)`|ldsfld,stloc|
|`Assign(LocalAccess,EnumConst)`|stloc|
|`Assign(LocalAccess,Subscript)`|ldelem,stloc|
|`Assign(LocalAccess,ArrayLiteral)`|stloc|
|`Assign(LocalAccess,IntegerLiteral)`|stloc|
|`Assign(LocalAccess,DoubleLiteral)`|stloc|
|`Assign(LocalAccess,StringLiteral)`|stloc|
|`Assign(LocalAccess,RuneLiteral)`|stloc|
|`Assign(LocalAccess,BooleanLiteral)`|stloc|
|`Assign(LocalAccess,NothingLiteral)`|stloc|
|`Assign(ArgumentAccess(ByRef),Unary)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),Binary)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),Assign)`|load value of assignment,ldarg,store|
|`Assign(ArgumentAccess(ByRef),Convert)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),Is)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),FtnCall)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),VFtnCall)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),SFtnCall)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),ForeignCall)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),CtorCall)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),NewArray)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),LocalAccess)`|ldloc,ldarg,store|
|`Assign(ArgumentAccess(ByRef),ArgumentAccess(ByRef))`|ldarg(rhs),load,ldarg(lhs),store|
|`Assign(ArgumentAccess(ByRef),ArgumentAccess(ByVal))`|ldarg(rhs),ldarg(lhs),store|
|`Assign(ArgumentAccess(ByRef),OptInit)`| N/A |
|`Assign(ArgumentAccess(ByRef),FldAccess)`|ldfld,ldarg,store|
|`Assign(ArgumentAccess(ByRef),SFldAccess)`|ldsfld,ldarg,store|
|`Assign(ArgumentAccess(ByRef),EnumConst)`|ldem,ldarg,store|
|`Assign(ArgumentAccess(ByRef),SubScript)`|ldelem,ldarg,store|
|`Assign(ArgumentAccess(ByRef),ArrayLiteral)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),IntegerLiteral)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),DoubleLiteral)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),StringLiteral)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),StringLiteral)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),RuneLiteral)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),BooleanLiteral)`|ldarg,store|
|`Assign(ArgumentAccess(ByRef),NothingLiteral)`|ldarg,store|
|`Assign(ArgumentAccess(ByVal),Unary)`|starg|
|`Assign(ArgumentAccess(ByVal),Binary)`|starg|
|`Assign(ArgumentAccess(ByVal),Assign)`|load value of assigment,starg|
|`Assign(ArgumentAccess(ByVal),Convert)`|starg|
|`Assign(ArgumentAccess(ByVal),Is)`|starg|
|`Assign(ArgumentAccess(ByVal),FtnCall)`|starg|
|`Assign(ArgumentAccess(ByVal),VFtnCall)`|starg|
|`Assign(ArgumentAccess(ByVal),SFtnCall)`|starg|
|`Assign(ArgumentAccess(ByVal),ForeignCall)`|starg|
|`Assign(ArgumentAccess(ByVal),CtorCall)`|starg|
|`Assign(ArgumentAccess(ByVal),NewArray)`|starg|
|`Assign(ArgumentAccess(ByVal),LocalAccess)`|ldloc,starg|
|`Assign(ArgumentAccess(ByVal),ArgumentAccess(ByRef))`|ldarg(rhs),load,starg(lhs)|
|`Assign(ArgumentAccess(ByVal),ArgumentAccess(ByVal))`|ldarg(rhs),starg(lhs)|
|`Assign(ArgumentAccess(ByVal),OptInit)`| - |
|`Assign(ArgumentAccess(ByVal),FldAccess)`|ldfld,starg|
|`Assign(ArgumentAccess(ByVal),SFldAccess)`|ldsfld,starg|
|`Assign(ArgumentAccess(ByVal),EnumConst)`|ldem,starg|
|`Assign(ArgumentAccess(ByVal),SubScript)`|ldelem,starg|
|`Assign(ArgumentAccess(ByVal),ArrayLiteral)`|starg|
|`Assign(ArgumentAccess(ByVal),IntegerLiteral)`|starg|
|`Assign(ArgumentAccess(ByVal),DoubleLiteral)`|starg|
|`Assign(ArgumentAccess(ByVal),StringLiteral)`|starg|
|`Assign(ArgumentAccess(ByVal),StringLiteral)`|starg|
|`Assign(ArgumentAccess(ByVal),RuneLiteral)`|starg|
|`Assign(ArgumentAccess(ByVal),BooleanLiteral)`|starg|
|`Assign(ArgumentAccess(ByVal),NothingLiteral)`|starg|
|`Assign(FldAccess,Unary)`|stfld|
|`Assign(FldAccess,Binary)`|stfld|
|`Assign(FldAccess,Assign)`|stfld|
|`Assign(FldAccess,Convert)`|stfld|
|`Assign(FldAccess,Is)`|stfld|
|`Assign(FldAccess,FtnCall)`|stfld|
|`Assign(FldAccess,VFtnCall)`|stfld|
|`Assign(FldAccess,SFtnCall)`|stfld|
|`Assign(FldAccess,ForeignCall)`|stfld|
|`Assign(FldAccess,CtorCall)`|stfld|
|`Assign(FldAccess,NewArray)`|stfld|
|`Assign(FldAccess,LocalAccess)`|ldloc,stfld|
|`Assign(FldAccess,ArgumentAccess(ByRef))`|ldarg,load,stfld|
|`Assign(FldAccess,ArgumentAccess(ByVal))`|ldarg,stfld|
|`Assign(FldAccess,OptInit)`| N/A |
|`Assign(FldAccess,FldAccess)`|ldfld(rhs),stfld(lhs)|
|`Assign(FldAccess,SFldAccess)`|ldsfld,stfld|
|`Assign(FldAccess,EnumConst)`|ldem,stfld|
|`Assign(FldAccess,SubScript)`|ldelem,stfld|
|`Assign(FldAccess,ArrayLiteral)`|stfld|
|`Assign(FldAccess,IntegerLiteral)`|stfld|
|`Assign(FldAccess,DoubleLiteral)`|stfld|
|`Assign(FldAccess,StringLiteral)`|stfld|
|`Assign(FldAccess,StringLiteral)`|stfld|
|`Assign(FldAccess,RuneLiteral)`|stfld|
|`Assign(FldAccess,BooleanLiteral)`|stfld|
|`Assign(FldAccess,NothingLiteral)`|stfld|
|`Assign(SFldAccess,Unary)`|stsfld|
|`Assign(SFldAccess,Binary)`|stsfld|
|`Assign(SFldAccess,Assign)`|stsfld|
|`Assign(SFldAccess,Convert)`|stsfld|
|`Assign(SFldAccess,Is)`|stsfld|
|`Assign(SFldAccess,FtnCall)`|stsfld|
|`Assign(SFldAccess,VFtnCall)`|stsfld|
|`Assign(SFldAccess,SFtnCall)`|stsfld|
|`Assign(SFldAccess,ForeignCall)`|stsfld|
|`Assign(SFldAccess,CtorCall)`|stsfld|
|`Assign(SFldAccess,NewArray)`|stsfld|
|`Assign(SFldAccess,LocalAccess)`|ldloc,stsfld|
|`Assign(SFldAccess,ArgumentAccess(ByRef))`|ldarg,load,sfsfld|
|`Assign(SFldAccess,ArgumentAccess(ByVal))`|ldarg,stsfld|
|`Assign(SFldAccess,OptInit)`| N/A |
|`Assign(SFldAccess,FldAccess)`|ldfld,stsfld|
|`Assign(SFldAccess,SFldAccess)`|ldsfld(rhs),stsfld(lhs)|
|`Assign(SFldAccess,EnumConst)`|ldem,stsfld|
|`Assign(SFldAccess,SubScript)`|ldelem,stsfld|
|`Assign(SFldAccess,ArrayLiteral)`|stsfld|
|`Assign(SFldAccess,IntegerLiteral)`|stsfld|
|`Assign(SFldAccess,DoubleLiteral)`|stsfld|
|`Assign(SFldAccess,StringLiteral)`|stsfld|
|`Assign(SFldAccess,StringLiteral)`|stsfld|
|`Assign(SFldAccess,RuneLiteral)`|stsfld|
|`Assign(SFldAccess,BooleanLiteral)`|stsfld|
|`Assign(SFldAccess,NothingLiteral)`|stsfld|
|`Assign(SubScript,Unary)`|stelem|
|`Assign(SubScript,Binary)`|stelem|
|`Assign(SubScript,Assign)`|load value of assignment,stelem|
|`Assign(SubScript,Convert)`|stelem|
|`Assign(SubScript,Is)`|stelem|
|`Assign(SubScript,FtnCall)`|stelem|
|`Assign(SubScript,VFtnCall)`|stelem|
|`Assign(SubScript,SFtnCall)`|stelem|
|`Assign(SubScript,ForeignCall)`|stelem|
|`Assign(SubScript,CtorCall)`|stelem|
|`Assign(SubScript,NewArray)`|stelem|
|`Assign(SubScript,LocalAccess)`|ldloc,stelem|
|`Assign(SubScript,ArgumentAccess(ByRef))`|ldarg,load,stelem|
|`Assign(SubScript,ArgumentAccess(ByVal))`|ldarg,stelem|
|`Assign(SubScript,OptInit)`| N/A |
|`Assign(SubScript,FldAccess)`|ldfld,stelem|
|`Assign(SubScript,SFldAccess)`|ldsfld,stelem|
|`Assign(SubScript,EnumConst)`|ldem,stelem|
|`Assign(SubScript,SubScript)`|ldelem(rhs),stelem(lhs)|
|`Assign(SubScript,ArrayLiteral)`|stelem|
|`Assign(SubScript,IntegerLiteral)`|stelem|
|`Assign(SubScript,DoubleLiteral)`|stelem|
|`Assign(SubScript,StringLiteral)`|stelem|
|`Assign(SubScript,StringLiteral)`|stelem|
|`Assign(SubScript,RuneLiteral)`|stelem|
|`Assign(SubScript,BooleanLiteral)`|stelem|
|`Assign(SubScript,NothingLiteral)`|stelem|



## Convert

如果是primitive type，使用convert指令；
如果是类的ref，使用castClass替换convert。

|AST of lhs |Instructions|
|-|-|
|`Unary`|convert|
|`Binary`|convert|
|`Assign`|load value of assignment,convert|
|`Convert`|convert|
|`Is`|convert|
|`FtnCall`|convert|
|`VFtnCall`|convert|
|`SFtnCall`|convert|
|`ForeignCall`|convert|
|`CtorCall`|convert|
|`NewArray`|convert|
|`LocalAccess`|ldloc,convert|
|`ArgumentAccess(ByRef)`|ldarg,load,convert|
|`ArgumentAccess(ByVal)`|ldarg,convert|
|`OptInit`| N/A |
|`FldAccess`|ldfld,convert|
|`SFldAccess`|ldsfld,convert|
|`EnumConst`|ldem,convert|
|`SubScript`|ldelem,convert|
|`ArrayLiteral`|convert|
|`IntegerLiteral`|convert|
|`DoubleLiteral`|convert|
|`StringLiteral`|convert|
|`StringLiteral`|convert|
|`RuneLiteral`|convert|
|`BooleanLiteral`|convert|
|`NothingLiteral`|convert|

## Is
|AST of lhs |Instructions|
|-|-|
|`Unary`|instanceof|
|`Binary`|instanceof|
|`Assign`|load value of assignment,instanceof|
|`Convert`|instanceof|
|`Is`| N/A |
|`FtnCall`|instanceof|
|`VFtnCall`|instanceof|
|`SFtnCall`|instanceof|
|`ForeignCall`|instanceof|
|`CtorCall`|instanceof|
|`NewArray`|instanceof|
|`LocalAccess`|ldloc,instanceof|
|`ArgumentAccess(ByRef)`|ldarg,load,instanceof|
|`ArgumentAccess(ByVal)`|ldarg,instanceof|
|`OptInit`| N/A |
|`FldAccess`|ldfld,instanceof|
|`SFldAccess`|ldsfld,instanceof|
|`EnumConst`| N/A |
|`SubScript`|ldelem,instanceof|
|`ArrayLiteral`| N/A |
|`IntegerLiteral`| N/A |
|`DoubleLiteral`| N/A |
|`StringLiteral`| N/A |
|`StringLiteral`| N/A |
|`RuneLiteral`| N/A |
|`BooleanLiteral`| N/A |
|`NothingLiteral`| N/A |


## FtnCall
|AST of lhs|Instructions|
|-|-|
|`Unary`| N/A |
|`Binary`| N/A |
|`Assign`|load value of assignment,ldftn,callmethod|
|`Convert`|ldftn,callmethod|
|`Is`| N/A |
|`FtnCall`|ldftn,callmethod|
|`VFtnCall`|ldftn,callmethod|
|`SFtnCall`|ldftn,callmethod|
|`ForeignCall`|ldftn,callmethod|
|`CtorCall`|ldftn,callmethod|
|`NewArray`|ldftn,callmethod|
|`LocalAccess`|ldloc,ldftn,callmethod|
|`ArgumentAccess(ByRef)`|ldarg,load,ldftn,callmethod|
|`ArgumentAccess(ByVal)`|ldarg,ldftn,callmethod|
|`OptInit`| N/A |
|`FldAccess`|ldfld,ldftn,callmethod|
|`SFldAccess`|ldsfld,ldftn,callmethod|
|`EnumConst`| N/A |
|`SubScript`|ldelem,ldftn,callmethod|
|`ArrayLiteral`|ldftn,callmethod|
|`IntegerLiteral`| N/A |
|`DoubleLiteral`| N/A |
|`StringLiteral`| N/A |
|`StringLiteral`| N/A |
|`RuneLiteral`| N/A |
|`BooleanLiteral`| N/A |
|`NothingLiteral`| N/A |

## VFtnCall
|AST of lhs|Instructions|
|-|-|
|`Unary`| N/A |
|`Binary`| N/A |
|`Assign`|load value of assignment,ldvftn,callvirtual|
|`Convert`|ldvftn,callvirtual|
|`Is`| N/A |
|`FtnCall`|ldvftn,callvirtual|
|`VFtnCall`|ldvftn,callvirtual|
|`SFtnCall`|ldvftn,callvirtual|
|`ForeignCall`|ldvftn,callvirtual|
|`CtorCall`|ldvftn,callvirtual|
|`NewArray`|ldvftn,callvirtual|
|`LocalAccess`|ldloc,ldvftn,callvirtual|
|`ArgumentAccess(ByRef)`|ldarg,load,ldvftn,callvirtual|
|`ArgumentAccess(ByVal)`|ldarg,ldvftn,callvirtual|
|`OptInit`| N/A |
|`FldAccess`|ldfld,ldvftn,callvirtual|
|`SFldAccess`|ldsfld,ldvftn,callvirtual|
|`EnumConst`| N/A |
|`SubScript`|ldelem,ldvftn,callvirtual|
|`ArrayLiteral`|ldvftn,callvirtual|
|`IntegerLiteral`| N/A |
|`DoubleLiteral`| N/A |
|`StringLiteral`| N/A |
|`StringLiteral`| N/A |
|`RuneLiteral`| N/A |
|`BooleanLiteral`| N/A |
|`NothingLiteral`| N/A |

## SFtnCall
|AST|Instructions|
|-|-|
| `SFtnCall` | ldsftn,callstatic |


## ForeignCall
|AST|Instructions|
|-|-|
| `ForeignCall` | ldforeign,callforeign |

## CtorCall
|AST|Instructions|
|-|-|
| `CtorCall` | ldctor,callctor |

## NewArray
|AST|Instructions|
|-|-|
| `NewArray(size)` | push size,newarray |

## LocalAccess
|AST|Instructions|
|-|-|
|`LocalAccess`|push offset|

## ArgumentAccess
|AST|Instructions|
|-|-|
|`ArgumentAccess`|push offset|

## ArgumentPass
传递参数给函数时的AST。

|AST|Instructions|
|-|-|
|`ArgumentPass(ByRef,Unary)`| N/A |
|`ArgumentPass(ByRef,Binary)`| N/A |
|`ArgumentPass(ByRef,Assign)`|load address of assignemnt|
|`ArgumentPass(ByRef,Convert)`| N/A |
|`ArgumentPass(ByRef,Is)`| N/A |
|`ArgumentPass(ByRef,FtnCall)`| N/A |
|`ArgumentPass(ByRef,VFtnCall)`| N/A |
|`ArgumentPass(ByRef,SFtnCall)`| N/A |
|`ArgumentPass(ByRef,ForeignCall)`| N/A |
|`ArgumentPass(ByRef,CtorCall)`| N/A |
|`ArgumentPass(ByRef,NewArray)`| N/A |
|`ArgumentPass(ByRef,LocalAccess)`|ldloca|
|`ArgumentPass(ByRef,ArgumentAccess(ByRef))`|ldarg|
|`ArgumentPass(ByRef,ArgumentAccess(ByVal))`|ldarga|
|`ArgumentPass(ByRef,OptInit)`| N/A |
|`ArgumentPass(ByRef,FldAccess)`|ldflda|
|`ArgumentPass(ByRef,SFldAccess)`|ldsflda|
|`ArgumentPass(ByRef,EnumConst)`| N/A |
|`ArgumentPass(ByRef,SubScript)`|ldelema|
|`ArgumentPass(ByRef,ArrayLiteral)`| N/A |
|`ArgumentPass(ByRef,IntegerLiteral)`| N/A |
|`ArgumentPass(ByRef,DoubleLiteral)`| N/A |
|`ArgumentPass(ByRef,StringLiteral)`| N/A |
|`ArgumentPass(ByRef,StringLiteral)`| N/A |
|`ArgumentPass(ByRef,RuneLiteral)`| N/A |
|`ArgumentPass(ByRef,BooleanLiteral)`| N/A |
|`ArgumentPass(ByRef,NothingLiteral)`| N/A |
|`ArgumentPass(ByVal,Unary)`| - |
|`ArgumentPass(ByVal,Binary)`| - |
|`ArgumentPass(ByVal,Assign)`| load value of assignment |
|`ArgumentPass(ByVal,Convert)`| - |
|`ArgumentPass(ByVal,Is)`| - |
|`ArgumentPass(ByVal,FtnCall)`| - |
|`ArgumentPass(ByVal,VFtnCall)`| - |
|`ArgumentPass(ByVal,SFtnCall)`| - |
|`ArgumentPass(ByVal,ForeignCall)`| - |
|`ArgumentPass(ByVal,CtorCall)`| - |
|`ArgumentPass(ByVal,NewArray)`| - |
|`ArgumentPass(ByVal,LocalAccess)`|ldloc|
|`ArgumentPass(ByVal,ArgumentAccess(ByRef))`|ldarg,load|
|`ArgumentPass(ByVal,ArgumentAccess(ByVal))`|ldarg|
|`ArgumentPass(ByVal,OptInit)`| - |
|`ArgumentPass(ByVal,FldAccess)`|ldfld|
|`ArgumentPass(ByVal,SFldAccess)`|ldsfld|
|`ArgumentPass(ByVal,EnumConst)`|ldem|
|`ArgumentPass(ByVal,SubScript)`|ldelem|
|`ArgumentPass(ByVal,ArrayLiteral)`| - |
|`ArgumentPass(ByVal,IntegerLiteral)`| - |
|`ArgumentPass(ByVal,DoubleLiteral)`| - |
|`ArgumentPass(ByVal,StringLiteral)`| - |
|`ArgumentPass(ByVal,StringLiteral)`| - |
|`ArgumentPass(ByVal,RuneLiteral)`| - |
|`ArgumentPass(ByVal,BooleanLiteral)`| - |
|`ArgumentPass(ByVal,NothingLiteral)`| - |

## OptInit
|AST|Instructions|
|-|-|
|`OptInit`|push token of optional parameter name|

## FldAccess
|AST|Instructions|
|-|-|
|`FldAccess(<other operand types>,Unary)`| N/A |
|`FldAccess(<other operand types>,Binary)`| N/A |
|`FldAccess(<other operand types>,Assign)`|load value of assignment, push.u8 1|
|`FldAccess(<other operand types>,Convert)`| N/A |
|`FldAccess(<other operand types>,Is)`| N/A |
|`FldAccess(<other operand types>,FtnCall)`| - , push.u8 1|
|`FldAccess(<other operand types>,VFtnCall)`| - , push.u8 1|
|`FldAccess(<other operand types>,SFtnCall)`| - , push.u8 1|
|`FldAccess(<other operand types>,ForeignCall)`| - , push.u8 1|
|`FldAccess(<other operand types>,CtorCall)`| - , push.u8 1|
|`FldAccess(<other operand types>,NewArray)`| N/A |
|`FldAccess(<other operand types>,LocalAccess)`|ldloc, push.u8 1|
|`FldAccess(<other operand types>,ArgumentAccess(ByRef))`|ldarg,load, push.u8 1|
|`FldAccess(<other operand types>,ArgumentAccess(ByVal))`|ldarg, push.u8 1|
|`FldAccess(<other operand types>,OptInit)`| N/A |
|`FldAccess(<other operand types>,FldAccess)`|ldfld(rhs), push.u8 1|
|`FldAccess(<other operand types>,SFldAccess)`|ldsfld, push.u8 1|
|`FldAccess(<other operand types>,EnumConst)`| N/A |
|`FldAccess(<other operand types>,SubScript)`|ldelem, push.u8 1|
|`FldAccess(<other operand types>,ArrayLiteral)`| N/A |
|`FldAccess(<other operand types>,IntegerLiteral)`| N/A |
|`FldAccess(<other operand types>,DoubleLiteral)`| N/A |
|`FldAccess(<other operand types>,StringLiteral)`| N/A |
|`FldAccess(<other operand types>,RuneLiteral)`| N/A |
|`FldAccess(<other operand types>,BooleanLiteral)`| N/A |
|`FldAccess(<other operand types>,NothingLiteral)`| N/A |
|`FldAccess(Record,Unary)`| N/A |
|`FldAccess(Record,Binary)`| N/A |
|`FldAccess(Record,Assign)`|load address of assignment, push.u8 2|
|`FldAccess(Record,Convert)`| N/A |
|`FldAccess(Record,Is)`| N/A |
|`FldAccess(Record,FtnCall)`| - , push.u8 3|
|`FldAccess(Record,VFtnCall)`| - , push.u8 3|
|`FldAccess(Record,SFtnCall)`| - , push.u8 3|
|`FldAccess(Record,ForeignCall)`| - , push.u8 3|
|`FldAccess(Record,CtorCall)`| N/A |
|`FldAccess(Record,NewArray)`| N/A |
|`FldAccess(Record,LocalAccess)`|ldloca, push.u8 2|
|`FldAccess(Record,ArgumentAccess(ByRef))`|ldarg, push.u8 2|
|`FldAccess(Record,ArgumentAccess(ByVal))`|ldarga, push.u8 2|
|`FldAccess(Record,OptInit)`| N/A |
|`FldAccess(Record,FldAccess)`|ldflda, push.u8 2|
|`FldAccess(Record,SFldAccess)`|ldsflda, push.u8 2|
|`FldAccess(Record,EnumConst)`| N/A |
|`FldAccess(Record,SubScript)`|ldelema, push.u8 2|
|`FldAccess(Record,ArrayLiteral)`| N/A |
|`FldAccess(Record,IntegerLiteral)`| N/A |
|`FldAccess(Record,DoubleLiteral)`| N/A |
|`FldAccess(Record,StringLiteral)`| N/A |
|`FldAccess(Record,StringLiteral)`| N/A |
|`FldAccess(Record,RuneLiteral)`| N/A |
|`FldAccess(Record,BooleanLiteral)`| N/A |
|`FldAccess(Record,NothingLiteral)`| N/A |


## SFldAccess
|AST|Instructions|
|-|-|
|`SFldAccess`| - |



## EnumConst
|AST|Instructions|
|-|-|
|`EnumConst`|ldem|

## Subscript
|AST|Instructions|
|-|-|
|`Subscript(Assign,Unary)`|load value of assignment|
|`Subscript(Assign,Binary)`|load value of assignment|
|`Subscript(Assign,Assign)`|load value of assignment(lhs),load value of assignment(rhs)|
|`Subscript(Assign,Convert)`|load value of assignment|
|`Subscript(Assign,Is)`|load value of assignment|
|`Subscript(Assign,FtnCall)`|load value of assignment|
|`Subscript(Assign,VFtnCall)`|load value of assignment|
|`Subscript(Assign,SFtnCall)`|load value of assignment|
|`Subscript(Assign,ForeignCall)`|load value of assignment|
|`Subscript(Assign,CtorCall)`|load value of assignment|
|`Subscript(Assign,NewArray)`|load value of assignment|
|`Subscript(Assign,LocalAccess)`|load value of assignment,ldloc|
|`Subscript(Assign,ArgumentAccess(ByRef))`|load value of assignment,ldarg,load|
|`Subscript(Assign,ArgumentAccess(ByVal))`|load value of assignment,ldarg|
|`Subscript(Assign,OptInit)`| N/A |
|`Subscript(Assign,FldAccess)`|load value of assignment,ldfld|
|`Subscript(Assign,SFldAccess)`|load value of assignment,ldsfld|
|`Subscript(Assign,EnumConst)`| N/A |
|`Subscript(Assign,SubScript)`|load value of assignment,ldelem|
|`Subscript(Assign,ArrayLiteral)`| N/A |
|`Subscript(Assign,IntegerLiteral)`|load value of assignment|
|`Subscript(Assign,DoubleLiteral)`| N/A |
|`Subscript(Assign,StringLiteral)`| N/A |
|`Subscript(Assign,RuneLiteral)`| N/A |
|`Subscript(Assign,BooleanLiteral)`| N/A |
|`Subscript(Assign,NothingLiteral)`| N/A |
|`Subscript(FtnCall,Unary)`| - |
|`Subscript(FtnCall,Binary)`| - |
|`Subscript(FtnCall,Assign)`| - |
|`Subscript(FtnCall,Convert)`| - |
|`Subscript(FtnCall,Is)`| N/A |
|`Subscript(FtnCall,FtnCall)`| - |
|`Subscript(FtnCall,VFtnCall)`| - |
|`Subscript(FtnCall,SFtnCall)`| - |
|`Subscript(FtnCall,ForeignCall)`| - |
|`Subscript(FtnCall,CtorCall)`| N/A |
|`Subscript(FtnCall,NewArray)`| N/A |
|`Subscript(FtnCall,LocalAccess)`| ldloc |
|`Subscript(FtnCall,ArgumentAccess(ByRef))`|ldarg,load|
|`Subscript(FtnCall,ArgumentAccess(ByVal))`|ldarg|
|`Subscript(FtnCall,OptInit)`| N/A |
|`Subscript(FtnCall,FldAccess)`|ldfld|
|`Subscript(FtnCall,SFldAccess)`|ldsfld|
|`Subscript(FtnCall,EnumConst)`| N/A |
|`Subscript(FtnCall,SubScript)`|ldelem|
|`Subscript(FtnCall,ArrayLiteral)`| N/A |
|`Subscript(FtnCall,IntegerLiteral)`| - |
|`Subscript(FtnCall,DoubleLiteral)`| N/A |
|`Subscript(FtnCall,StringLiteral)`| N/A |
|`Subscript(FtnCall,RuneLiteral)`| N/A |
|`Subscript(FtnCall,BooleanLiteral)`| N/A |
|`Subscript(FtnCall,NothingLiteral)`| N/A |
|`Subscript(VFtnCall,Unary)`| - |
|`Subscript(VFtnCall,Binary)`| - |
|`Subscript(VFtnCall,Assign)`|load value of assignment|
|`Subscript(VFtnCall,Convert)`| - |
|`Subscript(VFtnCall,Is)`| N/A |
|`Subscript(VFtnCall,FtnCall)`| - |
|`Subscript(VFtnCall,VFtnCall)`| - |
|`Subscript(VFtnCall,SFtnCall)`| - |
|`Subscript(VFtnCall,ForeignCall)`| - |
|`Subscript(VFtnCall,CtorCall)`| - |
|`Subscript(VFtnCall,NewArray)`| N/A |
|`Subscript(VFtnCall,LocalAccess)`|ldloc|
|`Subscript(VFtnCall,ArgumentAccess(ByRef))`|ldarg,load|
|`Subscript(VFtnCall,ArgumentAccess(ByVal))`|ldarg|
|`Subscript(VFtnCall,OptInit)`| - |
|`Subscript(VFtnCall,FldAccess)`|ldfld|
|`Subscript(VFtnCall,SFldAccess)`|ldsfld|
|`Subscript(VFtnCall,EnumConst)`| N/A |
|`Subscript(VFtnCall,SubScript)`|ldelem|
|`Subscript(VFtnCall,ArrayLiteral)`| N/A |
|`Subscript(VFtnCall,IntegerLiteral)`| - |
|`Subscript(VFtnCall,DoubleLiteral)`| N/A |
|`Subscript(VFtnCall,StringLiteral)`| N/A |
|`Subscript(VFtnCall,RuneLiteral)`| N/A |
|`Subscript(VFtnCall,BooleanLiteral)`| N/A |
|`Subscript(VFtnCall,NothingLiteral)`| N/A |
|`Subscript(SFtnCall,Unary)`| - |
|`Subscript(SFtnCall,Binary)`| - |
|`Subscript(SFtnCall,Assign)`|load value of assignment|
|`Subscript(SFtnCall,Convert)`| N/A |
|`Subscript(SFtnCall,Is)`| N/A |
|`Subscript(SFtnCall,FtnCall)`| - |
|`Subscript(SFtnCall,VFtnCall)`| - |
|`Subscript(SFtnCall,SFtnCall)`| - |
|`Subscript(SFtnCall,ForeignCall)`| - |
|`Subscript(SFtnCall,CtorCall)`| - |
|`Subscript(SFtnCall,NewArray)`| N/A |
|`Subscript(SFtnCall,LocalAccess)`|ldloc|
|`Subscript(SFtnCall,ArgumentAccess(ByRef))`|ldarg,load|
|`Subscript(SFtnCall,ArgumentAccess(ByVal))`|ldarg|
|`Subscript(SFtnCall,OptInit)`| N/A |
|`Subscript(SFtnCall,FldAccess)`|ldfld|
|`Subscript(SFtnCall,SFldAccess)`|ldsfld|
|`Subscript(SFtnCall,EnumConst)`| N/A |
|`Subscript(SFtnCall,SubScript)`|ldelem|
|`Subscript(SFtnCall,ArrayLiteral)`| N/A |
|`Subscript(SFtnCall,IntegerLiteral)`| - |
|`Subscript(SFtnCall,DoubleLiteral)`| N/A |
|`Subscript(SFtnCall,StringLiteral)`| N/A |
|`Subscript(SFtnCall,RuneLiteral)`| N/A |
|`Subscript(SFtnCall,BooleanLiteral)`| N/A |
|`Subscript(SFtnCall,NothingLiteral)`| N/A |
|`Subscript(ForeignCall,Unary)`| - |
|`Subscript(ForeignCall,Binary)`| - |
|`Subscript(ForeignCall,Assign)`|load value of assignment|
|`Subscript(ForeignCall,Convert)`| - |
|`Subscript(ForeignCall,Is)`| N/A |
|`Subscript(ForeignCall,FtnCall)`| - |
|`Subscript(ForeignCall,VFtnCall)`| - |
|`Subscript(ForeignCall,SFtnCall)`| - |
|`Subscript(ForeignCall,ForeignCall)`| - |
|`Subscript(ForeignCall,CtorCall)`| N/A |
|`Subscript(ForeignCall,NewArray)`| N/A |
|`Subscript(ForeignCall,LocalAccess)`|ldloc|
|`Subscript(ForeignCall,ArgumentAccess(ByRef))`|ldarg,load|
|`Subscript(ForeignCall,ArgumentAccess(ByVal))`|ldarg|
|`Subscript(ForeignCall,OptInit)`| N/A |
|`Subscript(ForeignCall,FldAccess)`|ldfld|
|`Subscript(ForeignCall,SFldAccess)`|ldsfld|
|`Subscript(ForeignCall,EnumConst)`| N/A |
|`Subscript(ForeignCall,SubScript)`|ldelem|
|`Subscript(ForeignCall,ArrayLiteral)`| N/A |
|`Subscript(ForeignCall,IntegerLiteral)`| - |
|`Subscript(ForeignCall,DoubleLiteral)`| N/A |
|`Subscript(ForeignCall,StringLiteral)`| N/A |
|`Subscript(ForeignCall,RuneLiteral)`| N/A |
|`Subscript(ForeignCall,BooleanLiteral)`| N/A |
|`Subscript(ForeignCall,NothingLiteral)`| N/A |
|`Subscript(NewArray,Unary)`| - |
|`Subscript(NewArray,Binary)`| - |
|`Subscript(NewArray,Assign)`|load value of assignment|
|`Subscript(NewArray,Convert)`| - |
|`Subscript(NewArray,Is)`| N/A |
|`Subscript(NewArray,FtnCall)`| - |
|`Subscript(NewArray,VFtnCall)`| - |
|`Subscript(NewArray,SFtnCall)`| - |
|`Subscript(NewArray,ForeignCall)`| - |
|`Subscript(NewArray,CtorCall)`| N/A |
|`Subscript(NewArray,NewArray)`| N/A |
|`Subscript(NewArray,LocalAccess)`|ldloc|
|`Subscript(NewArray,ArgumentAccess(ByRef))`|ldarg,load|
|`Subscript(NewArray,ArgumentAccess(ByVal))`|ldarg|
|`Subscript(NewArray,OptInit)`| N/A |
|`Subscript(NewArray,FldAccess)`|ldfld|
|`Subscript(NewArray,SFldAccess)`|ldsfld|
|`Subscript(NewArray,EnumConst)`| N/A |
|`Subscript(NewArray,SubScript)`|ldelem|
|`Subscript(NewArray,ArrayLiteral)`| N/A |
|`Subscript(NewArray,IntegerLiteral)`| - |
|`Subscript(NewArray,DoubleLiteral)`| N/A |
|`Subscript(NewArray,StringLiteral)`| N/A |
|`Subscript(NewArray,RuneLiteral)`| N/A |
|`Subscript(NewArray,BooleanLiteral)`| N/A |
|`Subscript(NewArray,NothingLiteral)`| N/A |
|`Subscript(LocalAccess,Unary)`| ldloc |
|`Subscript(LocalAccess,Binary)`| ldloc |
|`Subscript(LocalAccess,Assign)`|ldloc,load value of assign|
|`Subscript(LocalAccess,Convert)`| ldloc |
|`Subscript(LocalAccess,Is)`| N/A |
|`Subscript(LocalAccess,FtnCall)`|ldloc|
|`Subscript(LocalAccess,VFtnCall)`|ldloc|
|`Subscript(LocalAccess,SFtnCall)`|ldloc|
|`Subscript(LocalAccess,ForeignCall)`|ldloc|
|`Subscript(LocalAccess,CtorCall)`| N/A |
|`Subscript(LocalAccess,NewArray)`| N/A |
|`Subscript(LocalAccess,LocalAccess)`|ldloc(lhs),ldloc(rhs)|
|`Subscript(LocalAccess,ArgumentAccess(ByRef))`|ldloc,ldarg,load|
|`Subscript(LocalAccess,ArgumentAccess(ByVal))`|ldloc,ldarg|
|`Subscript(LocalAccess,OptInit)`| N/A |
|`Subscript(LocalAccess,FldAccess)`|ldloc,ldfld|
|`Subscript(LocalAccess,SFldAccess)`|ldloc,ldsfld|
|`Subscript(LocalAccess,EnumConst)`| N/A |
|`Subscript(LocalAccess,SubScript)`|ldloc,ldelem|
|`Subscript(LocalAccess,ArrayLiteral)`| N/A |
|`Subscript(LocalAccess,IntegerLiteral)`|ldloc|
|`Subscript(LocalAccess,DoubleLiteral)`| N/A |
|`Subscript(LocalAccess,StringLiteral)`| N/A |
|`Subscript(LocalAccess,RuneLiteral)`| N/A |
|`Subscript(LocalAccess,BooleanLiteral)`| N/A |
|`Subscript(LocalAccess,NothingLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByRef),Unary)`|ldarg,load|
|`Subscript(ArgumentAccess(ByRef),Binary)`|ldarg,load|
|`Subscript(ArgumentAccess(ByRef),Assign)`|ldarg,load|
|`Subscript(ArgumentAccess(ByRef),Convert)`|ldarg,load|
|`Subscript(ArgumentAccess(ByRef),Is)`| N/A |
|`Subscript(ArgumentAccess(ByRef),FtnCall)`|ldarg,load|
|`Subscript(ArgumentAccess(ByRef),VFtnCall)`|ldarg,load|
|`Subscript(ArgumentAccess(ByRef),SFtnCall)`|ldarg,load|
|`Subscript(ArgumentAccess(ByRef),ForeignCall)`|ldarg,load|
|`Subscript(ArgumentAccess(ByRef),CtorCall)`| N/A |
|`Subscript(ArgumentAccess(ByRef),NewArray)`| N/A |
|`Subscript(ArgumentAccess(ByRef),LocalAccess)`|ldarg,load,ldloc|
|`Subscript(ArgumentAccess(ByRef),ArgumentAccess(ByRef))`|ldarg(lhs),load(lhs),ldarg(rhs),load(rhs)|
|`Subscript(ArgumentAccess(ByRef),ArgumentAccess(ByVal))`|ldarg(lhs),load,ldarg(rhs)|
|`Subscript(ArgumentAccess(ByRef),OptInit)`| N/A |
|`Subscript(ArgumentAccess(ByRef),FldAccess)`|ldarg,load,ldfld|
|`Subscript(ArgumentAccess(ByRef),SFldAccess)`|ldarg,load,ldsfld|
|`Subscript(ArgumentAccess(ByRef),EnumConst)`| N/A |
|`Subscript(ArgumentAccess(ByRef),SubScript)`|ldarg,load,ldelem|
|`Subscript(ArgumentAccess(ByRef),ArrayLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByRef),IntegerLiteral)`|ldarg,load|
|`Subscript(ArgumentAccess(ByRef),DoubleLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByRef),StringLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByRef),RuneLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByRef),BooleanLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByRef),NothingLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByVal),Unary)`|ldarg|
|`Subscript(ArgumentAccess(ByVal),Binary)`|ldarg|
|`Subscript(ArgumentAccess(ByVal),Assign)`|ldarg|
|`Subscript(ArgumentAccess(ByVal),Convert)`|ldarg|
|`Subscript(ArgumentAccess(ByVal),Is)`| N/A |
|`Subscript(ArgumentAccess(ByVal),FtnCall)`|ldarg|
|`Subscript(ArgumentAccess(ByVal),VFtnCall)`|ldarg|
|`Subscript(ArgumentAccess(ByVal),SFtnCall)`|ldarg|
|`Subscript(ArgumentAccess(ByVal),ForeignCall)`|ldarg|
|`Subscript(ArgumentAccess(ByVal),CtorCall)`| N/A |
|`Subscript(ArgumentAccess(ByVal),NewArray)`| N/A |
|`Subscript(ArgumentAccess(ByVal),LocalAccess)`|ldarg,ldloc|
|`Subscript(ArgumentAccess(ByVal),ArgumentAccess(ByRef))`|ldarg(lhs),ldarg(rhs),load(rhs)|
|`Subscript(ArgumentAccess(ByVal),ArgumentAccess(ByVal))`|ldarg(lhs),ldarg(rhs)|
|`Subscript(ArgumentAccess(ByVal),OptInit)`| N/A |
|`Subscript(ArgumentAccess(ByVal),FldAccess)`|ldarg,ldfld|
|`Subscript(ArgumentAccess(ByVal),SFldAccess)`|ldarg,ldsfld|
|`Subscript(ArgumentAccess(ByVal),EnumConst)`| N/A |
|`Subscript(ArgumentAccess(ByVal),SubScript)`|ldarg,ldelem|
|`Subscript(ArgumentAccess(ByVal),ArrayLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByVal),IntegerLiteral)`| - |
|`Subscript(ArgumentAccess(ByVal),DoubleLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByVal),StringLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByVal),RuneLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByVal),BooleanLiteral)`| N/A |
|`Subscript(ArgumentAccess(ByVal),NothingLiteral)`| N/A |
|`Subscript(FldAccess,Unary)`|ldfld|
|`Subscript(FldAccess,Binary)`|ldfld|
|`Subscript(FldAccess,Assign)`|ldfld,load value of assignment|
|`Subscript(FldAccess,Convert)`|ldfld|
|`Subscript(FldAccess,Is)`| N/A |
|`Subscript(FldAccess,FtnCall)`|ldfld|
|`Subscript(FldAccess,VFtnCall)`|ldfld|
|`Subscript(FldAccess,SFtnCall)`|ldfld|
|`Subscript(FldAccess,ForeignCall)`|ldfld|
|`Subscript(FldAccess,CtorCall)`| N/A |
|`Subscript(FldAccess,NewArray)`| N/A |
|`Subscript(FldAccess,LocalAccess)`|ldfld,ldloc|
|`Subscript(FldAccess,ArgumentAccess(ByRef))`|ldfld,ldarg,load|
|`Subscript(FldAccess,ArgumentAccess(ByVal))`|ldfld,ldarg|
|`Subscript(FldAccess,OptInit)`| N/A |
|`Subscript(FldAccess,FldAccess)`|ldfld(lhs),ldfld(rhs)|
|`Subscript(FldAccess,SFldAccess)`|ldfld,ldsfld|
|`Subscript(FldAccess,EnumConst)`| N/A |
|`Subscript(FldAccess,SubScript)`|ldfld,ldelem|
|`Subscript(FldAccess,ArrayLiteral)`| N/A |
|`Subscript(FldAccess,IntegerLiteral)`|ldfld|
|`Subscript(FldAccess,DoubleLiteral)`| N/A |
|`Subscript(FldAccess,StringLiteral)`| N/A |
|`Subscript(FldAccess,RuneLiteral)`| N/A |
|`Subscript(FldAccess,BooleanLiteral)`| N/A |
|`Subscript(FldAccess,NothingLiteral)`| N/A |
|`Subscript(SFldAccess,Unary)`|ldsfld|
|`Subscript(SFldAccess,Binary)`|ldsfld|
|`Subscript(SFldAccess,Assign)`|ldsfld,load value of assignment|
|`Subscript(SFldAccess,Convert)`|ldsfld|
|`Subscript(SFldAccess,Is)`| N/A |
|`Subscript(SFldAccess,FtnCall)`|ldsfld|
|`Subscript(SFldAccess,VFtnCall)`|ldsfld|
|`Subscript(SFldAccess,SFtnCall)`|ldsfld|
|`Subscript(SFldAccess,ForeignCall)`|ldsfld|
|`Subscript(SFldAccess,CtorCall)`|ldsfld|
|`Subscript(SFldAccess,NewArray)`|ldsfld|
|`Subscript(SFldAccess,LocalAccess)`|ldsfld,ldloc|
|`Subscript(SFldAccess,ArgumentAccess(ByRef))`|ldsfld,ldarg,load|
|`Subscript(SFldAccess,ArgumentAccess(ByVal))`|ldsfld,ldarg|
|`Subscript(SFldAccess,OptInit)`| N/A |
|`Subscript(SFldAccess,FldAccess)`|ldsfld,ldfld|
|`Subscript(SFldAccess,SFldAccess)`|ldsfld(lhs),ldsfld(rhs)|
|`Subscript(SFldAccess,EnumConst)`| N/A |
|`Subscript(SFldAccess,SubScript)`|ldsfld,ldelem|
|`Subscript(SFldAccess,ArrayLiteral)`| N/A |
|`Subscript(SFldAccess,IntegerLiteral)`|ldsfld|
|`Subscript(SFldAccess,DoubleLiteral)`| N/A |
|`Subscript(SFldAccess,StringLiteral)`| N/A |
|`Subscript(SFldAccess,RuneLiteral)`| N/A |
|`Subscript(SFldAccess,BooleanLiteral)`| N/A |
|`Subscript(SFldAccess,NothingLiteral)`| N/A |
|`Subscript(SubScript,Unary)`|ldelem|
|`Subscript(SubScript,Binary)`|ldelem|
|`Subscript(SubScript,Assign)`|ldelem,load value of assignment|
|`Subscript(SubScript,Convert)`|ldelem|
|`Subscript(SubScript,Is)`| N/A |
|`Subscript(SubScript,FtnCall)`|ldelem|
|`Subscript(SubScript,VFtnCall)`|ldelem|
|`Subscript(SubScript,SFtnCall)`|ldelem|
|`Subscript(SubScript,ForeignCall)`|ldelem|
|`Subscript(SubScript,CtorCall)`| N/A |
|`Subscript(SubScript,NewArray)`| N/A |
|`Subscript(SubScript,LocalAccess)`|ldelem,ldloc|
|`Subscript(SubScript,ArgumentAccess(ByRef))`|ldelem,ldarg,load|
|`Subscript(SubScript,ArgumentAccess(ByVal))`|ldelem,ldarg|
|`Subscript(SubScript,OptInit)`| N/A |
|`Subscript(SubScript,FldAccess)`|ldelem,ldfld|
|`Subscript(SubScript,SFldAccess)`|ldelem,ldsfld|
|`Subscript(SubScript,EnumConst)`| N/A |
|`Subscript(SubScript,SubScript)`|ldelem(lhs),ldelem(rhs)|
|`Subscript(SubScript,ArrayLiteral)`| N/A |
|`Subscript(SubScript,IntegerLiteral)`|ldelem|
|`Subscript(SubScript,DoubleLiteral)`| N/A |
|`Subscript(SubScript,StringLiteral)`| N/A |
|`Subscript(SubScript,RuneLiteral)`| N/A |
|`Subscript(SubScript,BooleanLiteral)`| N/A |
|`Subscript(SubScript,NothingLiteral)`| N/A |
|`Subscript(ArrayLiteral,Unary)`| - |
|`Subscript(ArrayLiteral,Binary)`| - |
|`Subscript(ArrayLiteral,Assign)`|load value of assignment|
|`Subscript(ArrayLiteral,Convert)`| - |
|`Subscript(ArrayLiteral,Is)`| N/A |
|`Subscript(ArrayLiteral,FtnCall)`| - |
|`Subscript(ArrayLiteral,VFtnCall)`| - |
|`Subscript(ArrayLiteral,SFtnCall)`| - |
|`Subscript(ArrayLiteral,ForeignCall)`| - |
|`Subscript(ArrayLiteral,CtorCall)`| N/A |
|`Subscript(ArrayLiteral,NewArray)`| N/A |
|`Subscript(ArrayLiteral,LocalAccess)`|ldloc|
|`Subscript(ArrayLiteral,ArgumentAccess(ByRef))`|ldarg,load|
|`Subscript(ArrayLiteral,ArgumentAccess(ByVal))`|ldarg|
|`Subscript(ArrayLiteral,OptInit)`| N/A |
|`Subscript(ArrayLiteral,FldAccess)`|ldfld|
|`Subscript(ArrayLiteral,SFldAccess)`|ldsfld|
|`Subscript(ArrayLiteral,EnumConst)`| N/A |
|`Subscript(ArrayLiteral,SubScript)`|ldelem|
|`Subscript(ArrayLiteral,ArrayLiteral)`| N/A |
|`Subscript(ArrayLiteral,IntegerLiteral)`| - |
|`Subscript(ArrayLiteral,DoubleLiteral)`| N/A |
|`Subscript(ArrayLiteral,StringLiteral)`| N/A |
|`Subscript(ArrayLiteral,RuneLiteral)`| N/A |
|`Subscript(ArrayLiteral,BooleanLiteral)`| N/A |
|`Subscript(ArrayLiteral,NothingLiteral)`| N/A |

## ArrayLiteral
|AST|Instructions|
|-|-|
|`ArrayLiteral`|使用newarray创建数组，将所有元素存入数组并返回数组引用|

## IntegerLiteral
|AST|Instructions|
|-|-|
|`IntegerLiteral`|push|

## DoubleLiteral
|AST|Instructions|
|-|-|
|`DoubleLiteral`|push|

## StringLiteral
|AST|Instructions|
|-|-|
|`StringLiteral`|load StringLiteral object|

## RuneLiteral
|AST|Instructions|
|-|-|
|`RuneLiteral`|push|

## BooleanLiteral
|AST|Instructions|
|-|-|
|`BooleanLiteral`|push|

## NothingLiteral
|AST|Instructions|
|-|-|
|`BooleanLiteral`|ldnothing|
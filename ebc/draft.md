ModName
ClsName
VarName
FtnName
ItfName
EnumName
EmConstName

Mod Cls Var Ftn Itf Enum EnumConst Record

Mod . Mod -> find snd mod in fst mod
Mod . Cls -> find cls in mod
Mod . Var -> find var in mod
Mod . Ftn -> find ftn in mod
Mod . Enum -> find enum in mod
Mod . EmConst -> ERROR

Cls . Mod -> ERROR
Cls . Cls -> ERROR
Cls . Var(Non-static) -> ERROR
Cls . Var(Static) -> find static var in cls
Cls . Ftn(Non-static) -> ERROR
Cls . Ftn(Static) -> find static ftn in cls
Cls . Enum -> find enum in cls
Cls . EmConst -> ERROR

Var . Mod -> ERROR
Var . Cls -> ERROR
Var . Var -> find snd var in fst var type
Var . Ftn -> find ftn in var type
Var . Enum -> ERROR
Var . EmConst -> ERROR

Ftn . Mod
Ftn . Cls
Ftn . Var
Ftn . Enum
Ftn . EmConst -> ERROR

Enum . EmConst -> find EmConst in enum

Record . Var -> find variable in record



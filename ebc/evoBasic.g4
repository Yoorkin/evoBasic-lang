grammar evoBasic;

document : declaration* EOF;


declaration : accessFlag? declarationNoAccessFlag;

declarationNoAccessFlag : dimDecl
                      | typeDecl
                      | enumDecl
                      | externalFunc
                      | externalSub
                      | importDecl
                      | functionDecl
                      | subDecl
                      | moduleDecl
                      | classDecl
                      | ctorDecl
                      | newline
                      ;

ctorDecl : New parameterList extendPart? newline statement* End New newline;

extendPart : newline? Extend argumentList;

classDecl : Class ID (Extend extend=annotation)? implPart? newline members+=declaration* End Class newline;

implPart : (Impl annotation (',' annotation));


moduleDecl : Module ID newline members+=declaration* End Module newline;

dimDecl : (Dim|Const|Static) variableDecl (',' variableDecl)* newline;

typeDecl : Type ID (newline variableDecl?)* End Type newline;

enumDecl : Enum ID (newline enumConstant?)* End Enum newline;
enumConstant : ID ('=' Digit)?;

externalFunc : Declare Function ID (Lib lib=StringLiteral)? (Alias alias=StringLiteral)? parameterList (As retType=annotation)? newline;
externalSub : Declare Sub ID (Lib lib=StringLiteral)? (Alias alias=StringLiteral)? parameterList (As retType=annotation)? newline;

importDecl : Import StringLiteral newline;

functionDecl : methodFlag? Function ID parameterList (As retType=annotation)? newline statement* End Function newline;
subDecl : methodFlag? Sub ID parameterList (As retType=annotation)? newline statement* End Sub newline;

parameterList : '(' (parameter (',' parameter)*)? ')';

parameter : spec=(ParamArray|Optional)? eval=(Byval|Byref)? ID As annotation ('=' initial=expression)?;

accessFlag : Public | Private;
methodFlag : Virtual | Override | Static;

statement : forStmt
            | dimStmt
            | loopStmt
            | ifStmt
            | onelineIfStmt
            | selectStmt
            | controlStmt
            | exprStmt
            | tryStmt
            | throwStmt
            | newline
            ;

throwStmt : Throw expression;

tryStmt : Try newline tryBlock+=statement* catchPart* End Try;

catchPart : Catch ID As annotation newline catchBlock+=statement*;

forStmt : For forIterator '=' from=expression To to=expression (Step step=expression)? newline statement* Next;

forIterator : Dim ID | primaryExpression;


dimStmt : Dim variableDecl (',' variableDecl)* ;
variableDecl : ID (As type=annotation)? ('=' initial=expression)?;

loopStmt : While expression statement* Wend ;

onelineIfStmt : If expression Then truePath=statement (Else falsePart=statement)? ;

ifStmt : If expression Then newline statement* elseIfPart* elsePart? End If ;
elseIfPart : ElseIf expression Then newline statement*;
elsePart : Else newline statement*;

selectStmt : Select Case expression newline caseList* End Select ;
caseList : Case expression newline statement* newline;

controlStmt : Return expression
          | Continue
          | Exit (For|While|Sub)
          ;

exprStmt : expression ;

expression : assignmentExpression;

assignmentExpression : assignment | conditionalExpression;

assignment: primaryExpression '=' expression;

conditionalExpression : conditionalNotExpression
                      | conditionalExpression op=(And|Or|Xor) conditionalNotExpression
                      ;

conditionalNotExpression : Not? relationalExpression;

relationalExpression : additiveExpression
                     | relationalExpression relationalOp additiveExpression
                     ;
relationalOp : eqOp|neOp|ltOp|gtOp|leOp|geOp;

additiveExpression : multiplicativeExpression
                   | additiveExpression op=('+' | '-') multiplicativeExpression
                   ;

multiplicativeExpression : unaryExpression
                         | multiplicativeExpression op=('*'|'/'|Mod) unaryExpression
                         ;

unaryExpression : primaryExpression
                | op=('+'|'-') unaryExpression
                ;

primaryExpression : primaryExpression '.' terminalExpression #DotExpression
                  | primaryExpression As annotation          #CastExpression
                  | primaryExpression argumentList           #MethodInvocation
                  | primaryExpression '[' expression ']'     #ArraySubscription
                  | New typePath ('['subscript=expression']'|arglist=argumentList) #NewExpression
                  | primaryExpression Is annotation#IsExpression
                  | Optional (ID (',' ID)*) #OptionalTestExpression
                  | terminalExpression                       #PrimaryTerminal
                  ;

typePath : ID ('.' ID)*;

terminalExpression : ambiguousName=ID
                  | '(' expression ')'
                  | Self
                  | literal
                  ;
literal : Digit             #IntegerLiteral
        | FloatingPoint     #DoubleLiteral
        | StringLiteral     #StringLiteral
        | CharLiteral       #RuneLiteral
        | (True | False)    #BooleanLiteral
        | '[' (expression (',' expression)*)? ']' #ArrayLiteral
        | Nothing           #NothingLiteral
        ;

ltOp : '<';
leOp : '=''<';
eqOp : '=''=';
geOp : '>''=';
gtOp : '>';
neOp : '<''>';
divOp : '\\';
mulOp : '*';
plusOp : '+';
minusOp : '-';

argumentList : '(' (argument (',' argument)*)? ')';

argument : (option=ID ':')? (Byval|Byref)? expression;

annotation : annotation subScriptSuffix | typePath | subTypeAnnotation | functionTypeAnnotation;

subTypeAnnotation : Sub '(' annotationParameter (',' annotationParameter)* ')';

functionTypeAnnotation : Function '(' annotationParameter (',' annotationParameter)* ')' As annotation;

annotationParameter : spec=(ParamArray|Optional)? eval=(Byval|Byref)? annotation;


subScriptSuffix : '[' ']';

StringLiteral: '"' ~('"'|'\r'|'\n')* '"';
CharLiteral: '\'' ~('\''|'\r'|'\n')* '\'';


newline: '\n\r' | '\n' | '\r';

Digit: [0-9]+; //i32
FloatingPoint: [0-9]+'.'[0-9]+ | [0-9]+('E'|'e')'-'?[0-9]+;//f64


Comment: '//' ~('\r'|'\n')*  -> skip;
BlockComment: '/*' .*? '*/' -> skip;
WS: [ \t]->skip;

Impl: I M P L;
Mod: M O D;
Const: C O N S T;
Continue: C O N T I N U E;
Nothing : N O T H I N G;
Throw : T H R O W;
Extend : E X T E N D;
Module : M O D U L E;
True: T R U E;
False: F A L S E;
New: N E W;
Xor: X O R;
Not: N O T;
And: A N D;
Or: O R;
Try: T R Y;
Catch:C A T C H;
Virtual: V I R T U A L;
Override:O V E R R I D E;
Operator:O P E R A T O R;
Factory:F A C T O R Y;
Implements:I M P L E M E N T S;
Import:I M P O R T;
Class:C L A S S;
Preserve:P R E S E R V E;
Redim:R E D I M;
ParamArray:P A R A M A R R A Y;
Declare:D E C L A R E;
Lib:L I B;
Enum:E N U M;
If:I F;
Super: S U P E R;
Wend:W E N D;
From:F O R M;
Namespace:N A M E S P A C E;
Implement:I M P L E M E N T;
Type: T Y P E;
Alias:A L I A S;
Self:S E L F;
Static:S T A T I C;
ModuleInfo:M O D U L E;
Public:P U B L I C;
Private:P R I V A T E;
Protected:P R O T E C T E D;
Get:G E T;
Set:S E T;
Property:P R O P E R T Y;
Var:V A R;
Dim:D I M;
Let:L E T;
Return: R E T U R N;
Function:F U N C T I O N;
Difference: D I F F E R E N C E;
Union: U N I O N;
Case:C A S E;
Select:S E L E C T;
End:E N D;
Until:U N T I L;
Loop:L O O P;
Exit:E X I T;
While: W H I L E;
Do: D O;
Each: E A C H;
To: T O;
Step:S T E P;
Next: N E X T;
In: I N;
For: F O R;
Optional: O P T I O N A L;
Byval:B Y V A L;
Byref:B Y R E F;
Then:T H E N;
Else:E L S E;
ElseIf: E L S E I F;
Call:C A L L;
Sub:S U B;
As: A S;
Is: I S;
ID: [a-zA-Z_][a-zA-Z0-9_]*;

fragment A:('a'|'A');
fragment B:('b'|'B');
fragment C:('c'|'C');
fragment D:('d'|'D');
fragment E:('e'|'E');
fragment F:('f'|'F');
fragment G:('g'|'G');
fragment H:('h'|'H');
fragment I:('i'|'I');
fragment J:('j'|'J');
fragment K:('k'|'K');
fragment L:('l'|'L');
fragment M:('m'|'M');
fragment N:('n'|'N');
fragment O:('o'|'O');
fragment P:('p'|'P');
fragment Q:('q'|'Q');
fragment R:('r'|'R');
fragment S:('s'|'S');
fragment T:('t'|'T');
fragment U:('u'|'U');
fragment V:('v'|'V');
fragment W:('w'|'W');
fragment X:('x'|'X');
fragment Y:('y'|'Y');
fragment Z:('z'|'Z');

//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     ANTLR Version: 4.10.1
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

// Generated from D:/sources/evobasic-project-antlr/ebc\evoBasic.g4 by ANTLR 4.10.1

// Unreachable code detected
#pragma warning disable 0162
// The variable '...' is assigned but its value is never used
#pragma warning disable 0219
// Missing XML comment for publicly visible type or member '...'
#pragma warning disable 1591
// Ambiguous reference in cref attribute
#pragma warning disable 419

using System;
using System.IO;
using System.Text;
using Antlr4.Runtime;
using Antlr4.Runtime.Atn;
using Antlr4.Runtime.Misc;
using DFA = Antlr4.Runtime.Dfa.DFA;

[System.CodeDom.Compiler.GeneratedCode("ANTLR", "4.10.1")]
[System.CLSCompliant(false)]
public partial class evoBasicLexer : Lexer {
	protected static DFA[] decisionToDFA;
	protected static PredictionContextCache sharedContextCache = new PredictionContextCache();
	public const int
		T__0=1, T__1=2, T__2=3, T__3=4, T__4=5, T__5=6, T__6=7, T__7=8, T__8=9, 
		T__9=10, T__10=11, T__11=12, T__12=13, T__13=14, T__14=15, T__15=16, T__16=17, 
		T__17=18, StringLiteral=19, CharLiteral=20, Digit=21, FloatingPoint=22, 
		Comment=23, BlockComment=24, WS=25, Impl=26, Mod=27, Const=28, Continue=29, 
		Nothing=30, Throw=31, Extend=32, Module=33, True=34, False=35, New=36, 
		Xor=37, Not=38, And=39, Or=40, Try=41, Catch=42, Virtual=43, Override=44, 
		Operator=45, Factory=46, Implements=47, Import=48, Class=49, Preserve=50, 
		Redim=51, ParamArray=52, Declare=53, Lib=54, Enum=55, If=56, Super=57, 
		Wend=58, From=59, Namespace=60, Implement=61, Type=62, Alias=63, Self=64, 
		Static=65, ModuleInfo=66, Public=67, Private=68, Protected=69, Get=70, 
		Set=71, Property=72, Var=73, Dim=74, Let=75, Return=76, Function=77, Difference=78, 
		Union=79, Case=80, Select=81, End=82, Until=83, Loop=84, Exit=85, While=86, 
		Do=87, Each=88, To=89, Step=90, Next=91, In=92, For=93, Optional=94, Byval=95, 
		Byref=96, Then=97, Else=98, ElseIf=99, Call=100, Sub=101, As=102, Is=103, 
		ID=104;
	public static string[] channelNames = {
		"DEFAULT_TOKEN_CHANNEL", "HIDDEN"
	};

	public static string[] modeNames = {
		"DEFAULT_MODE"
	};

	public static readonly string[] ruleNames = {
		"T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "T__8", 
		"T__9", "T__10", "T__11", "T__12", "T__13", "T__14", "T__15", "T__16", 
		"T__17", "StringLiteral", "CharLiteral", "Digit", "FloatingPoint", "Comment", 
		"BlockComment", "WS", "Impl", "Mod", "Const", "Continue", "Nothing", "Throw", 
		"Extend", "Module", "True", "False", "New", "Xor", "Not", "And", "Or", 
		"Try", "Catch", "Virtual", "Override", "Operator", "Factory", "Implements", 
		"Import", "Class", "Preserve", "Redim", "ParamArray", "Declare", "Lib", 
		"Enum", "If", "Super", "Wend", "From", "Namespace", "Implement", "Type", 
		"Alias", "Self", "Static", "ModuleInfo", "Public", "Private", "Protected", 
		"Get", "Set", "Property", "Var", "Dim", "Let", "Return", "Function", "Difference", 
		"Union", "Case", "Select", "End", "Until", "Loop", "Exit", "While", "Do", 
		"Each", "To", "Step", "Next", "In", "For", "Optional", "Byval", "Byref", 
		"Then", "Else", "ElseIf", "Call", "Sub", "As", "Is", "ID", "A", "B", "C", 
		"D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", 
		"R", "S", "T", "U", "V", "W", "X", "Y", "Z"
	};


	public evoBasicLexer(ICharStream input)
	: this(input, Console.Out, Console.Error) { }

	public evoBasicLexer(ICharStream input, TextWriter output, TextWriter errorOutput)
	: base(input, output, errorOutput)
	{
		Interpreter = new LexerATNSimulator(this, _ATN, decisionToDFA, sharedContextCache);
	}

	private static readonly string[] _LiteralNames = {
		null, "','", "'='", "'('", "')'", "'+'", "'-'", "'*'", "'/'", "'.'", "'['", 
		"']'", "'<'", "'>'", "'\\'", "':'", "'\\n\\r'", "'\\n'", "'\\r'"
	};
	private static readonly string[] _SymbolicNames = {
		null, null, null, null, null, null, null, null, null, null, null, null, 
		null, null, null, null, null, null, null, "StringLiteral", "CharLiteral", 
		"Digit", "FloatingPoint", "Comment", "BlockComment", "WS", "Impl", "Mod", 
		"Const", "Continue", "Nothing", "Throw", "Extend", "Module", "True", "False", 
		"New", "Xor", "Not", "And", "Or", "Try", "Catch", "Virtual", "Override", 
		"Operator", "Factory", "Implements", "Import", "Class", "Preserve", "Redim", 
		"ParamArray", "Declare", "Lib", "Enum", "If", "Super", "Wend", "From", 
		"Namespace", "Implement", "Type", "Alias", "Self", "Static", "ModuleInfo", 
		"Public", "Private", "Protected", "Get", "Set", "Property", "Var", "Dim", 
		"Let", "Return", "Function", "Difference", "Union", "Case", "Select", 
		"End", "Until", "Loop", "Exit", "While", "Do", "Each", "To", "Step", "Next", 
		"In", "For", "Optional", "Byval", "Byref", "Then", "Else", "ElseIf", "Call", 
		"Sub", "As", "Is", "ID"
	};
	public static readonly IVocabulary DefaultVocabulary = new Vocabulary(_LiteralNames, _SymbolicNames);

	[NotNull]
	public override IVocabulary Vocabulary
	{
		get
		{
			return DefaultVocabulary;
		}
	}

	public override string GrammarFileName { get { return "evoBasic.g4"; } }

	public override string[] RuleNames { get { return ruleNames; } }

	public override string[] ChannelNames { get { return channelNames; } }

	public override string[] ModeNames { get { return modeNames; } }

	public override int[] SerializedAtn { get { return _serializedATN; } }

	static evoBasicLexer() {
		decisionToDFA = new DFA[_ATN.NumberOfDecisions];
		for (int i = 0; i < _ATN.NumberOfDecisions; i++) {
			decisionToDFA[i] = new DFA(_ATN.GetDecisionState(i), i);
		}
	}
	private static int[] _serializedATN = {
		4,0,104,904,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
		6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
		7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
		7,21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,
		7,28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,
		7,35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,
		7,42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,
		7,49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,2,54,7,54,2,55,7,55,2,56,
		7,56,2,57,7,57,2,58,7,58,2,59,7,59,2,60,7,60,2,61,7,61,2,62,7,62,2,63,
		7,63,2,64,7,64,2,65,7,65,2,66,7,66,2,67,7,67,2,68,7,68,2,69,7,69,2,70,
		7,70,2,71,7,71,2,72,7,72,2,73,7,73,2,74,7,74,2,75,7,75,2,76,7,76,2,77,
		7,77,2,78,7,78,2,79,7,79,2,80,7,80,2,81,7,81,2,82,7,82,2,83,7,83,2,84,
		7,84,2,85,7,85,2,86,7,86,2,87,7,87,2,88,7,88,2,89,7,89,2,90,7,90,2,91,
		7,91,2,92,7,92,2,93,7,93,2,94,7,94,2,95,7,95,2,96,7,96,2,97,7,97,2,98,
		7,98,2,99,7,99,2,100,7,100,2,101,7,101,2,102,7,102,2,103,7,103,2,104,7,
		104,2,105,7,105,2,106,7,106,2,107,7,107,2,108,7,108,2,109,7,109,2,110,
		7,110,2,111,7,111,2,112,7,112,2,113,7,113,2,114,7,114,2,115,7,115,2,116,
		7,116,2,117,7,117,2,118,7,118,2,119,7,119,2,120,7,120,2,121,7,121,2,122,
		7,122,2,123,7,123,2,124,7,124,2,125,7,125,2,126,7,126,2,127,7,127,2,128,
		7,128,2,129,7,129,1,0,1,0,1,1,1,1,1,2,1,2,1,3,1,3,1,4,1,4,1,5,1,5,1,6,
		1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,1,11,1,11,1,12,1,12,1,13,1,13,1,
		14,1,14,1,15,1,15,1,15,1,16,1,16,1,17,1,17,1,18,1,18,5,18,301,8,18,10,
		18,12,18,304,9,18,1,18,1,18,1,19,1,19,5,19,310,8,19,10,19,12,19,313,9,
		19,1,19,1,19,1,20,4,20,318,8,20,11,20,12,20,319,1,21,4,21,323,8,21,11,
		21,12,21,324,1,21,1,21,4,21,329,8,21,11,21,12,21,330,1,21,4,21,334,8,21,
		11,21,12,21,335,1,21,1,21,3,21,340,8,21,1,21,4,21,343,8,21,11,21,12,21,
		344,3,21,347,8,21,1,22,1,22,1,22,1,22,5,22,353,8,22,10,22,12,22,356,9,
		22,1,22,1,22,1,23,1,23,1,23,1,23,5,23,364,8,23,10,23,12,23,367,9,23,1,
		23,1,23,1,23,1,23,1,23,1,24,1,24,1,24,1,24,1,25,1,25,1,25,1,25,1,25,1,
		26,1,26,1,26,1,26,1,27,1,27,1,27,1,27,1,27,1,27,1,28,1,28,1,28,1,28,1,
		28,1,28,1,28,1,28,1,28,1,29,1,29,1,29,1,29,1,29,1,29,1,29,1,29,1,30,1,
		30,1,30,1,30,1,30,1,30,1,31,1,31,1,31,1,31,1,31,1,31,1,31,1,32,1,32,1,
		32,1,32,1,32,1,32,1,32,1,33,1,33,1,33,1,33,1,33,1,34,1,34,1,34,1,34,1,
		34,1,34,1,35,1,35,1,35,1,35,1,36,1,36,1,36,1,36,1,37,1,37,1,37,1,37,1,
		38,1,38,1,38,1,38,1,39,1,39,1,39,1,40,1,40,1,40,1,40,1,41,1,41,1,41,1,
		41,1,41,1,41,1,42,1,42,1,42,1,42,1,42,1,42,1,42,1,42,1,43,1,43,1,43,1,
		43,1,43,1,43,1,43,1,43,1,43,1,44,1,44,1,44,1,44,1,44,1,44,1,44,1,44,1,
		44,1,45,1,45,1,45,1,45,1,45,1,45,1,45,1,45,1,46,1,46,1,46,1,46,1,46,1,
		46,1,46,1,46,1,46,1,46,1,46,1,47,1,47,1,47,1,47,1,47,1,47,1,47,1,48,1,
		48,1,48,1,48,1,48,1,48,1,49,1,49,1,49,1,49,1,49,1,49,1,49,1,49,1,49,1,
		50,1,50,1,50,1,50,1,50,1,50,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,
		51,1,51,1,51,1,52,1,52,1,52,1,52,1,52,1,52,1,52,1,52,1,53,1,53,1,53,1,
		53,1,54,1,54,1,54,1,54,1,54,1,55,1,55,1,55,1,56,1,56,1,56,1,56,1,56,1,
		56,1,57,1,57,1,57,1,57,1,57,1,58,1,58,1,58,1,58,1,58,1,59,1,59,1,59,1,
		59,1,59,1,59,1,59,1,59,1,59,1,59,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,
		60,1,60,1,60,1,61,1,61,1,61,1,61,1,61,1,62,1,62,1,62,1,62,1,62,1,62,1,
		63,1,63,1,63,1,63,1,63,1,64,1,64,1,64,1,64,1,64,1,64,1,64,1,65,1,65,1,
		65,1,65,1,65,1,65,1,65,1,66,1,66,1,66,1,66,1,66,1,66,1,66,1,67,1,67,1,
		67,1,67,1,67,1,67,1,67,1,67,1,68,1,68,1,68,1,68,1,68,1,68,1,68,1,68,1,
		68,1,68,1,69,1,69,1,69,1,69,1,70,1,70,1,70,1,70,1,71,1,71,1,71,1,71,1,
		71,1,71,1,71,1,71,1,71,1,72,1,72,1,72,1,72,1,73,1,73,1,73,1,73,1,74,1,
		74,1,74,1,74,1,75,1,75,1,75,1,75,1,75,1,75,1,75,1,76,1,76,1,76,1,76,1,
		76,1,76,1,76,1,76,1,76,1,77,1,77,1,77,1,77,1,77,1,77,1,77,1,77,1,77,1,
		77,1,77,1,78,1,78,1,78,1,78,1,78,1,78,1,79,1,79,1,79,1,79,1,79,1,80,1,
		80,1,80,1,80,1,80,1,80,1,80,1,81,1,81,1,81,1,81,1,82,1,82,1,82,1,82,1,
		82,1,82,1,83,1,83,1,83,1,83,1,83,1,84,1,84,1,84,1,84,1,84,1,85,1,85,1,
		85,1,85,1,85,1,85,1,86,1,86,1,86,1,87,1,87,1,87,1,87,1,87,1,88,1,88,1,
		88,1,89,1,89,1,89,1,89,1,89,1,90,1,90,1,90,1,90,1,90,1,91,1,91,1,91,1,
		92,1,92,1,92,1,92,1,93,1,93,1,93,1,93,1,93,1,93,1,93,1,93,1,93,1,94,1,
		94,1,94,1,94,1,94,1,94,1,95,1,95,1,95,1,95,1,95,1,95,1,96,1,96,1,96,1,
		96,1,96,1,97,1,97,1,97,1,97,1,97,1,98,1,98,1,98,1,98,1,98,1,98,1,98,1,
		99,1,99,1,99,1,99,1,99,1,100,1,100,1,100,1,100,1,101,1,101,1,101,1,102,
		1,102,1,102,1,103,1,103,5,103,848,8,103,10,103,12,103,851,9,103,1,104,
		1,104,1,105,1,105,1,106,1,106,1,107,1,107,1,108,1,108,1,109,1,109,1,110,
		1,110,1,111,1,111,1,112,1,112,1,113,1,113,1,114,1,114,1,115,1,115,1,116,
		1,116,1,117,1,117,1,118,1,118,1,119,1,119,1,120,1,120,1,121,1,121,1,122,
		1,122,1,123,1,123,1,124,1,124,1,125,1,125,1,126,1,126,1,127,1,127,1,128,
		1,128,1,129,1,129,1,365,0,130,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,
		19,10,21,11,23,12,25,13,27,14,29,15,31,16,33,17,35,18,37,19,39,20,41,21,
		43,22,45,23,47,24,49,25,51,26,53,27,55,28,57,29,59,30,61,31,63,32,65,33,
		67,34,69,35,71,36,73,37,75,38,77,39,79,40,81,41,83,42,85,43,87,44,89,45,
		91,46,93,47,95,48,97,49,99,50,101,51,103,52,105,53,107,54,109,55,111,56,
		113,57,115,58,117,59,119,60,121,61,123,62,125,63,127,64,129,65,131,66,
		133,67,135,68,137,69,139,70,141,71,143,72,145,73,147,74,149,75,151,76,
		153,77,155,78,157,79,159,80,161,81,163,82,165,83,167,84,169,85,171,86,
		173,87,175,88,177,89,179,90,181,91,183,92,185,93,187,94,189,95,191,96,
		193,97,195,98,197,99,199,100,201,101,203,102,205,103,207,104,209,0,211,
		0,213,0,215,0,217,0,219,0,221,0,223,0,225,0,227,0,229,0,231,0,233,0,235,
		0,237,0,239,0,241,0,243,0,245,0,247,0,249,0,251,0,253,0,255,0,257,0,259,
		0,1,0,33,3,0,10,10,13,13,34,34,3,0,10,10,13,13,39,39,1,0,48,57,2,0,69,
		69,101,101,2,0,10,10,13,13,2,0,9,9,32,32,3,0,65,90,95,95,97,122,4,0,48,
		57,65,90,95,95,97,122,2,0,65,65,97,97,2,0,66,66,98,98,2,0,67,67,99,99,
		2,0,68,68,100,100,2,0,70,70,102,102,2,0,71,71,103,103,2,0,72,72,104,104,
		2,0,73,73,105,105,2,0,74,74,106,106,2,0,75,75,107,107,2,0,76,76,108,108,
		2,0,77,77,109,109,2,0,78,78,110,110,2,0,79,79,111,111,2,0,80,80,112,112,
		2,0,81,81,113,113,2,0,82,82,114,114,2,0,83,83,115,115,2,0,84,84,116,116,
		2,0,85,85,117,117,2,0,86,86,118,118,2,0,87,87,119,119,2,0,88,88,120,120,
		2,0,89,89,121,121,2,0,90,90,122,122,889,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,
		0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,
		17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,
		0,0,0,0,29,1,0,0,0,0,31,1,0,0,0,0,33,1,0,0,0,0,35,1,0,0,0,0,37,1,0,0,0,
		0,39,1,0,0,0,0,41,1,0,0,0,0,43,1,0,0,0,0,45,1,0,0,0,0,47,1,0,0,0,0,49,
		1,0,0,0,0,51,1,0,0,0,0,53,1,0,0,0,0,55,1,0,0,0,0,57,1,0,0,0,0,59,1,0,0,
		0,0,61,1,0,0,0,0,63,1,0,0,0,0,65,1,0,0,0,0,67,1,0,0,0,0,69,1,0,0,0,0,71,
		1,0,0,0,0,73,1,0,0,0,0,75,1,0,0,0,0,77,1,0,0,0,0,79,1,0,0,0,0,81,1,0,0,
		0,0,83,1,0,0,0,0,85,1,0,0,0,0,87,1,0,0,0,0,89,1,0,0,0,0,91,1,0,0,0,0,93,
		1,0,0,0,0,95,1,0,0,0,0,97,1,0,0,0,0,99,1,0,0,0,0,101,1,0,0,0,0,103,1,0,
		0,0,0,105,1,0,0,0,0,107,1,0,0,0,0,109,1,0,0,0,0,111,1,0,0,0,0,113,1,0,
		0,0,0,115,1,0,0,0,0,117,1,0,0,0,0,119,1,0,0,0,0,121,1,0,0,0,0,123,1,0,
		0,0,0,125,1,0,0,0,0,127,1,0,0,0,0,129,1,0,0,0,0,131,1,0,0,0,0,133,1,0,
		0,0,0,135,1,0,0,0,0,137,1,0,0,0,0,139,1,0,0,0,0,141,1,0,0,0,0,143,1,0,
		0,0,0,145,1,0,0,0,0,147,1,0,0,0,0,149,1,0,0,0,0,151,1,0,0,0,0,153,1,0,
		0,0,0,155,1,0,0,0,0,157,1,0,0,0,0,159,1,0,0,0,0,161,1,0,0,0,0,163,1,0,
		0,0,0,165,1,0,0,0,0,167,1,0,0,0,0,169,1,0,0,0,0,171,1,0,0,0,0,173,1,0,
		0,0,0,175,1,0,0,0,0,177,1,0,0,0,0,179,1,0,0,0,0,181,1,0,0,0,0,183,1,0,
		0,0,0,185,1,0,0,0,0,187,1,0,0,0,0,189,1,0,0,0,0,191,1,0,0,0,0,193,1,0,
		0,0,0,195,1,0,0,0,0,197,1,0,0,0,0,199,1,0,0,0,0,201,1,0,0,0,0,203,1,0,
		0,0,0,205,1,0,0,0,0,207,1,0,0,0,1,261,1,0,0,0,3,263,1,0,0,0,5,265,1,0,
		0,0,7,267,1,0,0,0,9,269,1,0,0,0,11,271,1,0,0,0,13,273,1,0,0,0,15,275,1,
		0,0,0,17,277,1,0,0,0,19,279,1,0,0,0,21,281,1,0,0,0,23,283,1,0,0,0,25,285,
		1,0,0,0,27,287,1,0,0,0,29,289,1,0,0,0,31,291,1,0,0,0,33,294,1,0,0,0,35,
		296,1,0,0,0,37,298,1,0,0,0,39,307,1,0,0,0,41,317,1,0,0,0,43,346,1,0,0,
		0,45,348,1,0,0,0,47,359,1,0,0,0,49,373,1,0,0,0,51,377,1,0,0,0,53,382,1,
		0,0,0,55,386,1,0,0,0,57,392,1,0,0,0,59,401,1,0,0,0,61,409,1,0,0,0,63,415,
		1,0,0,0,65,422,1,0,0,0,67,429,1,0,0,0,69,434,1,0,0,0,71,440,1,0,0,0,73,
		444,1,0,0,0,75,448,1,0,0,0,77,452,1,0,0,0,79,456,1,0,0,0,81,459,1,0,0,
		0,83,463,1,0,0,0,85,469,1,0,0,0,87,477,1,0,0,0,89,486,1,0,0,0,91,495,1,
		0,0,0,93,503,1,0,0,0,95,514,1,0,0,0,97,521,1,0,0,0,99,527,1,0,0,0,101,
		536,1,0,0,0,103,542,1,0,0,0,105,553,1,0,0,0,107,561,1,0,0,0,109,565,1,
		0,0,0,111,570,1,0,0,0,113,573,1,0,0,0,115,579,1,0,0,0,117,584,1,0,0,0,
		119,589,1,0,0,0,121,599,1,0,0,0,123,609,1,0,0,0,125,614,1,0,0,0,127,620,
		1,0,0,0,129,625,1,0,0,0,131,632,1,0,0,0,133,639,1,0,0,0,135,646,1,0,0,
		0,137,654,1,0,0,0,139,664,1,0,0,0,141,668,1,0,0,0,143,672,1,0,0,0,145,
		681,1,0,0,0,147,685,1,0,0,0,149,689,1,0,0,0,151,693,1,0,0,0,153,700,1,
		0,0,0,155,709,1,0,0,0,157,720,1,0,0,0,159,726,1,0,0,0,161,731,1,0,0,0,
		163,738,1,0,0,0,165,742,1,0,0,0,167,748,1,0,0,0,169,753,1,0,0,0,171,758,
		1,0,0,0,173,764,1,0,0,0,175,767,1,0,0,0,177,772,1,0,0,0,179,775,1,0,0,
		0,181,780,1,0,0,0,183,785,1,0,0,0,185,788,1,0,0,0,187,792,1,0,0,0,189,
		801,1,0,0,0,191,807,1,0,0,0,193,813,1,0,0,0,195,818,1,0,0,0,197,823,1,
		0,0,0,199,830,1,0,0,0,201,835,1,0,0,0,203,839,1,0,0,0,205,842,1,0,0,0,
		207,845,1,0,0,0,209,852,1,0,0,0,211,854,1,0,0,0,213,856,1,0,0,0,215,858,
		1,0,0,0,217,860,1,0,0,0,219,862,1,0,0,0,221,864,1,0,0,0,223,866,1,0,0,
		0,225,868,1,0,0,0,227,870,1,0,0,0,229,872,1,0,0,0,231,874,1,0,0,0,233,
		876,1,0,0,0,235,878,1,0,0,0,237,880,1,0,0,0,239,882,1,0,0,0,241,884,1,
		0,0,0,243,886,1,0,0,0,245,888,1,0,0,0,247,890,1,0,0,0,249,892,1,0,0,0,
		251,894,1,0,0,0,253,896,1,0,0,0,255,898,1,0,0,0,257,900,1,0,0,0,259,902,
		1,0,0,0,261,262,5,44,0,0,262,2,1,0,0,0,263,264,5,61,0,0,264,4,1,0,0,0,
		265,266,5,40,0,0,266,6,1,0,0,0,267,268,5,41,0,0,268,8,1,0,0,0,269,270,
		5,43,0,0,270,10,1,0,0,0,271,272,5,45,0,0,272,12,1,0,0,0,273,274,5,42,0,
		0,274,14,1,0,0,0,275,276,5,47,0,0,276,16,1,0,0,0,277,278,5,46,0,0,278,
		18,1,0,0,0,279,280,5,91,0,0,280,20,1,0,0,0,281,282,5,93,0,0,282,22,1,0,
		0,0,283,284,5,60,0,0,284,24,1,0,0,0,285,286,5,62,0,0,286,26,1,0,0,0,287,
		288,5,92,0,0,288,28,1,0,0,0,289,290,5,58,0,0,290,30,1,0,0,0,291,292,5,
		10,0,0,292,293,5,13,0,0,293,32,1,0,0,0,294,295,5,10,0,0,295,34,1,0,0,0,
		296,297,5,13,0,0,297,36,1,0,0,0,298,302,5,34,0,0,299,301,8,0,0,0,300,299,
		1,0,0,0,301,304,1,0,0,0,302,300,1,0,0,0,302,303,1,0,0,0,303,305,1,0,0,
		0,304,302,1,0,0,0,305,306,5,34,0,0,306,38,1,0,0,0,307,311,5,39,0,0,308,
		310,8,1,0,0,309,308,1,0,0,0,310,313,1,0,0,0,311,309,1,0,0,0,311,312,1,
		0,0,0,312,314,1,0,0,0,313,311,1,0,0,0,314,315,5,39,0,0,315,40,1,0,0,0,
		316,318,7,2,0,0,317,316,1,0,0,0,318,319,1,0,0,0,319,317,1,0,0,0,319,320,
		1,0,0,0,320,42,1,0,0,0,321,323,7,2,0,0,322,321,1,0,0,0,323,324,1,0,0,0,
		324,322,1,0,0,0,324,325,1,0,0,0,325,326,1,0,0,0,326,328,5,46,0,0,327,329,
		7,2,0,0,328,327,1,0,0,0,329,330,1,0,0,0,330,328,1,0,0,0,330,331,1,0,0,
		0,331,347,1,0,0,0,332,334,7,2,0,0,333,332,1,0,0,0,334,335,1,0,0,0,335,
		333,1,0,0,0,335,336,1,0,0,0,336,337,1,0,0,0,337,339,7,3,0,0,338,340,5,
		45,0,0,339,338,1,0,0,0,339,340,1,0,0,0,340,342,1,0,0,0,341,343,7,2,0,0,
		342,341,1,0,0,0,343,344,1,0,0,0,344,342,1,0,0,0,344,345,1,0,0,0,345,347,
		1,0,0,0,346,322,1,0,0,0,346,333,1,0,0,0,347,44,1,0,0,0,348,349,5,47,0,
		0,349,350,5,47,0,0,350,354,1,0,0,0,351,353,8,4,0,0,352,351,1,0,0,0,353,
		356,1,0,0,0,354,352,1,0,0,0,354,355,1,0,0,0,355,357,1,0,0,0,356,354,1,
		0,0,0,357,358,6,22,0,0,358,46,1,0,0,0,359,360,5,47,0,0,360,361,5,42,0,
		0,361,365,1,0,0,0,362,364,9,0,0,0,363,362,1,0,0,0,364,367,1,0,0,0,365,
		366,1,0,0,0,365,363,1,0,0,0,366,368,1,0,0,0,367,365,1,0,0,0,368,369,5,
		42,0,0,369,370,5,47,0,0,370,371,1,0,0,0,371,372,6,23,0,0,372,48,1,0,0,
		0,373,374,7,5,0,0,374,375,1,0,0,0,375,376,6,24,0,0,376,50,1,0,0,0,377,
		378,3,225,112,0,378,379,3,233,116,0,379,380,3,239,119,0,380,381,3,231,
		115,0,381,52,1,0,0,0,382,383,3,233,116,0,383,384,3,237,118,0,384,385,3,
		215,107,0,385,54,1,0,0,0,386,387,3,213,106,0,387,388,3,237,118,0,388,389,
		3,235,117,0,389,390,3,245,122,0,390,391,3,247,123,0,391,56,1,0,0,0,392,
		393,3,213,106,0,393,394,3,237,118,0,394,395,3,235,117,0,395,396,3,247,
		123,0,396,397,3,225,112,0,397,398,3,235,117,0,398,399,3,249,124,0,399,
		400,3,217,108,0,400,58,1,0,0,0,401,402,3,235,117,0,402,403,3,237,118,0,
		403,404,3,247,123,0,404,405,3,223,111,0,405,406,3,225,112,0,406,407,3,
		235,117,0,407,408,3,221,110,0,408,60,1,0,0,0,409,410,3,247,123,0,410,411,
		3,223,111,0,411,412,3,243,121,0,412,413,3,237,118,0,413,414,3,253,126,
		0,414,62,1,0,0,0,415,416,3,217,108,0,416,417,3,255,127,0,417,418,3,247,
		123,0,418,419,3,217,108,0,419,420,3,235,117,0,420,421,3,215,107,0,421,
		64,1,0,0,0,422,423,3,233,116,0,423,424,3,237,118,0,424,425,3,215,107,0,
		425,426,3,249,124,0,426,427,3,231,115,0,427,428,3,217,108,0,428,66,1,0,
		0,0,429,430,3,247,123,0,430,431,3,243,121,0,431,432,3,249,124,0,432,433,
		3,217,108,0,433,68,1,0,0,0,434,435,3,219,109,0,435,436,3,209,104,0,436,
		437,3,231,115,0,437,438,3,245,122,0,438,439,3,217,108,0,439,70,1,0,0,0,
		440,441,3,235,117,0,441,442,3,217,108,0,442,443,3,253,126,0,443,72,1,0,
		0,0,444,445,3,255,127,0,445,446,3,237,118,0,446,447,3,243,121,0,447,74,
		1,0,0,0,448,449,3,235,117,0,449,450,3,237,118,0,450,451,3,247,123,0,451,
		76,1,0,0,0,452,453,3,209,104,0,453,454,3,235,117,0,454,455,3,215,107,0,
		455,78,1,0,0,0,456,457,3,237,118,0,457,458,3,243,121,0,458,80,1,0,0,0,
		459,460,3,247,123,0,460,461,3,243,121,0,461,462,3,257,128,0,462,82,1,0,
		0,0,463,464,3,213,106,0,464,465,3,209,104,0,465,466,3,247,123,0,466,467,
		3,213,106,0,467,468,3,223,111,0,468,84,1,0,0,0,469,470,3,251,125,0,470,
		471,3,225,112,0,471,472,3,243,121,0,472,473,3,247,123,0,473,474,3,249,
		124,0,474,475,3,209,104,0,475,476,3,231,115,0,476,86,1,0,0,0,477,478,3,
		237,118,0,478,479,3,251,125,0,479,480,3,217,108,0,480,481,3,243,121,0,
		481,482,3,243,121,0,482,483,3,225,112,0,483,484,3,215,107,0,484,485,3,
		217,108,0,485,88,1,0,0,0,486,487,3,237,118,0,487,488,3,239,119,0,488,489,
		3,217,108,0,489,490,3,243,121,0,490,491,3,209,104,0,491,492,3,247,123,
		0,492,493,3,237,118,0,493,494,3,243,121,0,494,90,1,0,0,0,495,496,3,219,
		109,0,496,497,3,209,104,0,497,498,3,213,106,0,498,499,3,247,123,0,499,
		500,3,237,118,0,500,501,3,243,121,0,501,502,3,257,128,0,502,92,1,0,0,0,
		503,504,3,225,112,0,504,505,3,233,116,0,505,506,3,239,119,0,506,507,3,
		231,115,0,507,508,3,217,108,0,508,509,3,233,116,0,509,510,3,217,108,0,
		510,511,3,235,117,0,511,512,3,247,123,0,512,513,3,245,122,0,513,94,1,0,
		0,0,514,515,3,225,112,0,515,516,3,233,116,0,516,517,3,239,119,0,517,518,
		3,237,118,0,518,519,3,243,121,0,519,520,3,247,123,0,520,96,1,0,0,0,521,
		522,3,213,106,0,522,523,3,231,115,0,523,524,3,209,104,0,524,525,3,245,
		122,0,525,526,3,245,122,0,526,98,1,0,0,0,527,528,3,239,119,0,528,529,3,
		243,121,0,529,530,3,217,108,0,530,531,3,245,122,0,531,532,3,217,108,0,
		532,533,3,243,121,0,533,534,3,251,125,0,534,535,3,217,108,0,535,100,1,
		0,0,0,536,537,3,243,121,0,537,538,3,217,108,0,538,539,3,215,107,0,539,
		540,3,225,112,0,540,541,3,233,116,0,541,102,1,0,0,0,542,543,3,239,119,
		0,543,544,3,209,104,0,544,545,3,243,121,0,545,546,3,209,104,0,546,547,
		3,233,116,0,547,548,3,209,104,0,548,549,3,243,121,0,549,550,3,243,121,
		0,550,551,3,209,104,0,551,552,3,257,128,0,552,104,1,0,0,0,553,554,3,215,
		107,0,554,555,3,217,108,0,555,556,3,213,106,0,556,557,3,231,115,0,557,
		558,3,209,104,0,558,559,3,243,121,0,559,560,3,217,108,0,560,106,1,0,0,
		0,561,562,3,231,115,0,562,563,3,225,112,0,563,564,3,211,105,0,564,108,
		1,0,0,0,565,566,3,217,108,0,566,567,3,235,117,0,567,568,3,249,124,0,568,
		569,3,233,116,0,569,110,1,0,0,0,570,571,3,225,112,0,571,572,3,219,109,
		0,572,112,1,0,0,0,573,574,3,245,122,0,574,575,3,249,124,0,575,576,3,239,
		119,0,576,577,3,217,108,0,577,578,3,243,121,0,578,114,1,0,0,0,579,580,
		3,253,126,0,580,581,3,217,108,0,581,582,3,235,117,0,582,583,3,215,107,
		0,583,116,1,0,0,0,584,585,3,219,109,0,585,586,3,237,118,0,586,587,3,243,
		121,0,587,588,3,233,116,0,588,118,1,0,0,0,589,590,3,235,117,0,590,591,
		3,209,104,0,591,592,3,233,116,0,592,593,3,217,108,0,593,594,3,245,122,
		0,594,595,3,239,119,0,595,596,3,209,104,0,596,597,3,213,106,0,597,598,
		3,217,108,0,598,120,1,0,0,0,599,600,3,225,112,0,600,601,3,233,116,0,601,
		602,3,239,119,0,602,603,3,231,115,0,603,604,3,217,108,0,604,605,3,233,
		116,0,605,606,3,217,108,0,606,607,3,235,117,0,607,608,3,247,123,0,608,
		122,1,0,0,0,609,610,3,247,123,0,610,611,3,257,128,0,611,612,3,239,119,
		0,612,613,3,217,108,0,613,124,1,0,0,0,614,615,3,209,104,0,615,616,3,231,
		115,0,616,617,3,225,112,0,617,618,3,209,104,0,618,619,3,245,122,0,619,
		126,1,0,0,0,620,621,3,245,122,0,621,622,3,217,108,0,622,623,3,231,115,
		0,623,624,3,219,109,0,624,128,1,0,0,0,625,626,3,245,122,0,626,627,3,247,
		123,0,627,628,3,209,104,0,628,629,3,247,123,0,629,630,3,225,112,0,630,
		631,3,213,106,0,631,130,1,0,0,0,632,633,3,233,116,0,633,634,3,237,118,
		0,634,635,3,215,107,0,635,636,3,249,124,0,636,637,3,231,115,0,637,638,
		3,217,108,0,638,132,1,0,0,0,639,640,3,239,119,0,640,641,3,249,124,0,641,
		642,3,211,105,0,642,643,3,231,115,0,643,644,3,225,112,0,644,645,3,213,
		106,0,645,134,1,0,0,0,646,647,3,239,119,0,647,648,3,243,121,0,648,649,
		3,225,112,0,649,650,3,251,125,0,650,651,3,209,104,0,651,652,3,247,123,
		0,652,653,3,217,108,0,653,136,1,0,0,0,654,655,3,239,119,0,655,656,3,243,
		121,0,656,657,3,237,118,0,657,658,3,247,123,0,658,659,3,217,108,0,659,
		660,3,213,106,0,660,661,3,247,123,0,661,662,3,217,108,0,662,663,3,215,
		107,0,663,138,1,0,0,0,664,665,3,221,110,0,665,666,3,217,108,0,666,667,
		3,247,123,0,667,140,1,0,0,0,668,669,3,245,122,0,669,670,3,217,108,0,670,
		671,3,247,123,0,671,142,1,0,0,0,672,673,3,239,119,0,673,674,3,243,121,
		0,674,675,3,237,118,0,675,676,3,239,119,0,676,677,3,217,108,0,677,678,
		3,243,121,0,678,679,3,247,123,0,679,680,3,257,128,0,680,144,1,0,0,0,681,
		682,3,251,125,0,682,683,3,209,104,0,683,684,3,243,121,0,684,146,1,0,0,
		0,685,686,3,215,107,0,686,687,3,225,112,0,687,688,3,233,116,0,688,148,
		1,0,0,0,689,690,3,231,115,0,690,691,3,217,108,0,691,692,3,247,123,0,692,
		150,1,0,0,0,693,694,3,243,121,0,694,695,3,217,108,0,695,696,3,247,123,
		0,696,697,3,249,124,0,697,698,3,243,121,0,698,699,3,235,117,0,699,152,
		1,0,0,0,700,701,3,219,109,0,701,702,3,249,124,0,702,703,3,235,117,0,703,
		704,3,213,106,0,704,705,3,247,123,0,705,706,3,225,112,0,706,707,3,237,
		118,0,707,708,3,235,117,0,708,154,1,0,0,0,709,710,3,215,107,0,710,711,
		3,225,112,0,711,712,3,219,109,0,712,713,3,219,109,0,713,714,3,217,108,
		0,714,715,3,243,121,0,715,716,3,217,108,0,716,717,3,235,117,0,717,718,
		3,213,106,0,718,719,3,217,108,0,719,156,1,0,0,0,720,721,3,249,124,0,721,
		722,3,235,117,0,722,723,3,225,112,0,723,724,3,237,118,0,724,725,3,235,
		117,0,725,158,1,0,0,0,726,727,3,213,106,0,727,728,3,209,104,0,728,729,
		3,245,122,0,729,730,3,217,108,0,730,160,1,0,0,0,731,732,3,245,122,0,732,
		733,3,217,108,0,733,734,3,231,115,0,734,735,3,217,108,0,735,736,3,213,
		106,0,736,737,3,247,123,0,737,162,1,0,0,0,738,739,3,217,108,0,739,740,
		3,235,117,0,740,741,3,215,107,0,741,164,1,0,0,0,742,743,3,249,124,0,743,
		744,3,235,117,0,744,745,3,247,123,0,745,746,3,225,112,0,746,747,3,231,
		115,0,747,166,1,0,0,0,748,749,3,231,115,0,749,750,3,237,118,0,750,751,
		3,237,118,0,751,752,3,239,119,0,752,168,1,0,0,0,753,754,3,217,108,0,754,
		755,3,255,127,0,755,756,3,225,112,0,756,757,3,247,123,0,757,170,1,0,0,
		0,758,759,3,253,126,0,759,760,3,223,111,0,760,761,3,225,112,0,761,762,
		3,231,115,0,762,763,3,217,108,0,763,172,1,0,0,0,764,765,3,215,107,0,765,
		766,3,237,118,0,766,174,1,0,0,0,767,768,3,217,108,0,768,769,3,209,104,
		0,769,770,3,213,106,0,770,771,3,223,111,0,771,176,1,0,0,0,772,773,3,247,
		123,0,773,774,3,237,118,0,774,178,1,0,0,0,775,776,3,245,122,0,776,777,
		3,247,123,0,777,778,3,217,108,0,778,779,3,239,119,0,779,180,1,0,0,0,780,
		781,3,235,117,0,781,782,3,217,108,0,782,783,3,255,127,0,783,784,3,247,
		123,0,784,182,1,0,0,0,785,786,3,225,112,0,786,787,3,235,117,0,787,184,
		1,0,0,0,788,789,3,219,109,0,789,790,3,237,118,0,790,791,3,243,121,0,791,
		186,1,0,0,0,792,793,3,237,118,0,793,794,3,239,119,0,794,795,3,247,123,
		0,795,796,3,225,112,0,796,797,3,237,118,0,797,798,3,235,117,0,798,799,
		3,209,104,0,799,800,3,231,115,0,800,188,1,0,0,0,801,802,3,211,105,0,802,
		803,3,257,128,0,803,804,3,251,125,0,804,805,3,209,104,0,805,806,3,231,
		115,0,806,190,1,0,0,0,807,808,3,211,105,0,808,809,3,257,128,0,809,810,
		3,243,121,0,810,811,3,217,108,0,811,812,3,219,109,0,812,192,1,0,0,0,813,
		814,3,247,123,0,814,815,3,223,111,0,815,816,3,217,108,0,816,817,3,235,
		117,0,817,194,1,0,0,0,818,819,3,217,108,0,819,820,3,231,115,0,820,821,
		3,245,122,0,821,822,3,217,108,0,822,196,1,0,0,0,823,824,3,217,108,0,824,
		825,3,231,115,0,825,826,3,245,122,0,826,827,3,217,108,0,827,828,3,225,
		112,0,828,829,3,219,109,0,829,198,1,0,0,0,830,831,3,213,106,0,831,832,
		3,209,104,0,832,833,3,231,115,0,833,834,3,231,115,0,834,200,1,0,0,0,835,
		836,3,245,122,0,836,837,3,249,124,0,837,838,3,211,105,0,838,202,1,0,0,
		0,839,840,3,209,104,0,840,841,3,245,122,0,841,204,1,0,0,0,842,843,3,225,
		112,0,843,844,3,245,122,0,844,206,1,0,0,0,845,849,7,6,0,0,846,848,7,7,
		0,0,847,846,1,0,0,0,848,851,1,0,0,0,849,847,1,0,0,0,849,850,1,0,0,0,850,
		208,1,0,0,0,851,849,1,0,0,0,852,853,7,8,0,0,853,210,1,0,0,0,854,855,7,
		9,0,0,855,212,1,0,0,0,856,857,7,10,0,0,857,214,1,0,0,0,858,859,7,11,0,
		0,859,216,1,0,0,0,860,861,7,3,0,0,861,218,1,0,0,0,862,863,7,12,0,0,863,
		220,1,0,0,0,864,865,7,13,0,0,865,222,1,0,0,0,866,867,7,14,0,0,867,224,
		1,0,0,0,868,869,7,15,0,0,869,226,1,0,0,0,870,871,7,16,0,0,871,228,1,0,
		0,0,872,873,7,17,0,0,873,230,1,0,0,0,874,875,7,18,0,0,875,232,1,0,0,0,
		876,877,7,19,0,0,877,234,1,0,0,0,878,879,7,20,0,0,879,236,1,0,0,0,880,
		881,7,21,0,0,881,238,1,0,0,0,882,883,7,22,0,0,883,240,1,0,0,0,884,885,
		7,23,0,0,885,242,1,0,0,0,886,887,7,24,0,0,887,244,1,0,0,0,888,889,7,25,
		0,0,889,246,1,0,0,0,890,891,7,26,0,0,891,248,1,0,0,0,892,893,7,27,0,0,
		893,250,1,0,0,0,894,895,7,28,0,0,895,252,1,0,0,0,896,897,7,29,0,0,897,
		254,1,0,0,0,898,899,7,30,0,0,899,256,1,0,0,0,900,901,7,31,0,0,901,258,
		1,0,0,0,902,903,7,32,0,0,903,260,1,0,0,0,13,0,302,311,319,324,330,335,
		339,344,346,354,365,849,1,6,0,0
	};

	public static readonly ATN _ATN =
		new ATNDeserializer().Deserialize(_serializedATN);


}
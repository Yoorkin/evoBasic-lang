namespace evoBasic.AST{

    class AST{
        public virtual Node DumpDebugNode(){
            throw new NotImplementedException();
        }
    }

    enum ExprFlag{
        LValue,RValue
    }

    class Expression : AST{
        public Symbols.Type type;
        public ExprFlag exprFlag;
        public Symbols.Symbol exprSymbol;
        public Source.SourceLocation location;
    }

    class ErrorExpr : Expression{
        public ErrorExpr(){
            this.type = new Symbols.ErrorType();
        }
        public override Node DumpDebugNode() => new Node{value = "<ERROR>"};
    }

    class PseudoReference : Expression{}

    class Statement : AST{
        public UInt32 line;
    }

    class ErrorStmt : Statement{
        public override Node DumpDebugNode() => new Node{value = "ErrorStmt"};
    }

    class Block : AST{
        public List<Statement> statements = new List<Statement>();
        public override Node DumpDebugNode() => new Node{
            value = "Block",
            childs = statements.Select(x => x.DumpDebugNode())
        };
    }

    class DimStmt : Statement {
        public List<Tuple<Symbols.Variable,Expression>> variables = new List<Tuple<Symbols.Variable, Expression>>();
        public override Node DumpDebugNode() => new Node{
            value = $"Dim (line {line})", 
            childs = variables.Select(x => new Node{
                value = x.Item1.ToString(),
                childs = new List<Node> {
                    x.Item2!=null ? x.Item2.DumpDebugNode() : new Node{ value = "DefaultInitialValue" }
                }
            })
        };
    }

    class TryStmt : Statement {
        public Block tryBlock;
        public List<CatchPart> catchParts;
        public override Node DumpDebugNode() => new Node{
            value = $"Try (line {line})",
            childs = new List<Node>{tryBlock.DumpDebugNode()}.Concat(catchParts.Select(x => x.DumpDebugNode()))
        };
    }

    class CatchPart : AST {
        public Block catchBlock;
        public Symbols.Variable exceptionVariable;
        public override Node DumpDebugNode() => new Node{
            value = "Catch",
            childs = new List<Node>{
                new Node{ value = exceptionVariable.ToString() },
                catchBlock.DumpDebugNode()
            }
        };
    }

    class Loop : Statement {
        public Expression condition;
        public Block block;
        public override Node DumpDebugNode() => new Node{
            value = $"Loop (line {line})",
            childs = new List<Node>{
                condition.DumpDebugNode(),
                block.DumpDebugNode()
            }
        };
    }

    class If : Statement{
        public List<Case> cases = new List<Case>();
        public override Node DumpDebugNode() => new Node{
            value = $"If (line {line})",
            childs = cases.Select(x => x.DumpDebugNode())
        };
    }

    class Case : AST{
        public Expression condition;
        public Block block;
        public override Node DumpDebugNode() => new Node{
            value = "Case",
            childs = new List<Node>{
                condition == null ? new Node{value = "empty condition"} : condition.DumpDebugNode(),
                block.DumpDebugNode()
            }
        };
    }

    class For : Statement{
        public bool iteratorHasDim = false;
        public Expression iterator,begin,end,step;
        public Symbols.Variable iterVar,begVar,endVar,sepVar;
        public Block block;
        public override Node DumpDebugNode() => new Node{
            value = $"For (line {line})",
            childs = new List<AST>{
                iterator,begin,end,step,block
            }.Select(x => x == null ? new Node{value = "null"} : x.DumpDebugNode())
        };
    }

    class Throw : Statement{
        public Expression expression;
        public override Node DumpDebugNode() => new Node{
            value = $"Throw (line {line})",
            childs = new List<Node>{expression.DumpDebugNode()}
        };
    }

    class Return : Statement{
        public Expression expression;
        public override Node DumpDebugNode() => new Node{
            value = $"Return (line {line})",
            childs = new List<Node>{expression.DumpDebugNode()}
        };
    }

    class ExprStmt : Statement{
        public Expression expression;
        public override Node DumpDebugNode() => new Node{
            value = $"ExprStmt (line {line})",
            childs = new List<Node>{expression.DumpDebugNode()}
        };
    }
    
    class Exit : Statement{
        public enum Flag{
            For,While,Sub
        }
        public Flag flag;
        public override Node DumpDebugNode() => new Node{value = $"Exit (line {line}) {flag.ToString()}"};
    }

    class Continue : Statement{
        public override Node DumpDebugNode() => new Node{value = $"Continue (line {line})"};
    }

    class Unary : Expression{
        public Expression expression;
        public Op op;
        public override Node DumpDebugNode() => new Node{
            value = op.ToString(),
            childs = new List<Node>{expression.DumpDebugNode()}
        };
    }

    enum Op{
        And,Or,Xor,Not,EQ,NE,GE,LE,GT,LT,
        PLUS,MINUS,MUL,DIV,FDIV,Mod
    }

    class Binary : Expression{
        public Expression lhs,rhs;
        public Op op;
        public override Node DumpDebugNode() => new Node{
            value = op.ToString(),
            childs = new List<Node>{
                lhs.DumpDebugNode(),
                rhs.DumpDebugNode()
            }
        };
    }

    class Assign : Expression{
        public Expression lhs,rhs;
        public override Node DumpDebugNode() => new Node{
            value = "Assign",
            childs = new List<Node>{
                lhs.DumpDebugNode(),
                rhs.DumpDebugNode()
            }
        };
    }

    class Convert : Expression{
        public Expression expression;
        public Symbols.Type target;
        public override Node DumpDebugNode() => new Node{
            value = "Convert",
            childs = new List<Node>{
                expression.DumpDebugNode(),
                new Node{value = target.getFullName()}
            }
        };
    }

    class Is : Expression{
        public Expression expression;
        public Symbols.Type target;
        public override Node DumpDebugNode() => new Node{
            value = "Is",
            childs = new List<Node>{
                expression.DumpDebugNode(),
                new Node{value = target.getFullName()}
            }
        };
    }

    class OptionalTest : Expression{
        public List<Symbols.Parameter> optionalParameters;
        public override Node DumpDebugNode() => new Node{
            value = "OptionalTest",
            childs = optionalParameters.Select(x => new Node{value = x.ToString()})
        };
    }

    class Call : Expression{
        public List<ArgumentPass> args = new List<ArgumentPass>();
        public List<OptInit> opts = new List<OptInit>();
        public ParamArrayPass paramArrayPass;
        public Symbols.Function function;
    }

    class FtnCall : Call{
        public Expression reference;
        public override Node DumpDebugNode() => new Node{
            value = "FtnCall " + function.getFullName(),
            childs = new List<Node>{
                reference.DumpDebugNode(),
                new Node{
                    value = "Params",
                    childs = args.Select(x => x.DumpDebugNode())
                            .Concat(opts.Select(x => x.DumpDebugNode()))
                            .Append(paramArrayPass==null ? new Node{value = "EmptyParamArray"} : paramArrayPass.DumpDebugNode())
                }
            } 
        };
    }

    class VFtnCall : Call{
        public Expression reference;
        public override Node DumpDebugNode() => new Node{
            value = "VFtnCall " + function.getFullName(),
            childs = new List<Node>{
                reference.DumpDebugNode(),
                new Node{
                    value = "Params",
                    childs = args.Select(x => x.DumpDebugNode())
                            .Concat(opts.Select(x => x.DumpDebugNode()))
                            .Append(paramArrayPass==null ? new Node{value = "EmptyParamArray"} : paramArrayPass.DumpDebugNode())
                }
            } 
        };
    }

    class SFtnCall : Call{
        public override Node DumpDebugNode() => new Node{
            value = "SFtnCall " + function.getFullName(),
            childs = new List<Node>{
                new Node{
                    value = "Params",
                    childs = args.Select(x => x.DumpDebugNode())
                            .Concat(opts.Select(x => x.DumpDebugNode()))
                            .Append(paramArrayPass==null ? new Node{value = "EmptyParamArray"} : paramArrayPass.DumpDebugNode())
                }
            } 
        };
    }

    class ForeignCall : Call{
        public string? lib,alias;
        public override Node DumpDebugNode() => new Node{
            value = $"ForeignCall {function.getFullName()},lib:{(lib==null?"null":lib)},alias:{(alias==null?"null":alias)},",
            childs = new List<Node>{
                new Node{value = "Params",childs = args.Select(x => x.DumpDebugNode()).Concat(
                        opts.Select(x => x.DumpDebugNode())
                    )}
            } 
        };
    }

    class CtorCall : Call{
        public override Node DumpDebugNode() => new Node{
            value = "CtorCall " + function.getFullName(),
            childs = new List<Node>{
                new Node{
                    value = "Params",
                    childs = args.Select(x => x.DumpDebugNode())
                            .Concat(opts.Select(x => x.DumpDebugNode()))
                            .Append(paramArrayPass==null ? new Node{value = "EmptyParamArray"} : paramArrayPass.DumpDebugNode())
                }
            } 
        };
    }

    class BaseCtorCall : Call{
        public override Node DumpDebugNode() => new Node{
            value = "BaseCtorCall " + function.getFullName(),
            childs = new List<Node>{
                new Node{
                    value = "Params",
                    childs = args.Select(x => x.DumpDebugNode())
                            .Concat(opts.Select(x => x.DumpDebugNode()))
                            .Append(paramArrayPass==null ? new Node{value = "EmptyParamArray"} : paramArrayPass.DumpDebugNode())
                }
            } 
        };
    }

    class NewArray : Expression{
        public Symbols.Type elementType;
        public Expression size;
        public override Node DumpDebugNode() => new Node{
            value = "NewArray",
            childs = new List<Node>{
                new Node{value = elementType.getFullName()},
                new Node{value = size.DumpDebugNode()}
            }
        };
    }

    class IntrinsicCall : Call{
        //public IntrinsicEnum intrinsic; 
        public override Node DumpDebugNode() => new Node{
            value = "IntrinsicCall " + function.getFullName(),
            childs = new List<Node>{
                new Node{value = "Params",childs = args.Select(x => x.DumpDebugNode()).Concat(
                        opts.Select(x => x.DumpDebugNode())
                    )}
            }
        };
    }

    class SuperReference : Expression{
        public override Node DumpDebugNode() => new Node{
            value = "SuperReference"
        };
    }

    class SuperInitialize : Call{
        public SuperReference reference;
    }

    class LocalAccess : Expression{
        public Symbols.Variable local;
        public override Node DumpDebugNode() => new Node{
            value = "LocalAccess " + local.getFullName()
        };
    }

    class ArgumentAccess : Expression{
        public bool isRef;
        public Symbols.Parameter parameter;
        public override Node DumpDebugNode() => new Node{
            value = "ArgumentAccess " + (isRef ? "Ref ":"") + parameter.name
        };
    }

    class ArgumentPass : AST{
        public bool isRef;
        public Symbols.Parameter parameter;
        public Expression expression;
        public override Node DumpDebugNode() => new Node{
            value = "ArgumentPass " + (isRef ? "Ref ":"") + parameter.name,
            childs = new List<Node>{
                expression.DumpDebugNode()
            }
        };
    }
    
    class ParamArrayPass : AST{
        public Expression forward = null;
        public List<Expression> expressions;
        public override Node DumpDebugNode() => new Node{
            value = "ParamArrayPass",
            childs = forward==null ? expressions.Select(x => x.DumpDebugNode()) : new List<Node>{forward.DumpDebugNode()}
        };
    }

    class OptInit : AST{
        public bool isRef;
        public Symbols.Parameter parameter;
        public Expression expression;
        public string option;
        public override Node DumpDebugNode() => new Node{
            value = $"OptionInit {option} {(isRef ? "Ref" : "")}",
            childs = new List<Node>{
                expression.DumpDebugNode()
            }
        };
    }

    class FldAccess : Expression{
        public Expression reference;
        public Symbols.Variable variable;
        public override Node DumpDebugNode() => new Node{
            value = "FldAccess " + variable.getFullName(),
            childs = new List<Node>{
                reference.DumpDebugNode()
            }
        };
    }

    class SFldAccess : Expression{
        public Symbols.Variable variable;
        public override Node DumpDebugNode() => new Node{
            value = "SFldAccess " + variable.getFullName()
        };
    }

    class EnumConst : Expression{
        public Symbols.EnumConstant constant;
        public override Node DumpDebugNode() => new Node{
            value = "EnumConstant " + constant.getFullName()
        };
    }

    class Subscript : Expression{
        public Expression reference;
        public Expression subscript;
        public override Node DumpDebugNode() => new Node{
            value = "Subscript",
            childs = new List<Node>{
                reference.DumpDebugNode(),
                subscript.DumpDebugNode()
            }
        };
    }

    class ArrayLiteral : Expression{
        public List<Expression> expressions;
        public override Node DumpDebugNode() => new Node{
            value = "ArrayLiteral",
            childs = expressions.Select(x => x.DumpDebugNode())
        };
    }

    class IntegerLiteral : Expression{
        public Int32 value;
        public override Node DumpDebugNode() => new Node{
            value = "IntegerLiteral " + value.ToString()
        };
    }

    class DoubleLiteral : Expression{
        public Double value;
        public override Node DumpDebugNode() => new Node{
            value = "DoubleLiteral " + value.ToString()
        };
    }

    class StringLiteral : Expression{
        public String value;
        public override Node DumpDebugNode() => new Node{
            value = $"StringLiteral \"{value}\"",
        };
    }

    class RuneLiteral : Expression{
        public UInt32 value;
        public override Node DumpDebugNode() => new Node{
            value = $"RuneLiteral '{value}'"
        };
    }

    class BooleanLiteral : Expression{
        public bool value;
        public override Node DumpDebugNode() => new Node{
            value = $"BooleanLiteral {value}"
        };
    }

    class NothingLiteral : Expression{
        public override Node DumpDebugNode() => new Node{
            value = "Nothing"
        };
    }

}
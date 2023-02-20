namespace evoBasic;
using Antlr4.Runtime.Misc;
using System.Collections.Generic;
using Parser = evoBasicParser;
using Symbols;
using Antlr4.Runtime;
using System.Text;


class TrdPass : evoBasicBaseVisitor<AST.AST>{
    CompileEnv env;
    enum LocalScopeKind{FunctionScope,SubFunction,LoopScope,IfCaseScope};
    bool isInWhileLoop = false,isInForLoop = false;
    Stack<Scope> scope = new Stack<Scope>();
    Symbols.Class currentClass = null;
    Symbols.ExecuableFunction currentFunction = null;

    public TrdPass(CompileEnv env){
        this.env = env;
    }

    public void AnalyzeAll(){
        staticEnviormentFlag.Push(true);
        TraverseScope(env.global.scope);
        staticEnviormentFlag.Pop();
    }

    void TraverseScope(Scope scope){
        foreach((_,var symbol) in scope.childern){
            if(symbol is Module)VisitModule(symbol as Module);
            else if(symbol is Class)VisitClass(symbol as Class);
            else if(symbol is Method)VisitMethod(symbol as Method);
            else if(symbol is Constructor)VisitConstructor(symbol as Symbols.Constructor);
        }
    }

    void VisitModule(Module module){
        scope.Push(module.scope);
        staticEnviormentFlag.Push(true);
        TraverseScope(module.scope);
        staticEnviormentFlag.Pop();
        scope.Pop();
    }

    void VisitClass(Class cls){
        scope.Push(cls.scope);
        staticEnviormentFlag.Push(false);
        currentClass = cls;
        TraverseScope(cls.scope);
        currentClass = null;
        staticEnviormentFlag.Pop();
        scope.Pop();
    }

    void VisitConstructor(Constructor ctor){
        scope.Push(ctor.scope);
        currentFunction = ctor;
        staticEnviormentFlag.Push(ctor.flag.Exists(Flags.Static));
        if(ctor.location is Source.SourceLocation){
            ctor.extendCtorAST = new AST.BaseCtorCall{
                function = ctor.ownerClass.baseClass.ctor
            };

            if(ctor.extendPartContext!=null){
                FillCallArgs(ctor.extendPartContext.argumentList(), ctor.extendCtorAST);
            }

            ctor.astBlock = VisitStatements(ctor.parseTreeBlock);
        }
        staticEnviormentFlag.Pop();
        scope.Pop();
    }

    void VisitMethod(Method method){
        scope.Push(method.scope);
        currentFunction = method;
        staticEnviormentFlag.Push(method.flag.Exists(Flags.Static));
        if(method.location is Source.SourceLocation){
            method.astBlock = VisitStatements(method.parseTreeBlock);
        }
        staticEnviormentFlag.Pop();
        scope.Pop();
    }
    
    AST.Block VisitStatements(Parser.StatementContext[] statements){
        var block = new AST.Block();
        foreach(var statement in statements){
            var ast = (AST.Statement)Visit(statement);
            if(ast!=null)block.statements.Add(ast);
        }
        return block;
    }

    public override AST.AST VisitTryStmt([NotNull] evoBasicParser.TryStmtContext context){
        var ret = new AST.TryStmt {
            line = (UInt32)context.Try()[0].Symbol.Line
        };
        scope.Push(new Scope(currentFunction){ parent = scope.Peek() });
        ret.tryBlock = VisitStatements(context.statement());
        scope.Pop();
        ret.catchParts = new List<AST.CatchPart>();
        foreach(var catchPart in context.catchPart()){
            ret.catchParts.Add(VisitCatchPart(catchPart) as AST.CatchPart);
        }
        return ret;
    }

    public override AST.AST VisitCatchPart([NotNull] evoBasicParser.CatchPartContext context){
        var ret = new AST.CatchPart();
        scope.Push(new Scope(currentFunction){ parent = scope.Peek() });
        ret.exceptionVariable = new Variable{
            type = Utils.SearchAnnotation(context.annotation(), scope.Peek(), env),
            name = context.ID().GetText(),
            location = new Source.SourceLocation(context.ID())
        };
        scope.Peek().Add(ret.exceptionVariable);
        ret.catchBlock = VisitStatements(context.statement());
        scope.Pop();
        return ret;
    }
	
    public override AST.AST VisitThrowStmt([NotNull] evoBasicParser.ThrowStmtContext context){
        return new AST.Throw{
            line = (UInt32)context.Throw().Symbol.Line,
            expression = Visit(context.expression())as AST.Expression
        };
    }

    public override AST.AST VisitDimStmt([NotNull] evoBasicParser.DimStmtContext context){
        var ret = new AST.DimStmt{
             line = (UInt32)context.Dim().Symbol.Line
        };

        foreach(var varibaleAST in context.variableDecl()){
            var location = new Source.SourceLocation(varibaleAST.ID());
            var variable = new Variable(){
                type = Utils.SearchAnnotation(varibaleAST.annotation(), scope.Peek(), env),
                name = varibaleAST.ID().GetText(),
                location = location
            };

            if(!scope.Peek().Add(variable)){
                Logger.Add(Log.Error, location, Formater.singleton.NameDuplicate(variable.name));
            }

            AST.Expression initExpr = null;
            if (varibaleAST.initial != null) { 
                initExpr = VisitExpression(varibaleAST.initial) as AST.Expression;
            }

            ret.variables.Add(new Tuple<Variable, AST.Expression>(
                variable, initExpr
            ));
        }
        
        return ret;
    }

	public override AST.AST VisitLoopStmt([NotNull] evoBasicParser.LoopStmtContext context){
        scope.Push(new Scope(currentFunction){ parent = scope.Peek() });
        var prvValue = isInWhileLoop;
        isInWhileLoop = true;
        var expression = Visit(context.expression()) as AST.Expression;
        if(!expression.type.Equal(env.Boolean)){
            Logger.Add(Log.Error, context.expression(), Formater.singleton.TypeMismatch(env.Boolean.name,expression.type.name));
        }
        var block = VisitStatements(context.statement());
        isInWhileLoop = prvValue;
        scope.Pop();
        return new AST.Loop(){
            line = (UInt32)context.While().Symbol.Line,
            condition = expression,
            block = block
        };
    }

    public override AST.AST VisitForStmt([NotNull] evoBasicParser.ForStmtContext context){
        scope.Push(new Scope(currentFunction){ parent = scope.Peek() });
        var prvValue = isInForLoop;
        isInForLoop = true;

        bool iteratorHasDim = false;
        Variable iteratorVariable = null;
        AST.Expression iteratorAST = null;
        if(context.forIterator().Dim()!=null){
            iteratorVariable = new Variable(){
                name = context.forIterator().ID().GetText(),
                type = env.Integer
            };
            scope.Peek().Add(iteratorVariable);
            iteratorAST = new AST.LocalAccess(){
                local = iteratorVariable,
                type = iteratorVariable.type,
                exprSymbol = iteratorVariable,
                exprFlag = AST.ExprFlag.LValue,
                location = new Source.SourceLocation(context.forIterator())
            };
            iteratorHasDim = true;
        }
        else if(context.forIterator().primaryExpression()!=null){
            iteratorAST = Visit(context.forIterator().primaryExpression()) as AST.Expression;
            if(!iteratorAST.type.Equal(env.Integer)){
                Logger.Add(Log.Error, context.forIterator(), Formater.singleton.TypeMismatch(env.Integer.name,iteratorAST.type.name));
            }
        }

        var begAST = Visit(context.from) as AST.Expression;
        var endAST = Visit(context.to) as AST.Expression;
        AST.Expression sepAST = null;


        //添加循环中的beg/end/sep变量到当前函数的作用域
        var begVar = new Variable(){
            name = "#for_beg_"+scope.Peek().childern.Count(),
            type = env.Integer
        };
        scope.Peek().Add(begVar);

        var endVar = new Variable(){
            name = "#for_end_"+scope.Peek().childern.Count(),
            type = env.Integer
        };
        scope.Peek().Add(endVar);

        var sepVar = new Variable(){
            name = "#for_sep_"+scope.Peek().childern.Count(),
            type = env.Integer
        };
        scope.Peek().Add(sepVar);


        if(!begAST.type.Equal(env.Integer)){
            Logger.Add(Log.Error, context.from, Formater.singleton.TypeMismatch(env.Integer.name,begAST.type.name));
        }
        if(!endAST.type.Equal(env.Integer)){
            Logger.Add(Log.Error, context.to, Formater.singleton.TypeMismatch(env.Integer.name,endAST.type.name));
        }

        if(context.step!=null){
            sepAST = Visit(context.step) as AST.Expression;
            if(!sepAST.type.Equal(env.Integer)){
                Logger.Add(Log.Error, context.step, Formater.singleton.TypeMismatch(env.Integer.name,sepAST.type.name));
            }
        }

        var block = VisitStatements(context.statement()) as AST.Block;

        scope.Pop();
        isInForLoop = prvValue;
        return new AST.For(){
            line = (UInt32)context.For().Symbol.Line,
            iteratorHasDim = iteratorHasDim,
            iterator = iteratorAST,
            begin = begAST,
            end = endAST,
            step = sepAST,
            iterVar = iteratorVariable,
            begVar = begVar,
            endVar = endVar,
            sepVar = sepVar,
            block = block
        };
    }

    public override AST.AST VisitOnelineIfStmt([NotNull] evoBasicParser.OnelineIfStmtContext context){
        var ret = new AST.If{
            line = (UInt32)context.If().Symbol.Line
        };

        var expression = Visit(context.expression()) as AST.Expression;
        if(!(expression.type == env.Boolean)){
            Logger.Add(Log.Error, context.expression(), Formater.singleton.TypeMismatch(env.Boolean.name,expression.type.name));
        }

        ret.cases.Add(new AST.Case{
            condition = expression,
            block = VisitStatements(new Parser.StatementContext[]{context.truePath})
        });

        if(context.falsePart!=null){
            ret.cases.Add(new AST.Case{
                condition = null,
                block = VisitStatements(new Parser.StatementContext[]{context.falsePart})
            });
        }
        return ret;
    }

	public override AST.AST VisitIfStmt([NotNull] evoBasicParser.IfStmtContext context){
        var ret = new AST.If{
            line = (UInt32)context.If().First().Symbol.Line,
        };

        var expression = Visit(context.expression()) as AST.Expression;
        if(!(expression.type == env.Boolean)){
            Logger.Add(Log.Error, context.expression(), Formater.singleton.TypeMismatch(env.Boolean.name,expression.type.name));
        }

        scope.Push(new Scope(currentFunction){ parent = scope.Peek() });
        ret.cases.Add(new AST.Case(){
            condition = expression,
            block = VisitStatements(context.statement())
        });
        scope.Pop();

        foreach(var elseif in context.elseIfPart()){
            ret.cases.Add((AST.Case)VisitElseIfPart(elseif));
        }

        if(context.elsePart()!=null){
            ret.cases.Add((AST.Case)VisitElsePart(context.elsePart()));
        }

        return ret;
    }

	public override AST.AST VisitElseIfPart([NotNull] evoBasicParser.ElseIfPartContext context){
        var expression = Visit(context.expression()) as AST.Expression;
        if(!expression.type.Equal(env.Boolean)){
            Logger.Add(Log.Error, context.expression(), Formater.singleton.TypeMismatch(env.Boolean.name,expression.type.name));
        }
        var block = VisitStatements(context.statement());
        return new AST.Case(){
            condition = expression,
            block = block
        };
    }

	public override AST.AST VisitElsePart([NotNull] evoBasicParser.ElsePartContext context){
        scope.Push(new Scope(currentFunction){ parent = scope.Peek() });
        var block = VisitStatements(context.statement());
        scope.Pop();
        return new AST.Case(){
            condition = null,
            block = block
        };
    }

	public override AST.AST VisitSelectStmt([NotNull] evoBasicParser.SelectStmtContext context){
        throw new NotImplementedException();
    }

	public override AST.AST VisitCaseList([NotNull] evoBasicParser.CaseListContext context){
        throw new NotImplementedException();
    }

	public override AST.AST VisitControlStmt([NotNull] evoBasicParser.ControlStmtContext context){
        if(context.Return()!=null){
            if(currentFunction.retType == env.Void){
                Logger.Add(Log.Error, context, Formater.singleton.CannotReturnInSub());
                return new AST.ErrorStmt();
            }
            else{
                var expression = VisitExpressionWithImplicitConversion(context.expression(),currentFunction.retType) as AST.Expression;
                if(!expression.type.Equal(currentFunction.retType)){
                    Logger.Add(Log.Error, context, Formater.singleton.TypeMismatch(currentFunction.retType.name,expression.type.name));
                    return new AST.ErrorStmt();
                }

                return new AST.Return(){
                    line = (UInt32)context.Return().Symbol.Line,
                    expression = expression
                };
            }
        }
        else if(context.Continue()!=null){
            if(isInForLoop||isInWhileLoop){
                return new AST.Continue{ line = (UInt32)context.Continue().Symbol.Line };
            }
            else{
                Logger.Add(Log.Error, context, Formater.singleton.ContinueOnlyAllowedInLoop());
                return new AST.ErrorStmt();
            }
        }
        else if(context.Exit()!=null && context.For()!=null){
            if(isInForLoop){
                return new AST.Exit{
                    line = (UInt32)context.Exit().Symbol.Line,
                    flag = AST.Exit.Flag.For
                };
            }
            else{
                Logger.Add(Log.Error, context, Formater.singleton.ExitOnlyAllowedIn("For"));
                return new AST.ErrorStmt();
            }
        }
        else if(context.Exit()!=null && context.While()!=null){
            if(isInWhileLoop){
                return new AST.Exit{
                    line = (UInt32)context.Continue().Symbol.Line,
                    //flag = AST.Exit.Flag.While
                };
            }
            else{
                Logger.Add(Log.Error, context, Formater.singleton.ExitOnlyAllowedIn("While"));
                return new AST.ErrorStmt();
            }
        }
        else if(context.Exit()!=null && context.Sub()!=null){
            if(currentFunction.retType.Equal(env.Void)){
                return new AST.Exit{
                    line = (UInt32)context.Exit().Symbol.Line,
                    flag = AST.Exit.Flag.Sub
                };
            }
            else{
                Logger.Add(Log.Error, context, Formater.singleton.ExitOnlyAllowedIn("Sub"));
                return new AST.ErrorStmt();
            }
        }

        throw new ArgumentException();
    }

    public override AST.AST VisitExprStmt([NotNull] evoBasicParser.ExprStmtContext context){
        return new AST.ExprStmt{
            line = (UInt32)context.Start.Line,
            expression = (AST.Expression)Visit(context.expression())
        };
    }

    public AST.AST WrapBoxing(Symbols.Primitive primitive, AST.Expression expression){
        var boxClass = env.boxing.FindBox(expression.type);
        return new AST.CtorCall{
            function = boxClass.ctor,
            type = boxClass,
            exprSymbol = new Variable{type = boxClass},
            exprFlag = AST.ExprFlag.RValue,
            args = new List<AST.ArgumentPass>{
                new AST.ArgumentPass{
                    isRef = false,
                    parameter = boxClass.ctor.normalParameters[0],
                    expression = expression
                }
            }
        };
    }

    public AST.AST WrapUnboxing(Symbols.Primitive primitive, AST.Expression expression){
        var boxClass = env.boxing.FindBox(primitive);
        var convertAST = WrapConversion(expression,boxClass);
        return new AST.FtnCall{
            reference = convertAST,
            function = boxClass.scope.Find("unbox") as Function,
            args = new List<AST.ArgumentPass>(),
            type = primitive,
            exprSymbol = new Variable{type = primitive},
            exprFlag = AST.ExprFlag.RValue
        };
    }

    public AST.AST VisitExpressionWithImplicitConversion([NotNull] Parser.ExpressionContext context,Type expectedType){
        var ret = Visit(context) as AST.Expression;
        if(!ret.type.Equal(expectedType)){
            if(env.truncation.Exist(ret.type,expectedType)){
                //warning
                return WrapConversion(ret,expectedType);
            }
            else if(env.widening.Exist(ret.type,expectedType)){
                //warning
                return WrapConversion(ret,expectedType);
            }
            else if(env.floatingPointConvertion.Exist(ret.type,expectedType)){
                //warning
                return WrapConversion(ret,expectedType);
            }
            else if(expectedType==env.EBObject && env.boxing.Exist(ret.type)){
                // 偷偷进行Primitive Type到object的隐式装箱,
                // 假装Primitive Type也是Object的子类型
                return WrapBoxing(ret.type as Symbols.Primitive, ret);
            }
            else{
                return ret;
            }
        }
        else{
            return ret;
        }
    }

    public override AST.AST VisitExpression([NotNull] evoBasicParser.ExpressionContext context){
        return Visit(context.assignmentExpression());
    }

    public override AST.AST VisitAssignmentExpression([NotNull] evoBasicParser.AssignmentExpressionContext context){
        if(context.assignment()!=null)
            return Visit(context.assignment());
        else if(context.conditionalExpression()!=null)
            return Visit(context.conditionalExpression());
        
        throw new ArgumentException();
    }

    public override AST.AST VisitAssignment([NotNull] evoBasicParser.AssignmentContext context){
        var lhs = (AST.Expression)Visit(context.primaryExpression());
        if(lhs is AST.ErrorExpr)return lhs;
        var rhs = (AST.Expression)VisitExpressionWithImplicitConversion(context.expression(),lhs.type);
        if(rhs is AST.ErrorExpr)return rhs;

        if(lhs.exprFlag!=AST.ExprFlag.LValue){
            Logger.Add(Log.Error,context.primaryExpression(),Formater.singleton.InvalidAssign());
        }

        if(!lhs.type.Equal(rhs.type)){
            Logger.Add(Log.Error, context.expression(), Formater.singleton.TypeMismatch(lhs.type.name,rhs.type.name));
        }

        return new AST.Assign(){
            lhs = lhs,
            rhs = rhs,
            type = lhs.type,
            exprFlag = AST.ExprFlag.LValue,
            location = new Source.SourceLocation(context)
        };
    }

    public override AST.AST VisitConditionalExpression([NotNull] evoBasicParser.ConditionalExpressionContext context){
        if(context.conditionalExpression()!=null){
            var lhs = (AST.Expression)Visit(context.conditionalExpression());
            var rhs = (AST.Expression)Visit(context.conditionalNotExpression());
            AST.Op op = AST.Op.And;

            if(context.And()!=null) op = AST.Op.And;
            else if(context.Or()!=null) op = AST.Op.Or;
            else if(context.Xor()!=null) op = AST.Op.Xor;

            if(!lhs.type.Equal(env.Boolean)){
                Logger.Add(Log.Error, context.conditionalExpression(), Formater.singleton.TypeMismatch(env.Boolean.name,lhs.type.name));
            }

            if(!rhs.type.Equal(env.Boolean)){
                Logger.Add(Log.Error, context.conditionalNotExpression(), Formater.singleton.TypeMismatch(env.Boolean.name,rhs.type.name));
            }

            return new AST.Binary(){
                lhs = lhs,
                rhs = rhs,
                op = op,
                type = env.Boolean,
                exprFlag = AST.ExprFlag.RValue,
                location = new Source.SourceLocation(context)
            };
        }
        else{
            return Visit(context.conditionalNotExpression());
        }
    }

    public override AST.AST VisitConditionalNotExpression([NotNull] evoBasicParser.ConditionalNotExpressionContext context){
        if(context.Not()!=null){
            var rhs = (AST.Expression)Visit(context.relationalExpression());

            if(!rhs.type.Equal(env.Boolean)){
                Logger.Add(Log.Error, context.relationalExpression(), Formater.singleton.TypeMismatch(env.Boolean.name,rhs.type.name));
            }

            return new AST.Unary(){
                op = AST.Op.Not,
                expression = rhs,
                type = env.Boolean,
                exprFlag = AST.ExprFlag.RValue,
                location = new Source.SourceLocation(context)
            };
        }
        else{
            return Visit(context.relationalExpression());
        }
    }

    public AST.Expression WrapConversion(AST.Expression expression,Type target){
        return new AST.Convert{
            expression = expression,
            target = target,
            type = target,
            exprSymbol = new Variable{type = target},
            exprFlag = AST.ExprFlag.RValue
        };
    }

    public void InsertImplicitPromotion(AST.Binary binary,bool fixResuleType = false){
        var resultType = env.promotion.Query(binary.lhs.type,binary.rhs.type);
        if(!binary.lhs.type.Equal(resultType)){
            binary.lhs = WrapConversion(binary.lhs,resultType);
        }
        if(!binary.rhs.type.Equal(resultType)){
            binary.rhs = WrapConversion(binary.rhs,resultType);
        }
        binary.exprFlag = AST.ExprFlag.RValue;
        if(!fixResuleType){
            binary.type = resultType;
            binary.exprSymbol = new Variable{type = resultType};
        }
    }

    public override AST.AST VisitRelationalExpression([NotNull] evoBasicParser.RelationalExpressionContext context){
        if(context.relationalExpression()!=null){
            var lhs = (AST.Expression)Visit(context.relationalExpression());
            var rhs = (AST.Expression)Visit(context.additiveExpression());
            AST.Op op = AST.Op.EQ;

            if(context.relationalOp().eqOp()!=null) op = AST.Op.EQ;
            else if(context.relationalOp().neOp()!=null) op = AST.Op.NE;
            else if(context.relationalOp().ltOp()!=null) op = AST.Op.LT;
            else if(context.relationalOp().gtOp()!=null) op = AST.Op.GT;
            else if(context.relationalOp().leOp()!=null) op = AST.Op.LE;
            else if(context.relationalOp().geOp()!=null) op = AST.Op.GE; 

            var ret = new AST.Binary(){
                lhs = lhs,
                rhs = rhs,
                op = op,
                type = env.Boolean,
                exprFlag = AST.ExprFlag.RValue,
                exprSymbol = new Variable{type = env.Boolean},
                location = new Source.SourceLocation(context)
            };

            if(!lhs.type.Equal(rhs.type)){
                if(env.promotion.Exist(lhs.type,rhs.type)){
                    InsertImplicitPromotion(ret,fixResuleType:true);
                }
                else{
                    Logger.Add(Log.Error, context.additiveExpression(), Formater.singleton.TypeMismatch(lhs.type.name,rhs.type.name));
                }
            }

            return ret;
        }
        else{
            return Visit(context.additiveExpression());
        }
    }

	public override AST.AST VisitAdditiveExpression([NotNull] evoBasicParser.AdditiveExpressionContext context){
        if(context.additiveExpression()!=null){
            AST.Op op = AST.Op.PLUS;
            if(context.op.Text == "-") op = AST.Op.MINUS;
            else if(context.op.Text == "+") op = AST.Op.PLUS;
            
            var lhs =  (AST.Expression)Visit(context.additiveExpression());
            var rhs = (AST.Expression)Visit(context.multiplicativeExpression());

            var ret = new AST.Binary(){
                lhs = lhs,
                op = op,
                rhs = rhs,
                type = lhs.type,
                exprFlag = AST.ExprFlag.RValue,
                exprSymbol = new Variable{type = lhs.type},
                location = new Source.SourceLocation(context)
            };

            if(!lhs.type.Equal(rhs.type)){
                if(env.promotion.Exist(lhs.type,rhs.type)){
                    InsertImplicitPromotion(ret);
                }
                else{
                    Logger.Add(Log.Error, context.multiplicativeExpression(), Formater.singleton.TypeMismatch(lhs.type.name,rhs.type.name));
                }
            }

            return ret;
        }
        else{
            return Visit(context.multiplicativeExpression());
        }
    }

	public override AST.AST VisitMultiplicativeExpression([NotNull] evoBasicParser.MultiplicativeExpressionContext context){
        if(context.multiplicativeExpression()!=null){
            AST.Op op = AST.Op.MUL;
            if(context.op.Text == "*")      op = AST.Op.MUL;
            else if(context.op.Text == "/") op = AST.Op.DIV;
            else if(context.Mod()!=null)    op = AST.Op.Mod;

            var lhs = (AST.Expression)Visit(context.multiplicativeExpression());
            var rhs = (AST.Expression)Visit(context.unaryExpression());

            var ret = new AST.Binary(){
                lhs = lhs,
                op = op,
                rhs = rhs,
                type = lhs.type,
                exprFlag = AST.ExprFlag.RValue,
                exprSymbol = new Variable{type = lhs.type},
                location = new Source.SourceLocation(context)
            };

            if(!lhs.type.Equal(rhs.type)){
                if(env.promotion.Exist(lhs.type,rhs.type)){
                    InsertImplicitPromotion(ret);
                }
                else{
                    Logger.Add(Log.Error, context.multiplicativeExpression(), Formater.singleton.TypeMismatch(lhs.type.name,rhs.type.name));
                }
            }
            
            // ****用\/区分浮点和整数除法的屑特性还是先不添加吧****
            //
            // //如果是除法运算，返回类型为Double
            // if(op == AST.Op.DIV){
            //     ret.type = env.Double;
            //     ret.exprSymbol = new Variable{type = env.Double};
            // }

            return ret;
        }
        else{
            return Visit(context.unaryExpression());
        }
    }

    public override AST.AST VisitUnaryExpression([NotNull] evoBasicParser.UnaryExpressionContext context){
        if(context.unaryExpression()!=null){
            var rhs = (AST.Expression)Visit(context.unaryExpression());
            AST.Op op = AST.Op.PLUS;

            if(context.op.Text == "+")  op = AST.Op.PLUS;
            else if(context.op.Text == "-") op = AST.Op.MINUS;

            return new AST.Unary(){
                op = op,
                expression = rhs,
                type = rhs.type,
                exprFlag = AST.ExprFlag.RValue,
                exprSymbol = new Variable{type = rhs.type},
                location = new Source.SourceLocation(context)
            };
        }
        else{
            return Visit(context.primaryExpression());
        }
    }

    public override AST.AST VisitDotExpression([NotNull] evoBasicParser.DotExpressionContext context){
        if(context.terminalExpression().ambiguousName != null){
            var lhsAST = Visit(context.primaryExpression()) as AST.Expression;
            if(lhsAST is AST.ErrorExpr)return lhsAST;
            var lhs = lhsAST.exprSymbol;
            string rhsName = context.terminalExpression().ambiguousName.Text;
            /*
             *      可能的exprSymbol类型：
             *         Mod Cls Var Ftn Enum EmConst
             *      可能的点运算和需要附加的动作：
             *         Mod . Mod' -> 找到Mod里的Mod',对Mod'进行可达性判定
             *         Mod . Cls -> 找到Mod里的Cls,对Cls进行可达性判定
             *         Mod . Var -> 找到Mod里的Var,对Var进行可达性判定
             *         Mod . Ftn -> 找到Mod里的Ftn,对Ftn进行可达性判定
             *         Mod . Enum -> 找到Mod里的Enum,对Enum进行可达性判定
             *         Mod . EmConst -> 无效
             *         Cls . Mod -> 无效
             *         Cls . Cls -> 无效
             *         Cls . Var(非静态) -> 非法。不可能通过类名访问实例的非静态变量
             *         Cls . Var(静态)  -> 找到Cls类里的静态变量Var，对Var进行可达性判定
             *         Cls . Ftn(非静态) -> 非法。理由同上
             *         Cls . Ftn(静态) -> 合法。找到Cls类里的静态函数Ftn，对Ftn进行可达性判定
             *         Cls . Enum -> 找到Cls里的Enum符号，对Enum进行可达性判定
             *         Cls . EmConst -> 无效
             *         Var . Mod -> 无效
             *         Var . Var' -> 找到Var的类型中的字段Var'，对Var'进行可达性判定
             *         Var . Ftn(非静态) -> 找到Var的类型中的Ftn函数，对Ftn进行可达性判定
             *         Var . Ftn(静态) -> 非法。不能通过实例访问类的静态函数
             *         Var . Enum -> 无效
             *         Var . EmConst -> 无效
             *         Ftn . 任意类型 -> 无效
             *         Enum . EmConst -> 找到Enum中的枚举值
             *
             */
            if(lhs is Module mod){
                var rhs = mod.scope.Find(rhsName);
                if(rhs==null){
                    Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.ObjectNotfound(rhsName));
                    return new AST.ErrorExpr();
                }
                else{
                    if(!scope.Peek().Reachable(env.global.scope,rhs)){
                        Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.TargetUnreachable(rhs.getFullName()));
                    }

                    if(rhs is Module || rhs is Class || rhs is Enumeration){
                        return new AST.PseudoReference(){
                            exprSymbol = rhs,
                            location = new Source.SourceLocation(context.terminalExpression())
                        };
                    }
                    else if(rhs is Variable variable){
                        return new AST.SFldAccess(){
                            variable = variable,
                            exprFlag = AST.ExprFlag.LValue,
                            exprSymbol = variable,
                            type = variable.type,
                            location = new Source.SourceLocation(context.terminalExpression())
                        };
                    }
                    else if(rhs is Method function){
                        return new AST.SFtnCall(){
                            function = function,
                            exprSymbol = function
                        };
                    }
                    else if(rhs is ForeignEntry foreignEntry){
                        return new AST.ForeignCall(){
                            function = foreignEntry,
                            exprSymbol = foreignEntry
                        };
                    }
                }
            }
            else if(lhs is Class cls){
                var rhs = cls.scope.Find(rhsName);
                if(rhs==null){
                    Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.ObjectNotfound(rhsName));
                    return new AST.ErrorExpr();
                }
                else if(!rhs.flag.Exists(Flags.Static)){
                    Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.CannotAccessNonStaticMember());
                    return new AST.ErrorExpr();
                }
                else{ //从类名访问静态函数或静态字段
                    if(!scope.Peek().Reachable(env.global.scope,rhs)){
                        Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.TargetUnreachable(rhs.getFullName()));
                    }

                    if(rhs is Variable variable){
                        return new AST.SFldAccess(){
                            variable = variable,
                            exprSymbol = variable,
                            type = variable.type,
                            location = new Source.SourceLocation(context.terminalExpression())
                        };
                    }
                    else if(rhs is Function function){
                        return new AST.SFtnCall(){
                            function = function,
                            exprSymbol = function
                        };
                    }
                }
            }
            else if(lhs is Variable variable){
                var lhsScope = Utils.GetSymbolScope(variable.type);
                if(lhsScope==null){
                    Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.LhsIsNotScope());
                    return new AST.ErrorExpr();
                }
                var rhs = lhsScope.Find(rhsName);
                if(rhs == null){
                    Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.ObjectNotfound(rhsName));
                    return new AST.ErrorExpr();
                }

                if(variable.type is Symbols.Record) { //特判record： 当lhs是RValue时，record.variable也是RValue。
                    return new AST.FldAccess() {
                        reference = lhsAST,
                        variable = rhs as Variable,
                        type = (rhs as Variable).type,
                        exprFlag = lhsAST.exprFlag==AST.ExprFlag.RValue ? AST.ExprFlag.RValue : AST.ExprFlag.LValue,
                        exprSymbol = rhs,
                        location = new Source.SourceLocation(context)
                    };
                }
                else {
                    if (rhs.flag.Exists(Flags.Static)) {
                        Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.CannotAccessStaticMember());
                        return new AST.ErrorExpr();
                    }
                    else { //从实例变量访问函数或字段
                        if (!scope.Peek().Reachable(env.global.scope, rhs)) {
                            Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.TargetUnreachable(rhs.getFullName()));
                        }

                        if (rhs is Variable) {
                            return new AST.FldAccess() {
                                reference = lhsAST,
                                variable = rhs as Variable,
                                type = (rhs as Variable).type,
                                exprFlag = AST.ExprFlag.LValue,
                                exprSymbol = rhs,
                                location = new Source.SourceLocation(context)
                            };
                        }
                        else if (rhs is Function rhsFunction) {
                            // 区分调用的是虚函数还是普通方法
                            if (rhsFunction.flag.Exists(Flags.Override | Flags.Virtual)) {
                                return new AST.VFtnCall() {
                                    reference = lhsAST,
                                    function = rhsFunction,
                                    exprSymbol = rhs,
                                };
                            }
                            else {
                                return new AST.FtnCall() {
                                    reference = lhsAST,
                                    function = rhsFunction,
                                    exprSymbol = rhs,
                                };
                            }
                        }
                    }
                }
            }
            else if(lhs is Function ftn || lhs is EnumConstant){
                Logger.Add(Log.Error, context.primaryExpression(), Formater.singleton.InvalidDotExpression());
                return new AST.ErrorExpr();
            }
            else if(lhs is Enumeration em){
                var rhs = em.scope.Find(rhsName);
                if(rhs==null){
                    Logger.Add(Log.Error, context.terminalExpression(), Formater.singleton.ObjectNotfound(rhsName));
                    return new AST.ErrorExpr();
                }
                else{
                    var enumConst = rhs as EnumConstant;
                    return new AST.EnumConst(){
                        constant = enumConst,
                        type = enumConst.owner.attch as Type,
                        exprSymbol = enumConst.owner.attch as Type,
                        location = new Source.SourceLocation(context),
                    };
                }
            }
        }
        else if(context.terminalExpression().Self() != null
                || context.terminalExpression().expression() != null 
                || context.terminalExpression().literal() != null){
            Logger.Add(Log.Error, context, Formater.singleton.InvalidDotExpression());
            return new AST.ErrorExpr();
        }

        throw new ArgumentException();
    }

    public override AST.AST VisitCastExpression([NotNull] evoBasicParser.CastExpressionContext context){
        var primary = (AST.Expression)Visit(context.primaryExpression());
        if(primary is AST.ErrorExpr)return primary;
        var targetType = Utils.SearchAnnotation(context.annotation(), scope.Peek(), env);
        
        // Object到Primitive Type的拆箱
        if(primary.type==env.EBObject && targetType is Symbols.Primitive){
            return WrapUnboxing(targetType as Symbols.Primitive, primary);
        }
        // Primitive Type到Object的装箱
        else if(primary.type is Symbols.Primitive && targetType==env.EBObject){
            return WrapBoxing(targetType as Symbols.Primitive, primary);
        }

        return WrapConversion(primary,targetType);
    }

    public void FillCallArgs(Parser.ArgumentListContext context,AST.Call call){
        
        int providedNormalArgumentCount = 0;
        int requiredNormalParemterCount =  call.function.normalParameters.Count;

        foreach(var argument in context.argument()){
            if(argument.ID()!=null)break;
            providedNormalArgumentCount++;
        }

        if((providedNormalArgumentCount > requiredNormalParemterCount && call.function.paramArray==null) || 
            (providedNormalArgumentCount < requiredNormalParemterCount)){
            Logger.Add(Log.Error, context, 
                Formater.singleton.ArgCountMismatch(requiredNormalParemterCount,
                                                    call.function.name,
                                                    providedNormalArgumentCount));
        }

        int argIndex;
        // 遍历检查函数必选参数
        for(argIndex = 0; argIndex < call.function.normalParameters.Count && argIndex < context.argument().Count(); argIndex++){
            var argument = context.argument()[argIndex];
            var parameter = call.function.normalParameters[argIndex];
            if(argument.ID()!=null)break; // 参数带有可选参数初始化标识(遇到可选参数的实参)，非法

            // 获取实参语法树，顺便检查是否需要以及是否可以进行隐式转换并插入
            var argExpAST = VisitExpressionWithImplicitConversion(argument.expression(),parameter.type) as AST.Expression;

            // 检查实参形参类型是否匹配
            if(!parameter.type.Equal(argExpAST.type)){
                Logger.Add(Log.Error, argument, Formater.singleton.TypeMismatch(parameter.type.name,argExpAST.type.name));
            }
            
            // 检查是否有将右值按引用传递的非法情况
            if(argExpAST.exprFlag == AST.ExprFlag.RValue &&  parameter.flag.Exists(Flags.Byref)){
                Logger.Add(Log.Error, argument, Formater.singleton.CannotPassRValueByVal());
            }

            call.args.Add(new AST.ArgumentPass{
                isRef = parameter.flag.Exists(Flags.Byref),
                parameter = parameter,
                expression = argExpAST
            });
        }

        // 处理可选参数
        var processed_option = 0;
        while(argIndex < context.argument().Count()){
            var argument = context.argument()[argIndex];
            if(argument.ID()==null){ 
                // 实参没有可选参数标签，函数也没有设置ParamArray，而且在可选参数之后遇到必选参数，非法
                if(call.function.paramArray == null && processed_option > 0)
                    Logger.Add(Log.Error, context, Formater.singleton.ArgAfterOptNotAllowed(argument.GetText()));
                else //剩下的实参都是传递给ParamArray的
                    break;
            }
            else{
                processed_option++;
                var optionaName = argument.ID().GetText();
                // 检查可选参数是否存在
                if(call.function.optionMap.ContainsKey(optionaName.ToLower())){
                    Parameter parameter = call.function.optionMap[optionaName.ToLower()];
                    var optExpAST = VisitExpressionWithImplicitConversion(argument.expression(),parameter.type) as AST.Expression;

                    // 诊断形参实参类型是否匹配
                    if(!parameter.type.Equal(optExpAST.type)){
                        Logger.Add(Log.Error, argument, Formater.singleton.TypeMismatch(parameter.type.name,optExpAST.type.name));
                    }
                    // 诊断是否有将右值按引用传递的非法情况
                    if(optExpAST.exprFlag == AST.ExprFlag.RValue &&  parameter.flag.Exists(Flags.Byref)){
                        Logger.Add(Log.Error, argument, Formater.singleton.CannotPassRValueByVal());
                    }

                    call.opts.Add(new AST.OptInit{
                        option = optionaName,
                        isRef = parameter.flag.Exists(Flags.Byref),
                        parameter = parameter,
                        expression = optExpAST
                    });
                }
                else{
                    Logger.Add(Log.Error, argument, Formater.singleton.OptNotFound(optionaName,call.function.name));
                }
            }
            argIndex++;
        }

        // 处理ParamArray
        if(argIndex != context.argument().Count() && call.function.paramArray!=null){
            //这里需要提示目标函数没有paramarray

            // 传递的参数(类型匹配的数组)直接作为paramArray
            var argAST = VisitExpression(context.argument()[argIndex].expression()) as AST.Expression;
            if(argIndex + 1 == context.argument().Count() && call.function.paramArray.type.Equal(argAST.type)){
                call.paramArrayPass = new AST.ParamArrayPass{
                    forward = argAST
                };
            }
            else{
                // 传递的参数作为paramArray的元素
                var paramArrayPassAST = new AST.ParamArrayPass{expressions = new List<AST.Expression>()};
                while(argIndex < context.argument().Count()){
                    var argument = context.argument()[argIndex];
                    if(argument.ID()!=null){
                        Logger.Add(Log.Error, argument, Formater.singleton.InvalidOptInit());
                    }
                    var elementExpAST = VisitExpressionWithImplicitConversion(argument.expression(),env.EBObject) as AST.Expression;
                    paramArrayPassAST.expressions.Add(elementExpAST);
                    argIndex++;
                }
                call.paramArrayPass = paramArrayPassAST;
            }
        }
    }

    public override AST.AST VisitMethodInvocation([NotNull] evoBasicParser.MethodInvocationContext context){
        var primary = (AST.Expression)Visit(context.primaryExpression());
        if(primary is AST.ErrorExpr)return primary;
        if(primary is AST.Call call){
            var funcRetType = (primary.exprSymbol as Function).retType;
            call.exprFlag = AST.ExprFlag.RValue;
            call.exprSymbol = new Variable(){type = funcRetType};
            call.type = funcRetType;
            call.location = new Source.SourceLocation(context);
            FillCallArgs(context.argumentList(), call);
            return call;
        }
        else{
            Logger.Add(Log.Error, context, Formater.singleton.TargetIsNotCallable(primary.exprSymbol.name));
            return new AST.ErrorExpr();
        }
    }

    public override AST.AST VisitArraySubscription([NotNull] evoBasicParser.ArraySubscriptionContext context){
        var primary = (AST.Expression)Visit(context.primaryExpression());
        if(primary is AST.ErrorExpr)return primary;
        var subscript = (AST.Expression)Visit(context.expression());

        if(!subscript.type.Equal(env.Integer)){
            Logger.Add(Log.Error, context.expression(), Formater.singleton.TypeMismatch(env.Integer.name,subscript.type.name));
        }

        if(primary.type is Array array){
            return new AST.Subscript(){
                reference = primary,
                subscript = subscript,
                type = array.type,
                exprSymbol = new Variable(){type = array.type},
                exprFlag = primary.exprFlag
            };
        }
        else{
            Logger.Add(Log.Error, context, Formater.singleton.TargetIsNotArray(primary.exprSymbol.name));
            return new AST.ErrorExpr();
        }
    }

    public override AST.AST VisitNewExpression([NotNull] evoBasicParser.NewExpressionContext context){
        var target = Utils.SearchTypePath(context.typePath(), scope.Peek(), env);
        if(context.subscript!=null){
            //创建数组
            var sizeExpr = Visit(context.subscript) as AST.Expression;
            if(!sizeExpr.type.Equals(env.Integer)){
                Logger.Add(Log.Error, Formater.singleton.TypeMismatch(env.Integer.name,sizeExpr.type.getFullName()));
            }

            var expType = new Array(env){
                type = target
            };

            return new AST.NewArray{
                elementType = target,
                size = sizeExpr,
                exprFlag = AST.ExprFlag.RValue,
                exprSymbol = new Variable{type = expType},
                type = expType
            };
        }
        else if(target is Class cls){
            var call = new AST.CtorCall(){
                exprSymbol = new Variable(){type = cls},
                type = cls,
                function = cls.ctor,
                location = new Source.SourceLocation(context)
            };
            FillCallArgs(context.argumentList(), call);
            return call;
        }
        else{
            if(!(target is ErrorType))
                Logger.Add(Log.Error, context, Formater.singleton.TargetIsNotClass(context.typePath().GetText()));
            return new AST.ErrorExpr();
        }
    }

    public override AST.AST VisitIsExpression([NotNull] evoBasicParser.IsExpressionContext context){
        var targetType =  Utils.SearchAnnotation(context.annotation(),scope.Peek(),env);
        // 如果is的比较目标对象是Primitive Type T，偷偷将其替换成TBox；
        // 降低TBox类的存在感，保持装箱后的Primitive Type和未装箱的Primitive Type使用体验一致。
        if(targetType is Symbols.Primitive){
            targetType = env.boxing.FindBox(targetType);
        }
        return new AST.Is{
            expression = Visit(context.primaryExpression()) as AST.Expression,
            target = targetType,
            exprFlag = AST.ExprFlag.RValue,
            exprSymbol = new Variable{type = env.Boolean},
            type = env.Boolean
        };
    }

    public override AST.AST VisitOptionalTestExpression([NotNull] evoBasicParser.OptionalTestExpressionContext context){
        var options = new List<Parameter>();
        foreach(var id in context.ID()){
            var target = this.currentFunction.scope.Find(id.GetText().ToLower());
            if(target is Parameter parameter && parameter.flag.Exists(Flags.Optional)){
                options.Add(parameter);
            }
            else{
                Logger.Add(Log.Error, Formater.singleton.IsNotOptionalParameter(id.GetText()));
            }
        }
        return new AST.OptionalTest{
            optionalParameters = options,
            type = env.Boolean,
            exprSymbol = new Variable{ type = env.Boolean},
            exprFlag = AST.ExprFlag.RValue,
        };
    }

    public AST.ArgumentAccess GetSelfReference(Class klass){
        if(klass==null)throw new ArgumentException();
        var hided_parameter = new Parameter{name = "#self",type = klass};
        hided_parameter.setIndex(0);
        return new AST.ArgumentAccess(){
            isRef = false,
            parameter = hided_parameter,
            type = klass,
            exprSymbol = hided_parameter
        };
    } 

    Stack<bool> staticEnviormentFlag = new Stack<bool>();

    public override AST.AST VisitPrimaryTerminal([NotNull] evoBasicParser.PrimaryTerminalContext context){
        if(context.terminalExpression().ambiguousName != null){
            var sym = scope.Peek().LookUp(context.terminalExpression().ambiguousName.Text);

            if(sym == null){
                Logger.Add(Log.Error, context, Formater.singleton.ObjectNotfound(context.terminalExpression().ambiguousName.Text));
                return new AST.ErrorExpr();
            }
            if(sym is Module || sym is Class || sym is Enumeration){ //返回包含当前符号的伪AST
                return new AST.PseudoReference(){
                    exprSymbol = sym
                };
            }
            else if(sym is Function function){ //返回函数调用AST
                if(sym.owner.attch is Module){
                    if(sym is ForeignEntry fe){
                        if(fe.lib==null && fe.alias==null){     //intrinsic
                            return new AST.IntrinsicCall(){
                                function = function,
                                exprSymbol = function,
                            };
                        }
                        else{   //foregin function
                            return new AST.ForeignCall(){
                                function = function,
                                exprSymbol = function,
                                lib = fe.lib,
                                alias = fe.alias
                            };
                        }
                    }
                    else if(sym is Method){ // static function
                        return new AST.SFtnCall(){
                            function = function,
                            exprSymbol = function
                        };
                    }
                }
                else if(sym.owner.attch is Class){
                    if(sym.flag.Exists(Flags.Static)){
                        //在任意环境调用静态对象
                        return new AST.SFtnCall(){
                            function = function,
                            exprSymbol = function
                        };
                    }
                    else if(!sym.flag.Exists(Flags.Static) && !staticEnviormentFlag.Peek()){
                        //在非静态环境里调用非静态对象
                        if(sym.flag.Exists(Flags.Virtual | Flags.Override)){
                            return new AST.VFtnCall(){
                                reference = GetSelfReference(currentClass),
                                function = function,
                                exprSymbol = function
                            };
                        }
                        else{
                            return new AST.FtnCall(){
                                reference = GetSelfReference(currentClass),
                                function = function,
                                exprSymbol = function
                            };
                        }
                    }
                    else if(!sym.flag.Exists(Flags.Static) && staticEnviormentFlag.Peek()){
                        //在静态环境里调用非静态对象,不合法
                        Logger.Add(Log.Error, context, Formater.singleton.CannotAccessNonStaticMember());
                        return new AST.ErrorExpr();
                    }
                }
            }
            else if(sym is Parameter parameter){ //返回参数调用AST
                return new AST.ArgumentAccess(){
                    isRef = parameter.flag.Exists(Flags.Byref),
                    parameter = parameter,
                    type = parameter.type,
                    exprFlag = AST.ExprFlag.LValue,
                    exprSymbol = parameter,
                    location = new Source.SourceLocation(context)
                };
            }
            else if(sym is Variable variable){ //返回变量调用AST
                if(sym.owner.attch is Module){
                    return new AST.SFldAccess(){
                        variable = variable,
                        type = variable.type,
                        exprFlag = AST.ExprFlag.LValue,
                        exprSymbol = variable,
                        location = new Source.SourceLocation(context)
                    };
                }
                else if(sym.owner.attch is Class){
                    if(sym.flag.Exists(Flags.Static)){
                        //在任意环境调用静态对象
                        return new AST.SFldAccess(){
                            variable = variable,
                            type = variable.type,
                            exprFlag = AST.ExprFlag.LValue,
                            exprSymbol = variable,
                            location = new Source.SourceLocation(context)
                        };
                    }
                    else if(!sym.flag.Exists(Flags.Static) && !staticEnviormentFlag.Peek()){
                        //在非静态环境里调用非静态对象
                        return new AST.FldAccess(){
                            reference = GetSelfReference(currentClass),
                            variable = variable,
                            type = variable.type,
                            exprFlag = AST.ExprFlag.LValue,
                            exprSymbol = variable,
                            location = new Source.SourceLocation(context)
                        };
                    }
                    else if(!sym.flag.Exists(Flags.Static) && staticEnviormentFlag.Peek()){
                        //在静态环境里调用非静态对象,不合法
                        Logger.Add(Log.Error, context, Formater.singleton.CannotAccessNonStaticMember());
                        return new AST.ErrorExpr();
                    }
                }
                else if(sym.owner.attch is Function){//本地变量
                    return new AST.LocalAccess(){
                        local = variable,
                        type = variable.type,
                        exprFlag = AST.ExprFlag.LValue,
                        exprSymbol = variable,
                        location = new Source.SourceLocation(context)
                    };
                }
            }
        }
        // self 引用
        else if(context.terminalExpression().Self() != null){ 
            return GetSelfReference(currentClass);
        }
        else if(context.terminalExpression().expression() != null){
            return Visit(context.terminalExpression().expression());
        }
        else if(context.terminalExpression().literal() != null){
            return Visit(context.terminalExpression().literal());
        }

        throw new ArgumentException();
    }

    public override AST.AST VisitArrayLiteral([NotNull] evoBasicParser.ArrayLiteralContext context){
        List<AST.Expression> expressions = new List<AST.Expression>();
        List<Symbols.Type> types = new List<Type>();
        foreach(var expr in context.expression()){
            var astExpr = Visit(expr) as AST.Expression;
            types.Add(astExpr.type);
            expressions.Add(astExpr);
        }
        
        bool listTypeValid = true;
        Type firstEleType = types.Count() == 0 ? new Any() : types.First();
        for(int i = 1; i < types.Count(); i++){
            if(!firstEleType.Equal(types[i])){
                listTypeValid = false;
                break;
            }
        }

        if(!listTypeValid){
            Logger.Add(Log.Error, context, Formater.singleton.ElementTypeDifferent(types));
            return new AST.ErrorExpr();
        }
        
        var retType = new Array(env){type = firstEleType};

        return new AST.ArrayLiteral{
            expressions = expressions,
            exprSymbol = new Variable{type = retType},
            exprFlag = AST.ExprFlag.RValue,
            type = retType
        };
    }
    public override AST.AST VisitIntegerLiteral([NotNull] evoBasicParser.IntegerLiteralContext context){
        return new AST.IntegerLiteral(){
            value = Int32.Parse(context.GetText()),
            type = env.Integer,
            exprFlag = AST.ExprFlag.RValue,
            exprSymbol = new Variable{ type = env.Integer},
            location = new Source.SourceLocation(context)
        };
    }
	public override AST.AST VisitDoubleLiteral([NotNull] evoBasicParser.DoubleLiteralContext context){
        return new AST.DoubleLiteral(){
            value = Double.Parse(context.GetText()),
            type = env.Double,
            exprFlag = AST.ExprFlag.RValue,
            exprSymbol = new Variable{ type = env.Double},
            location = new Source.SourceLocation(context)
        };
    }
	public override AST.AST VisitStringLiteral([NotNull] evoBasicParser.StringLiteralContext context){
        var ret = new AST.StringLiteral(){
            value = Utils.TextEscape(Utils.RemoveQuote(context.StringLiteral().GetText())),
            type = env.EBString,
            exprFlag = AST.ExprFlag.RValue,
            exprSymbol = new Variable{ type = env.EBString},
            location = new Source.SourceLocation(context)
        };
        return ret;
    }
	public override AST.AST VisitRuneLiteral([NotNull] evoBasicParser.RuneLiteralContext context){
        var text = context.CharLiteral().GetText();
        text = Utils.TextEscape(Utils.RemoveQuote(text));
        UInt32 val = BitConverter.ToUInt32(Encoding.UTF32.GetBytes(text));
        return new AST.RuneLiteral(){
            value = val,
            type = env.Rune,
            exprFlag = AST.ExprFlag.RValue,
            exprSymbol = new Variable{ type = env.Rune },
            location = new Source.SourceLocation(context)
        };
    }
	public override AST.AST VisitBooleanLiteral([NotNull] evoBasicParser.BooleanLiteralContext context){
        return new AST.BooleanLiteral(){
            value = context.True!=null,
            type = env.Boolean,
            exprFlag = AST.ExprFlag.RValue,
            exprSymbol = new Variable{ type = env.Boolean },
            location = new Source.SourceLocation(context)
        };
    }
    public override AST.AST VisitNothingLiteral([NotNull] evoBasicParser.NothingLiteralContext context){
        return new AST.NothingLiteral{
            type = env.EBNothing,
            exprFlag = AST.ExprFlag.RValue,
            exprSymbol = env.EBNothing,
            location = new Source.SourceLocation(context)
        };
    }

}
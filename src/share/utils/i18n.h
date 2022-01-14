//
// Created by yorkin on 12/17/21.
//

#ifndef EVOBASIC_I18N_H
#define EVOBASIC_I18N_H
#include <string>
#include <functional>
#include <memory>
#include "format.h"

namespace evoBasic::i18n{

#define Fmt(Name,Stream) virtual std::string fmt##Name(Var##Name){return Format() << Stream;}
#define Msg(Name,Stream) virtual std::string msg##Name(){return Format() << Stream;}


#define VarObjectNotFound                   std::string target
#define VarCtorRedefine                     std::string target
#define VarOptNotFoundInFtn                 std::string init_name,std::string ftn_name
#define VarArgCannotMatchParam              std::string param_type,std::string arg_type
#define VarImplicitCvtFromAToB              std::string a,std::string b
#define VarLetStmtVariableInitialNotMatch   std::string init_type,std::string anno_type
#define VarSelectCaseTypeNotMatch           std::string case_type,std::string condition_type
#define VarIfConditionExpectedBooleanButA   std::string a
#define VarForStmtBeginExpNotMatch          std::string begin_type,std::string iterator_type
#define VarForStmtEndExpNotMatch            std::string end_type,std::string iterator_type
#define VarForStmtStepExpNotMatch           std::string step_type,std::string iterator_type
#define VarCannotImplicitCvtAToB            std::string a,std::string b
#define VarNotCallableTarget                std::string target_type
#define VarCtorUndefined                    std::string cls
#define VarIsPrivate                        std::string target
#define VarIsProtected                      std::string target
#define VarBinaryOpInvalid                  std::string lhs,std::string rhs
#define VarFtnCallTooManyArg                int params_count,int args_count
#define VarFtnCallTooFewArg                 int params_count,int args_count
#define VarStaticAccessInvalid              std::string lhs_state,std::string rhs_state



    class Language{
    public:
        Fmt(ObjectNotFound,"object '" << target << "' not found")
        Msg(InheritRecursive,"inheritance is recursive")
        Msg(FtnInModuleCannotMarkStatic,"Function in Module cannot be marked by 'Static'")
        Msg(FtnInModuleCannotMarkVirtual,"Function in Module cannot be marked by 'Virtual'")
        Msg(FtnInModuleCannotMarkOverride,"Function in Module cannot be marked by 'Override'")
        Msg(SFtnCannotMarkVirtual,"Static Method cannot be marked by 'Virtual'")
        Msg(SFtnCannotMarkOverride,"Static Method cannot be marked by 'Override'")
        Msg(CtorOnlyAllowedInCls,"constructor is only allowed in class")
        Fmt(CtorRedefine,"constructor for '" << target << "' has defined")
        Msg(NotOptCannotSetDefaultVal,"cannot set default value for non-optional parameter")
        Msg(ExpectedParamNameInOptInitialization,"parameter initialization expected a parameter name here")
        Fmt(OptNotFoundInFtn,"option '" << init_name << "' in function '" << ftn_name << "' not found")
        Fmt(ArgCannotMatchParam,"parameter type is '" << param_type << "' but the argument type is '" << arg_type << "'")
        Fmt(ImplicitCvtFromAToB,"implicit conversion from '" << a << "' to '" << b << "'")
        Msg(ExpectedByValButByRef,"require ByVal but declared ByRef")
        Msg(CannotPassTmpValByRefImplicit,"can not pass a temporary value ByRef."
                                            << "Change parameter to Byval or explicit declare 'ByVal' here.\n"
                                            << "Syntax: exampleFunction(Byval <Expression>) ")
        Fmt(LetStmtVariableInitialNotMatch,"initialize expression type '" << init_type
                                << "' is not equivalent to variable type '" << anno_type << "'")
        Msg(LetStmtNeedInitialValueOrTypeMark,"need initial expression or type mark");
        Fmt(SelectCaseTypeNotMatch,"case condition type '" << case_type
                                                           << "' is not equivalent to select expression type '"
                                                           << condition_type << "'")
        Msg(MsgLoopingConditionIsNotBoolean,"expression type of loop condition must be Boolean")
        Fmt(IfConditionExpectedBooleanButA ,"expression type of if condition must be boolean but here is '" << a)
        Msg(ExpExpectedBoolean,"expression type must be Boolean")
        Msg(InvalidExp,"invalid expression")

        Msg(ForStmtIteratorNotFound,"iterator not found")
        Msg(ForStmtIteratorMustBeLValue,"iterator must be a lvalue")
        Fmt(ForStmtBeginExpNotMatch,"Begin expression type '"
                                    << begin_type << "' is not equivalent to iterator type '"
                                    << iterator_type << "'")
        Fmt(ForStmtEndExpNotMatch,"End expression type '"
                                    << end_type << "' is not equivalent to iterator type '"
                                    << iterator_type << "'")
        Fmt(ForStmtStepExpNotMatch,"Step expression type '"
                                   << step_type << "' is not equivalent to iterator type '"
                                   << iterator_type << "'")
        Fmt(CannotImplicitCvtAToB, "cannot implicit convert '" << a << "' to '" << b << "'")
        Msg(AssignmentRequireLvalue,"lvalue required as left operand of assignment")
        Fmt(NotCallableTarget,"'" << target_type << "' is not a callable target")
        Msg(NotClass,"type is not a Class")
        Msg(CannotInitAbstractCls,"cannot instantiate abstract class")
        Fmt(CtorUndefined,"constructor for '" << cls << "' is undefined")
        Fmt(IsPrivate,"'" << target << "' is private")
        Fmt(IsProtected,"'" << target << "' is protected")
        Fmt(BinaryOpInvalid,"invalid operands to binary expression.lhs type is '" << lhs << "' and rhs type is'" << rhs)
        Msg(RegularAppearAfterOpt,"regular argument is not allowed after optional argument")
        Fmt(FtnCallTooManyArg,"too many arguments to function call, expected " << params_count << ", have " << args_count)
        Fmt(FtnCallTooFewArg,"too few arguments to function call, expected " << params_count << ", have " << args_count)
        Fmt(StaticAccessInvalid,"cannot find object. lhs is " << lhs_state << " but rhs is " << rhs_state)
        Msg(ParamInitialExpectedIDInLhs,"lhs of parameter initialization must be an id")
        Msg(ParamInitialNotAllowed,"a parameter initialization is allowed only in function or sub")
        Msg(NameConflicit,"Naming conflict in current scope")
        Msg(OptionalNeedDefaultValue,"optional parameter must have a default value")
    };



    class Chs : public Language{
    public:
    };

    extern std::shared_ptr<Language> lang;

#undef Fmt
#undef Msg

#undef VarObjectNotFound
#undef VarCtorRedefine
#undef VarOptNotFoundInFtn
#undef VarArgCannotMatchParam
#undef VarImplicitCvtFromAToB
#undef VarLetStmtVariableInitialNotMatch
#undef VarSelectCaseTypeNotMatch
#undef VarForStmtIfConditionExpectedBooleanButA
#undef VarIfConditionExpectedBooleanButA
#undef VarForStmtBeginExpNotMatch
#undef VarForStmtEndExpNotMatch
#undef VarForStmtStepExpNotMatch
#undef VarCannotImplicitCvtAToB
#undef VarNotCallableTarget
#undef VarCtorUndefined
#undef VarIsPrivate
#undef VarIsProtected
#undef VarBinaryOpInvalid
#undef VarFtnCallTooManyArg
#undef VarFtnCallTooFewArg
#undef VarStaticAccessInvalid


}

#endif //EVOBASIC_I18N_H

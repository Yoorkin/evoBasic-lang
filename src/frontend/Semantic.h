//
// Created by yorkin on 7/20/21.
//

#ifndef EVOBASIC_SEMANTIC_H
#define EVOBASIC_SEMANTIC_H
#include"Type.h"
#include"AST.h"
#include"../utils/Logger.h"
#include<tuple>
#include<memory>
namespace evoBasic{

    class SymbolTable{

        void visitParameterList(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Type::Function> function,
                                        std::shared_ptr<Node> parameter_list);

        static std::shared_ptr<Type::primitive::VariantClass> variant_class;
        static std::shared_ptr<Type::primitive::Primitive<bool>> boolean_prototype;
        static std::shared_ptr<Type::primitive::Primitive<int>> integer_prototype;
        static std::shared_ptr<Type::primitive::Primitive<double>> double_prototype;

    public:
        explicit SymbolTable(const vector<AST>& ast_list);

        /*
         * collect information of Function,Declare,Type Member,Enum Member
         */
        void collectDetail(AST ast);

        /*
         *  collect information of Module,Class,Global,Type,Enum
         */
        void collectSymbol(const shared_ptr<Node>& ast);

        static std::tuple<std::shared_ptr<Type::Symbol>,Position,std::string>
        visitPath(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> path);

        static std::tuple<std::shared_ptr<Type::Symbol>,Position,std::string>
        visitAnnotation(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);

        shared_ptr<Type::Module> global;
        shared_ptr<Type::UserFunction> entrance;

        static std::shared_ptr<Type::primitive::VariantClass> getVariantClass();
        static std::shared_ptr<Type::primitive::Primitive<bool>> getBooleanPrototype();
        static std::shared_ptr<Type::primitive::Primitive<int>> getIntegerPrototype();
        static std::shared_ptr<Type::primitive::Primitive<double>> getDoublePrototype();
    };

    class ExprException : public exception{
        Position pos;
        std::string str;
    public:
        ExprException(Position pos, std::string msg): pos(pos), str(msg){}
        const char * what() const noexcept override{
            return str.c_str();
        }
        Position& getPos(){return pos;}
        std::string& getMsg(){return str;}
    };

    class UnitException : public ExprException{
    public:
        UnitException(Position pos, std::string msg): ExprException(std::move(pos),std::move(msg)){}
    };

    class FactorException : public ExprException{
    public:
        FactorException(Position pos, std::string msg): ExprException(std::move(pos),std::move(msg)){}
    };

    enum class ExprKind{lvalue,rvalue};
    using ExprResult = std::pair<std::shared_ptr<Type::Prototype>,ExprKind>;

    class TypeAnalyzer{

        using PromotionRuleFunction = std::function<std::shared_ptr<Type::Prototype>(std::shared_ptr<Node>)>;
        using PrototypePtr = Type::Prototype*;
        using BinaryOpSignature = std::pair<PrototypePtr,PrototypePtr>;

        static std::map<BinaryOpSignature,std::shared_ptr<Type::Prototype>> binary_op_result_type;
        static std::map<BinaryOpSignature,PromotionRuleFunction> typePromotionRule;
        static std::set<BinaryOpSignature> isCastRuleExist;
    public:
        static void check(std::vector<AST>& ast_list,SymbolTable& table);
        static void visitStructure(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> root);
        static void visitStatement(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Type::UserFunction> function,std::shared_ptr<Node> root);
        static void visitLetStmt(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> root);



        static ExprResult visitExpression(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        static ExprResult visitLogic(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        static ExprResult visitTermAddCmp(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        static ExprResult visitFactor(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        static std::shared_ptr<Type::Symbol> visitUnit(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        static void visitParameterList(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Type::Function> function,std::shared_ptr<Node> node);

    };

    class TemporaryScope : public Type::Domain {
        std::shared_ptr<Type::UserFunction> current_function;
        std::map<std::string,Type::Member> local_variables;
    public:
        explicit TemporaryScope(std::weak_ptr<Type::Domain> parent,std::shared_ptr<Type::UserFunction> current);
        Type::Member find(const string& name)override;
        void add(Type::Member member)override;
        void add(std::string name,std::shared_ptr<Type::Prototype> prototype);

        std::shared_ptr<Type::Value> create()override{
            throw "error";
        };

        bool equal(std::shared_ptr<Prototype> ptr)override{
            throw "error";
        };

        void addImport(std::shared_ptr<Symbol> child)override{
            throw "unimpl";//TODO
        };
    };

/*
 *                                        | ->  VariableTypeDependGraph -> | TypeInference                |
 *                                        |                                | ImplicitCastOrOverloadInsert |
 * AST->SymbolCollector->MemberCollector -|                                                               | -> TypeCheckCover
 *                                        | -> InheritMap -> InheritCheck                                 |
 * 
 */

    /*
     * 收集Type,Enum,Module,Class符号
     */
    class SymbolCollector{

    };

    /*
     * 收集Enum，Type的成员、Module的成员函数签名、Class的成员函数签名
     */
    class MemberCollector{

    };

    /*
     * 输入AST构造类继承关系图
     */
    class InheritMap{

    };

    /*
    * 检查InheritMap是否为一颗树、函数覆写时签名是否一致
    */
    class InheritCheck{

    };

    /*
     * 收集局部与非局部变量符号并建立类型推导拓扑
     */
    class VariableTypeDependGraph{

    };

    /*
     * 根据拓扑进行类型推导，
     * 同时调用ImplicitCastOrOverloadInsert对Let语句的AST插入隐式转换或运算符重载调用节点
     */
    class TypeInference{

    };

    /*
     * 输入表达式AST以及所在domain，插入隐式转换或运算符重载调用节点
     */
    class ImplicitCastOrOverloadInsert{

    };

    /*
     * 完成余下所有的类型检查工作
     * 即Let语句之外的Expression分析
     */
    class TypeCheckCover{

    };

    /*
     * 检查对象成员调用是否违反权限
     */
    class AccessCheck{

    };




//    class Analyzer{
//    public:
//        static void check(SymbolTable &table,vector<AST> trees);
//        static void typeInference(SymbolTable &table,AST ast);
//        static void overrideCheck(SymbolTable &table,AST ast);
//        static void typeCheck(SymbolTable &table,AST ast);
//
//        static void typeInferenceTraverse(SymbolTable &table,AST &ast,shared_ptr<Node> node);
//        void typeCheck_dfs(shared_ptr<IRObject> domain, shared_ptr<Node> node);
//    };


}


#endif //EVOBASIC_SEMANTIC_H

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
        std::shared_ptr<Logger> logger;
        std::tuple<std::shared_ptr<Type::DeclarationSymbol>,Position,std::string>
            visitPath(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> path);

        std::tuple<std::shared_ptr<Type::DeclarationSymbol>,Position,std::string>
            visitAnnotation(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> path);

        void visitParameterList(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Type::Function> function,
                                        std::shared_ptr<Node> parameter_list);

        std::shared_ptr<Type::primitive::VariantClass> variant_class;
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


        shared_ptr<Type::Module> global;
        shared_ptr<Type::UserFunction> entrance;
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

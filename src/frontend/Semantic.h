//
// Created by yorkin on 7/20/21.
//

#ifndef EVOBASIC_SEMANTIC_H
#define EVOBASIC_SEMANTIC_H
#include"Type.h"
#include"AST.h"
#include"../utils/Logger.h"
#include"../utils/dataDef.h"
#include<tuple>
#include<memory>
namespace evoBasic{

    using SymbolPtr = Type::Symbol*;
    using PromotionRuleFunction = std::function<std::shared_ptr<Type::Prototype>(std::shared_ptr<Node>)>;
    using BinaryOpSignature = std::pair<SymbolPtr,SymbolPtr>;
    class BuiltIn;
    class ConversionRules;
    class Semantics;

    class BuiltIn{
        friend Semantics;
        explicit BuiltIn();

        using booleanTypePtr = std::shared_ptr<Type::primitive::Primitive<dataDef::boolean>>;
        using byteTypePtr = std::shared_ptr<Type::primitive::Primitive<dataDef::i8>>;
        using shortTypePtr = std::shared_ptr<Type::primitive::Primitive<dataDef::i16>>;
        using integerTypePtr = std::shared_ptr<Type::primitive::Primitive<dataDef::i32>>;
        using longTypePtr = std::shared_ptr<Type::primitive::Primitive<dataDef::i64>>;
        using singleTypePtr = std::shared_ptr<Type::primitive::Primitive<dataDef::f32>>;
        using doubleTypePtr = std::shared_ptr<Type::primitive::Primitive<dataDef::f64>>;

        std::shared_ptr<Type::primitive::VariantClass> variant_class;
        booleanTypePtr boolean_prototype;
        byteTypePtr byte_prototype;
        shortTypePtr short_prototype;
        integerTypePtr integer_prototype;
        longTypePtr long_prototype;

        singleTypePtr single_prototype;
        doubleTypePtr double_prototype;
        std::shared_ptr<Type::Error> error_symbol;
    public:
        std::shared_ptr<Type::primitive::VariantClass> getVariantClass();
        booleanTypePtr getBooleanPrototype();
        byteTypePtr getBytePrototype();
        shortTypePtr getShortPrototype();
        integerTypePtr getIntegerPrototype();
        longTypePtr getLongPrototype();
        singleTypePtr getSinglePrototype();
        doubleTypePtr getDoublePrototype();
        std::shared_ptr<Type::Error> getErrorPrototype();
    };

    class ConversionRules{
        friend Semantics;
        explicit ConversionRules(BuiltIn* builtIn);
        std::map<BinaryOpSignature,PromotionRuleFunction> typePromotionRule;
        std::set<BinaryOpSignature> castRuleFlag;
    public:
        using promotion_iterator = std::map<BinaryOpSignature,PromotionRuleFunction>::iterator;
        promotion_iterator promotion(SymbolPtr lhs, SymbolPtr rhs);
        bool isCastRuleExist(SymbolPtr lhs,SymbolPtr rhs);
        bool isEmpty(promotion_iterator iterator);
    };

    class Semantics{
        static std::shared_ptr<Semantics> instance;
        BuiltIn *builtIn;
        ConversionRules *conversionRules;
        explicit Semantics();
    public:
        ~Semantics();
        static std::shared_ptr<Semantics> Instance();
        BuiltIn& getBuiltIn();
        ConversionRules& getConversionRules();
    };

    template<typename T>
    class Dependencies{
        struct Data{
            int in_degree = 0;
            std::shared_ptr<T> obj;
            std::list<Data*> be_depend_list;
            bool visited=false;
        };

        std::map<T*,Data*> vex;
        std::list<std::shared_ptr<T>> inference_order;
        std::list<std::list<std::shared_ptr<T>>> circles;

    public:
        using sharedPtr = std::shared_ptr<T>;
        using sharedPtrList = std::list<sharedPtr>;
        explicit Dependencies(std::list<std::pair<sharedPtr,sharedPtrList>> dependent_rules){
            for(auto& p:dependent_rules){
                addDependencies(p.first,p.second);
            }
        }

        explicit Dependencies()=default;

        void addDependencies(std::shared_ptr<T> obj, std::list<std::shared_ptr<T>> depend){
            auto tmp = vex.find(obj.get());
            Data *data;

            if(tmp!=vex.end())data = tmp->second;
            else data = new Data;

            data->obj = obj;
            for(auto& ptr:depend){
                if(!ptr->isNeedInference())continue;
                auto target = vex.find(ptr.get());
                if(target==vex.end()){
                    Data *d = new Data;
                    d->obj = ptr;
                    d->be_depend_list.push_back(data);
                    data->in_degree++;
                    vex.emplace(ptr.get(),d);
                }
                else{
                    target->second->be_depend_list.push_back(data);
                    data->in_degree++;
                }
            }
            vex.emplace(obj.get(), data);
        }

        bool solve(){
            inference_order.clear();
            //拓扑排序得出可行的类型推断顺序
            stack<Data*> topo_stack;
            for(auto& v:vex){
                if(v.second->in_degree == 0)topo_stack.push(v.second);
            }

            while(!topo_stack.empty()){
                auto topo = topo_stack.top();
                topo->visited = true;
                topo_stack.pop();
                inference_order.push_back(topo->obj);
                for(auto dep:topo->be_depend_list){
                    dep->in_degree--;
                    if(dep->in_degree == 0)
                        topo_stack.push(dep);
                }
            }

            if(inference_order.size() == vex.size()){
                return true;
            }
            else {
                //存在无法推断的obj，算出存在的依赖环路
                for(auto& [k,v]:vex){
                    if(v->visited)continue;
                    list<std::shared_ptr<Type::Field>> c;
                    auto iter = v;
                    do{
                        iter->visited = true;
                        c.push_back(iter->obj);
                        auto tmp = iter->be_depend_list.front();
                        iter->be_depend_list.pop_front();
                        iter = tmp;
                    }while(iter!=v);
                    c.push_back(v->obj);
                    circles.push_back(std::move(c));
                }
                return false;
            }
        }

        const std::list<std::shared_ptr<T>>& getTopologicalOrder(){
            return inference_order;
        }

        const std::list<std::list<std::shared_ptr<T>>>& getCircles(){
            return circles;
        }

        ~Dependencies(){
            for(auto& v:vex){
                delete v.second;
            }
        }
    };


    class SymbolTable{

        void visitParameterList(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Type::Function> function,
                                        std::shared_ptr<Node> parameter_list);

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

    };

//    class DetailCollector{
//    public:
//        //以后再改吧
//    };
    


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

    class SkipExprException : public exception{
    public:
        const char * what() const noexcept override{
            return "skip exprssion analyze";
        }
    };

    class UnitException : public ExprException{
    public:
        UnitException(Position pos, std::string msg): ExprException(std::move(pos),std::move(msg)){}
    };

    class FactorException : public ExprException{
    public:
        FactorException(Position pos, std::string msg): ExprException(std::move(pos),std::move(msg)){}
    };

    enum class ExprKind{lvalue,rvalue,error};
    using ExprResult = std::pair<std::shared_ptr<Type::Symbol>,ExprKind>;

    class TypeAnalyzer{
        const std::shared_ptr<SymbolTable> table;
    public:
        explicit TypeAnalyzer(const std::shared_ptr<SymbolTable> table);

        void check(std::vector<AST>& ast_list);
        void visitStructure(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> root);
        void visitStatement(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Type::UserFunction> function,std::shared_ptr<Node> root);
        void visitLoadedLetStmt(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node);
        void visitLocalLetStmt(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node);

        ExprResult visitExpression(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        ExprResult visitLogic(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        ExprResult visitTermAddCmp(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        ExprResult visitFactor(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        std::shared_ptr<Type::Symbol> visitUnit(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node);
        void visitParameterList(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Type::Function> function,std::shared_ptr<Node> node);

        ExprResult visitExprWithoutCatch(shared_ptr<Type::Domain> domain, shared_ptr<Node> node);
    };

    class TemporaryScope : public Type::Domain {
        std::shared_ptr<Type::UserFunction> current_function;
        std::map<std::string,std::shared_ptr<Type::Symbol>> local_variables;
    public:
        explicit TemporaryScope(std::weak_ptr<Type::Domain> parent,std::shared_ptr<Type::UserFunction> current);
        std::shared_ptr<Type::Symbol> find(const string& name)override;
        void add(std::shared_ptr<Type::Symbol> symbol)override;

        std::shared_ptr<Type::Value> create()override{
            throw "error";
        };

        bool equal(std::shared_ptr<Prototype> ptr)override{
            throw "error";
        };

        void addImport(std::shared_ptr<Symbol> child)override{
            throw "unimpl";//TODO
        };

        std::string debug(int indent)override{}
    };


}


#endif //EVOBASIC_SEMANTIC_H

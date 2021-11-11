//
// Created by yorkin on 11/11/21.
//

#ifndef EVOBASIC2_DEPENDENCIES_H
#define EVOBASIC2_DEPENDENCIES_H
#include <stack>
#include <list>
namespace evoBasic{
    template<typename T>
    class Dependencies{
        struct Data{
            int in_degree = 0;
            T obj;
            std::list<Data*> be_depend_list;
            bool visited=false;
        };

        std::map<T*,Data*> vex;
        std::list<T> inference_order;
        std::list<std::list<T>> circles;

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
            std::stack<Data*> topo_stack;
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
                    std::list<T> c;
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
}


#endif //EVOBASIC2_DEPENDENCIES_H

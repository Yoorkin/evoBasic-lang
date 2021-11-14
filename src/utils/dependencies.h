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
            std::set<T> be_depend_set;
            bool visited = false;
        };

        std::map<T,Data*> vex;
        std::list<T> inference_order;
        std::list<std::list<T>> circles;

        Data *getData(T obj){
            auto target = vex.find(obj);
            if(target == vex.end()){
                auto data = new Data;
                data->obj = obj;
                vex.insert(std::make_pair(obj,data));
                return data;
            }
            else{
                return target->second;
            }
        }
    public:
        explicit Dependencies()=default;

        void addIsolate(T obj){
            getData(obj);
        }

        void addDependent(T obj,T depend){
             auto depend_data = getData(depend);
             if(!depend_data->be_depend_set.contains(obj)){
                 auto obj_data = getData(obj);
                 depend_data->be_depend_list.push_back(obj_data);
                 obj_data->in_degree++;
             }
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

        const std::list<T>& getTopologicalOrder(){
            return inference_order;
        }

        const std::list<std::list<T>>& getCircles(){
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

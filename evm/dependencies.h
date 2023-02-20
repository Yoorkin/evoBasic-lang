//
// Created by yorkin on 11/11/21.
//

#ifndef EVOBASIC2_DEPENDENCIES_H
#define EVOBASIC2_DEPENDENCIES_H
#include <stack>
#include <map>
#include <set>
#include <list>
#include <queue>

template<class T>
class TopologicalSort{
    struct Vex{
        T obj;
        bool visited = false;
        int in_degree = 0;
        std::set<Vex*> dst,src;
        Vex(T t) : obj(t){}
    };

    std::map<T,Vex*> vexs;
    std::list<T> order;

public:

    Vex *addVex(T t){
        auto target = vexs.find(t);
        if(target == vexs.end() ){
            auto v = new Vex(t);
            vexs.insert({t,v});
            return v;
        }
        return target->second;
    }

    void addEdge(T src, T dst){
        auto a = addVex(src);
        auto b = addVex(dst);
        a->dst.insert(b);

        b->in_degree++;
        b->src.insert(a);
    }


    const std::list<T> &getOrder(){
        return order;
    }


    bool solve(){
        std::queue<Vex*> q;
        for(auto [_,v] : vexs){
            if(v->in_degree==0)q.push(v);
        }

        while(!q.empty()){
            auto v = q.front();
            q.pop();
            order.push_back(v->obj);
            for(auto dst : v->dst){
                if(dst->visited)continue;
                dst->in_degree--;
                if(dst->in_degree==0){
                    q.push(dst);
                    dst->visited = true;
                }
            }
        }

        return order.size()==vexs.size();
    }

    ~TopologicalSort(){
        for(auto [_,v] : vexs) delete v;
    }
};


#endif //EVOBASIC2_DEPENDENCIES_H
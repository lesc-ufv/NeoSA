#ifndef __EVALUATE_H
#define __EVALUATE_H

#include <Graph.h>

int dfs(Graph g, int node, bool *visited, map<pair<int,int>,int> edges_cost) {

    if (visited[node]) return 0;
    visited[node] = true;

    int maior = 0, cost;
    pair<int, int> key;
    vector<int> son = g.get_predecessors(node);
    for (int i = 0; i < son.size(); ++i) {
        key = make_pair(son[i], node);
        if (edges_cost.count(key)) {
            cost = dfs(g, son[i], visited, edges_cost) + edges_cost[key];
        } else {
            key = make_pair(node, son[i]);
            cost = dfs(g, son[i], visited, edges_cost) + edges_cost[key];
        }
        if (cost > maior) maior = cost;
    }
    return maior;
}

int path_critical_evaluate(Graph g, const int NODE_SIZE, 
    map<pair<int,int>,int> &edges_cost){
    
    queue<pair<int,int>> q;
    vector<int> son, outputs;
    int dad, child, sum = 0, big_sum, new_cost, cost, big_cost, index;
    pair<int, int> key;

    outputs = g.get_outputs();
    
    for (int i = 0; i < outputs.size(); ++i) 
        q.push(make_pair(outputs[i],0));

    big_sum = 0;
    while(!q.empty()) {
        dad = q.front().first;
        cost = q.front().second;
        q.pop();

        son = g.get_predecessors(dad);
        for (int i = 0, n = son.size(); i < n; ++i) {
            child = son[i];
            if (dad == child) continue;
            
            key = make_pair(child, dad);
            if (edges_cost.count(key)) {
                new_cost = cost + edges_cost[key];
            } else {
                key = make_pair(dad, child);
                new_cost = cost + edges_cost[key];
            }
            q.push(make_pair(child,new_cost));
            if (new_cost > big_sum) big_sum = new_cost;
        }
    }
    return big_sum;
}

int path_critical_evaluate_perfect(Graph g, const int NODE_SIZE) {
    queue<pair<int,int>> q;
    vector<int> son, outputs;
    int dad, child, big_sum, new_cost, cost;
    pair<int, int> key;

    outputs = g.get_outputs();
    
    for (int i = 0; i < outputs.size(); ++i) 
        q.push(make_pair(outputs[i],0));
    
    //memset(visited, 0, sizeof(int)*NODE_SIZE);
    
    big_sum = 0;
    while(!q.empty()) {
        dad = q.front().first;
        cost = q.front().second;
        q.pop();
        //visited[dad] = 1;

        son = g.get_predecessors(dad);
        for (int i = 0, n = son.size(); i < n; ++i) {
            child = son[i];
            if (dad == child) continue;
            new_cost = cost + 1;
            q.push(make_pair(child,new_cost));
            if (new_cost > big_sum) big_sum = new_cost;
        }
    }
    return big_sum;
}

#endif
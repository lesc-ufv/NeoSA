#ifndef __PLACEMENT__H
#define __PLACEMENT__H

#include <Graph.h>
#include <list_adjacency.h>
#include <get_cost.h>
#include <placed.h>
#include <try_adjacency.h>
#include <get_tips.h>

void get_pos_neighbor(int pos_node, int dist_max, int *grid_freedom, 
    const int GRID_SIZE, const int TOTAL_GRID_SIZE, vector<pair<int,int>> &aux) {
    
    aux.resize(0);

    int pos_i, pos_j, new_pos_i, new_pos_j, pos_global, degree;
    pos_i = pos_node / GRID_SIZE;
    pos_j = pos_node % GRID_SIZE;
    int diff;
    bool can;

    #if __ARCH == 1
        dist_max *= 2;
    #endif
    if (dist_max == 0) dist_max = 1;

    for (int i = -dist_max; i <= dist_max; ++i){
        for (int j = -dist_max; j <= dist_max; ++j) {
            can = false;
            new_pos_i = i + pos_i;
            new_pos_j = j + pos_j;
            
            #if __ARCH == 0
                diff = abs(i) + abs(j) - 1;
            #elif __ARCH == 1
                diff = (abs(i)/2 + abs(i)%2 + abs(j)/2 + abs(j)%2 - 1);
            #endif
            
            can = (diff >= 0 && diff < dist_max);

            pos_global = new_pos_i*GRID_SIZE + new_pos_j;
            degree = grid_freedom[pos_global];

            if (can && new_pos_i >= 0 && new_pos_j >= 0 &&
                new_pos_i < GRID_SIZE && new_pos_j < GRID_SIZE && degree > 0){
                aux.push_back({degree, pos_global});
            }
        }
    }
}

void intersection(vector<pair<int,int>> &pos_inter, vector<pair<int,int>> &pos_aux) {
    vector<pair<int,int>> pos_aux_inter;

    sort(pos_inter.begin(), pos_inter.end());
    sort(pos_aux.begin(), pos_aux.end());

    set_intersection(pos_inter.begin(), pos_inter.end(), pos_aux.begin(), pos_aux.end(), back_inserter(pos_aux_inter));

    // update pos_inter
    pos_inter = pos_aux_inter;
}

void subtraction(vector<pair<int,int>> &pos, vector<pair<int,int>> &pos_aux) {
    vector<pair<int,int>> pos_aux_sub;

    sort(pos.begin(), pos.end());
    sort(pos_aux.begin(), pos_aux.end());

    set_difference(pos.begin(), pos.end(), pos_aux.begin(), pos_aux.end(), back_inserter(pos_aux_sub));

    // update position
    pos = pos_aux_sub;
}

int best_place_degree(vector<pair<int,int>> &pos_aux, int pos_size, 
    int *grid_place, int pos_degree, int type_node_pos, int *type_node) {

    // pos_aux = pair<degree, pos>
    // degree fanin e fanout do no em questao
    int best_degree = pos_aux[0].first;
    int best_pos = pos_aux[0].second;
    int pos, degree;

#if __NEIGHBOURHOOD == 1 // greedy
    for (int j = 0; j < pos_size; ++j) {
        pos = pos_aux[j].second;
        degree = pos_aux[j].first;
        // condicao 1
        if (type_node_pos == grid_place[pos] && degree >= pos_degree) return pos;
    }
#elif __NEIGHBOURHOOD == 2 //search 
    for (int j = 0; j < pos_size; ++j) {
        pos = pos_aux[j].second;
        degree = pos_aux[j].first;
        // condicao 2
        //pos_degree = |PE|, degree = |b|
        if(type_node_pos == grid_place[pos] && best_degree >= degree && pos_degree >= degree) {//best_degree >= degree porque senão nunca considera o primeiro
            best_degree = degree;
            best_pos = pos;
        } 
    }
    return best_pos;
#elif __NEIGHBOURHOOD == 3 //search - limiar 1
    for (int j = 1; j < pos_size; ++j) {
        pos = pos_aux[j].second;
        degree = pos_aux[j].first;
        // condicao 3
        if(type_node_pos == grid_place[pos] && best_degree >= degree && pos_degree > degree) {
            best_degree = degree;
            best_pos = pos;
        } 
    }
    return best_pos;
#elif __NEIGHBOURHOOD >= 4
     for (int j = 1; j < pos_size; ++j) {
        pos = pos_aux[j].second;
        degree = pos_aux[j].first;

        if (pos_degree > 3) { // get the most degree 
            if (pos > best_degree && type_node_pos == grid_place[pos]) {
                best_degree = degree;
                best_pos = pos;
            }
        } else { 
            // get the minimun degree acceptable
            if (best_degree > degree && pos_degree >= degree && type_node_pos == grid_place[pos]) {
                best_degree = degree;
                best_pos = pos;
            }
        }
    }
#endif
    return best_pos;
}

void placement(const int NODE_SIZE, const int GRID_SIZE, const int EDGE_SIZE, 
        const int start, const int times, vector<tuple3> &vector_edges, int *node_degree,
        int *grid_freedom_original, int *pos_i, int* pos_j, int *type_node, int *type_matrix, 
        int *pos_io_i, int *pos_io_j, const int IN_OUT_SIZE, 
        vector<map<pair<int,int>,int>> &edges_cost, int *vector_cost, 
        int *list_borders, vector<map<pair<int, int>,vector<tuple3>>> &dic_CYCLE){
    
    int cost = -1, dist_border;
    const int TOTAL_GRID_SIZE = GRID_SIZE * GRID_SIZE;
    int pos_tips_size = 0, pos_inter_size = 0, pos_aux_size = 0, pos_forward_size = 0;
    pair<int, int> key, next_key;
    bool found;
    int a, b, pos_node, dist_max, degree, pos_a_i, pos_a_j, pos_b_i, pos_b_j;
    pair<int,int> biggest;
    vector<pair<int,int>> pos_tips, pos_forward, pos_inter, pos_aux;
    vector<int> pos_cost, pos_node_try;
    int size_pos_node_try, count, cost_min, cost_place, global_pos_a, global_pos_b, local_edge;
    int best_cost = 9999999, local_grid, rand_pos, pos_a, pos_b;
    int* grid_place = new int[TOTAL_GRID_SIZE*__LAYERS];
    int* grid_freedom = new int[TOTAL_GRID_SIZE];
    int type_node_a, type_node_b, pos_cycle_size;
    map<pair<int,int>,int> edges_cost_local;
    vector<tuple3> pos_cycle;

    for (int t = 0; t < times; ++t) {
        pos_a = pos_b = -1;

#if __DEBUG == 1
        printf("---> try: %d\n",t);
#endif

        edges_cost_local.clear();
#if __NEIGHBOURHOOD > 0
        for(int i = 0; i < TOTAL_GRID_SIZE; ++i) grid_freedom[i] = grid_freedom_original[i];
#endif
        for(int i = 0; i < TOTAL_GRID_SIZE*__LAYERS; ++i) grid_place[i] = type_matrix[i];

        cost_place = 0;
        for (int i = 0; i < EDGE_SIZE; ++i) {
            
            local_edge = i+(t+start)*EDGE_SIZE;

            a = vector_edges[local_edge].v0;
            b = vector_edges[local_edge].v1;

            type_node_a = type_node[a];
            type_node_b = type_node[b];

            global_pos_a = a+(t+start)*NODE_SIZE;
            global_pos_b = b+(t+start)*NODE_SIZE;
            
            pos_a_i = pos_i[global_pos_a];
            pos_a_j = pos_j[global_pos_a];
            pos_b_i = pos_i[global_pos_b];
            pos_b_j = pos_j[global_pos_b];

            pos_a = pos_a_i * GRID_SIZE + pos_a_j;
            pos_b = pos_b_i * GRID_SIZE + pos_b_j;

            key = make_pair(a, b);

#if __DEBUG == 1
            printf("\n---Operation: %d(%d,%d) -> %d (%d,%d)\n", a, pos_a_i, pos_a_j, b, pos_b_i, pos_b_j);
#endif
            
            if (pos_a_i != -1 && pos_b_i != -1) {
                cost = cost_local(pos_a_i, pos_a_j, pos_b_i, pos_b_j, GRID_SIZE);
                cost_place += cost;
                edges_cost_local[key] = cost;
#if __DEBUG == 1
                printf("PLACE TYPE ALREADY: %2d -> %2d Cost: %d\n", a, b, cost);
                print_grid_elements(pos_i, pos_j, t, NODE_SIZE, GRID_SIZE, a, b);
#endif
                continue;
            }
            
            size_pos_node_try = 0;

            // verify if side 'a' already place 
            if (pos_a_i == -1 && pos_b_i == -1) {
                // get the position random
                while (true) {
                    rand_pos = rand() % (IN_OUT_SIZE);
                    pos_a_i = pos_io_i[rand_pos];
                    pos_a_j = pos_io_j[rand_pos];
                    pos_a = pos_a_i * GRID_SIZE + pos_a_j;
                    //printf("pos_a (%d,%d)\n", pos_a_i, pos_a_j);
                    if (placed(type_node_a, pos_a_i, pos_a_j, pos_a, GRID_SIZE, TOTAL_GRID_SIZE, 
                        global_pos_a, grid_place, pos_i, pos_j, node_degree, grid_freedom)) {

                        get_tips(pos_a_i, pos_a_j, grid_freedom, GRID_SIZE, type_node_b, grid_place, 0, pos_tips, pos_io_i, pos_io_j, IN_OUT_SIZE);
                        break;
                    }
                }
            } else { // get tips of pos_a
                get_tips(pos_a_i, pos_a_j, grid_freedom, GRID_SIZE, type_node_b, grid_place, list_borders[b], pos_tips, pos_io_i, pos_io_j, IN_OUT_SIZE);
            }

            if (pos_b_i == -1) { // verify if side 'b' already place
                found = false;
#if __NEIGHBOURHOOD > 0
                pos_tips_size = pos_tips.size();
#else
                pos_tips_size = 0;
#endif
#if __CYCLE == 1
                pos_cycle = dic_CYCLE[t][key];
                pos_cycle_size = pos_cycle.size();
#else
                pos_cycle_size = 0;
#endif
                pos_forward = pos_tips;
                pos_forward_size = 0;
                pos_node_try.clear();
#if __DEBUG == 1
                cout << "POS TIPS: ";
                for (int j = 0; j < pos_tips_size; ++j) {
                    cout << pos_tips[j].second << " ";
                }
                cout << endl;
#endif

                if (pos_cycle_size > 0 && pos_tips_size > 0){
                    // forward
                    if (i < EDGE_SIZE-1) {
                        int next_a = vector_edges[i+1].v0;
                        int next_b = vector_edges[i+1].v1;
                        next_key = func_key(next_a, next_b);

                        for (int j = 0, n = dic_CYCLE[t][next_key].size(); j < n; ++j) {
                            if (dic_CYCLE[t][next_key][j].v1 == 0) {
#if __DEBUG == 1
                                cout << "FORWARD ";
                                cout << dic_CYCLE[t][next_key][j].v0 << "d" << dic_CYCLE[t][next_key][j].v1 << endl;
#endif
                                pos_node = pos_a; //pos[dic_CYCLE[next_key][j].first];
                                
                                /*cout << "NODE " << dic_CYCLE[next_key][j].first << " pos= " << pos_node << " pos: ";
                                //pos_aux = get_tips(pos_node, grid_freedom, GRID_SIZE, TOTAL_GRID_SIZE, 0);
                                
                                get_pos_neighbor(pos_node, 0, grid_freedom, GRID_SIZE, TOTAL_GRID_SIZE, pos_aux);
                                
                                for (int k = 0; k < pos_aux.size(); ++k) {
                                    cout << pos_aux[k].second << " ";
                                }
                                cout << endl;
                                pos_aux_size = pos_aux.size();
                                subtraction(pos_forward, pos_aux);
                                pos_forward_size = pos_forward.size();
                                
                                cout << "pos forward: ";
                                for (int k = 0; k < pos_forward.size(); ++k) {
                                    cout << pos_forward[k].second << " ";
                                }
                                cout << endl;
                                */
                            }
                        }
                        /*
                        if (pos_forward_size > 0) {
                            found = place_local(pos_forward, pos_forward_size, grid_place, b, node_degree, 
                                        pos, grid_freedom, GRID_SIZE, TOTAL_GRID_SIZE, arch, a, 5);
                            
                            cout << "place b = " << b << " pos= " << pos[b] << endl;
                            // skip to next edge
                            if (found) continue;
                        }*/
                    }

                    pos_inter = pos_forward;
                    pos_inter_size = pos_forward.size();

#if __DEBUG == 1                  
                    if (pos_cycle_size > 0) cout << "\nLIST CYCLE: ";
                    for (int j = 0; j < pos_cycle_size; ++j) cout << pos_cycle[j].v0 << "d" << pos_cycle[j].v1 << " ";
                    cout << endl;
#endif

                    for (int j = 0; j < pos_cycle_size; ++j) {
                        
                        if (pos_cycle[j].v1 > __MAX_ANNOTATION) continue;

                        pos_node_try.push_back(pos_cycle[j].v0);
                        
                        pos_node = pos_i[pos_cycle[j].v0] * GRID_SIZE + pos_j[pos_cycle[j].v0];
                        dist_max = pos_cycle[j].v1;
                        
                        get_pos_neighbor(pos_node, dist_max, grid_freedom, GRID_SIZE, TOTAL_GRID_SIZE, pos_aux);

#if DEBUG == 1
                        cout << "Node " << pos_cycle[j].v0 << "d" << dist_max << " pos= " << pos[pos_cycle[j].v0] << endl;
                        for (int k = 0; k < pos_aux.size(); ++k) {
                            cout << pos_aux[k].second << " ";
                        }
                        cout << endl;
#endif

                        pos_aux_size = pos_aux.size();
                        intersection(pos_inter, pos_aux);
                        pos_inter_size = pos_inter.size();

#if __DEBUG == 1
                        cout << "intersection: ";
                        for (int j = 0; j < pos_inter_size; ++j) {
                            cout << pos_inter[j].second << " "; 
                        }
                        cout << endl;
#endif
                        if (pos_inter_size == 0) {
#if __DEBUG == 1
                            cout << "VAZIO INTERSECAO" << endl;
#endif
                            break;
                        } 
                    }
                }

                if (pos_inter_size > 0) {
                    
                    pos_b = best_place_degree(pos_inter, pos_inter_size, grid_place, 
                        node_degree[global_pos_b], type_node_b, type_node);
                    pos_b_i = pos_b / GRID_SIZE;
                    pos_b_j = pos_b % GRID_SIZE;

                    if (placed(type_node_b, pos_b_i, pos_b_j, pos_b, GRID_SIZE, TOTAL_GRID_SIZE, 
                        global_pos_b, grid_place, pos_i, pos_j, node_degree, grid_freedom)) {
                        
                        cost = cost_local(pos_a_i, pos_a_j, pos_b_i, pos_b_j, GRID_SIZE);

                        #if __DEBUG == 1
                            printf("PLACE TYPE INTERCESSAO: %2d -> %2d Cost: %d\n", a, b, cost);
                            print_grid_elements(pos_i, pos_j, t, NODE_SIZE, GRID_SIZE, a, b);
                        #endif

                        cost_place += cost;
                        edges_cost_local[key] = cost;
                        continue; // skip to next edge
                    }
                }

                if (pos_tips_size > 0) {
                    // Trying the get cost better
                    pos_aux = pos_tips;
                    size_pos_node_try = pos_node_try.size();
                    pos_cost.clear();
                    
                    if (size_pos_node_try > 0) {
                        for (int k = 0; k < size_pos_node_try; ++k) {
                            for (int j = 0; j < pos_tips_size; ++j) {
                                int aux = pos_node_try[k]+(t+start)*NODE_SIZE;
                                cost = cost_local(pos_i[aux]/GRID_SIZE,pos_j[aux]%GRID_SIZE,pos_tips[j].second/GRID_SIZE,pos_tips[j].second%GRID_SIZE, GRID_SIZE);
                                if (k == 0) {
                                    pos_cost.push_back(cost);
                                } else {
                                    pos_cost[j] += cost;
                                }
                            }
                        }
                        // choose the min cost
                        cost_min = pos_cost[0];
                        pos_aux.clear();
                        for (int j = 1; j < pos_tips_size; ++j) {
                            cost = pos_cost[j];
#if __DEBUG == 1
                            cout << "Pos = " << pos_tips[j].second << " weight " << cost << endl;
#endif
                            if (cost_min > cost) {
                                cost_min = cost;
                                pos_aux.clear();
                                pos_aux.push_back({pos_tips[j].first, pos_tips[j].second});
                            } else if (cost == cost_min) {
                                pos_aux.push_back({pos_tips[j].first, pos_tips[j].second});
                            }
                        }
                    }

                    pos_aux_size = pos_aux.size();
#if __DEBUG == 1
                    printf("Place TIPS: ");
                    for (int j = 0; j < pos_aux_size; ++j) printf("%d ", pos_aux[j].second);
                    printf("\n");
#endif

                    int pos_b = best_place_degree(pos_aux, pos_aux_size, grid_place, 
                        node_degree[global_pos_b], type_node_b, type_node);

                    pos_b_i = pos_b / GRID_SIZE;
                    pos_b_j = pos_b % GRID_SIZE;

                    if (placed(type_node_b, pos_b_i, pos_b_j, pos_b, GRID_SIZE, TOTAL_GRID_SIZE, 
                        global_pos_b, grid_place, pos_i, pos_j, node_degree, grid_freedom)) {

                        cost = cost_local(pos_a_i, pos_a_j, pos_b_i, pos_b_j, GRID_SIZE);

                        #if __DEBUG == 1
                            printf("PLACE TYPE DICAS: %2d -> %2d Cost: %d\n", a, b, cost);
                            print_grid_elements(pos_i, pos_j, t, NODE_SIZE, GRID_SIZE, a, b);
                        #endif

                        cost_place += cost;
                        edges_cost_local[key] = cost;
                        continue; // skip to next edge
                    }                                     
                }

                // try more closed
                if (try_adjacency(a, b, pos_a_i, pos_a_j, pos_b_i, pos_b_j, dist_border, 
                type_node_b, GRID_SIZE, TOTAL_GRID_SIZE, NODE_SIZE, t, key, global_pos_b, 
                grid_place, pos_i, pos_j, node_degree, grid_freedom, pos_io_i, pos_io_j, 
                IN_OUT_SIZE, edges_cost_local, cost_place)) continue;
            } 

            if (pos_a == -1 || pos_b == -1){
                //cerr << "No solution found!" << endl; 
                cost_place += 9999999;
                edges_cost_local[key] = 1000;
            }
        }

        // get the best
        edges_cost.push_back(edges_cost_local);
        vector_cost[t+start] = cost_place;
    }
    return;
}

#endif
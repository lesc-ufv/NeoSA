#include <Graph.h>
#include <buffer.h>
#include <routing.h>
#include <get_critical_path.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <chrono>
#include <algorithm> 
#include <fstream>
#include <omp.h>
#include <map>
#include <annealing.h>
#include <instance.h>

#define NGRIDS 1

using namespace std;
using namespace std::chrono;

//Tables with costs for mesh and 1hop
vector<vector<int>> tablemesh;
vector<vector<int>> table1hop;
vector<vector<int>> tablechess1hop;
vector<vector<int>> tablechessmesh;
vector<vector<int>> tablehex;

int main(int argc, char** argv) {
    srand (time(NULL));
    
    int **results;
    results = new int*[NGRIDS];
    for(int i=0; i<NGRIDS; i++){
        results[i] = new int[4];
    }
    double *randomvec;
    randomvec = new double[1000000];
    for(int i=0;i<1000000;i++){
        randomvec[i] = (double)rand() / (double)(RAND_MAX);
    }
    //Cria a estrutura do grafo com os vetores (A, v e v_i) à partir do grafo g
    string path = "", name = "";
    path = argv[1]; name = argv[2];
    Graph g(path);
    bool isTree = checkTree(g);
    // if(isTree) cout << "É árvore!" << endl;
    // else cout << "Não é árvore!" << endl;
    // return 0;
    int nodes = g.num_nodes();
    int edges = g.num_edges();
    int *h_edgeA, *h_edgeB;
    h_edgeA = new int[edges];
    h_edgeB = new int[edges];
    vector<pair<int,int>> edge_list = g.get_edges();
    vector<int> A;
    int *v, *v_i;
    bool *io, *mults, *others;
    v = new int[nodes];
    v_i = new int[nodes];
    io = new bool[nodes];
    mults = new bool[nodes];
    others = new bool[nodes];

    

    //Matriz de booleanos para identificar ios e mults
    for(int i=0; i<nodes; i++){
        v[i]=0; v_i[i]=0;
        io[i] = false;
        mults[i] = true;
        others[i] = false;
    }

    //Preenche a estrutura do grafo
    for(int i=0; i<edge_list.size(); i++){
        int n1 = edge_list[i].first;
        int n2 = edge_list[i].second;
        h_edgeA[i] = n1;
        h_edgeB[i] = n2;
        v[n1]++;
        if(n1!=n2) v[n2]++;
    }

    for(int i=1; i<nodes; i++){
        v_i[i] = v_i[i-1] + v[i-1];
    }

    for(int i=0; i<nodes; i++){
        for(int j=0; j<edges; j++){
            if (h_edgeA[j] != h_edgeB[j]) {
                if(h_edgeA[j]==i) A.push_back(h_edgeB[j]);
                if(h_edgeB[j]==i) A.push_back(h_edgeA[j]);
            } else {
                if(h_edgeA[j]==i) A.push_back(h_edgeB[j]);
            }
        }
    }

    //Variáveis para o placement
    int dim, gridSize;
    int **grid;
    int **positions;
    bool *pattern;         
    bool *borders;        
    int cost = 100000;
    int nGrids = NGRIDS; 
    
    //Marca quais nós são IO
    for(int i=0; i<nodes; i++){
        if(g.get_predecessors(i).size()==0 || g.get_sucessors(i).size()==0) io[i] = true;
    }

    dim = ceil(sqrt(nodes));
    //Cria matrizes de distâncias
    
    buildMatrices(dim, tablemesh, table1hop, tablechess1hop, tablechessmesh, tablehex);
    gridSize = dim*dim;
    const int num_arch = 4;

    //Guarda numero de PES utilizados (Remover em breve)
    vector<vector<vector<int>>> activeFifos(num_arch, vector<vector<int>>(NGRIDS,vector<int>(500,0)));
    //Vetores para guardar o numero de buffers por aresta
    vector<vector<map<pair<int,int>,int>>> edges_cost(num_arch);
    vector<vector<map<pair<int,int>,int>>> buffers(num_arch);
    //Distancia manhattan por aresta
    map<pair<int,int>,int> manh;
    vector<pair<int,int>> edges_ = g.get_edges();
    for(int arch=0; arch<num_arch; arch++){
        edges_cost[arch].resize(NGRIDS);
        buffers[arch].resize(NGRIDS);
        for(int k=0; k<NGRIDS; k++){
            for(int i = 0; i < edges; i++){
                pair<int,int> aux = edges_[i];
                edges_cost[arch][k][aux] = 0;
                buffers[arch][k][aux] = 0;
            }   
        }
    }
    double time_total[num_arch] = {0.0, 0.0, 0.0, 0.0};
    int cost_min[num_arch] = {-1,-1,-1, -1};

    //Aloca memoria pros grids
    grid = new int*[nGrids];
    positions = new int*[nGrids];
    for(int i=0; i<nGrids; i++) grid[i] = new int[gridSize];
    for(int i=0; i<nGrids; i++) {
        positions[i] = new int[nodes];
        for(int j=0; j<nodes;j++) positions[i][j]=0;
    }    
    borders = new bool[gridSize];
    pattern = new bool[gridSize];
    for(int i=0; i<gridSize; i++){
        borders[i] = false;
        //true for NORMAL, false for patterns
        pattern[i] = true;
    }
    int availableMults=0;
    for(int i=0; i<gridSize; i++){
        //set borders
        if(i/dim==0) borders[i] = true;
        if(i/dim==dim-1) borders[i] = true;
        if(i%dim==0) borders[i] = true;
        if(i%dim==dim-1) borders[i] = true;  
        //set mults chess
        // if(abs(i%dim-i/dim)%2==0){
        //     pattern[i]=true;
        //     availableMults++;
        // }
        // else pattern[i]=false;

        //set mults cols
        // if((i%dim)%2==0){
        //     pattern[i]=true;
        //     availableMults++;
        // }
        // else pattern[i]=false;

        //set mults cols 2 spaces
        // if((i%dim)%3==0){
        //     pattern[i]=true;
        //     availableMults++;
        // }
        // else pattern[i]=false;

        //set borders = mults   
        // if(i/dim==0){
        //     pattern[i] = true;
        //     availableMults++;
        // }
        // if(i/dim==dim-1){
        //     pattern[i] = true;
        //     availableMults++;
        // } 
        // if(i%dim==0){
        //     pattern[i] = true;
        //     availableMults++;
        // } 
        // if(i%dim==dim-1){
        //     pattern[i] = true;
        //     availableMults++;
        // }     
    }
    //REATIVAR SE FOR HETEROGENEO
    // if(numMults>=availableMults) return 0;

    //Para marcar quais instancias tiveram roteamento feito com sucesso
    vector<vector<bool>> successfulRoutings(num_arch,vector<bool>(nGrids,true));
    //Vetor da estrutura Instance que guarda informações de resultado de uma da NGRIDS instâncias
    vector<vector<Instance>> instances(num_arch);

    //0 = mesh 1 = 1hop
    for (int k = 0; k < 1; ++k) {       
        //Preenche os grids
        for(int i=0; i<nGrids; i++){
            for(int j=0; j<gridSize; j++){
                grid[i][j] = 255;
            }
        }
        for(int i=0; i<nGrids; i++){
            bool *setNodes;
            setNodes = new bool[nodes];
            for(int j=0; j<nodes; j++){
                setNodes[j] = false;
            }
            for(int j=0; j<nodes; j++){
                if(io[j]==false || mults[j]==false) continue;
                for(int t=0; t<gridSize; t++){
                    if(borders[t]==true && pattern[t]==true && setNodes[j]==false && grid[i][t]==255){
                        grid[i][t]=j;
                        setNodes[j] = true;
                        break;
                    }
                }
            }
            for(int j=0; j<nodes; j++){
                if(io[j]==false || setNodes[j]==true) continue;
                for(int t=0; t<gridSize; t++){
                    if(borders[t]==true  && grid[i][t]==255){
                        grid[i][t]=j;
                        setNodes[j] = true;
                        break;
                    }
                }
            }
            for(int j=0; j<nodes; j++){
                if(mults[j]==false || setNodes[j]==true) continue;
                for(int t=0; t<gridSize; t++){
                    if(pattern[t]==true && grid[i][t]==255){
                        grid[i][t]=j;
                        break;
                    }
                }
            }
            for(int j=0; j<nodes; j++){
                if(others[j]==false || setNodes[j]==true) continue;
                for(int t=0; t<gridSize; t++){
                    if(grid[i][t]==255){
                        grid[i][t]=j;
                        break;
                    }
                }
            }
            for(int j=0; j<gridSize; j++){
                for(int t=0; t<gridSize; t++){
                    double aleat = ((rand()%1000)/1000.0);
                    if(aleat > 0.5 && (io[j]&&io[t]&&mults[j]&&mults[t])){
                        int temp = grid[i][j];
                        grid[i][j] = grid[i][t];
                        grid[i][t] = temp;
                    }
                    if(aleat > 0.5 && (others[j]&&others[t])){
                        int temp = grid[i][j];
                        grid[i][j] = grid[i][t];
                        grid[i][t] = temp;
                    }  
                }
            }
        }
        
        
        for(int t=0; t<nGrids; t++){
            for(int i=0; i<gridSize; i++){
                for(int j=0; j<nodes; j++){
                    if(j==grid[t][i]) {
                        positions[t][j]=i;
                    }
                }
            }
        }       
        //fclose(fptr);
        auto start = high_resolution_clock::now();

        #pragma omp parallel for
        for (int i = 0; i < nGrids; ++i) {
            int cost = gridCost(edges, dim, h_edgeA, h_edgeB, positions[i], k, tablemesh, table1hop, tablechess1hop, tablechessmesh, tablehex);
            // printPlacementInfo(nodes, gridSize, grid[i], positions[i], cost);
            results[i][0] = cost;
            if(cost == 0){
                results[i][1] = cost;
                results[i][2] = 0;
                results[i][3] = 0;  
                continue;
            }
            annealing2(i, nodes, dim, gridSize, cost, grid[i], positions[i], v_i, v, A, k, io, borders, mults, others, pattern, results, randomvec, tablemesh, table1hop, tablechess1hop, tablechessmesh, tablehex);
        }
        auto stop = high_resolution_clock::now();

        std::chrono::duration<double, std::milli> duration = (stop-start);
        //auto duration = duration_cast<seconds>(stop - start);

        time_total[k] = duration.count()/1000;
        
 
        int totalTime=0;
        for(int i=0; i<nGrids; i++){
            Instance temp(grid[i], positions[i], gridSize, nodes, dim, results[i][1], k);
            instances[k].push_back(temp);
            // if(results[i][1]>=0) costs.push_back(results[i][1]);
            // totalTime += results[i][2];
        }
        // cost_min[k] = *min_element(costs.begin(), costs.end());
        
        //construct edges_cost
        edgesCostConstructor(g, manh, edges_cost[k], grid, positions, k, dim, nGrids, tablemesh, table1hop, tablechess1hop, tablechessmesh, tablehex); 
         
        int *gridFlat = new int[NGRIDS*gridSize];
        int *positionsXFlat = new int[NGRIDS*nodes];
        int *positionsYFlat = new int[NGRIDS*nodes];
        //vector<bool> successfulRoutings(nGrids, true);
        for(int i=0; i<nGrids; i++){
            for(int j=0; j<gridSize; j++){
                gridFlat[i*gridSize + j] = grid[i][j];
            }
            for(int j=0; j<nodes; j++){
                positionsXFlat[i*nodes + j] = positions[i][j]%dim;
                positionsYFlat[i*nodes + j] = positions[i][j]/dim;
            }
        }

        for(int i=0; i<edge_list.size(); i++){
            pair<int,int> aux = edge_list[i];
            cout << aux.first << "->" << aux.second <<  " " << edges_cost[0][0][aux] << endl;
        }
        cout << endl;
        

        routing(gridSize, edges, nodes, edge_list, edges_cost[k], positionsXFlat, positionsYFlat, NGRIDS, k, successfulRoutings[k], instances[k]);
        
        int minBuff = buffer(g, activeFifos[k], manh, edges_cost[k], buffers[k], successfulRoutings[k], instances[k], isTree);
        //cout << minBuff << " ";

        for(int i=0; i<edge_list.size(); i++){
            pair<int,int> aux = edge_list[i];
            cout << aux.first << "->" << aux.second <<  " " << edges_cost[0][0][aux] << endl;
        }
        cout << endl;

        //printGrid(0,grid,dim, gridSize);
        int bestIdx = -1;
        int lowest = 100000;
        for(int i=0; i<instances[k].size(); i++){
            int current = instances[k][i].getLongestFIFO();
            instances[k][i].checkStrand();
            if(current < lowest && instances[k][i].getStrand()){
                lowest = current;
                bestIdx = i;
            } 
        }

        instances[k][bestIdx].writePES(bestIdx);
        instances[k][bestIdx].writeGrid(g, bestIdx);
        if(bestIdx != -1) printBestSol(instances[k][bestIdx], k, gridSize, dim);
        //printBestSolClean(instances[k][bestIdx], k, gridSize, dim);
        //cout << argv[2] << endl;
    }

    delete h_edgeA;
    delete h_edgeB;
    delete v;
    delete v_i;
    delete io;
    delete borders;

    //writeFile(activeFifos, argv[2], gridSize, successfulRoutings);
   
    //printf("%s,%d,%d,%d,%d,%.2lf,%.2lf,%.2lf,%.2lf\n", argv[2], cost_min[0], cost_min[1], cost_min[2], cost_min[3], time_total[0], time_total[1], time_total[2], time_total[3]);
    return 0;
}
#ifndef __ANNEALING__H
#define __ANNEALING__H

#include <chrono>

using namespace std::chrono;

void printMatrix(vector<vector<int>> &table){
    for(int i=0; i<table.size(); i++){
        for(int j=0; j<table[i].size(); j++){
            if(j==table[i].size()-1) cout << table[i][j] << endl;
            else cout << table[i][j] << "\t";
        } 
    }
    cout << endl;
}

void buildMatrices(int dim, vector<vector<int>> &tablemesh, vector<vector<int>> &table1hop, vector<vector<int>> &tablechess1hop, vector<vector<int>> &tablechessmesh, vector<vector<int>> &tablehex){
    table1hop.resize(dim);
    tablemesh.resize(dim);
    tablechess1hop.resize(dim);
    tablechessmesh.resize(dim);
    tablehex.resize(dim);
    for(int i=0; i<dim; i++){
        table1hop[i].resize(dim);
        tablemesh[i].resize(dim);
        tablechess1hop[i].resize(dim);
        tablechessmesh[i].resize(dim);
        tablehex[i].resize(dim);
    }
    int cont=1; bool rep = false;
    for(int i=0; i<dim; i++){
        tablechessmesh[i][0] = cont;
        if(i%2==1){
            cont++;
            for(int j=1; j<dim; j++){
                if(j%2==0) tablechessmesh[i][j] = tablechessmesh[i][j-1];
                else tablechessmesh[i][j] = tablechessmesh[i][j-1]+1;//tablechessmesh[i][j]--;
            }
        } else {
            for(int j=1; j<dim; j++){
                if(j%2==1) tablechessmesh[i][j] = tablechessmesh[i][j-1];
                else tablechessmesh[i][j] = tablechessmesh[i][j-1]+1;
                
            }
        }
        
        for(int j=0; j<dim; j++){
            int jfrom = 0;
            int ifrom = 0;
            int distX = abs(jfrom-j);
            int distY = abs(ifrom-i);
            tablechessmesh[i][j]--; 
            tablemesh[i][j] = max(0,distY+distX-1);
            table1hop[i][j] = tablechess1hop[i][j] = max(0,distX/2+distX%2+distY/2+distY%2-1);
        }
    }
    // for (int i = 0; i < dim; i++) {
    //     for (int j = 0; j < dim; j++) {
    //         cout << tablechessmesh[i][j] << "\t";
    //     }
    //     cout << endl;
    // }
}




int gridCost(int edges, int dim, int *h_edgeA, int *h_edgeB, int *positions, int arch, vector<vector<int>> &tablemesh, vector<vector<int>> &table1hop, vector<vector<int>> &tablechess1hop, vector<vector<int>> &tablechessmesh, vector<vector<int>> &tablehex){
    int cost_ = 0, increment=0, distManhattanI, distManhattanJ, chess;
    for(int k=0; k<edges; k++){
        int ifrom = positions[h_edgeA[k]]/dim; 
        int jfrom = positions[h_edgeA[k]]%dim; 
        int ito = positions[h_edgeB[k]]/dim; 
        int jto = positions[h_edgeB[k]]%dim;
        // cout << ifrom << " " << jfrom << " " << ito << " " << jto << endl; 
        
        distManhattanJ = abs(jto - jfrom);
        distManhattanI = abs(ito - ifrom);

        if (arch == 0)
            increment = tablemesh[distManhattanI][distManhattanJ];
        else if (arch == 2) {
            chess = (abs(jfrom-ifrom)%2==0) ? 1 : 0;
            increment = (chess==1) ? tablechess1hop[distManhattanI][distManhattanJ] : tablechessmesh[distManhattanI][distManhattanJ];        
        } else if (arch == 1) {
            increment = table1hop[distManhattanI][distManhattanJ];
        } else if (arch == 3) {
            increment = tablehex[distManhattanI][distManhattanJ];
        }
        cost_ += increment;
    }
    return cost_;
}

void annealing2(int iresult, int nodes, int dim, int gridSize, int &cost, int *grid, int *positions, int *v_i, int *v, vector<int> &A, int arch, bool *io, bool *borders, bool *mults, bool *others, bool *pattern, int **results, double *randomvec, vector<vector<int>> &tablemesh, vector<vector<int>> &table1hop, vector<vector<int>> &tablechess1hop, vector<vector<int>> &tablechessmesh, vector<vector<int>> &tablehex, double timeout){
    int *localGrid, *localPositions;
    int currentCost = cost, nextCost, swapCount=0, increment, distManhattanI, distManhattanJ, chess;
    localGrid = new int[gridSize];
    for(int i=0; i<gridSize; i++) localGrid[i] = grid[i];
    localPositions = new int[nodes];
    for(int i=0; i<nodes; i++) localPositions[i] = positions[i];    
    //random vector index
    double random, valor;
    int randomctrl = 0;
    double T=100;
    auto start = high_resolution_clock::now();
    while(T>=0.00001){
        for(int i=0; i<gridSize; i++){
            for(int j=i+1; j<gridSize; j++){                
                //if we're looking at 2 empty spaces, skip                   
                if(localGrid[i]==-1 && localGrid[j]==-1)
                    continue;

                int node1 = localGrid[i], node2 = localGrid[j];
                nextCost = currentCost;
                int old1, old2;
                bool test = false, test2 = false;
                
                if(borders[i]==false && borders[j]==false) test = true;
                if(borders[i]==true && borders[j]==true) test = true;
                if((borders[i]==true && node1!=-1) && borders[j]==false) test = false;
                if(borders[i]==false && (borders[j]==true && node2!=-1)) test = false;
                if(pattern[i]==false && pattern[j]==false) test2 = true;
                if(pattern[i]==true && pattern[j]==true) test2 = true;
                if((pattern[i]==true && node1!=-1) && pattern[j]==false) test2 = false;
                if(pattern[i]==false && (pattern[j]==true && node2!=-1)) test2 = false;

                //test for the heterogeneous mult mesh
                // bool MchessFrom = pattern[i];
                // bool MchessTo = pattern[j];
                // if(mults[node1]==true && MchessTo==true && mults[node2]==true && MchessFrom==true) test2=true;
                // else test2 = false;
                
                if(test && test2){
                    //remove cost from object edges                
                    if(node1!=-1){
                        for(int i=0; i<v[node1]; i++){
                            // int posVizNode1 = localPositions[A[v_i[node1]+i]];
                            // int posNode1 = localPositions[node1];
                            // increment = matriz[posViznode1, posNode1];

                            int ifrom = localPositions[node1]/dim; 
                            int jfrom = localPositions[node1]%dim; 
                            int ito = localPositions[A[v_i[node1]+i]]/dim; 
                            int jto = localPositions[A[v_i[node1]+i]]%dim;
                            distManhattanJ = abs(jto - jfrom);
                            distManhattanI = abs(ito - ifrom);
                            if (arch == 0)
                                increment = tablemesh[distManhattanI][distManhattanJ];
                            else if (arch == 2) {
                                chess = (abs(jfrom-ifrom)%2==0) ? 1 : 0;
                                increment = (chess==1) ? tablechess1hop[distManhattanI][distManhattanJ] : tablechessmesh[distManhattanI][distManhattanJ];     
                            } else if (arch == 1) {
                                increment = table1hop[distManhattanI][distManhattanJ];
                            }  else if (arch == 3) {
                                increment = tablehex[distManhattanI][distManhattanJ];
                            }

                            nextCost -= increment; 
                        }
                    }
                    if(node2!=-1){
                        for(int i=0; i<v[node2]; i++){
                            int ifrom = localPositions[node2]/dim; 
                            int jfrom = localPositions[node2]%dim; 
                            int ito = localPositions[A[v_i[node2]+i]]/dim; 
                            int jto = localPositions[A[v_i[node2]+i]]%dim; 
                            distManhattanJ = abs(jto - jfrom);
                            distManhattanI = abs(ito - ifrom);

                            if (arch == 0)
                                increment = tablemesh[distManhattanI][distManhattanJ];
                            else if (arch == 2) {
                                chess = (abs(jfrom-ifrom)%2==0) ? 1 : 0;
                                increment = (chess==1) ? tablechess1hop[distManhattanI][distManhattanJ] : tablechessmesh[distManhattanI][distManhattanJ];        
                            } else if (arch == 1) {
                                increment = table1hop[distManhattanI][distManhattanJ];
                            } else if (arch == 3) {
                                increment = tablehex[distManhattanI][distManhattanJ];
                            }

                            nextCost -= increment; 
                        }
                    }
                    //swap positions
                    // int old1, old2;
                    old1 = i;
                    old2 = j;
                    if(node1!=-1) localPositions[node1] = old2;
                    if(node2!=-1) localPositions[node2] = old1;
                    localGrid[j] = node1;
                    localGrid[i] = node2;
                    //recalculate cost
                    if(node1!=-1){
                        for(int i=0; i<v[node1]; i++){
                            int ifrom = localPositions[node1]/dim; 
                            int jfrom = localPositions[node1]%dim; 
                            int ito = localPositions[A[v_i[node1]+i]]/dim; 
                            int jto = localPositions[A[v_i[node1]+i]]%dim; 
                            distManhattanJ = abs(jto - jfrom);
                            distManhattanI = abs(ito - ifrom);

                            if (arch == 0)
                                increment = tablemesh[distManhattanI][distManhattanJ];
                            else if (arch == 2) {
                                chess = (abs(jfrom-ifrom)%2==0) ? 1 : 0;
                                increment = (chess==1) ? tablechess1hop[distManhattanI][distManhattanJ] : tablechessmesh[distManhattanI][distManhattanJ];        
                            } else if (arch == 1) {
                                increment = table1hop[distManhattanI][distManhattanJ];
                            } else if (arch == 3) {
                                increment = tablehex[distManhattanI][distManhattanJ];
                            }

                            nextCost += increment;                         
                        }
                    }
                    if(node2!=-1){
                        for(int i=0; i<v[node2]; i++){
                            int ifrom = localPositions[node2]/dim; 
                            int jfrom = localPositions[node2]%dim; 
                            int ito = localPositions[A[v_i[node2]+i]]/dim; 
                            int jto = localPositions[A[v_i[node2]+i]]%dim; 
                            distManhattanJ = abs(jto - jfrom);
                            distManhattanI = abs(ito - ifrom);

                            if (arch == 0)
                                increment = tablemesh[distManhattanI][distManhattanJ];
                            else if (arch == 2) {
                                chess = (abs(jfrom-ifrom)%2==0) ? 1 : 0;
                                increment = (chess==1) ? tablechess1hop[distManhattanI][distManhattanJ] : tablechessmesh[distManhattanI][distManhattanJ];       
                            } else if (arch == 1) {
                                increment = table1hop[distManhattanI][distManhattanJ];
                            } else if (arch == 3) {
                                increment = tablehex[distManhattanI][distManhattanJ];
                            }
                            nextCost += increment;   
                        }
                    }
                }
                //parameter for annealing probability
                valor = exp(-1*(nextCost - currentCost)/T);
                //random number between 0 and 1
                random = randomvec[randomctrl];
                randomctrl++;
                if(randomctrl==1000000) randomctrl=0;

                //if cost after changes is less than before or if cost is higher but we're in the annealing probanility range, return
                if(nextCost <= currentCost || random <= valor){
                    currentCost = nextCost;
                    swapCount++;
                }
                //else, undo changes and stay with previous cost
                else{
                    if(node1!=-1) localPositions[node1] = old1;
                    if(node2!=-1) localPositions[node2] = old2;
                    localGrid[j] = node2;
                    localGrid[i] = node1;
                }
            }
            if(cost==0) break;             
            T*=0.999;
        }
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        if(duration.count() >= timeout) break;   
        //aqui T*=0.999;
    }
    

    for(int i=0; i<gridSize; i++) grid[i] = localGrid[i];
    for(int i=0; i<nodes; i++) positions[i] = localPositions[i]; 
    
    results[iresult][1] = currentCost;
    //results[iresult][2] = duration.count()/1000;
    results[iresult][3] = swapCount;

    delete localPositions;
    delete localGrid;
}

void edgesCostConstructor(Graph g, map<pair<int,int>,int> &manh, vector<map<pair<int,int>,int>> &edges_cost, int** grid, int **positions, int arch, int dim, int nGrids, vector<vector<int>> &tablemesh, vector<vector<int>> &table1hop, vector<vector<int>> &tablechess1hop, vector<vector<int>> &tablechessmesh, vector<vector<int>> &tablehex){
    int num_nodes = g.num_nodes();
    int num_edges = g.num_edges();
    int increment=0;
    int distManhattanI, distManhattanJ, chess;
    vector<pair<int,int>> edges = g.get_edges();
    for(int i=0; i<nGrids; i++){
        for(int j=0; j<edges.size(); j++){
            pair<int,int> aux = edges[j];
            int A = aux.first; int B = aux.second;
            int ifrom = positions[i][A]/dim; 
            int jfrom = positions[i][A]%dim; 
            int ito = positions[i][B]/dim; 
            int jto = positions[i][B]%dim;

            distManhattanJ = abs(jto - jfrom);
            distManhattanI = abs(ito - ifrom);

            if (arch == 0){
                increment = tablemesh[distManhattanI][distManhattanJ];
                // cout << "increment: " << increment << endl;
            }
            else if (arch == 2) {
                chess = (abs(jfrom-ifrom)%2==0) ? 1 : 0;
                increment = (chess==1) ? tablechess1hop[distManhattanI][distManhattanJ] : tablechessmesh[distManhattanI][distManhattanJ];        
            } else if (arch == 1) {
                increment = table1hop[distManhattanI][distManhattanJ];
            } else if (arch == 3) {
                increment = tablehex[distManhattanI][distManhattanJ];
            }
            //edges_cost[i].insert(pair<pair<int,int>,int>(aux,increment));
            edges_cost[i][aux] = increment;
            // cout << "edgescost: " << edges_cost[i][aux] << endl;

            //fill manh only on the first pass
            if(i==0) manh[aux] = tablemesh[distManhattanI][distManhattanJ];
        }
    }
}

bool isCycle(int NODE_SIZE, int u, bool visited[], int parent, vector<vector<int>> &adj) {
   visited[u] = true;    //mark v as visited
   for(int v = 0; v<NODE_SIZE; v++) {
      if(adj[u][v]) {
         if(!visited[v]) {     //when the adjacent node v is not visited
            if(isCycle(NODE_SIZE, v, visited, u, adj)) {
               return true;
            }
         } else if(v != parent) {    //when adjacent vertex is visited but not parent
            return true;    //there is a cycle
         }
      }
   }
   return false;
}

bool checkTree(Graph g){
    int numNodes = g.num_nodes();
    int numEdges = g.num_edges();
    if(numEdges <= numNodes - 1) return true;
    else return false;
    // vector<int> nodes = g.get_nodes();
    // int NODE_SIZE = nodes.size();

    // vector<vector<int>> adj(NODE_SIZE, vector<int>(NODE_SIZE,0));
    // for(int i=0; i<NODE_SIZE; i++){
    //     int currentNode = nodes[i];
    //     vector<int> suc = g.get_sucessors(currentNode);
    //     vector<int> pred = g.get_predecessors(currentNode);
    //     for(int j=0; j<suc.size(); j++){
    //         adj[currentNode][suc[j]] = 1;
    //     }
    //     for(int j=0; j<pred.size(); j++){
    //         adj[currentNode][pred[j]] = 1;
    //     }
    // }

    // bool *vis = new bool[NODE_SIZE];

    // for(int i = 0; i<NODE_SIZE; i++)
    //     vis[i] = false;    //initialize as no node is visited
         
    // if(isCycle(NODE_SIZE, 0, vis, -1, adj))    //check if there is a cycle or not
    //     return false;
         
    // for(int i = 0; i<NODE_SIZE; i++) {
    //     if(!vis[i])    //if there is a node, not visited by traversal, graph is not connected
    //     return false;
    // }
    // return true;
}

void printGrid(int idx, int **grid, int dim, int gridSize){
    printf("grid:\n");
    for (int i = 0; i < gridSize; ++i){
        if(i%dim==dim-1) printf("%d\n", grid[idx][i]);
        else printf("%d\t", grid[idx][i]);
    }
    printf("\n");
}

void writeFile(vector<vector<vector<int>>> A, char* name, int gridSize, vector<vector<bool>> &successfulRoutings){   
    
    ofstream ofile, ofile1, ofile2, ofile3;
    char buffer[strlen(name)+20] = "\0";
    char beg[7] = "mesh/"; 
    char out[7] = ".mesh";
    strcat(buffer,beg);
    strcat(buffer,name);
    strcat(buffer,out);
    ofile.open(buffer);     

    char buffer1[strlen(name)+20] = "\0";
    char beg1[7] = "1hop/"; 
    char out1[7] = ".1hop";
    strcat(buffer1,beg1);
    strcat(buffer1,name);
    strcat(buffer1,out1);
    ofile1.open(buffer1);

    char buffer2[strlen(name)+20] = "\0";
    char beg2[7] = "chess/"; 
    char out2[7] = ".chess";
    strcat(buffer2,beg2);
    strcat(buffer2,name);
    strcat(buffer2,out2);
    ofile2.open(buffer2);

    char buffer3[strlen(name)+20] = "\0";
    char beg3[7] = "hex/"; 
    char out3[7] = ".hex";
    strcat(buffer3,beg3);
    strcat(buffer3,name);
    strcat(buffer3,out3);
    ofile3.open(buffer3);

    int dummy = -1;    
    for(int i=0; i<A.size(); i++){
        for(int j=0; j<A[i].size();j++){
            for(int k=A[i][j].size()-1; k>=0; k--){
                if(A[i][j][k-1]!=0){
                    A[i][j][k] = -2;
                    break;
                } 
                else A[i][j][k] = -2;
            }
        }
    }
    for(int i=0; i<A.size(); i++){
        if(i==0){
            ofile << gridSize << endl;
            for(int j=0; j<A[i][0].size(); j++){
                if(j==A[i][j].size()-1){
                    ofile << j << endl;
                }
                else {
                    ofile << j << "\t";
                }
            }
            for(int j=0; j<A[i].size(); j++){
                for(int k=0; k<A[i][j].size(); k++){
                    if(k==A[i][j].size()-1){
                        if(successfulRoutings[i][j] && A[i][j][k] != -2) ofile << A[i][j][k] << endl;
                        else ofile << "\t" << endl;
                    }
                    else {
                        if(successfulRoutings[i][j] && A[i][j][k] != -2) ofile << A[i][j][k] << "\t";
                        else ofile << "\t" << "\t";
                    }
                }
            }
            ofile << endl;
        } else if(i==1){
            ofile1 << gridSize << endl;
            for(int j=0; j<A[i][0].size(); j++){
                if(j==A[i][j].size()-1){
                    ofile1 << j << endl;
                }
                else {
                    ofile1 << j << "\t";
                }
            }
            for(int j=0; j<A[i].size(); j++){
                for(int k=0; k<A[i][j].size(); k++){
                    if(k==A[i][j].size()-1){
                        if(successfulRoutings[i][j]&& A[i][j][k] != -2) ofile1 << A[i][j][k] << endl;
                        else ofile1 << "\t" << endl;
                    }
                    else {
                        if(successfulRoutings[i][j]&& A[i][j][k] != -2) ofile1 << A[i][j][k] << "\t";
                        else ofile1 << "\t" << "\t";
                    }
                }
            }
            ofile1 << endl;
        } else if(i==2){
            ofile2 << gridSize << endl;
            for(int j=0; j<A[i][0].size(); j++){
                if(j==A[i][j].size()-1){
                    ofile2 << j << endl;
                }
                else {
                    ofile2 << j << "\t";
                }
            }
            for(int j=0; j<A[i].size(); j++){
                for(int k=0; k<A[i][j].size(); k++){
                    if(k==A[i][j].size()-1){
                        if(successfulRoutings[i][j] && A[i][j][k] != -2) ofile2 << A[i][j][k] << endl;
                        else ofile2 << "\t" << endl;
                    }
                    else {
                        if(successfulRoutings[i][j] && A[i][j][k] != -2) ofile2 << A[i][j][k] << "\t";
                        else ofile2 << "\t" << "\t";
                    }
                }
            }
            ofile2 << endl;
        } //include HEX
    }
}

void printBuffers(vector<map<pair<int,int>,int>> &buffers){
    
    for(int i=0; i<buffers.size(); i++){
        for(auto it = buffers[i].begin(); it != buffers[i].end(); ++it) {
            cout << it->first.first << "->" << it->first.second << " " << it->second << "\n";
        }
    }
}

#endif
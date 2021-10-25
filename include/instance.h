#ifndef __INSTANCE__H
#define __INSTANCE__H

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <stdio.h>
#include <Graph.h>

using namespace std;

class Instance{
    public:        
        Instance(int* inGrid, int* inPositions, int gridSize, int numNodes, int dim, int cost, int arch);
        void setGrid(int *inGrid, int gridSize);
        void setPositions(int* inPositions, int numNodes);
        void setCost(int cost);
        void printInstanceGrid();
        void insertBufferedEdge(pair<int,int> &edge, int buffers, int numPES);
        void setMinBufferSize(int size);
        vector<int> getGrid();
        vector<pair<pair<int,int>,int>> getEdgesBuffers();
        int getLongestFIFO();
        int getShortestFIFO();
        int getCost();
        void setRouting(vector<vector<int>> &inR, int arch);
        vector<vector<int>> routing;
        void setPES(int idx, int buffers);
        vector<int> getPES();
        vector<int> PES;
        void writeGrid(Graph &g, int idx, string name);
        void writePES(int idx);
        int getActive();
        void setActive();
        
        int dim;
        int gridSize;
        void checkStrand();
        bool getStrand();
        
    private:
        bool strand = false;
        vector<int> grid;
        vector<int> positions;
        int minNumBuffers;
        int shortestFIFO;
        int longestFIFO;
        int activePES=0;
        vector<int> numberActivePES;
        int cost;
        vector<pair<pair<int,int>,int>> edgesBuffers;
};

Instance::Instance(int* inGrid, int* inPositions, int gridSize, int numNodes, int dim, int cost, int arch){
    this->routing.resize(gridSize);
    this->PES.resize(gridSize,0);
    this->dim = dim;
    this->gridSize = gridSize;
    if(arch == 0) {
        for(int i=0; i<gridSize; i++){
            this->routing[i].resize(4);
        }
    } else if(arch == 1){
        for(int i=0; i<gridSize; i++){
            this->routing[i].resize(8);
        }
    }
    setGrid(inGrid, gridSize);
    setPositions(inPositions, numNodes);
    setCost(cost);
    setMinBufferSize(-1);
    this->numberActivePES.resize(1,0);
    this->shortestFIFO = 10000;
    this->longestFIFO = 0;
}

void Instance::writePES(int idx){
    ofstream ofile;
    ofile.open("heatmap.txt",ios_base::app);

    // if(idx != -1){
    //     for(int i=0; i < this->gridSize; i++){
    //         if(i==this->gridSize-1) ofile << this->PES[i] << "\n";
    //         else if(i%dim == dim-1) ofile << this->PES[i] << "\n";
    //         else ofile << this->PES[i] << " ";
    //     }
    //     ofile << endl;
    // } else ofile << "NULL" << endl;

    if(idx != -1){
        ofile << "[[";
        for(int i=0; i < this->gridSize; i++){
            if(i==this->gridSize-1) ofile << this->PES[i] << "]]\n";
            else if(i%dim == dim-1) ofile << this->PES[i] << "],\n[";
            else ofile << this->PES[i] << ",";
        }
        ofile << endl;
    } else ofile << "NULL" << endl;
}

void Instance::writeGrid(Graph &g, int idx, string name){
    ofstream ofile;
    ofile.open("bestGrids.txt",ios_base::app);
    ofstream ofile1;
    ofile1.open("fifos.txt",ios_base::app);

    if(idx != -1) {
        for(int i=0; i < this->gridSize; i++){
            if(i%dim == dim-1) ofile << this->grid[i] << endl;
            else ofile << this->grid[i] << "\t";
        }
        ofile << endl;
        for(int i=0; i<this->gridSize; i++){
            if(this->grid[i] != -1 && this->PES[i] != 0){
                ofile << g.get_name_node(this->grid[i]) << " has " << this->PES[i] << " buffers." << endl;
            }
        }
        ofile << endl;
        // ofile1 << name << "\t" << this->getLongestFIFO() << endl;
        ofile1 << this->getLongestFIFO() << endl;
    } else ofile << "NULL" << endl;
}

void Instance::setPES(int idx, int buffers){
    if(buffers > this->PES[idx])   this->PES[idx] = buffers;
}

void Instance::setGrid(int *inGrid, int gridSize){
    grid.resize(gridSize);
    for(int i=0; i<gridSize; i++){
        this->grid[i] = inGrid[i];
    }
}

void Instance::setPositions(int* inPositions, int numNodes){
    positions.resize(numNodes);
    for(int i=0; i<numNodes; i++){
        this->positions[i] = inPositions[i];
    }
}

void Instance::insertBufferedEdge(pair<int,int> &edge, int buffers, int numPES){
    this->edgesBuffers.push_back(make_pair(edge,buffers));
    if(buffers > this->minNumBuffers){
        this->minNumBuffers = buffers;
        this->longestFIFO = buffers;
        this->numberActivePES.resize(buffers+1,0);
    }
    if(buffers < this->shortestFIFO) this->shortestFIFO = buffers;

    this->numberActivePES[buffers] += numPES;
}

void Instance::setCost(int cost){
    this->cost = cost;
}

void Instance::setMinBufferSize(int size){
    this->minNumBuffers = size;
}

int Instance::getLongestFIFO(){
    return this->longestFIFO;
}

int Instance::getShortestFIFO(){
    return this->shortestFIFO;
}

int Instance::getCost(){
    return this->cost;
}

int Instance::getActive(){
    return this->activePES;
}

void Instance::setActive(){
    for(int i=0; i<gridSize; i++){
        if(this->PES[i]>0) activePES++;
    }
}

vector<int> Instance::getGrid(){
    return this->grid;
}

vector<int> Instance::getPES(){
    return this->PES;
}

vector<pair<pair<int,int>,int>> Instance::getEdgesBuffers(){
    return this->edgesBuffers;
}

bool Instance::getStrand(){
    return this->strand;
}

void Instance::setRouting(vector<vector<int>> &inR, int arch){
    if(arch == 0) {
        for(int i=0; i<this->routing.size(); i++){
            for(int j=0; j<this->routing[i].size(); j++){
                this->routing[i][j] = inR[i][j];
            }
        }
    } else if(arch == 1){
        for(int i=0; i<this->routing.size(); i++){
            for(int j=0; j<this->routing[i].size(); j++){
                this->routing[i][j] = inR[i][j];
            }
        }
    }    
}

void Instance::checkStrand(){
    vector<bool> A(gridSize, false);
    bool dummy = true;
    for(int i=0; i<this->gridSize; i++){
        if(i/this->dim == dim/2-3||i/this->dim == dim/2-2 || i/this->dim == dim/2-1 || i/this->dim == dim/2 || i/this->dim == dim/2+1) {//|| i/this->dim == dim/2+2){
            A[i] = true;
        }
        if(A[i]==false && this->PES[i]!=0) dummy = false;
    }
    if(dummy) this->strand = true;
}


void printBestSol(Instance A, int arch, int gridSize, int dim){
    if(arch == 0) cout << "Architecture MESH" << endl;
    else if(arch == 1) cout << "Architecture MESH+" << endl;
    else if(arch == 3) cout << "Architecture HEX" << endl;

    // cout << "Grid: " << endl;
    vector<int> inGrid = A.getGrid();
    vector<int> inPES = A.getPES();
    
    // for(int i=0; i<gridSize; i++){
    //     if(i%dim == dim-1) cout << inGrid[i] << endl;
    //     else cout << inGrid[i] << "\t";
    // }
    // vector<pair<pair<int,int>,int>> edges = A.getEdgesBuffers();
    // for(int i=0; i<edges.size(); i++){
    //     cout << "Edge " << edges[i].first.first << " -> " << edges[i].first.second << " needs " << edges[i].second << " buffers per PE" << endl;
    // }
    // cout << endl;
    // cout << "Longest FIFO needed: " << A.getLongestFIFO() << endl << endl;
    
    cout << "Heatmap: " << endl;
        cout << "[[";
        for(int i=0; i < gridSize; i++){
            if(i==gridSize-1) cout << inPES[i] << "]]\n";
            else if(i%dim == dim-1) cout << inPES[i] << "],\n[";
            else cout << inPES[i] << ",";
        }
        cout << endl;

    // for(int i=0; i<gridSize; i++){
    //     if(i%dim == dim-1) cout << inPES[i] << endl;
    //     else cout << inPES[i] << "\t";
    // }
    cout << endl;


    // cout << "Occupied routing resources:" << endl;
    // for(int i=0; i<A.routing.size(); i++){
    //     for(int j=0; j<A.routing[i].size(); j++){
    //         if(A.routing[i][j]!=-1) cout << "PE #" << i << " has edge originating on node " << A.routing[i][j] << " using routing position " << j << endl;
    //     }
    // }
}

void printBestSolClean(Instance A, int arch, int gridSize, int dim){
    vector<int> inGrid = A.getGrid();
    
    for(int i=0; i<gridSize; i++){
        if(i== gridSize-1) cout << inGrid[i] << endl;
        else cout << inGrid[i] << " ";
    }
}

#endif
#ifndef __VPR_H
#define __VPR_H

#include <stdio.h>
#include <fstream>
#include <iostream>

bool verify_size_input(int GRID_SIZE, int nodes) {
    int sum = 0;
    for(int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) {
        if (i == 0 || i == GRID_SIZE-1 || i == GRID_SIZE*(GRID_SIZE-1) || i == GRID_SIZE*GRID_SIZE-1) continue;
        if (i < GRID_SIZE || i > GRID_SIZE*(GRID_SIZE-1) || i%GRID_SIZE==0 || (i+1)%GRID_SIZE==0) sum += __LAYERS;
    }
    return sum >= nodes;
}

void print_vpr(Graph g, string name, const unsigned int NODE_SIZE, 
    const unsigned int GRID_SIZE, int* pos, const int index){

    string name_destiny = "vpr/place/" + name + ".out";
    std::ofstream file(name_destiny);

    file << "Netlist file: vpr/net/"<< name.c_str() <<".net   Architecture file: vpr/arch/k4-n1.xml\n";
    file << "Array size: " << GRID_SIZE-2 << " x " << GRID_SIZE-2 << " logic blocks\n\n";
    file << "#block name	x	y	subblk	block number\n";
    file << "#----------	--	--	------	------------\n";
    int x, y, global_pos;

    map<int,int> count_pos;
    for(int i = 0; i < NODE_SIZE; ++i) {
        global_pos = pos[i+index*NODE_SIZE];
        x = global_pos/GRID_SIZE;
        y = global_pos%GRID_SIZE;
        
        if (count_pos.count(global_pos))
            count_pos[global_pos] += 1;
        else
            count_pos[global_pos] = 0;

        file << g.get_name_node(i) << "\t" << x << "\t" << y << "\t" << count_pos[global_pos] << "\t#" << i << "\n";
        //printf("%s\t%d\t%d\t0\t#%d\n", .c_str(), x, y, i);
    }
    file << "\n";
    //printf("\n");
    file.close();
}

#endif
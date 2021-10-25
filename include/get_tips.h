#ifndef __GET_TIPS_H
#define __GET_TIPS_H


void get_tips(int pos_i, int pos_j, int *grid_freedom, const int GRID_SIZE, int node_type, 
    int *grid_place, int dist_border, vector<pair<int,int>> &aux, int *pos_io_i, 
    int *pos_io_j, int IN_OUT_SIZE) {
    
    aux.resize(0);
    
    int aux_pos, x, y, cost;

    if (dist_border == 0) { // node not closed to IO
        for (int i = 0; i < SIZE_POS_TIPS_AUX; ++i){
            x = pos_i + POS_TIPS_AUX[i][0];
            y = pos_j + POS_TIPS_AUX[i][1];
            aux_pos = x*GRID_SIZE + y;
            if (x > 0 && x < GRID_SIZE && y > 0 && y < GRID_SIZE 
#if __NEIGHBOURHOOD > 0
                && grid_freedom[aux_pos] > 0 
#endif                
                && grid_place[aux_pos] == node_type) {
                
                aux.push_back({grid_freedom[aux_pos],aux_pos});
            }
        }
    } else { // node close to IO
#if __DEBUG == 1
    printf("Pos Border: %d\n", dist_border);
#endif
        for (int i = 0; i < SIZE_POS_TIPS_AUX; ++i){
            x = pos_i + POS_TIPS_AUX[i][0];
            y = pos_j + POS_TIPS_AUX[i][1];
            aux_pos = x*GRID_SIZE + y;
            if (x > 0 && x < GRID_SIZE && y > 0 && y < GRID_SIZE 
#if __NEIGHBOURHOOD > 0
                && grid_freedom[aux_pos] > 0 
#endif
                && grid_place[aux_pos] == node_type) {
                for (int j = 0; j < IN_OUT_SIZE; ++j) {
                    cost = cost_local(x, y, pos_io_i[j], pos_io_j[j], GRID_SIZE);
                    if (cost <= dist_border) {
                        aux.push_back({grid_freedom[aux_pos],aux_pos});
                        break;
                    }
                }
            }
        }
    }
}

#endif
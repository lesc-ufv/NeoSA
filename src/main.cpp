// 1 activated and 0 not activated
#define __DEBUG 0
#define __ARCH 1 // 0 = mesh and 1 = 1hop
#define __CYCLE 1 // annotattion cycle ON=1/OFF=0
#define __LOOKAHEAD 0
#define __MAX_ANNOTATION 2 // limit annotation n, n is distance on graph
#define __NEIGHBOURHOOD -1

// number of layers on the borders
#define __LAYERS 3

#include <stdio.h> 
#include <Graph.h>
#include <print_grid.h>
#include <smart_transversal.h>
#include <get_critical_path.h>
#include <placement.h>
#include <evaluate.h>
#include <list.h>
#include <type_node.h>
#include <placed.h>
#include <vpr.h>
#include <split.h>
#include <buffer.h>
#include <statistic.h>
#include <math.h>
#include <time.h>
#include <chrono>
#include <omp.h>

using namespace chrono;

using Milliseconds = duration<double, ratio<1,1000>>;
    
int main(int argc, char** argv){

  int times = 1;
  int parallel_mode = 0;
  string path_dot = "", name = "";
  vector<string> aux;
  int max_threads = 1;

  if (argc > 3){
    times = stoi(argv[1]);
    parallel_mode = atoi(argv[2]);

    if (parallel_mode == 1) {
      max_threads = omp_get_max_threads(); //get number of threads
      times += (max_threads - times % max_threads) % max_threads;
    }
    path_dot = argv[3];

    aux = split(path_dot, "/");
    name = split(aux[aux.size()-1], ".")[0];
  } else {
    printf("ERROR: anyone dot input!\n Example: ./main <times> <parallel_mode> ../dot/simple.dot\n");
    return EXIT_FAILURE;
  }

  time_point<high_resolution_clock> start_clock, end_clock;
  double time_placement = 0, time_list, time_cp = 0;

  /* initialize random seed: */
  srand (time(NULL)+rand());

  // Constructor Graph
  Graph g(path_dot);

  const unsigned int NODE_SIZE = g.num_nodes();
  const unsigned int EDGE_SIZE = g.num_edges();
  const unsigned int GRID_SIZE = ceil(sqrt(NODE_SIZE-g.get_inputs().size()-g.get_outputs().size())) + 2;
  const unsigned int TOTAL_GRID_SIZE = GRID_SIZE * GRID_SIZE;

  if (!verify_size_input(GRID_SIZE, g.get_inputs().size()+g.get_outputs().size())){
    printf("%s, Not solution for this GRID_SIZE %d\n", name.c_str(), GRID_SIZE);
    return 0;
  }

  vector<pair<int,int>> edges = g.get_edges();

  int* type_node = new int[NODE_SIZE];
  int* type_matrix = new int[GRID_SIZE*GRID_SIZE*__LAYERS];
  
  vector<int> aux_in_out;

  get_type_node(g.get_inputs(), g.get_outputs(), type_node, NODE_SIZE, type_matrix, 
    GRID_SIZE, aux_in_out);

  const int IN_OUT_SIZE = aux_in_out.size();
  int pos_io_i[IN_OUT_SIZE], pos_io_j[IN_OUT_SIZE];
  for (int i = 0; i < IN_OUT_SIZE; ++i) { 
    pos_io_i[i] = aux_in_out[i] / GRID_SIZE;
    pos_io_j[i] = aux_in_out[i] % GRID_SIZE;
  }

  /*
  printf("NODE_SIZE: %u\n", NODE_SIZE);
  printf("EDGE_SIZE: %u\n", EDGE_SIZE);
  printf("GRID_SIZE: %u\n", GRID_SIZE);
  printf("TOTAL_GRID_SIZE: %u\n", TOTAL_GRID_SIZE);
  */

  vector<tuple3> vector_edges;
  vector<map<pair<int, int>,vector<tuple3>>> dic_CYCLE;
  /** generate start sequence:
    0: greedy neihborhood (large fanin/fanout)
    1: betweens centrality 
    2: Critical path
    3: random (ZigZag switch)
    4 or otherwise: zigzag without help preprocessing
  **/
  const int change = 5;
  int *node_degree = new int[times*NODE_SIZE];
  int *list_borders = new int[NODE_SIZE];

  //g.print_graph_number();
#if __NEIGHBOURHOOD > 0
  get_node_degree(g, times, NODE_SIZE, node_degree);
#endif

  create_list_borders(g, NODE_SIZE, GRID_SIZE, list_borders);

  //for (int i = 0; i < NODE_SIZE; ++i) printf("%d %d\n", i, list_borders[i]);

  start_clock = high_resolution_clock::now();
  
  smart_transversal_algorithm(g, edges, NODE_SIZE, vector_edges, dic_CYCLE, 
    change, times);
  //dfs_position_order(g, vector_edges, NODE_SIZE, times, critical_path);
  //bfs_position_order(g, vector_edges, NODE_SIZE, times);
  /**********/
  
  end_clock = high_resolution_clock::now();
  time_list = duration_cast<Milliseconds>(end_clock - start_clock).count();

/*
  for (int i = 0; i < times; ++i) {
    //printf("list %d\n", i);
    for (int j = 0; j < EDGE_SIZE; ++j) {
      printf("%d-%d ", vector_edges[i*EDGE_SIZE+j].first.first, vector_edges[i*EDGE_SIZE+j].first.second);
    }
    printf("\n");
  }
*/
  //g.print_graph_number();

  //printf("time create list: %.2lf\n", time_placement);
  
  int cost = 99999999;

  int *grid_freedom;
  int *pos_i = new int[times*NODE_SIZE];
  int *pos_j = new int[times*NODE_SIZE];
  
  for(int i = 0; i < times*NODE_SIZE; ++i) pos_i[i] = pos_j[i] = -1;
  
  vector<map<pair<int,int>,int>> edges_cost;
  int *vector_cost = new int[times];

  if (parallel_mode == 0) {
    grid_freedom = new int[TOTAL_GRID_SIZE];

    create_freedrom_degree(GRID_SIZE, grid_freedom);

    start_clock = high_resolution_clock::now();
    placement(NODE_SIZE, GRID_SIZE, EDGE_SIZE, 0, times, vector_edges, 
      node_degree, grid_freedom, pos_i, pos_j, type_node, type_matrix, pos_io_i,
      pos_io_j, IN_OUT_SIZE, edges_cost, vector_cost, list_borders, dic_CYCLE);
    end_clock = high_resolution_clock::now();
    
    time_placement = duration_cast<Milliseconds>(end_clock-start_clock).count();
  } else {

    //Init arrays of each thread
    int **grid_freedom_thread = new int*[max_threads];
    vector<map<pair<int,int>,int>> *edges_cost_threads = new vector<map<pair<int,int>,int>>[max_threads];

    for(int i = 0; i < max_threads; i++) {
      grid_freedom_thread[i] = new int[TOTAL_GRID_SIZE];
      create_freedrom_degree(GRID_SIZE, grid_freedom_thread[i]);
    }

    int block_threads = times / max_threads;

    start_clock = high_resolution_clock::now();
    #pragma omp parallel for
    for(int i = 0; i < max_threads; i++){
      placement(NODE_SIZE, GRID_SIZE, EDGE_SIZE, i*block_threads,
        times/max_threads + (i < times % max_threads), vector_edges, 
        node_degree, grid_freedom_thread[i], pos_i, pos_j, type_node, type_matrix, 
        pos_io_i, pos_io_j, IN_OUT_SIZE, edges_cost_threads[i], vector_cost, 
        list_borders, dic_CYCLE);
    }
    end_clock = high_resolution_clock::now();
    time_placement = duration_cast<Milliseconds>(end_clock - start_clock).count();

    for(int i = 0; i < max_threads; i++){
      edges_cost.insert(edges_cost.end(), edges_cost_threads[i].begin(), edges_cost_threads[i].end()); 
    }
  }

  //printf("pior aresta = %d\n", worst_edge);

  //print_grid(pos_i, pos_j, index, NODE_SIZE, GRID_SIZE);

  statistic(g, name, times, type_node, vector_edges, edges_cost, vector_cost);

  buffer(g, GRID_SIZE, edges_cost, pos_i, pos_j);

  printf("%.2lf,%.2lf\n",time_placement,time_list);

  return 0;
}

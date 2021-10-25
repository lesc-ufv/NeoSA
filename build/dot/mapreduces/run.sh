# change number of threads


set -e

GRAPH=(
    # 'tree_n_15_t_1' 
	# 'tree_n_31_t_1' 
	# 'tree_n_63_t_1' 
	# 'tree_n_127_t_1' 
	# 'tree_n_255_t_1' 
	# 'tree_n_15_t_2' 
	# 'tree_n_31_t_2' 
	# 'tree_n_63_t_2' 
	# 'tree_n_127_t_2' 
	# 'tree_n_15_t_3' 
	# 'tree_n_31_t_3' 
	# 'tree_n_63_t_3' 
	# 'tree_n_15_t_4' 
	'tree_n_31_t_4' 
	# 'tree_n_63_t_4'
	# 'tree_n_15_t_1_r_3'
	# 'tree_n_31_t_1_r_3'
	# 'tree_n_63_t_1_r_3'
	# 'tree_n_15_t_2_r_3'
	# 'tree_n_31_t_2_r_3'
	# 'tree_n_15_t_3_r_3'
	# 'tree_n_31_t_3_r_3'
	# 'tree_n_15_t_4_r_3'
)

for ((i = 0; i < ${#GRAPH[@]}; ++i)) do
    echo ${GRAPH[i]}".dot"
	./a.out ${GRAPH[i]}".dot" #>> "results/res.out" #> "results/"${GRAPH[i]}".out"
done


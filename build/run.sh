# change number of threads
NUM_THREADS=16

export OMP_NUM_THREADS=$NUM_THREADS

set -e

# GRAPH=(
#     # 'Tree_n_15_t_1' 
# 	# 'Tree_n_31_t_1' 
# 	# 'Tree_n_63_t_1' 
# 	# 'Tree_n_127_t_1' 
# 	# 'Tree_n_15_t_2' 
# 	# 'Tree_n_31_t_2' 
# 	# 'Tree_n_63_t_2'  
# 	# 'Tree_n_15_t_3' 
# 	# 'Tree_n_31_t_3' 
# 	# 'Tree_n_15_t_4' 
# 	# 'Tree_n_31_t_4' 
# 	# 'matmul22'
# 	# 'matmul33' 
# 	# 'matmul44' 
# 	# 'matmul55' 
# 	# 'matmul66' 
# 	# 'matmul22normal' 
# 	# 'convolution22' 
# 	# 'convolution33' 
# 	# 'convolution44' 
# 	# 'convolution55' 
# 	# 'tree_n_15_t_1_r_3'
# 	# 'tree_n_31_t_1_r_3'
# 	# 'tree_n_63_t_1_r_3'
# 	# 'tree_n_15_t_2_r_3'
# 	# 'tree_n_31_t_2_r_3'
# 	# 'tree_n_15_t_3_r_3'
# 	# 'tree_n_15_t_4_r_3'
# 	# 'K4N4' 
# 	# 'K4N5' 
# 	# 'K4N6' 
# 	'K4N7' 
# 	# 'K4N8' 
# 	# 'K5N4' 
# 	# 'K6N4' 
# 	'K7N4' 
# 	'K8N4'	
# )

GRAPH=(
    mac
    simple
    horner_bs
    mults1
    arf
    conv3
    motion_vec
    fir2
    fir1
    fdback_pts
    k4n4op
    h2v2_smo
    cosine1
    ewf
    Cplx8
    Fir16
    cosine2
    FilterRGB
    collapse_pyr
    interpolate
    w_bmp_head
    matmul
    invert_matrix
)

for ((i = 0; i < ${#GRAPH[@]}; ++i)) do
    echo ${GRAPH[i]}".dot"
	./main "dot/"${GRAPH[i]}".dot" ${GRAPH[i]} $i >> results.csv #>> "results/res.out" #> "results/"${GRAPH[i]}".out"
done


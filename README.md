# Histogram hybrid parallel computation with MPI OpenMP and CUDA
Program contains 2 processes - primary and replica\
Primary process reads array of integers in range [0,255] from file data.txt, keep half of the array and sends the other half to replica\
Both processes calculates the histogram with both OpenMP and CUDA, giving half of it's part to each.

# OpenMP
Each process creates a matrix of size number_of_omp_threads x 256 (histogram size).\
In the omp section, the array part for omp is divided equally between threads which calculates an histogram for its part and insert it's results to the matrix based on it's id.

# CUDA 
Each process creates a matrix of size number_of_cuda_blocks x 256 (histogram size).\
The array part of cuda is divided equally between the blocks, when each block contains 256 threads.\
Each thread is responsible for one value - for each appearence of the value in the block's part of the array the thread raises the counter of it's value entering result to the matrix based on block id (which row) and thread id (which column).

# Termination
The Replica proccess calculates a result histogram by summing all OpenMP threads results and all CUDA blocks results, then send the histogram to primary process.\
Primary process calculates final histogram by adding the sums to the result histogram received from replica.\
Finally, the primary process test the results by comparing with sequential calculation.

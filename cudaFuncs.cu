/*
 ============================================================================
 Name        : cudaFunction.cu
 Author      : Miriam Assraf
 Description : histogram calculation with CUDA
 ============================================================================
 */

#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <stdio.h>
#include "cudaFuncs.h"
#include "constants.h"

__global__ void histogramKernel(const int* A, int* results, int size, int num_blocks)
{
    int block = blockIdx.x;
    int thread = threadIdx.x;

    int start = (block * size) / num_blocks;
    int end = ((block + 1) * size) / num_blocks;

    for (int i = start; i < end; i++) {
        if (thread < HISTO_SZ) {
            if (A[i] == thread) {
                results[block * HISTO_SZ + thread]++;	// increase value thread (in range 0-255) for current block
            }
        }
    }
}

__host__ void checkErrors(cudaError_t err, const char *error_msg)
{
	if (err != cudaSuccess) {
			fprintf(stderr, error_msg, cudaGetErrorString(err));
			exit(EXIT_FAILURE);
		}
}

int histogramWithCuda(const int* A, int* results, int size, int threadsPerBlock, int blocksPerGrid)
{
	cudaError_t err = cudaSuccess; // Error code to check return values for CUDA calls
	int* dev_A = 0;
    	int* dev_results = 0;

	// Allocate memory on GPU to copy the data from the host
   	err = cudaMalloc((void**)&dev_A, size * sizeof(int));
    	checkErrors(err, "Failed to allocate device memory - %s\n");

	// Copy data from host to the GPU memory
	err = cudaMemcpy(dev_A, A, size * sizeof(int), cudaMemcpyHostToDevice);
	checkErrors(err, "Failed to copy data from host to device - %s\n");
	
	// Allocate memory on GPU for results to send to host
	err =cudaMalloc((void**)&dev_results, blocksPerGrid* HISTO_SZ * sizeof(int));	// each block check on it's part for values 0-255
	checkErrors(err, "Failed to allocate device memory - %s\n");

	cudaMemset(dev_results, 0, blocksPerGrid * HISTO_SZ * sizeof(int));	// initialize results array with zeros

	// Launch the Kernel
	histogramKernel << <blocksPerGrid, threadsPerBlock >> > (dev_A, dev_results, size, blocksPerGrid);
	err = cudaGetLastError();
	checkErrors(err, "Failed to launch histogram kernel -  %s\n");

	// Copy the  result from GPU to the host memory.
	err = cudaMemcpy(results, dev_results, blocksPerGrid*256 * sizeof(int), cudaMemcpyDeviceToHost);
	checkErrors(err, "Failed to copy result array from device to host -%s\n");
	
	// Free allocated memory on GPU
	err = cudaFree(dev_A);
	checkErrors(err, "Failed to free device data - %s\n");

	err = cudaFree(dev_results);
	checkErrors(err, "Failed to free device results - %s\n");

	return 0;
}


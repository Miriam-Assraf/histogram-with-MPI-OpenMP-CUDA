/*
 ===========================================================================================
 Author      : Miriam Assraf
 Description : Calculation functions with OpenMP and CUDA, and final result for both
 ===========================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <omp.h>
#include "calcHisto.h"
#include "cudaFuncs.h"
#include "constants.h"

int* calcWithOpenMP(int *A, int part, int num_threads)
{
	// initiate results for omp
		int tid;
		int omp_part = part / 2;	// half of part with openmp
		int *omp_results = (int*) malloc(num_threads * HISTO_SZ * sizeof(int));
		memset(omp_results, 0, num_threads * HISTO_SZ * sizeof(int));

		// each process perform the first half of its task with OpenMP
		#pragma omp parallel private(tid) num_threads(num_threads)
		{
			tid = omp_get_thread_num();

			int start = tid * (omp_part / num_threads);	// each thread start after the part of last one
			int end = (tid + 1) * (omp_part / num_threads);

			for (int i = start; i < end; i++) {
				for (int j = 0; j < HISTO_SZ; j++) {
					if (A[i] == j) {
						omp_results[tid * HISTO_SZ + j] += 1;
					}
				}
			}

		}

		return omp_results;
}

int* calcWithCuda(int *A, int *blocksPerGrid, int part)
{
	// initiate results for cuda
		int cuda_part = part / 2;	// half of part with cuda
		int threadsPerBlock = HISTO_SZ;
		*blocksPerGrid = (cuda_part + threadsPerBlock - 1) / threadsPerBlock;
		int *cuda_results = (int*) malloc(*blocksPerGrid * HISTO_SZ * sizeof(int));
		// each process perform the second half of its task with CUDA
		if (histogramWithCuda(A + part/2, cuda_results, cuda_part,
				threadsPerBlock, *blocksPerGrid) != 0)	// the other half for cuda
			MPI_Abort(MPI_COMM_WORLD, __LINE__);

		return cuda_results;
}

void calcResult(int num_threads, int blocksPerGrid, int result[], int *omp_results, int *cuda_results)
{
	for (int i = 0; i < num_threads; i++) {
		for (int j = 0; j < HISTO_SZ; j++) {
			result[j] += omp_results[i * HISTO_SZ + j];
		}
	}
	for (int i = 0; i < blocksPerGrid; i++) {
		for (int j = 0; j < HISTO_SZ; j++) {
			result[j] += cuda_results[i * HISTO_SZ + j];
		}
	}

	for (int i = 0; i < HISTO_SZ; i++) {
				printf("%d - %d\n", i, result[i]);
			}
}

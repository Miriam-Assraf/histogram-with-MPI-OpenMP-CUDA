/*
 ===========================================================================================
 Name        : main.c
 Author      : Miriam Assraf
 Description : Histogram calculation with hybrid MPI, OpenMP and CUDA parallel programming
 ===========================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include "calcHisto.h"
#include "test.h"
#include "constants.h"

void checkNumProcs(int size, int n);
int* readFile(int *n);
void collectAndFinish(int *A, int *result, int *omp_results, int *cuda_results, int n, int num_threads, int blocksPerGrid, int rank);

int main(int argc, char *argv[]) {
	int *A;
	int n;
	int part;
	int size, rank;
	int result[HISTO_SZ] = { 0 };
	MPI_Status status;
	// initialize MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	checkNumProcs(size, 2);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// process 0 read array from file and divide half between the both processes
	if (rank == PRIMARY) {
		A = readFile(&n);
		// send n and half of the array to process 1
		part = n / 2;
		MPI_Send(&part, 1, MPI_INT, REPLICA, 0, MPI_COMM_WORLD);
		MPI_Send(A + part, part, MPI_INT, REPLICA, 0, MPI_COMM_WORLD);
	}

	else {
		// receive part to allocate memory
		MPI_Recv(&part, 1, MPI_INT, PRIMARY, 0, MPI_COMM_WORLD, &status);
		A = (int*) malloc(part * sizeof(int));
		// receive half of array
		MPI_Recv(A, part, MPI_INT, PRIMARY, 0, MPI_COMM_WORLD, &status);
	}

	// each process perform the first half of its task with OpenMP
	int num_threads = 4;
	int *omp_results = calcWithOpenMP(A, part, num_threads);

	// each process perform the second half of its task with CUDA
	int blocksPerGrid;
	int *cuda_results = calcWithCuda(A, &blocksPerGrid, part);

	collectAndFinish(A, result, omp_results, cuda_results, n, num_threads, blocksPerGrid, rank);

	MPI_Finalize();
	return 0;
}

void checkNumProcs(int size, int n)
{
	if (size != n) {
			printf("Run with two processes only\n");
			MPI_Abort(MPI_COMM_WORLD, __LINE__);
		}
}

int* readFile(int *n)
{
	FILE *dataFile = fopen("data.txt", "r");

	if (dataFile == NULL) {
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}
	// read size of array to n
	fscanf(dataFile, "%d", n);
	// allocate n integers
	int *A = (int*) malloc(*n * sizeof(int));

	// read the rest n integers to A
	int i = 0;
	while (fscanf(dataFile, "%d", &A[i]) != EOF) {
		i++;
	}
	fclose(dataFile);
	return A;
}

void collectAndFinish(int *A, int *result, int *omp_results, int *cuda_results, int n, int num_threads, int blocksPerGrid, int rank)
{
	MPI_Status status;

	// process zero collects results
		if (rank == PRIMARY) {
			MPI_Recv(result, HISTO_SZ, MPI_INT, REPLICA, 0, MPI_COMM_WORLD, &status);
			calcResult(num_threads, blocksPerGrid, result, omp_results,
					cuda_results);

			test(A, result, n);
			free(A);
			free(omp_results);
			free(cuda_results);

		} else {	// process 1 send it's results
			calcResult(num_threads, blocksPerGrid, result, omp_results,
					cuda_results);
			MPI_Send(result, HISTO_SZ, MPI_INT, PRIMARY, 0, MPI_COMM_WORLD);
		}

}

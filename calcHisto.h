#pragma once
int* calcWithOpenMP(int* A, int part, int num_threads);
int* calcWithCuda(int* A, int* blocksPerGrid, int part);
void calcResult(int num_threads, int blocksPerGrid, int result[], int* omp_results, int* cuda_results);
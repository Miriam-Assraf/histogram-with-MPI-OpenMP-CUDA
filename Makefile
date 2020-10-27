build:
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -c calcHisto.c -o calcHisto.o
	mpicxx -fopenmp -c test.c -o test.o
	nvcc -I./inc -c cudaFuncs.cu -o cudaFuncs.o
	mpicxx -fopenmp -o calcHistoParallelHybrid  main.o calcHisto.o test.o cudaFuncs.o  /usr/local/cuda-9.1/lib64/libcudart_static.a -ldl -lrt

clean:
	rm -f *.o ./calcHistoParallelHybrid

run:
	mpiexec -np 2 ./calcHistoParallelHybrid


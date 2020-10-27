/*
 ===========================================================================================
 Author      : Miriam Assraf
 Description : Test histogram calculation comparing with sequential result
 ===========================================================================================
 */
#include <stdio.h>
#include "test.h"
#include "constants.h"

void test(int* A, int* result, int n)
{
	int test[HISTO_SZ] = { 0 };
	int count = 0;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < HISTO_SZ; j++) {
			if (A[i] == j) {
				test[j]++;
			}
		}
	}
	for (int i = 0; i < HISTO_SZ; i++) {
		if (test[i] != result[i]) {
			printf("Wrong Calculations - Failure of the test at result %d-%d!=%d\n", i, test[i], result[i]);

			return;
		}
	}
	printf("\nThe test passed successfully!\n");

}


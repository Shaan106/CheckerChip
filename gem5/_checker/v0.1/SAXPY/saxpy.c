#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define NUM_THREADS 8

int main() {
    float a = 2.0f;
    float *x, *y;

    // Allocate memory on the heap
    x = (float *)malloc(N * sizeof(float));
    y = (float *)malloc(N * sizeof(float));

    if (x == NULL || y == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    // Initialize vectors
    for (int i = 0; i < N; i++) {
        x[i] = i * 1.0f;
        y[i] = i * 2.0f;
    }

    // Set the number of threads
    omp_set_num_threads(NUM_THREADS);

    // SAXPY operation using OpenMP
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        y[i] = a * x[i] + y[i];
    }

    // Optional: verify results or output
    printf("Completed SAXPY computation (v2).\n");

    // Free allocated memory
    free(x);
    free(y);

    return 0;
}

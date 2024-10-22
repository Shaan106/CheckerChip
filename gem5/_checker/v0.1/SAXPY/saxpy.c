#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000 //number of operations to do 
#define NUM_THREADS 8 //how many threads

int main() {
    float a = 2.0f; //constant
    float *x, *y; // x,y pointers

    //alloc mem
    x = (float *)malloc(N * sizeof(float));
    y = (float *)malloc(N * sizeof(float));

    if (x == NULL || y == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    //initialise x and y
    for (int i = 0; i < N; i++) {
        x[i] = i * 1.0f;
        y[i] = i * 2.0f;
    }

    //num threads to run on
    omp_set_num_threads(NUM_THREADS);

    //OpenMP parallelism
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        y[i] = a * x[i] + y[i];
    }

    //output when complete
    printf("Completed SAXPY computation (v2).\n");

    // free the stuff
    free(x);
    free(y);

    return 0;
}

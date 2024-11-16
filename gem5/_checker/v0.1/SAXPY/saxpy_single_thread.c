#include <stdio.h>
#include <stdlib.h>

#define N 1000 // number of operations to do

int main() {
    float a = 2.0f; // constant
    float *x, *y; // x, y pointers

    // allocate memory
    x = (float *)malloc(N * sizeof(float));
    y = (float *)malloc(N * sizeof(float));

    if (x == NULL || y == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    // initialize x and y
    for (int i = 0; i < N; i++) {
        x[i] = i * 1.0f;
        y[i] = i * 2.0f;
    }

    // Perform SAXPY computation sequentially
    for (int i = 0; i < N; i++) {
        y[i] = a * x[i] + y[i];
    }

    // output when complete
    printf("Completed SAXPY computation (sequential).\n");

    // free the memory
    free(x);
    free(y);

    return 0;
}

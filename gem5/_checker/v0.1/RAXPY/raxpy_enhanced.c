#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000          // number of operations
#define NUM_THREADS 8   // number of threads
#define INT_OP_RATIO 80 // integer operation ratio (80% int, 20% float)

// Function to generate a random index in the range [0, N-1]
int random_index(int limit) {
    return rand() % limit;
}

// Function to randomly choose an integer or floating-point operation
int choose_op() {
    return rand() % 100 < INT_OP_RATIO; // 80% chance for integer operation
}

// Function to simulate complex operations (e.g., matrix multiplication)
void complex_operations(int idx_x, int idx_y, float *x, float *y, int *z) {
    // Floating point operations: Example complex combination
    float val = x[idx_x] * y[idx_y] + sinf(x[idx_x]) - cosf(y[idx_y]);
    y[idx_y] = sqrtf(fabs(val));  // Sqrt of absolute value for complexity
    if (idx_x % 2 == 0) {
        y[idx_y] *= 1.5f;  // Scale on even indices
    } else {
        y[idx_y] /= 2.0f;  // Divide on odd indices
    }
}

// Function for integer-based random operations
void integer_operations(int idx_x, int idx_y, int *z, float a, float *x) {
    // Integer operation
    int int_val = (int)(a * x[idx_x]) + z[idx_y];
    int_val = int_val % 100;  // Modulo operation

    if (idx_x % 2 == 0) {
        z[idx_y] += 10;  // Add 10 on even indices
    } else {
        z[idx_y] -= 10;  // Subtract 10 on odd indices
    }
    z[idx_y] = int_val;
}

int main() {
    float a = 2.0f; // constant
    float *x, *y;   // x, y pointers
    int *z;         // integer array for integer-based operations

    // Allocating memory
    x = (float *)malloc(N * sizeof(float));
    y = (float *)malloc(N * sizeof(float));
    z = (int *)malloc(N * sizeof(int));

    if (x == NULL || y == NULL || z == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    // Initialize x, y, and z with some random values
    for (int i = 0; i < N; i++) {
        x[i] = (float)(rand() % 100);  // Random values between 0 and 99
        y[i] = (float)(rand() % 100);  // Random values
        z[i] = rand() % 100;           // Random integer values
    }

    // Set the number of threads to use
    omp_set_num_threads(NUM_THREADS);

    // Parallel computation
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        // Generate random indices for x, y to simulate non-contiguous access
        int idx_x = random_index(N);
        int idx_y = random_index(N);
        
        // Mixed operations: floating point vs integer
        if (choose_op()) {
            // Perform complex floating point operations
            complex_operations(idx_x, idx_y, x, y, z);
        } else {
            // Perform integer operations
            integer_operations(idx_x, idx_y, z, a, x);
        }
    }

    // Output when computation is complete
    printf("Completed mixed operations with random access and task-based parallelism.\n");

    // Free memory
    free(x);
    free(y);
    free(z);

    return 0;
}

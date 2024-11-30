#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000          // number of operations to do 
#define NUM_THREADS 8   // number of threads
#define INT_OP_RATIO 80 // integer operation ratio (20% int, 80% float)

// Function to generate a random index in the range [0, N-1]
int random_index(int limit) {
    return rand() % limit;
}

// Function to randomly choose an integer or floating-point operation
int choose_op() {
    return rand() % 100 < INT_OP_RATIO; // 20% chance to return 1 for integer, 80% for float
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

    // Initialize x and y with some random values
    for (int i = 0; i < N; i++) {
        x[i] = (float)(rand() % 100); // Random values between 0 and 99
        y[i] = (float)(rand() % 100); // Random values between 0 and 99
        z[i] = rand() % 100;          // Random integer values for int operations
    }

    // Set the number of threads to use
    omp_set_num_threads(NUM_THREADS);

    // OpenMP parallelism with random access pattern and mixed operations
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        // Generate random indices for x and y to avoid predictable memory access patterns
        int idx_x = random_index(N);
        int idx_y = random_index(N);
        
        // Decide whether to perform a floating-point or integer operation
        if (choose_op()) {
            // Floating-point operation (SAXPY + additional functions)
            float val = a * x[idx_x] + y[idx_y];  // SAXPY operation
            float sin_val = sinf(val);              // sine function
            float cos_val = cosf(val);              // cosine function
            float sqrt_val = sqrtf(val);            // square root function

            // Combine operations to ensure different functional units are used
            y[idx_y] = sin_val + cos_val + sqrt_val;

            // Random branching for further diversity
            if (idx_x % 2 == 0) {
                y[idx_y] *= 2.0f; // Multiply by 2 on even indices
            } else {
                y[idx_y] /= 2.0f; // Divide by 2 on odd indices
            }
        } else {
            // Integer operation (example: adding random integer and then performing modulus)
            int int_val = (int)(a * x[idx_x]) + z[idx_y];  // Integer-based operation
            int_val = int_val % 100;  // Modulus operation
            z[idx_y] = int_val;

            // Random branching for integer operations
            if (idx_x % 2 == 0) {
                z[idx_y] += 10;  // Add 10 on even indices
            } else {
                z[idx_y] -= 10;  // Subtract 10 on odd indices
            }
        }
    }

    // Output when computation is complete
    printf("Completed SAXPY computation with random access, mixed integer/floating-point operations.\n");

    // Free memory
    free(x);
    free(y);
    free(z);

    return 0;
}

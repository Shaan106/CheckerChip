#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define N 1000  // Matrix size (N x N)
#define THREADS 8  // Number of threads

// Function to multiply two matrices A and B, storing the result in C
void matrix_multiply(double *A, double *B, double *C, int n) {
    int i, j, k;

    // Parallelize the matrix multiplication
    #pragma omp parallel for private(i, j, k) num_threads(THREADS)
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double sum = 0.0;
            for (k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * n + j];  // Floating-point operation
            }
            C[i * n + j] = sum;
        }
    }
}

// Function to initialize the matrices A, B, and C
void initialize_matrices(double *A, double *B, double *C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i * n + j] = rand() % 100;  // Integer assignment (random values)
            B[i * n + j] = rand() % 100;  // Integer assignment (random values)
            C[i * n + j] = 0.0;  // Initialize C to 0
        }
    }
}

// Function to print the result matrix (optional for large matrices)
void print_matrix(double *C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%f ", C[i * n + j]);
        }
        printf("\n");
    }
}

int main() {
    srand(time(NULL));  // Seed for random number generation

    double *A, *B, *C;
    A = (double *)malloc(N * N * sizeof(double));
    B = (double *)malloc(N * N * sizeof(double));
    C = (double *)malloc(N * N * sizeof(double));

    if (A == NULL || B == NULL || C == NULL) {
        printf("Memory allocation failed!\n");
        return -1;
    }

    // Initialize matrices A, B, and C
    initialize_matrices(A, B, C, N);

    // Start timing the matrix multiplication
    double start_time = omp_get_wtime();

    // Perform matrix multiplication
    matrix_multiply(A, B, C, N);

    // End timing
    double end_time = omp_get_wtime();

    printf("Matrix multiplication completed.\n");
    printf("Time taken: %f seconds\n", end_time - start_time);

    // Optionally print the result matrix (caution for large matrices)
    // print_matrix(C, N);

    // Free allocated memory
    free(A);
    free(B);
    free(C);

    return 0;
}

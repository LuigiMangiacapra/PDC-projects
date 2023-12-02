# include <stdio.h>
# include <stdlib.h>
# include <omp.h>
# include <time.h>
# include <sys/time.h>

# include "Lib.h"

void matxvet(double *A, int N, int M, double *x, double *b, int n_threads){
    int i,j;
    #pragma omp parallel for default(none) shared(N, M, A, x, b) private(i, j)
    // prodotto matrice vettore
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            b[i] += A[i * M + j] * x[j];
        }
        printf("Hello from thread %d, nthreads %d\n", omp_get_thread_num(), omp_get_num_threads());
    }
    //
}

void initialize_matrix(double **A, int N, int M) {
    *A = (double *)calloc(N * M, sizeof(double));

    if (*A == NULL) {
        fprintf(stderr, "Error allocating memory for the 'matrix' array!\n");
        exit(EXIT_FAILURE);
    }

    // fill matrix randomly
    fill_matrix(*A, N, M);
}


void fill_matrix(double *A, int rows, int cols){
    srand(time(NULL));

    int index = 0, value = 0;
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++)
            A[index++] = ((double) rand() / RAND_MAX) * (100 - 1) + 1;
    }
}


void print_matrix(double *matrix, int rows, int cols) {
    printf("[");
    for (int i = 0; i < rows; i++) {
        (i == 0) ? printf(" ") : printf("  "); 
        for (int j = 0; j < cols; j++) {
            // Calcola l'indice nell'array
            int index = i * cols + j;

            printf("%2f", matrix[index]);

            // Aggiungi la virgola se non è l'ultimo elemento della riga
            if (j < cols - 1) {
                printf(", ");
            }
        }

        // Aggiungi il punto e virgola se non è l'ultima riga
        if (i < rows - 1) {
            printf("\n");
        }
    }
    printf(" ]\n");
}


void read_input(int argc, char **argv, int *N, int *M, int *n_threads){
    if(argc != 4){
        fprintf(stderr, "Usage: <rows matrix> <column matrix> <n. threads>\n");
        fprintf(stderr, "Exit from program...");
        exit(EXIT_FAILURE);
    }

    *N = atoi(argv[1]);
    *M = atoi(argv[2]);
    *n_threads = atoi(argv[3]);

    if (*N < 1 || *M < 1){
        fprintf(stderr, "Usage: <N> (risp. <M>) must be greater or equal than 1!\nExit from program...\n");
        exit(EXIT_FAILURE);
    }

    if(*n_threads < 1){
        fprintf(stderr, "Usage: <n_threads> must be greater or equal than 1!\nExit from program...\n");
        exit(EXIT_FAILURE);
    }
}


void initialize_array(double **array, int size){
    // array malloc
    *array = (double *)calloc(sizeof(double), size);
    if(*array == NULL){
        fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
        exit(EXIT_FAILURE);
    }
}


void fill_array(double *array, int size){
    srand(time(NULL));

    for(int i = 0; i < size; i++){
        array[i] = ((double) rand() / RAND_MAX) * (100 - 1) + 1;
    }
}


void print_array(double *array, int size){
    printf("Vettore: [");
    for(int i = 0; i < size; i++)
        printf("%f ", array[i]);
    printf("]\n");
}
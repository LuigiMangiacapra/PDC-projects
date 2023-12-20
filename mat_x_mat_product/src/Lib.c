/**
 * @author Fabrizio Vitale
 * @author Giovanni Falcone
 * @author Luigi Mangiacapra
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

# include "mpi.h"


void read_input(int argc, char **argv, int *N){
    if(argc != 2){
        fprintf(stderr, "Usage: <number of row and column>\n");
        fprintf(stderr, "Exit from program...");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }

    *N = atoi(argv[1]);

    if (*N < 1){
        fprintf(stderr, "Usage: <N> must equal or greater than 1\n");
        fprintf(stderr, "Exit from program...");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
}

void check_if_grid_can_be_created(int nproc){
    double root = sqrt(nproc);

    if(root != (int) root){
        fprintf(stdout, "Grid cannot be created!\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
}

void initialize_matrix(int **matrix, int N) {
    *matrix = (int *)calloc(N * N, sizeof(int));

    if (*matrix == NULL) {
        fprintf(stderr, "Error allocating memory for the 'matrix' array!\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
}

void print_matrix(int *matrix, int N) {
    printf("[");
    for (int i = 0; i < N; i++) {
        (i == 0) ? printf(" ") : printf("  "); 
        for (int j = 0; j < N; j++) {
            // Calcola l'indice nell'array
            int index = i * N + j;

            printf("%2d", matrix[index]);

            // Aggiungi la virgola se non è l'ultimo elemento della riga
            if (j < N - 1) {
                printf(", ");
            }
        }

        // Aggiungi il punto e virgola se non è l'ultima riga
        if (i < N - 1) {
            printf("\n");
        }
    }
    printf(" ]\n");
}

void fill_matrix(int *matrix, int N){
    int index = 0;
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++)
            matrix[index++] = rand() % 10 + 1;
    }

    printf("---------------------------------------------\n--- \t\t Matrix \t\t  --- \n"
               "---------------------------------------------\n");
    print_matrix(matrix, N);
    printf("---------------------------------------------\n");
}

void print_array(int *array, int size){
    printf("(0) displs: [ ");
    for (int i = 0; i < size; i++) 
        printf("%d ", array[i]);
    printf("]\n\n");
}

void create_grid (MPI_Comm *griglia, MPI_Comm *grigliar, MPI_Comm *grigliac, 
                     int menum, int nproc, int riga, int col, int *coordinate) {
    int dim = 2, ndim[2], reorder, period[2], vc[2];

    ndim[0] = riga;
    ndim[1] = col;
    period[0] = period [1] = 0;
    reorder = 0;

    // definizione griglia
    MPI_Cart_create(MPI_COMM_WORLD, dim, ndim, period, reorder, griglia);
    
    // ciascun processo calcola le proprie coordinate
    MPI_Cart_coords (*griglia, menum, 2, coordinate);

    // divisione in righe del communicator
    vc[0] = 0;
    vc[1] = 1;
    MPI_Cart_sub(*griglia, vc, grigliar);

    // divisione in colonne del communicator
    vc[0] = 1;
    vc[1] = 0;
    MPI_Cart_sub(*griglia, vc, grigliac);

    return;
}

void get_offset(int *displs, int row_grid, int col_grid, int n_loc, int N){
    int offset = 0;

    for (int i = 0; i < row_grid; i++) {
        for (int j = 0; j < col_grid; j++) {
            displs[i * col_grid + j] = i * N * n_loc + j * n_loc;
        }
    }
}

void matrix_distribution(int nproc, int *matrix, int *elements_loc, int *displs, int n_loc, int block_size, int stride){
    MPI_Datatype vectorType, block_Type;            // blocco 

    MPI_Type_vector(n_loc, n_loc, stride, MPI_INT, &vectorType);
    MPI_Type_create_resized(vectorType, 0, sizeof(int), &block_Type);
    MPI_Type_commit(&block_Type);

    int s[nproc];
    for(int i = 0; i < nproc; i++) s[i] = 1;

    MPI_Scatterv(matrix, s, displs, block_Type, elements_loc, block_size, MPI_INT, 0, MPI_COMM_WORLD);

    // free mpi types
    MPI_Type_free(&vectorType);
    MPI_Type_free(&block_Type);
}

void mat_product(int *A, int *B, int *C, int dim){
    for (int i = 0; i < dim; i++){ 
        for (int k = 0; k < dim; k++){ 
            for (int j = 0; j < dim; j++){ 
                C[i * dim + k] += A[i * dim + j] * B[j * dim + k];
            }
        }
    }
}

void broadcastMultiplyRolling(int *A_loc, int *B_loc, int *C_loc, int block_loc, MPI_Comm colSub, int menum, int nproc){
    MPI_Status status;

    // Broadcasting delle matrici locali
    for (int i = 0; i < block_loc; i++){
        MPI_Bcast(&A_loc[i], block_loc, MPI_INT, i % (int)sqrt(nproc), colSub);
    }

    // Moltiplicazione delle matrici locali
    printf("ok prima\n");
    mat_product(A_loc, B_loc, C_loc, block_loc);
    printf("ok dopo\n");

    // Rolling delle colonne
    for (int i = 0; i < block_loc; i++){
        int targetRank = (menum + i) % nproc;

        // Invia la colonna di BLoc a targetRank
        MPI_Sendrecv_replace(&B_loc[i], block_loc, MPI_INT, targetRank, i, targetRank, i, colSub, &status);
    }
    printf("ok fine\n");
}
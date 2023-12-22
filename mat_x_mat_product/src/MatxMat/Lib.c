/**
 * @author Fabrizio Vitale
 * @author Giovanni Falcone
 * @author Luigi Mangiacapra
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "mpi.h"

void read_input(int argc, char **argv, int *N)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: <number of row and column>\n");
        fprintf(stderr, "Exit from program...");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }

    *N = atoi(argv[1]);

    if (*N < 1)
    {
        fprintf(stderr, "Usage: <N> must equal or greater than 1\n");
        fprintf(stderr, "Exit from program...");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
}

void check_if_grid_can_be_created(int nproc){
    double root = sqrt(nproc);

    if (root != (int)root){
        fprintf(stdout, "Grid cannot be created!\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
}

void initialize_matrix(double **matrix, int N){
    *matrix = (double *)calloc(N * N, sizeof(double));

    if (*matrix == NULL){
        fprintf(stderr, "Error allocating memory for the 'matrix' array!\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
}

void print_matrix(double *matrix, int N){
    printf("[");
    for (int i = 0; i < N; i++){
        (i == 0) ? printf(" ") : printf("  ");
        for (int j = 0; j < N; j++){
            // computes the index of the array
            int index = i * N + j;

            printf("%.2lf", matrix[index]);

            // adds the comma if it's not the last element of the row
            if (j < N - 1){
                printf(", ");
            }
        }

        // adds the semicolon if it's not the last row
        if (i < N - 1){
            printf("\n");
        }
    }
    printf(" ]\n");
}

void fill_matrix(double *matrix, int N){
    int index = 0;

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++)
            matrix[index++] = rand() % 10 + 1;//2;
    }

    printf("---------------------------------------------\n--- \t\t Matrix \t\t  --- \n"
           "---------------------------------------------\n");
    print_matrix(matrix, N);
    printf("---------------------------------------------\n");
}

void print_array(double *array, int size){
    printf("(0) displs: [ ");
    for (int i = 0; i < size; i++)
        printf("%2f ", array[i]);
    printf("]\n\n");
}

void create_grid(MPI_Comm *griglia, MPI_Comm *grigliar, MPI_Comm *grigliac, int menum, int nproc, int riga, int col, int *coordinate){
    int dim = 2, ndim[2], reorder, period[2], vc[2];

    ndim[0] = riga;
    ndim[1] = col;
    period[0] = period[1] = 0;
    reorder = 0;

    // definizione griglia
    MPI_Cart_create(MPI_COMM_WORLD, dim, ndim, period, reorder, griglia);

    // ciascun processo calcola le proprie coordinate
    MPI_Cart_coords(*griglia, menum, 2, coordinate);

    // divisione in righe del communicator
    vc[0] = 0;
    vc[1] = 1;
    MPI_Cart_sub(*griglia, vc, grigliar);

    // divisione in colonne del communicator
    vc[0] = 1;
    vc[1] = 0;
    MPI_Cart_sub(*griglia, vc, grigliac);
}

void get_offset(int *displs, int row_grid, int col_grid, int n_loc, int N){
    int offset = 0;

    for (int i = 0; i < row_grid; i++){
        for (int j = 0; j < col_grid; j++){
            displs[i * col_grid + j] = i * N * n_loc + j * n_loc;
        }
    }
}

void matrix_distribution(int nproc, double *matrix, double *elements_loc, int *displs, int n_loc, int stride){
    MPI_Datatype vectorType, block_Type; // blocco

    MPI_Type_vector(n_loc, n_loc, stride, MPI_DOUBLE, &vectorType);
    MPI_Type_create_resized(vectorType, 0, sizeof(double), &block_Type);
    MPI_Type_commit(&block_Type);

    int s[nproc];
    for (int i = 0; i < nproc; i++) s[i] = 1;

    MPI_Scatterv(matrix, s, displs, block_Type, elements_loc, n_loc * n_loc, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // free mpi types
    MPI_Type_free(&vectorType);
    MPI_Type_free(&block_Type);
}

void copyMatrix(double **dest, double *source, int size){
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++)
            dest[i][j] = source[i * size + j]; 
    }
}

void localProduct(double **A, double *B, double *res, int n_loc){
    for (int i = 0; i < n_loc; i++){
        for (int j = 0; j < n_loc; j++){
            for (int k = 0; k < n_loc; k++){
                res[i * n_loc + j] += A[i][k] * B[k * n_loc + j];
            }
        }
    }
}

void create_matrix(double ***matrix, int dim){
    // allocazione matrice 2D
    *matrix = (double **)calloc(dim, sizeof(double *));
    for (int i = 0; i < dim; i++)
        (*matrix)[i] = (double *)calloc(dim, sizeof(double));
}

void BMR(int menum, int n_loc, int grid_dim, double *C_loc, double *A_loc, double *B_loc, int *coordinate, MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc){
    int step, tag, j, i, sender, menumRow, menumCol, senderCol, receiverCol;
    double **bufferA;
    MPI_Status status;
    create_matrix(&bufferA, n_loc);

    MPI_Comm_rank(*gridc, &menumCol);
    MPI_Comm_rank(*gridr, &menumRow);

    for (step = 0; step < grid_dim; step++){
        if (coordinate[1] == (coordinate[0] + step) % grid_dim){
            sender = menumRow;
            copyMatrix(bufferA, A_loc, n_loc);
        }
        else
            sender = (coordinate[0] + step) % grid_dim;

        for (j = 0; j < n_loc; j++){
            MPI_Bcast(bufferA[j], n_loc, MPI_DOUBLE, sender, *gridr);
        }

        localProduct(bufferA, B_loc, C_loc, n_loc);


        if (menumCol - 1 < 0)
            receiverCol = (menumCol - 1) + grid_dim;
        else
            receiverCol = (menumCol - 1) % grid_dim;

        if (menumCol + 1 < 0)
            senderCol = (menumCol + 1) + grid_dim;
        else
            senderCol = (menumCol + 1) % grid_dim;

        MPI_Send(B_loc, n_loc * n_loc, MPI_DOUBLE, receiverCol, 20 + receiverCol, *gridc);
        MPI_Recv(B_loc, n_loc * n_loc, MPI_DOUBLE, senderCol, 20 + menumCol, *gridc, &status);
    }
}

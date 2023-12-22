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

// reads the number of rows and columns of the matrix, because it's square the system reads only one argument
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

// check if the number of processors can be placed as a quadratic grid
void check_if_grid_can_be_created(int nproc)
{
    double root = sqrt(nproc);

    if (root != (int)root)
    {
        fprintf(stdout, "Grid cannot be created!\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
}

// initialization of the matrix
void initialize_matrix(double **matrix, int N)
{
    *matrix = (double *)calloc(N * N, sizeof(double));

    if (*matrix == NULL)
    {
        fprintf(stderr, "Error allocating memory for the 'matrix' array!\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
}

// prints the matrix
void print_matrix(double *matrix, int N)
{
    printf("[");
    for (int i = 0; i < N; i++)
    {
        (i == 0) ? printf(" ") : printf("  ");
        for (int j = 0; j < N; j++)
        {
            // computes the index of the array
            int index = i * N + j;

            printf("%.2lf", matrix[index]);

            // adds the comma if it's not the last element of the row
            if (j < N - 1)
            {
                printf(", ");
            }
        }

        // adds the semicolon if it's not the last row
        if (i < N - 1)
        {
            printf("\n");
        }
    }
    printf(" ]\n");
}

// adds random elements to the matrix
void fill_matrix(double *matrix, int N)
{
    int index = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            matrix[index++] = rand() % 10 + 1; // 2;
    }

    printf("---------------------------------------------\n--- \t\t Matrix \t\t  --- \n"
           "---------------------------------------------\n");
    print_matrix(matrix, N);
    printf("---------------------------------------------\n");
}

// print the array displs
void print_array_displs(double *array, int size)
{
    printf("(0) displs: [ ");
    for (int i = 0; i < size; i++)
        printf("%2f ", array[i]);
    printf("]\n\n");
}

// create the grid of processors
void create_grid(MPI_Comm *griglia, MPI_Comm *grigliar, MPI_Comm *grigliac,
                 int menum, int nproc, int riga, int col, int *coordinate)
{
    int dim = 2, ndim[2], reorder, period[2], vc[2];

    ndim[0] = riga;
    ndim[1] = col;
    period[0] = period[1] = 0;
    reorder = 0;

    // grid definition
    MPI_Cart_create(MPI_COMM_WORLD, dim, ndim, period, reorder, griglia);

    // each process calculates its own coordinates
    MPI_Cart_coords(*griglia, menum, 2, coordinate);

    // division into rows of the communicator
    vc[0] = 0;
    vc[1] = 1;
    MPI_Cart_sub(*griglia, vc, grigliar);

    // division into columns of the communicator
    vc[0] = 1;
    vc[1] = 0;
    MPI_Cart_sub(*griglia, vc, grigliac);

    return;
}

// compute the offset for the distribution of the elements of the matrix
void get_offset(int *displs, int row_grid, int col_grid, int n_loc, int N)
{
    int offset = 0;

    for (int i = 0; i < row_grid; i++)
    {
        for (int j = 0; j < col_grid; j++)
        {
            displs[i * col_grid + j] = i * N * n_loc + j * n_loc;
        }
    }
}

// distribute the matrix to the processors using the function scatterv
void matrix_distribution(int nproc, double *matrix, double *elements_loc, int *displs, int n_loc, int block_size, int stride)
{
    MPI_Datatype vectorType, block_Type; // block

    MPI_Type_vector(n_loc, n_loc, stride, MPI_DOUBLE, &vectorType);
    MPI_Type_create_resized(vectorType, 0, sizeof(double), &block_Type);
    MPI_Type_commit(&block_Type);

    int s[nproc];
    for (int i = 0; i < nproc; i++)
        s[i] = 1;

    MPI_Scatterv(matrix, s, displs, block_Type, elements_loc, block_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // free mpi types
    MPI_Type_free(&vectorType);
    MPI_Type_free(&block_Type);
}

// copy matrix m2 into matrix m1
void copyMatrix(double **m1, double *m2, int rowsM2, int colsM2)
{
    for (int i = 0; i < rowsM2; i++)
    {
        for (int j = 0; j < colsM2; j++)
            m1[i][j] = m2[i * colsM2 + j]; // m1: risultato
    }
}

// compute the local products of the processor
void localProduct(double **m1, double *m2, double *res, int colsM1, int rowsM2, int menum)
{
    for (int i = 0; i < rowsM2; i++)
    {
        for (int j = 0; j < colsM1; j++)
        {
            for (int k = 0; k < colsM1; k++)
            {
                res[i * colsM1 + j] += m1[i][k] * m2[k * colsM1 + j];
            }
        }
    }
}

// function that allocate memory for the matrix
void create_matrix(double ***matrix, int dim)
{
    int i, j;
    // matrix allocation
    *matrix = (double **)calloc(dim, sizeof(double *));
    for (i = 0; i < dim; i++)
        (*matrix)[i] = (double *)calloc(dim, sizeof(double));
}

// algorithm BMR(Broadcast-Multiply-Rolling) for the compute of the product between matrices
void BMR(int menum, int dimSubmatrix, int dimGrid, double *partialResult, double *submatrixA, double *submatrixB, int *coordinate, MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc)
{
    int step, tag, j, i, sender, menumRow, menumCol, senderCol, receiverCol;
    double **bufferA;
    MPI_Status status;
    create_matrix(&bufferA, dimSubmatrix);

    MPI_Comm_rank(*gridc, &menumCol);
    MPI_Comm_rank(*gridr, &menumRow);

    /*
        At each step:
        1. The sender processor sends submatrix A to the other processors in the same row.
        2. Each processor multiplies the received/copied submatrix A (in the case of step 0, bufferA) by its own submatrix B.
        3. Each processor receives a submatrix B from the processor in the same column, in the next row, and updates its own submatrix.
    */
    for (step = 0; step < dimGrid; step++)
    {
        if (coordinate[1] == (coordinate[0] + step) % dimGrid)
        {
            sender = menumRow;
            copyMatrix(bufferA, submatrixA, dimSubmatrix, dimSubmatrix);
        }
        else
            sender = (coordinate[0] + step) % dimGrid;

        for (j = 0; j < dimSubmatrix; j++)
        {
            MPI_Bcast(bufferA[j], dimSubmatrix, MPI_DOUBLE, sender, *gridr);
        }

        localProduct(bufferA, submatrixB, partialResult, dimSubmatrix, dimSubmatrix, menum);

        if (menumCol - 1 < 0)
            receiverCol = (menumCol - 1) + dimGrid;
        else
            receiverCol = (menumCol - 1) % dimGrid;

        if (menumCol + 1 < 0)
            senderCol = (menumCol + 1) + dimGrid;
        else
            senderCol = (menumCol + 1) % dimGrid;

        MPI_Send(submatrixB, dimSubmatrix * dimSubmatrix, MPI_DOUBLE, receiverCol, 20 + receiverCol, *gridc);
        MPI_Recv(submatrixB, dimSubmatrix * dimSubmatrix, MPI_DOUBLE, senderCol, 20 + menumCol, *gridc, &status);
    }
}

// print the result
void print_result(double *result, int menum, int nproc, int dimSubmatrix)
{
    int i;
    printf("result for processor %d\n", menum);
    for (i = 0; i < dimSubmatrix * dimSubmatrix; i++)
    {
        if (i == dimSubmatrix)
            printf("\n");
        printf("%.2lf ", result[i]);
    }
    printf("\n\n");

    return;
}

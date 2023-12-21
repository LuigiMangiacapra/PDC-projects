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

void print_matrix(double *matrix, int N)
{
    printf("[");
    for (int i = 0; i < N; i++)
    {
        (i == 0) ? printf(" ") : printf("  ");
        for (int j = 0; j < N; j++)
        {
            // Calcola l'indice nell'array
            int index = i * N + j;

            printf("%.2lf", matrix[index]);

            // Aggiungi la virgola se non è l'ultimo elemento della riga
            if (j < N - 1)
            {
                printf(", ");
            }
        }

        // Aggiungi il punto e virgola se non è l'ultima riga
        if (i < N - 1)
        {
            printf("\n");
        }
    }
    printf(" ]\n");
}

void fill_matrix(double *matrix, int N)
{
    int index = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            matrix[index++] = rand() % 10 + 1;
    }

    printf("---------------------------------------------\n--- \t\t Matrix \t\t  --- \n"
           "---------------------------------------------\n");
    print_matrix(matrix, N);
    printf("---------------------------------------------\n");
}

void print_array(double *array, int size)
{
    printf("(0) displs: [ ");
    for (int i = 0; i < size; i++)
        printf("%.2lf ", array[i]);
    printf("]\n\n");
}

void create_grid(MPI_Comm *griglia, MPI_Comm *grigliar, MPI_Comm *grigliac,
                 int menum, int nproc, int riga, int col, int *coordinate)
{
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

    return;
}

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

void matrix_distribution(int nproc, double *matrix, double *elements_loc, int *displs, int n_loc, int block_size, int stride)
{
    MPI_Datatype vectorType, block_Type; // blocco

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

void mat_product(double *A, double *B, double *C, int dim)
{
    for (int i = 0; i < dim; i++)
    {
        for (int k = 0; k < dim; k++)
        {
            for (int j = 0; j < dim; j++)
            {
                C[i * dim + k] += A[i * dim + j] * B[j * dim + k];
            }
        }
    }
}

void copyMatrix(double **m1, double **m2, int rowsM2, int colsM2)
{
    int i, j;
    for (i = 0; i < rowsM2; i++)
    {
        for (j = 0; j < colsM2; j++)
            m1[i][j] = m2[i][j]; // m1: risultato
    }

    return;
}

void localProduct(double **m1, double *m2, double *res, int colsM1, int rowsM2)
{
    int i, j, k;

    for (i = 0; i < rowsM2; i++)
    {
        for (j = 0; j < colsM1; j++)
        {
            for (k = 0; k < colsM1; k++)
                res[i * colsM1 + j] += m1[i][j] * m2[k * colsM1 + j];
        }
    }
}

void create_matrix(double ***matrix, int dim)
{
    int i, j;

    // allocazione matrice
    *matrix = (double **)calloc(dim, sizeof(double *));
    for (i = 0; i < dim; i++)
        (*matrix)[i] = (double *)calloc(dim, sizeof(double));
}

void BMR(int menum, int dimSubatrix, int dimGrid, double *partialResult, double *submatrixA, double *submatrixB, int *coordinate, MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc)
{
    int step, tag, j, i, sender, menumRow, menumCol, senderCol, receiverCol;
    double **bufferA;
    MPI_Status status;
    create_matrix(&bufferA, dimSubatrix);

    MPI_Comm_rank(*gridc, &menumCol);
    MPI_Comm_rank(*gridr, &menumRow);

    /*
        Ad ogni step:
         1. il processore sender invia la sottomatrice A agli altri processori sulla riga;
         2. ogni processore moltiplica la sottomatrice A che ha ricevuto/copiato nel caso del step 0 (bufferA) per la sottomatrice B che possiede;
         3. ogni processore riceve una sottomatrice B dal processore nella stessa colonna, nella riga successiva ed aggiorna la propria.
    */
    for (step = 0; step < dimGrid; step++)
    {
        if (coordinate[1] == (coordinate[0] + step) % dimGrid)
        {
            sender = menumRow;
            copyMatrix(bufferA, &submatrixA, dimSubatrix, dimSubatrix);
        }
        else
            sender = (coordinate[0] + step) % dimGrid;

        for (j = 0; j < dimSubatrix; j++)
            MPI_Bcast(bufferA[j], dimSubatrix, MPI_DOUBLE, sender, *gridr);

        localProduct(bufferA, submatrixB, partialResult, dimSubatrix, dimSubatrix);

        if (menumCol - 1 < 0)
            receiverCol = (menumCol - 1) + dimGrid;
        else
            receiverCol = (menumCol - 1) % dimGrid;

        if (menumCol + 1 < 0)
            senderCol = (menumCol + 1) + dimGrid;
        else
            senderCol = (menumCol + 1) % dimGrid;
        for (j = 0; j < dimSubatrix; j++)
        {
            MPI_Send(&submatrixB[j * dimSubatrix], dimSubatrix, MPI_DOUBLE, receiverCol, 20 + receiverCol, *gridc);
            MPI_Recv(&submatrixB[j * dimSubatrix], dimSubatrix, MPI_DOUBLE, senderCol, 20 + menumCol, *gridc, &status);
        }
    }
}

void createResult(double *partial, double *final, int menum, int nproc, int dimMat, int dimSubatrix)
{
    int i, j, l, p, k;
    MPI_Status status;
    double **A;

    create_matrix(&A, dimSubatrix);

    if (menum == 0)
    {
        for (i = 0; i < dimSubatrix; i++)
        {
            for (j = 0; j < dimSubatrix; j++)
                final[i * dimSubatrix + j] = partial[i * dimSubatrix + j];
        }

        i = 1;
        p = 0;
        while (i < nproc)
        {
            k = (i * dimSubatrix) % dimMat;
            for (j = 0; j < dimSubatrix; j++)
            {
                MPI_Recv(A[j], dimSubatrix, MPI_DOUBLE, i, 20 + i, MPI_COMM_WORLD, &status);
            }
            for (l = 0; l < dimSubatrix; l++)
            {
                for (j = 0; j < dimSubatrix; j++)
                    final[(j + p) * dimSubatrix + k] = A[j][l];

                k++;
                if (k == dimMat)
                    p = p + dimSubatrix;
            }
            ++i;
        }
    }
    else
    {
        for (i = 0; i < dimSubatrix; i++)
            MPI_Send(&partial[i], dimSubatrix, MPI_DOUBLE, 0, 20 + menum, MPI_COMM_WORLD);
    }

    return;
}

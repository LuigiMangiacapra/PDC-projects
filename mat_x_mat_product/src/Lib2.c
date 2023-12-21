#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include "mpi.h"

#define DIM 2

// Funzione creazione matrice rows*columns
void createMat(double ***matrix, int rows, int columns, bool fill)
{
    int i, j;

    // allocazione matrice
    *matrix = (double **)calloc(rows, sizeof(double *));
    for (i = 0; i < rows; i++)
        (*matrix)[i] = (double *)calloc(columns, sizeof(double));

    // popolamento matrice
    if (fill)
    {
        for (i = 0; i < rows; i++)
        {
            for (j = 0; j < columns; j++)
                (*matrix)[i][j] = (double)rand() / RAND_MAX * 150;
        }
    }

    return;
}

void print_matrix(double **matrix, int dimMat)
{
    printf("[");
    for (int i = 0; i < dimMat; i++)
    {
        (i == 0) ? printf(" ") : printf("  ");
        for (int j = 0; j < dimMat; j++)
        {

            printf("%.2lf", matrix[i][j]);

            // Aggiungi la virgola se non è l'ultimo elemento della riga
            if (j < dimMat - 1)
            {
                printf(", ");
            }
        }

        // Aggiungi il punto e virgola se non è l'ultima riga
        if (i < dimMat - 1)
        {
            printf("\n");
        }
    }
    printf(" ]\n");
}

// Funzione creazione griglia
void createGrid(MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc, int menum, int nproc, int row, int colGrid, int *coordinate)
{
    int *ndim, reorder, *period, vc[2];

    ndim = (int *)calloc(DIM, sizeof(int));
    ndim[0] = row;
    ndim[1] = colGrid;
    period = (int *)calloc(DIM, sizeof(int));
    period[0] = period[1] = 1;
    reorder = 0;

    MPI_Cart_create(MPI_COMM_WORLD, DIM, ndim, period, reorder, grid);
    MPI_Cart_coords(*grid, menum, DIM, coordinate);

    // Creazione sottogrid
    vc[0] = 0;
    vc[1] = 1;
    MPI_Cart_sub(*grid, vc, gridr);

    vc[0] = 1;
    vc[1] = 0;
    MPI_Cart_sub(*grid, vc, gridc);

    return;
}

void get_offset(int *displs, int dimGrid, int dimSubMatrix, int dimMat)
{
    int offset = 0;

    for (int i = 0; i < dimGrid; i++)
    {
        for (int j = 0; j < dimGrid; j++)
        {
            displs[i * dimGrid + j] = i * dimMat * dimSubMatrix + j * dimSubMatrix;
        }
    }
}

void print_array(int *array, int size)
{
    printf("(0) displs: [ ");
    for (int i = 0; i < size; i++)
        printf("%d ", array[i]);
    printf("]\n\n");
}

void copy_in_vector(double **matrix, double *vect, int dimMat)
{
    for (int i = 0; i < dimMat; i++)
    {
        for (int j = 0; j < dimMat; j++)
        {
            vect[i * dimMat + j] = matrix[i][j];
        }
    }

    return;
}

void matrix_distribution(int nproc, double **matrix, double **elements_loc, int *displs, int dimSubatrix, int block_size, int stride)
{
    MPI_Datatype vectorType, block_Type; // blocco
    double *vect;

    // copy_in_vector(matrix, &vect, stride);

    MPI_Type_vector(dimSubatrix, dimSubatrix, stride, MPI_DOUBLE, &vectorType);
    MPI_Type_create_resized(vectorType, 0, sizeof(double), &block_Type);
    MPI_Type_commit(&block_Type);

    int s[nproc];
    for (int i = 0; i < nproc; i++)
        s[i] = 1;

    MPI_Scatterv(vect, s, displs, block_Type, elements_loc, block_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // free mpi types
    MPI_Type_free(&vectorType);
    MPI_Type_free(&block_Type);
}

// Funzione di inoltro ai processori di sottomatrici da parte del processore 0
void distribute_matrix(int menum, int nproc, double **matrix, double **submatrix, int dimSubatrix, int dimGrid, int dimMat, MPI_Comm comm_grid, int *displs)
{
    get_offset(displs, dimGrid, dimSubatrix, dimMat);

    MPI_Barrier(comm_grid);

    if (menum == 0)
        print_array(displs, nproc);

    matrix_distribution(nproc, matrix, submatrix, displs, dimSubatrix, dimSubatrix * dimSubatrix, dimMat);

    MPI_Barrier(comm_grid);

    // printf("(%d) sendcounts: [%d]\n", menum, dimSubatrix * dimSubatrix);

    return;
}

void BMR(int menum, int dimSubatrix, int dimGrid, double **partialResult, double **submatrixA, double **submatrixB, int *coordinate, MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc)
{
    int step, tag, j, i, sender, menumRow, menumCol, senderCol, receiverCol;
    double **bufferA;
    MPI_Status status;
    createMat(&bufferA, dimSubatrix, dimSubatrix, false);

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
            copyMatrix(bufferA, submatrixA, dimSubatrix, dimSubatrix);
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
            MPI_Send(submatrixB[j], dimSubatrix, MPI_DOUBLE, receiverCol, 20 + receiverCol, *gridc);
            MPI_Recv(submatrixB[j], dimSubatrix, MPI_DOUBLE, senderCol, 20 + menumCol, *gridc, &status);
        }
    }
}

void localProduct(double **m1, double **m2, double **res, int colsM1, int rowsM2)
{
    int i, j, k;

    for (i = 0; i < rowsM2; i++)
    {
        for (j = 0; j < colsM1; j++)
        {
            for (k = 0; k < colsM1; k++)
                res[i][j] += m1[i][k] * m2[k][j];
        }
    }

    return;
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

void createResult(double **partial, double **final, int menum, int nproc, int dimMat, int dimSubatrix)
{
    int i, j, l, p, k;
    MPI_Status status;
    double **A;

    createMat(&A, dimSubatrix, dimSubatrix, false);

    if (menum == 0)
    {
        for (i = 0; i < dimSubatrix; i++)
        {
            for (j = 0; j < dimSubatrix; j++)
                final[i][j] = partial[i][j];
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
                    final[j + p][k] = A[j][l];

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
            MPI_Send(partial[i], dimSubatrix, MPI_DOUBLE, 0, 20 + menum, MPI_COMM_WORLD);
    }

    return;
}
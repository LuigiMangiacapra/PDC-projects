#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "mpi.h"
#include "Lib2.h"

#define DIM 2

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int menum, nproc, check, dimMat, dimGrid, dimSubatrix, i, j, k, *coordinate;
    double **matrixA, **MatrixB, **result, **partialResult, **submatrixA, **submatrixB, startTime, stopTime;
    int submatrixParameters[4];
    MPI_Comm grid, gridr, gridc;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    /*
        Check dei parametri in input da parte del processore
    */
    if (menum == 0)
    {
        if (argc != 2)
        {
            printf("Errore: numeri di parametri non valido.\n");
            check = -1;
        }
        else
        {
            dimMat = atoi(argv[1]);
            dimGrid = sqrt(nproc);

            // check integrit� parametri e validit� matrice
            if (nproc != dimGrid * dimGrid || dimMat % dimGrid != 0)
            {
                printf("Errore: impossibile accettare parametri in input.\n");
                check = -1;
            }
            else
            {
                check = 1;

                createMat(&matrixA, dimMat, dimMat, true);
                createMat(&MatrixB, dimMat, dimMat, true);
                createMat(&result, dimMat, dimMat, false);

                printf("matrixA:\n");
                for (i = 0; i < dimMat; i++)
                {
                    for (j = 0; j < dimMat; j++)
                        printf("%.2lf ", matrixA[i][j]);
                    printf("\n");
                }
                printf("\n");
                printf("\n");

                printf("MatrixB:\n");
                for (i = 0; i < dimMat; i++)
                {
                    for (j = 0; j < dimMat; j++)
                        printf("%.2lf ", MatrixB[i][j]);
                    printf("\n");
                }
                printf("\n");
                printf("\n");
            }
        }
    }
    MPI_Bcast(&check, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (check == 1)
    {
        MPI_Bcast(&dimMat, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&dimGrid, 1, MPI_INT, 0, MPI_COMM_WORLD);

        dimSubatrix = dimMat / dimGrid;

        coordinate = (int *)calloc(2, sizeof(int));
        createGrid(&grid, &gridr, &gridc, menum, nproc, dimGrid, dimGrid, coordinate);

        createMat(&partialResult, dimSubatrix, dimSubatrix, false);
        createMat(&submatrixA, dimSubatrix, dimSubatrix, false);
        createMat(&submatrixB, dimSubatrix, dimSubatrix, false);

        if (menum == 0)
        {
            int *displs = malloc(sizeof(int) * dimGrid * dimGrid);

            // printf("Qui\n");
            distribute_matrix(menum, nproc, matrixA, submatrixA, dimSubatrix, dimGrid, dimMat, grid, displs);
            // printf("Qui 2\n");
            distribute_matrix(menum, nproc, MatrixB, submatrixB, dimSubatrix, dimGrid, dimMat, grid, displs);
        }

        // printf("Qui 3\n");

        printf("Process %d received A_loc:  \n", menum);
        print_matrix(submatrixA, dimSubatrix);
        printf("\n\n");
        printf("Process %d received B_loc:  \n", menum);
        print_matrix(submatrixB, dimSubatrix);
        printf("\n\n");

        startTime = MPI_Wtime();
        BMR(menum, dimSubatrix, dimGrid, partialResult, submatrixA, submatrixB, coordinate, &grid, &gridr, &gridc);
        createResult(partialResult, result, menum, nproc, dimMat, dimSubatrix);
        stopTime = MPI_Wtime();

        if (menum == 0)
        {
            printf("result:\n");
            for (i = 0; i < dimMat; i++)
            {
                for (j = 0; j < dimMat; j++)
                    printf("%.2lf ", result[i][j]);
                printf("\n");
            }
            printf("\nTime: %lf seconds\n", stopTime - startTime);
        }
    }

    MPI_Finalize();
    return 0;
}

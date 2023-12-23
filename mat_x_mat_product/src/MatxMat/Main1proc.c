#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    int **matA,
        **matB,
        **matRes;
    int n, i, j, k;
    double t0, t1 = 0, time;

    MPI_Init(&argc, &argv);

    n = atoi(argv[1]);

    // alloco matrice a
    matA = (int **)calloc(n, sizeof(int *));
    for (i = 0; i < n; i++)
    {
        matA[i] = (int *)calloc(n, sizeof(int));
    }
    // riempio matrice a
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            matA[i][j] = rand() % 50;
        }
    }

    // alloco matrice b
    matB = (int **)calloc(n, sizeof(int *));
    for (i = 0; i < n; i++)
    {
        matB[i] = (int *)calloc(n, sizeof(int));
    }
    // riempio matrice b
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            matB[i][j] = rand() % 50;
        }
    }

    // alloco matrice ris
    matRes = (int **)calloc(n, sizeof(int *));
    for (i = 0; i < n; i++)
    {
        matRes[i] = (int *)calloc(n, sizeof(int));
    }

    // prodotto matrice-matrice
    t0 = MPI_Wtime();
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            matRes[i][j] = 0;
            for (k = 0; k < n; k++)
            {
                matRes[i][j] = matRes[i][j] + matA[i][k] * matB[k][j];
            }
        }
    }
    t1 = MPI_Wtime();
    // printf("\n\n");
    //  stampa matrice A
    printf("Matrice A:\n");
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            printf("%d ", matA[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");

    // stampa matrice B
    printf("Matrice B:\n");
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            printf("%d ", matB[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");

    // stampa matrice Res
    printf("Matrice Risultato:\n");
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            printf("%d ", matRes[i][j]);
        }
        printf("\n");
    }

    time = t1 - t0;

    printf("Il tempo di esecuzione è stato: %e\n", time);
    MPI_Finalize();

    return 0;
}

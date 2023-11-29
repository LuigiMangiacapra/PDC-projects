#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "sys/time.h"

int main(int argc, char *argv[])
{
    int i, j, maxThread;
    int righe, colonne;
    int **mat;
    int *vet, *risVet;
    struct timeval time;
    double inizio, fine;

    righe = atoi(argv[1]);
    colonne = atoi(argv[2]);

    // creo matrice e vettori
    mat = (int **)calloc(righe, sizeof(int *));

    for (i = 0; i < righe; i++)
    {
        mat[i] = (int *)calloc(colonne, sizeof(int));
    }
    printf("prova\n");
    vet = calloc(colonne, sizeof(int));
    risVet = calloc(righe, sizeof(int));

    // riempio matrice
    for (i = 0; i < righe; i++)
    {
        for (j = 0; j < colonne; j++)
        {
            mat[i][j] = 1;
        }
    }
    // riempio vettore
    for (i = 0; i < colonne; i++)
    {
        vet[i] = 1;
    }

    i = 0;
    j = 0;

    gettimeofday(&time, NULL);
    inizio = time.tv_sec + (time.tv_usec / 1000000.0);
#pragma omp parallel for default(none) shared(righe, colonne, mat, vet, risVet) private(i, j)
    // prodotto matrice vettore
    for (i = 0; i < righe; i++)
    {
        for (j = 0; j < colonne; j++)
        {
            risVet[i] = risVet[i] + mat[i][j] * vet[j];
        }
        printf("Thread totali essere tipo: %d\n", omp_get_num_threads());
    }
    gettimeofday(&time, NULL);
    fine = time.tv_sec + (time.tv_usec / 1000000.0);

    // printf matrice
    for (i = 0; i < righe; i++)
    {
        printf("\n");
        for (j = 0; j < colonne; j++)
        {
            printf("%d ", mat[i][j]);
        }
    }
    printf("\n\n");
    // printf vettore
    for (i = 0; i < colonne; i++)
    {
        printf("%d ", vet[i]);
    }
    printf("\n");

    // printf vet finale
    printf("Risultato:\n");
    for (i = 0; i < righe; i++)
    {
        printf("%d ", risVet[i]);
    }
    printf("\n");
    printf("Il tempo registrato e' stato %fs\n", fine - inizio);
    return 0;
}

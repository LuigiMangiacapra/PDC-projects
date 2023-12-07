# include <stdio.h>
# include <stdlib.h>
# include <omp.h>
# include <sys/time.h>

# include "Lib.h"

int main(int argc, char *argv[])
{
    int N, M;                                // numero di righe e colonne della matrice di valori numerici
    int n_threads;                           // numero di thread da usare per il calcolo in parallelo
    double *A;                               // matrice di elementi fornita in input
    double *x;                               // vettore di elementi generato randomicamente
    double *b;                               // vettore risultante
    double start, end;
    struct timeval time;

    // get input data and initialize the matrix
    read_input(argc, argv, &N, &M, &n_threads);
    initialize_matrix(&A, N, M);
    initialize_array(&x, M);
    fill_array(x, M);
    initialize_array(&b, N);

    // set max number of threads
    omp_set_num_threads(n_threads);

    gettimeofday(&time, NULL);
    start = time.tv_sec + (time.tv_usec / 1000000.0);
    
    // compute Ax=b in parallel
    matxvet(A, N, M, x, b);

    gettimeofday(&time, NULL);
    end = time.tv_sec + (time.tv_usec / 1000000.0);

    // for debug
    // print_matrix(A, N, M);
    // print_array(x, M);
    //printf("Result of Ax=b:\n");
    //print_array(b, N);
    
    printf("\n");
    printf("Il tempo registrato e' stato %fs\n", end - start);
    return 0;
}

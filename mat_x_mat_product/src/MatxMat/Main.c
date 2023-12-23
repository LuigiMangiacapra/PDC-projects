/**
 * @author Fabrizio Vitale
 * @author Giovanni Falcone
 * @author Luigi Mangiacapra
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "mpi.h"
#include "Lib.h"

int main(int argc, char **argv)
{
    int menum;                     // id processo
    int nproc;                     // numero di processi
    int row_grid, col_grid;        // numero di righe e di colonne della griglia di processori
    int N;                         // numero di righe e colonne della matrice di valori numerici
    int *coordinate;               // coordinate griglia
    double *A_loc, *B_loc, *C_loc; // matrice di elementi locale
    double *A, *B;                 // matrice di elementi fornita in input
    double startTime, stopTime;
    MPI_Comm comm_grid;                    // griglia
    MPI_Comm comm_grid_row, comm_grid_col; // sotto griglie

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    srand(time(NULL));
    // get input data and initialize the matrix
    if (menum == 0)
    {
        read_input(argc, argv, &N);

        // exit if matrix cannot be divided equally
        const int rest = N % nproc;
        const int is_there_a_rest = rest != 0;
        if (is_there_a_rest)
        {
            fprintf(stderr, "Matrix size cannot be divided equally!\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            exit(EXIT_FAILURE);
        }

        // if the grid cannot be created exit from program
        check_if_grid_can_be_created(nproc);

        // create and fill A matrix
        initialize_matrix(&A, N);
        fill_matrix(A, N);
        // create and fill B matrix
        initialize_matrix(&B, N);
        fill_matrix(B, N);
    }

    // send data from process with rand 0 to all process
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // compute column number of grid
    col_grid = row_grid = sqrt(nproc);

    // else create grid
    const int grid_dim = 2;
    coordinate = (int *)calloc(grid_dim, sizeof(int));
    create_grid(&comm_grid, &comm_grid_row, &comm_grid_col, menum, nproc, row_grid, col_grid, coordinate);

    const int n_loc = N / row_grid; // number of rows of each process
    const int stride = N;           // movement between one cell and another

    // defination of displs for MPI_Scatterv
    int *displs = malloc(sizeof(int) * row_grid * col_grid);
    // compute offset in order to understand where to start from matrix to send a block
    get_offset(displs, row_grid, col_grid, n_loc, N);

    // Allocazione della matrice locale su ogni processo
    A_loc = (double *)calloc(n_loc * n_loc, sizeof(double));
    B_loc = (double *)calloc(n_loc * n_loc, sizeof(double));
    C_loc = (double *)calloc(n_loc * n_loc, sizeof(double));
    if (A_loc == NULL || B_loc == NULL || C_loc == NULL)
    {
        fprintf(stderr, "Error allocating memory for the local matrix!\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    matrix_distribution(nproc, A, A_loc, displs, n_loc, stride);
    matrix_distribution(nproc, B, B_loc, displs, n_loc, stride);

    // apply BMR strategy and get time
    MPI_Barrier(comm_grid);
    startTime = MPI_Wtime();
    BMR(menum, n_loc, grid_dim, C_loc, A_loc, B_loc, coordinate, &comm_grid, &comm_grid_row, &comm_grid_col);
    stopTime = MPI_Wtime();

    MPI_Barrier(comm_grid);

    if (menum == 0)
    {
        printf("\nTempo impiegato per svolgere l'algoritmo: %lf seconds\n", stopTime - startTime);
    }

    // in order to print in sequential way each result
    // MPI_Barrier(comm_grid);
    // for (int i = 0; i < nproc; i++)
    // {
    //     MPI_Barrier(comm_grid);
    //     if (menum == i)
    //     {
    //         if (menum == 0)
    //             printf("\n");
    //         printf("result of P%d:\n", menum);
    //         print_matrix(C_loc, n_loc);
    //     }
    // }

    // deallocate memory
    if (menum == 0)
    {
        free(A);
        free(B);
    }

    free(displs);
    free(A_loc);
    free(B_loc);
    free(C_loc);
    free(coordinate);

    MPI_Finalize();

    return 0;
}

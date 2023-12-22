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
    double *A, *B, *C;             // matrice di elementi fornita in input
    double *partialResult;
    int check;
    int dimSubMatrix, dimGrid;
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
        dimGrid = sqrt(nproc);
        // check integrità parametri e validità matrice
        if (nproc != dimGrid * dimGrid || N % dimGrid != 0)
        {
            printf("Errore: impossibile accettare parametri in input.\n");
            check = -1;
        }
        else
        {
            check = 1;
            initialize_matrix(&A, N);
            fill_matrix(A, N);
            initialize_matrix(&B, N);
            fill_matrix(B, N);
            initialize_matrix(&C, N);
        }
    }

    MPI_Bcast(&check, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (check == 1)
    {
        // send data from process with rand 0 to all process
        MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&dimGrid, 1, MPI_INT, 0, MPI_COMM_WORLD);
        dimSubMatrix = N / dimGrid;

        initialize_matrix(&partialResult, dimSubMatrix);
        // if the grid cannot be created exit from program
        check_if_grid_can_be_created(nproc);
        // compute column number of grid
        col_grid = row_grid = sqrt(nproc);

        // else create grid
        const int grid_dim = 2;
        coordinate = (int *)calloc(grid_dim, sizeof(int));
        create_grid(&comm_grid, &comm_grid_row, &comm_grid_col, menum, nproc, row_grid, col_grid, coordinate);

        int n_loc = N / row_grid;            // number of rows of each process
        const int block_loc = n_loc * n_loc; // block size of each process
        const int stride = N;                // movement between one cell and another

        // defination of displs for MPI_Scatterv
        int *displs = malloc(sizeof(int) * row_grid * col_grid);
        // compute offset in order to understand where to start from matrix to send a block
        get_offset(displs, row_grid, col_grid, n_loc, N);

        MPI_Barrier(comm_grid);
        // printf("(%d) sendcounts: [%d]\n", menum, block_loc);

        // Allocazione della matrice locale su ogni processo
        A_loc = (double *)calloc(block_loc, sizeof(double));
        B_loc = (double *)calloc(block_loc, sizeof(double));
        C_loc = (double *)calloc(block_loc, sizeof(double));
        if (A_loc == NULL || B_loc == NULL || C_loc == NULL)
        {
            fprintf(stderr, "Error allocating memory for the local matrix!\n");
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // distribuzione della matrice per ciascun processo
        MPI_Barrier(comm_grid);
        matrix_distribution(nproc, A, A_loc, displs, n_loc, block_loc, stride);
        matrix_distribution(nproc, B, B_loc, displs, n_loc, block_loc, stride);

        startTime = MPI_Wtime();
        BMR(menum, dimSubMatrix, dimGrid, partialResult, A_loc, B_loc, coordinate, &comm_grid, &comm_grid_row, &comm_grid_col);
        createResult(partialResult, C, menum, nproc, N, dimSubMatrix);
        stopTime = MPI_Wtime();

        // Sincronizzazione per garantire che tutti i processi abbiano ricevuto i dati
        MPI_Barrier(comm_grid);

        // Stampa in ordine sequenziale
        if (menum == 0)
        {
            printf("\n");
            /*printf("Process %d received A_loc:  \n", menum);
            print_matrix(A_loc, n_loc);
            printf("\n\n");
            printf("Process %d received B_loc:  \n", menum);
            print_matrix(B_loc, n_loc);
            printf("\n\n");
            printf("Process %d results C_loc:  \n", menum);
            print_matrix(C_loc, n_loc);
            printf("\n\n");*/

            printf("result:\n");
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                    printf("%.2lf ", C[i * N + j]);
                printf("\n");
            }
            printf("\nTime: %lf seconds\n", stopTime - startTime);
        }

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
        // free(C);

        MPI_Finalize();
    }
    return 0;
}

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
    int menum;             // id of each processor
    int nproc;             // number of processors
    int dimGrid;           // number of rows and columns of the grid of processors
    int N;                 // number of rows and columns of the matrix of elements
    int *coordinate;       // coordinates of the grid
    double *A_loc, *B_loc; // local matrices of elements
    double *A, *B;         // matrices of elements provided as input
    double *partialResult; // matrix that contains the results
    int check;
    int dimSubMatrix;
    double startTime, stopTime;
    MPI_Comm comm_grid;                    // grid
    MPI_Comm comm_grid_row, comm_grid_col; // row grid and column grid of processors

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    srand(time(NULL));
    // get input data and initialize the matrix
    if (menum == 0)
    {
        read_input(argc, argv, &N);
        dimGrid = sqrt(nproc);

        // if the grid cannot be created exit from program
        check_if_grid_can_be_created(nproc);

        // check parameter integrity and matrix validity
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
        }
    }

    MPI_Bcast(&check, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (check == 1)
    {
        // send data from process with rand 0 to all process
        MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&dimGrid, 1, MPI_INT, 0, MPI_COMM_WORLD);
        dimSubMatrix = N / dimGrid;

        // Allocation of local matrices on each process
        initialize_matrix(&A_loc, dimSubMatrix);
        initialize_matrix(&B_loc, dimSubMatrix);
        initialize_matrix(&partialResult, dimSubMatrix);

        if (A_loc == NULL || B_loc == NULL || partialResult == NULL)
        {
            fprintf(stderr, "Error allocating memory for the local matrix!\n");
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // else create grid
        const int grid_dim = 2;
        coordinate = (int *)calloc(grid_dim, sizeof(int));
        create_grid(&comm_grid, &comm_grid_row, &comm_grid_col, menum, nproc, dimGrid, dimGrid, coordinate);

        int n_loc = N / dimGrid;             // number of rows of each process
        const int block_loc = n_loc * n_loc; // block size of each process
        const int stride = N;                // movement between one cell and another

        // defination of displs for MPI_Scatterv
        int *displs = malloc(sizeof(int) * dimGrid * dimGrid);

        // compute offset in order to understand where to start from matrix to send a block
        get_offset(displs, dimGrid, dimGrid, n_loc, N);

        // synchronization to ensure that all processes have received data
        MPI_Barrier(comm_grid);

        // matrix distribution for each process
        matrix_distribution(nproc, A, A_loc, displs, n_loc, block_loc, stride);
        matrix_distribution(nproc, B, B_loc, displs, n_loc, block_loc, stride);

        startTime = MPI_Wtime();
        BMR(menum, dimSubMatrix, dimGrid, partialResult, A_loc, B_loc, coordinate, &comm_grid, &comm_grid_row, &comm_grid_col);
        stopTime = MPI_Wtime();

        // print of the result obtained by each processor
        // print_result(partialResult, menum, nproc, n_loc);

        // print time taken to perform the algorithm
        if (menum == 0)
        {
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
        free(partialResult);
        free(coordinate);

        MPI_Finalize();
    }
    return 0;
}

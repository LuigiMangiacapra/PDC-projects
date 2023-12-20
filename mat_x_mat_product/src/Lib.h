#ifndef LIB_H
#define LIB_H

#include "mpi.h"


void read_input(int argc, char **argv, int *N);

void initialize_matrix(int **matrix, int N) ;

void fill_matrix(int *matrix, int N);

void print_matrix(int *matrix, int N);

void print_array(int *array, int size);

void check_if_grid_can_be_created(int nproc);

void create_grid(MPI_Comm *griglia, MPI_Comm *grigliar, MPI_Comm *grigliac, int menum, int nproc, int riga, int col, int *coordinate);

void get_offset(int *displs, int row_grid, int col_grid, int n_loc, int N);

void matrix_distribution(int nproc, int *matrix, int *elements_loc, int *displs, int n_loc, int block_size, int stride);

void mat_product(int *A, int *B, int *C, int dim);

void broadcastMultiplyRolling(int *A_loc, int *B_loc, int *C_loc, int block_loc, MPI_Comm colSub, int menum, int nproc);

#endif

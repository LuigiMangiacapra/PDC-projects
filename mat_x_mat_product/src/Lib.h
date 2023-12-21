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

void BMR(int menum, int dimSubatrix, int dimGrid, int* partialResult, int* submatrixA, int* submatrixB, int* coordinate, MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc);

void mat_product(int *A, int *B, int *C, int dim);

void copyVec(int* m1, int* m2, int rowsM2, int colsM2);

void localProduct(int* m1, int* m2, int* res, int colsM1, int rowsM2);

void createResult(int* partial, int* final, int menum, int nproc, int dimMat, int dimSubatrix);

#endif

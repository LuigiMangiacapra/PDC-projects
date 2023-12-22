#ifndef LIB_H
#define LIB_H

#include "mpi.h"

void read_input(int argc, char **argv, int *N);

void initialize_matrix(double **matrix, int N);

void fill_matrix(double *matrix, int N);

void print_matrix(double *matrix, int N);

void print_array(double *array, int size);

void check_if_grid_can_be_created(int nproc);

void create_grid(MPI_Comm *griglia, MPI_Comm *grigliar, MPI_Comm *grigliac, int menum, int nproc, int riga, int col, int *coordinate);

void get_offset(int *displs, int row_grid, int col_grid, int n_loc, int N);

void matrix_distribution(int nproc, double *matrix, double *elements_loc, int *displs, int n_loc, int block_size, int stride);

void BMR(int menum, int dimSubatrix, int dimGrid, double *partialResult, double *submatrixA, double *submatrixB, int *coordinate, MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc);

void mat_product(double *A, double *B, double *C, int dim);

void copyMatrix(double **m1, double *m2, int rowsM2, int colsM2);

void localProduct(double **m1, double *m2, double *res, int colsM1, int rowsM2, int menum);

void create_matrix(double ***matrix, int dim);

void print_result(double *result, int menum, int nproc, int dimSubmatrix);

#endif

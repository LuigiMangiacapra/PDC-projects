#ifndef LIB2_H
#define LIB2_H

#include "mpi.h"

void BMR(int menum, int dimSubatrix, int dimGrid, double **partialResult, double **submatrixA, double **submatrixB, int *coordinate, MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc);

void localProduct(double **m1, double **m2, double **res, int colsM1, int rowsM2);

void createResult(double **partial, double **final, int menum, int nproc, int dimMat, int dimSubatrix);

void createMat(double ***matrix, int rows, int columns, bool fill);

void print_matrix(double **matrix, int dimMat);

void print_array(double *array, int size);

void createGrid(MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc, int menum, int nproc, int row, int colGrid, int *coordinate);

void matrix_distribution(int nproc, double **matrix, double **elements_loc, int *displs, int dimSubatrix, int block_size, int stride);

void get_offset(int *displs, int dimGrid, int dimSubatrix, int dimMat);

void copyMatrix(double **m1, double **m2, int rowsM2, int colsM2);

void copy_in_vector(double **matrix, double *vect, int dimMat);

#endif
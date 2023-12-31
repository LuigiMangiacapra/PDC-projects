#ifndef LIB_H
#define LIB_H

#include "mpi.h"

/**
 * @brief Reads input arguments and exits from program if they are incorrect.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param N Pointer to the number of matrix size variable.
 */
void read_input(int argc, char **argv, int *N);

/**
 * @brief Initializes a 1D matrix with calloc.
 * 
 * @param matrix Pointer to the matrix.
 * @param N Matrix size.
 */
void initialize_matrix(double **matrix, int N);

/**
 * @brief Fills a 1D matrix with random values.
 * 
 * @param matrix Pointer to the matrix.
 * @param N matrix size.
 */
void fill_matrix(double *matrix, int N);

/**
 * @brief Prints the contents of a 1D matrix.
 * 
 * @param matrix Pointer to the matrix.
 * @param N Matrix size.
 */
void print_matrix(double *matrix, int N);

/**
 * @brief Prints the contents of an array.
 * 
 * @param array Pointer to the array.
 * @param size Size of the array.
 */
void print_array(double *array, int size);

/**
 * @brief Checks if a grid can be created i.e if nproc is not a perfect square then the grid cannot be created.
 * 
 * @param nproc Number of MPI processes.
 */
void check_if_grid_can_be_created(int nproc);

/**
 * @brief Create a grid object.
 * 
 * @param griglia grid communicator.
 * @param grigliar row grid communicator.
 * @param grigliac column grid communicator.
 * @param menum Process ID.
 * @param nproc Number of MPI processes.
 * @param row Grid row number.
 * @param col Grid column number.
 * @param coordinate The grid coordinates of a process
 */
void create_grid(MPI_Comm *griglia, MPI_Comm *grigliar, MPI_Comm *grigliac, int menum, int nproc, int row, int col, int *coordinate);

/**
 * @brief Get the offset in order to split the matrix in subblocks.
 * 
 * @param displs The array which will contain the offset for each process
 * @param row_grid Grid row number.
 * @param col_grid Grid column number.
 * @param n_loc Numer of elements for a row (resp. column)
 * @param N Matrix size.
 */
void get_offset(int *displs, int row_grid, int col_grid, int n_loc, int N);

/**
 * @brief Divides the matrix into equal subblocks. Each process will have one subblock.
 * 
 * @param nproc number of process.
 * @param source The 1D matrix to split.
 * @param dest The 1D matrix that each process will receive.
 * @param displs The array of offset.
 * @param n_loc Numer of elements for a row (resp. column)
 * @param stride Movement between one cell and another.
 */
void matrix_distribution(int nproc, double *source, double *dest, int *displs, int n_loc, int stride);

/**
 * @brief Apply BMR strategy between A_loc and B_loc in order to get C_loc.
 * 
 * @param menum Process ID.
 * @param n_loc Numer of elements for a row (resp. column).
 * @param grid_dim Grid size.
 * @param C_loc Dest matrix which will contain partial result for each process.
 * @param A_loc Subblock matrix
 * @param B_loc Subblock matrix
 * @param coordinate The grid coordinates of a process
 * @param grid Grid communicator
 * @param gridr Grid row communicator
 * @param gridc Grid column communicator
 */
void BMR(int menum, int n_loc, int grid_dim, double *C_loc, double *A_loc, double *B_loc, int *coordinate, MPI_Comm *grid, MPI_Comm *gridr, MPI_Comm *gridc);

/**
 * @brief Copy the source matrix to dest.
 * 
 * @param dest Destination matrix
 * @param source Source matrix
 * @param size Size of source matrix
 */
void copyMatrix(double **dest, double *source, int size);

/**
 * @brief Apply the product between matrix A and B and save it in res.
 * 
 * @param A matrix 2D.
 * @param B matrix 1D.
 * @param res result matrix
 * @param size Numer of elements for a row (resp. column).
 */
void localProduct(double **A, double *B, double *res, int size);

/**
 * @brief Create a 2D matrix object
 * 
 * @param matrix Pointer to 2D matrix
 * @param dim Size matrix
 */
void create_matrix(double ***matrix, int dim);

#endif

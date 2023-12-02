#ifndef LIB_H
#define LIB_H

/**
 * @brief Compute the product Ax=b using threads.
 * 
 * @param A Pointer to the matrix.
 * @param N Number of rows of A.
 * @param M number of columns of A.
 * @param x Pointer to array.
 * @param b Result of product.
 * @param n_threads Number of threads to use for product.
 */
void matxvet(double *A, int N, int M, double *x, double *b, int n_threads);


/**
 * @brief Fills a 1D matrix with random values.
 * 
 * @param matrix Pointer to the matrix.
 * @param row Number of rows.
 * @param col Number of columns.
 */
void fill_matrix(double *matrix, int row, int col);

/**
 * @brief Prints the contents of a 1D matrix.
 * 
 * @param matrix Pointer to the matrix.
 * @param row Number of rows.
 * @param col Number of columns.
 */
void print_matrix(double *matrix, int row, int col);

/**
 * @brief Reads input arguments and exits from program if they are incorrect.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param N Pointer to the number of matrix rows variable.
 * @param M Pointer to the number of matrix columns variable.
 * @param n_thread Pointer to the number of thread to use.
 */
void read_input(int argc, char **argv, int *N, int *M, int *n_threads);

/**
 * @brief Initializes a 1D matrix with random values.
 * 
 * @param matrix Pointer to the matrix.
 * @param N Number of rows.
 * @param M Number of columns.
 */
void initialize_matrix(double **matrix, int N, int M);

/**
 * @brief Initializes a 1D array with values based on the index.
 * 
 * @param array Pointer to the array.
 * @param size Size of the array.
 */
void initialize_array(double **array, int size);

/**
 * @brief Prints the contents of a 1D array.
 * 
 * @param array Pointer to the array.
 * @param size Size of the array.
 */
void print_array(double *array, int size);

/**
 * @brief Initializes a 1D array with random values.
 * 
 * @param array Pointer to the array.
 * @param size Size of the array.
 */
void fill_array(double *array, int size);

#endif
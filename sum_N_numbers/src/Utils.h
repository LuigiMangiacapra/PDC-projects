#ifndef UTILS_H
#define UTILS_H 

/**
 * @brief check if the inpurs are correct in order to sum
 * 
 * @param argc number of parameters
 * @param N number of elements to sum
 * @param strategy the strategy to apply
*/
int check_if_inputs_are_valid(int argc, int N, int strategy);

/**
 * @brief fill the array randomly if N is greater than 20, from argv otherwise
 * 
 * @param elements the array of integers
 * @param N capacity of array
 * @param argv the elements to insert into the array
 */
void fill_array(int *elements, int N, char *argv[]);

/**
 * @brief check if the strategy 2 (or 3) is applicable: the number of processor must be a power of 2
 * 
 * @param strategy the strategy to apply (2 or 3)
 * @param nproc the number of processor
 * @return int 1 if it's applicable, 0 otherwise
 */
int strategy_2_OR_3_are_applicable(int strategy, int nproc);

/**
 * @brief performs the sum of each array value and returns it
 * 
 * @param array the array of integer
 * @param n size of array
 * @return int the sum
 */
int sequential_sum(int *array, int n);

/**
 * @brief the processor with id 0 send the elements to sum to the other processor
 * 
 * @param menum id of the processor
 * @param elements array of all integers
 * @param elements_loc local array for each processor
 * @param nloc number of elements to sum for each processor
 * @param nproc number of processor
 * @param rest the rest of division between the all numbers to sum and number of processor
 */
void operand_distribution(int menum, int *elements, int *elements_loc, int nloc, int nproc, int rest);

/**
 * @brief print results: print the partial sum for each processor, the time spent for each partial sum,
 * the total sum and total time spent for the total sum
 * 
 * @param menum id of processor
 * @param strategy the strategy applied
 * @param sum the result to print
 * @param timetot time taken
 */
void print_result(int menum, int strategy, int sum, double timetot);

#endif 
#ifndef STRATEGY_H
#define STRATEGY_H 

/**
 * @brief apply the first strategy
 * 
 * @param menum id of the processor
 * @param nproc number of processor
 * @param sum partial sum performed at the first step
 * @return int total sum
 */
int first_strategy(int menum, int nproc, int sum);

/**
 * @brief apply the second strategy
 * 
 * @param menum id of the processor
 * @param logNproc number of steps
 * @param array the array of powers of two
 * @param sum  partial sum performed at the first step
 * @return int total sum
 */
int second_strategy(int menum, int logNproc, int *array, int sum);

/**
 * @brief apply the third strategy
 * 
 * @param menum id of the processor
 * @param logNproc number of steps
 * @param array the array of powers of two
 * @param sum  partial sum performed at the first step
 * @return int total sum
 */
int third_strategy(int menum, int logNproc, int *array, int sum);

#endif 
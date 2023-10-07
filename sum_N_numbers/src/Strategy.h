#ifndef STRATEGY_H
#define STRATEGY_H 

/**
 * @brief fill the array randomly if N is greater than 20, from argv otherwise
 * 
 * @param elements the array of integers
 * @param N capacity of array
 * @param argv the elements to insert into the array
 */
void get_numbers(int *elements, int N, char *argv[]);

/**
 * @brief check if strategy is a number between 1 and 3
 *
 * @param strategy the integer
 *
 * @return 0 if it's valid, 1 otherwise
 *
*/
int strategy_is_valid(int strategy);

// da aggiustare in termini di passaggio di parametri, ecc ...

void first_strategy();

void second_strategy();

void third_strategy();

#endif 
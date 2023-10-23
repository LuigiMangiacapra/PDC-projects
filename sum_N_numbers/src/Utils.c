/**
 * @author Fabrizio Vitale
 * @author Giovanni Falcone
 * @author Luigi Mangiacapra
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

# include "mpi.h"

/* *************************************************************************** */
/*                      SUPPORT FUNCTION                                       */
/* *************************************************************************** */

static void fill_array_randomly(int *elements, int N);

static void fill_array_by_argv(int *elements, int N, char *argv[]);

/**
 * @brief check if strategy is a number between 1 and 3
 *
 * @param strategy the integer
 *
 * @return 0 if it's valid, 1 otherwise
 *
*/
static int strategy_is_valid(int strategy);

/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */

int check_if_inputs_are_valid(int argc, int N, int strategy){
    if(N <= 0){
        fprintf(stderr, "Inserire un numero maggiore di 0!\n");
        return EXIT_FAILURE;
    }

    if(strategy_is_valid(strategy) != 0){
        fprintf(stderr, "Strategia non valida: inserire un numero compreso fra 1 e 3!\n");
        return EXIT_FAILURE;
    }

    if(N <= 20 && argc - 3 != N){
        fprintf(stderr, "Il numero di elementi inserito non corrisponde ad N!\n");
        return EXIT_FAILURE;
    }

    return 0; // valid input
}

int strategy_is_valid(int strategy){
    if(strategy < 1 || strategy > 3)
        return EXIT_FAILURE;

    // it's valid 
    return 0;
}

void fill_array(int *elements, int N, char *argv[]){
    if(N > 20)
        fill_array_randomly(elements, N);
    else
        fill_array_by_argv(elements, N, argv);
}

void fill_array_randomly(int *elements, int N){
    srand(time(NULL));

    printf("Generazione numeri randomici...\n");

    for(int i = 0; i < N; i++){
        elements[i] = 1; // rand() % 100;
    }
}

void fill_array_by_argv(int *elements, int N, char *argv[]){
    printf("Inserimento dei numeri forniti da terminale...\n");

    for(int i = 0; i < N; i++){
        elements[i] = atoi(argv[i + 3]);    // 0: name src; 1: N; 2: strategy; starting from 3 we have all numbers
    }
}

int strategy_2_OR_3_are_applicable(int strategy, int nproc){
    return !(((strategy == 2 || strategy == 3) && ((nproc & (nproc - 1)) != 0)) || (nproc == 1)) ? 1 : 0;
}

int sequential_sum(int *array, int n){
    int sum = 0;

    for(int i = 0; i < n; i++){
        sum += array[i];
    }

    return sum;
}

void operand_distribution(int menum, int *elements, int *elements_loc, int nloc, int nproc, int rest){
    int tag;
    MPI_Status status;
    
    if (menum == 0){
        for (int i = 0; i < nloc; i++){
            elements_loc[i] = elements[i];
        }

        int tmp = nloc;
        int start = 0;
        for (int i = 1; i < nproc; i++){
            start += tmp;
            tag = 22 + i;
            if (i == rest)
                tmp -= 1;

            MPI_Send(&elements[start], tmp, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
    } else {
        tag = 22 + menum;
        MPI_Recv(elements_loc, nloc, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    }
}

void print_result(int menum, int strategy, int sum, double timetot){
    if(strategy == 1){
        if(menum == 0)
            printf("La somma totale e' %d e l'algoritmo, per calcolarla, ha impiegato %e.\n", sum, timetot);
    } else {
        printf("\n Sono il processo %d e la somma totale e' %d\n", menum, sum);
        if(menum == 0)
            printf("Tempo totale impiegato per l'algoritmo: %e\n", timetot);
    }
}

void compute_power_of_two(int logNproc, int *array){
    for(int i = 0; i < logNproc + 1; i++){
        array[i] = (int) pow(2, i);
    }
}
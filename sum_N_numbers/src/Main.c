/**
 * @author Fabrizio Vitale
 * @author Giovanni Falcone
 * @author Luigi Mangiacapra
*/  
    
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mpi.h"
#include "Strategy.h"
#include "Utils.h"

# define STRATEGY_1 1
# define STRATEGY_2 2
# define STRATEGY_3 3

int main(int argc, char *argv[]){
    int menum;                      // id del processore
    int nproc;                      // numero processori
    int N;                          // numero di elementi da sommare
    int sum;                        // somma totale da stampare
    int logNproc;                   // numero di passi da effettuare per la II, III strategia
    int strategy;                   // strategia con cui sommare 
    int nloc;                       // numero di elementi che ciascun processore deve sommare
    int rest;                       // resto della divisione 
    int *elements;                  // array completo
    int *elements_loc;              // vettore di elementi locale
    int *array_of_powers_of_two;    // vettore di potenze di 2
    double end_time;
    double start_time;
    double timetot = 0;

    if(argc < 3){
        fprintf(stderr, "Utilizzo: <numeri da sommare> <tipo di strategia> <numeri da sommare se N>\n");
        return EXIT_FAILURE;
    }

    // MPI initialization
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if(menum == 0){
        // convert to integer the number to sum and strategy to apply
        N = atoi(argv[1]);
        strategy = atoi(argv[2]);

        if(check_if_inputs_are_valid(argc, N, strategy) != 0){
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // array malloc
        elements = (int *)malloc(sizeof(int) * N);
        if(elements == NULL){
            fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
            return EXIT_FAILURE;
        }

        // fill array with N elements
        fill_array(elements, N, argv);

        // Verifica se la strategia 2 (o 3) è applicabile: se il numero dei processori non è potenza di 2 applica la strategia 1
        if(!strategy_2_OR_3_are_applicable(strategy, nproc)){ 
            strategy = STRATEGY_1;
            printf("Applico la prima strategia.\n");
        } 
    }

    // send data to all other processors
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&strategy, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // compute the logarithm and powers of two for second and third strategy 
    if(strategy == STRATEGY_2 || strategy == STRATEGY_3) {
        // get number of steps
        logNproc = log2(nproc); 
        // allocation of array of powers of 2
        array_of_powers_of_two = (int *)malloc(sizeof(int) * logNproc);
        if(array_of_powers_of_two == NULL){
            fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'array_of_powers_of_two'!\n");
            return EXIT_FAILURE;
        }
        // fill the array with powers of 2
        compute_power_of_two(logNproc, array_of_powers_of_two);
    }


    // in order to check how many elements each processor must sum
    nloc = N / nproc;
    rest = N % nproc;

    if(menum < rest){
        nloc = nloc + 1;
    }

    // allocation of local array for each processor
    elements_loc = (int *)malloc(sizeof(int) * nloc);
    if(elements_loc == NULL){
        fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
        return EXIT_FAILURE;
    }
    
    // invia elementi da sommare agli altri processori
    operand_distribution(menum, elements, elements_loc, nloc, nproc, rest);

    // attendiamo che i processi si sincronizzino
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    sum = 0;
    // first step: each processor performs the first partial sum
    sum = sequential_sum(elements_loc, nloc);

    // check the strategy to apply
    if(strategy == STRATEGY_1){
        sum = first_strategy(menum, nproc, sum);
    }else if(strategy == STRATEGY_2){
        sum = second_strategy(menum, logNproc, array_of_powers_of_two, sum);
    } else{ // third_strategy
        sum = third_strategy(menum, logNproc, array_of_powers_of_two, sum);
    }

    end_time = MPI_Wtime();

    double timeP = end_time - start_time;
    printf("Il tempo impiegato da %d è di %e s\n", menum, timeP);

    // compute total time
    MPI_Reduce(&timeP, &timetot, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);    

    // print sum total and partial sum of each processor and the time
    print_result(menum, strategy, sum, timetot);

    // freeing memory before program termination
    if(menum == 0){ 
        free(elements);
        free(elements_loc);
        free(array_of_powers_of_two);
    }else{
        free(elements_loc);
        free(array_of_powers_of_two);
    }

    MPI_Finalize();
    return 0;
}

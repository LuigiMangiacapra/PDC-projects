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

// Support function

/**
 * @brief check if the inpurs are correct in order to sum
 * 
 * @param argc number of parameters
 * @param N number of elements to sum
 * @param strategy the strategy to apply
*/
static int check_if_inputs_are_valid(int argc, int N, int strategy);

/****************************************************/

int main(int argc, char *argv[]){
    int menum, nproc, tag, i;
    int n, nloc, N, strategia, sum, logNproc, randomNumber;
    int *x;
    int *xloc;
    MPI_Status status;
    int sumparz;
    int strategy;           // strategia con cui sommare 
    int *elements;          // array completo

    if(argc < 3){
        fprintf(stderr, "Utilizzo: <numeri da sommare> <tipo di strategia> <numeri da sommare se N>\n");
        return EXIT_FAILURE;
    }

    // convert to integer the number to sum and strategy to apply
    N = atoi(argv[1]);
    strategy = atoi(argv[2]);

    if(check_if_inputs_are_valid(argc, N, strategy) != 0)
        return EXIT_FAILURE;

    // array malloc
    elements = (int *)malloc(sizeof(int) * N);
    if(elements == NULL){
        fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
        return EXIT_FAILURE;
    }

    // fill array
    get_numbers(elements, N, argv);
    
    // show array
    for(int i = 0; i < N; i++)
        printf("%d ", elements[i]);
    printf("\n");

    //Seleziona la strategia a seconda se il numero di elementi è potenza di 2, 
    // se lo e' seleziona la strategia sceltaa se sono la 2 o la 3 altrimenti passa alla 1
    if((strategy == 2 || strategy == 3) && ((N & (N - 1)) != 0)){
        strategy = 1;
    }

    //per risparmiare tempo computazionale
    if(strategy == 2 || strategy == 3){
        logNproc = log2(nproc); 
    }

    // MPI initialization
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    // fare funzione esterna
    //il processo 0 invia a tutti gli altri processi il valore con tag 10+i
    if(menum == 0){
        //Lettura dei dati di input
        for(i = 1; i < nproc; i++){
            tag = 10 + i;
            MPI_Send(&n, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
    }else{ //gli altri processi ricevono il valore dal processo 0 con tag 10+numero_processo
        tag = 10 + menum;
        MPI_Recv(&n, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    }
    
    // xloc, elements, boh 
    sum = 0;
    for(i = 0; i < N; i++){
        sum = sum + xloc[i];
    }

    // check the strategy to apply
    if(strategy == 1)
        first_strategy();
    else if(strategy == 2)
        second_strategy();
    else
        third_strategy();

    // print result
    if(strategy == 1){
        if(menum == 0)
            printf("\nSomma totale = %d\n", sum);
    }else{
        printf("\nSono il processo %d: e la somma totale= %d\n", menum, sum);
    }

    // freeing memory before program termination 
    free(elements);

    return 0;
}

static int check_if_inputs_are_valid(int argc, int N, int strategy){
    if(N <= 20 && argc - 3 != N && strategy_is_valid(strategy) == 0){
        fprintf(stderr, "Il numero di elementi inserito non corrisponde ad N!\n");
        return EXIT_FAILURE;
    }

    if(N <= 0){
        fprintf(stderr, "Inserire un numero maggiore di 0!\n");
        return EXIT_FAILURE;
    }

    if(strategy_is_valid(strategy) != 0){
        fprintf(stderr, "La strategia deve essere un valore compreso tra 1 e 3!\n");
        return EXIT_FAILURE;
    }

    return 0; // valid input
}
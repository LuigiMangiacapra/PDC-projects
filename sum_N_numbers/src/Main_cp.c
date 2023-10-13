/**
 * @author Fabrizio Vitale
 * @author Giovanni Falcone
 * @author Luigi Mangiacapra
*/  
    
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "mpi.h"
//#include "Strategy.h"

// Support function

/**
 * @brief check if the inpurs are correct in order to sum
 * 
 * @param argc number of parameters
 * @param N number of elements to sum
 * @param strategy the strategy to apply
*/
//static int check_if_inputs_are_valid(int argc, int N, int strategy);
int strategy_is_valid(int strategy);

/****************************************************/

int main(int argc, char *argv[]){
    int menum, nproc, tag;
    int n, N, sum, logNproc;
    int *values;
    MPI_Status status;
    int strategy;           // strategia con cui sommare 
    int *elements;          // array completo
    int nloc, rest;
    int sumparz=0;
    int j=0, i;
    double t1, t0, timeP;
    double timetot = 0;

    if(argc < 3){
        fprintf(stderr, "Utilizzo: <numeri da sommare> <tipo di strategia> <numeri da sommare se N>\n");
        return EXIT_FAILURE;
    }

    // convert to integer the number to sum and strategy to apply
    N = atoi(argv[1]);
    strategy = atoi(argv[2]);

    // MPI initialization
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    //if(check_if_inputs_are_valid(argc, N, strategy) != 0)
      //  return EXIT_FAILURE;

    // fill array
    //get_numbers(elements, N, argv);
 
    
    if(menum == 0){
        // array malloc
        elements = (int *)malloc(sizeof(int) * N);
        if(elements == NULL){
            fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
            return EXIT_FAILURE;
        }

        if(N > 20){
            srand(time(NULL));
            printf("Generazione numeri randomici...\n");

            for(int i = 0; i < N; i++){
                elements[i] = rand() % 100;
                printf("Element %d: %d\n", i, elements[i]);
            }
        }
        else{
            printf("Inserimento dei numeri forniti da terminale...\n");

            for(int i = 0; i < N; i++){
                elements[i] = 1; //da rimuovere !!!
                //elements[i] = atoi(argv[i + 3]);    // 0: name src; 1: N; 2: strategy; starting from 3 we have all numbers
            }
        }
    }

    nloc=N / nproc;
    rest=N % nproc;

    if(menum<rest){
        nloc=nloc+1;
    }
    values = (int *)malloc(sizeof(int) * nloc);
    
    //Seleziona la strategia a seconda se il numero di elementi è potenza di 2, 
    // se lo e' seleziona la strategia sceltaa se sono la 2 o la 3 altrimenti passa alla 1
    if((strategy == 2 || strategy == 3) && ((nproc & (nproc - 1)) != 0)){
        strategy = 1;
    }

    //per risparmiare tempo computazionale
    if(strategy == 2 || strategy == 3){
        logNproc = log2(nproc); 
    }

    // fare funzione esterna
    //il processo 0 invia a tutti gli altri processi il valore con tag 10+i
    j=0;
    if(menum == 0){
        for(i=0, j=0; i < N; i += nproc){
                values[j] = elements[i];
                j++;
        }

        for(i=1; i<nproc; i++){
            tag= 10+i;
            for(j=0; j<N; j += nproc){
                MPI_Send(&elements[j], 1, MPI_INT, i, tag, MPI_COMM_WORLD); 
            }
        }        
    }else{ //gli altri processi ricevono il valofore dal processo 0 con tag 10+numero_processo
        while(j<nloc){
            tag = 10 + menum;
            MPI_Recv(&n, nloc, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
            values[j] = n;
            j++;
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    t0 = MPI_Wtime();
    sum = 0;
    for(i = 0; i < nloc; i++){
        sum = sum + values[i];
    }

    // check the strategy to apply
    if(strategy == 1){
        if(menum == 0){
            for(int i=1; i<nproc; i++){
                tag = 80+i;
                MPI_Recv(&sumparz, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
                sum = sum+sumparz;
            }
        }else{
            tag = menum+80;
            MPI_Send(&sum, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
        }
        t1 = MPI_Wtime();
    }
    else if(strategy == 2){ //second_strategy
        for(int i=0; i<logNproc; i++){
            int partner;

            if ((menum % (int)pow(2, i + 1)) < (1 << i)) {
                partner = menum + (1 << i);
                tag = 60 + i;

                // Ricevi da menum + 2^i
                MPI_Recv(&sumparz, 1, MPI_INT, partner, tag, MPI_COMM_WORLD, &status);
                sum += sumparz;
            } else {
                partner = menum - (1 << i);
                tag = 60 + i;

                // Invia a menum - 2^i
                MPI_Send(&sum, 1, MPI_INT, partner, tag, MPI_COMM_WORLD);
            }
            t1 = MPI_Wtime();
        }
    } else{ //third_strategy
        for(int i=0; i<logNproc; i++){
            int partner = menum ^ (1 << i); // Calcola il processo partner

            if (menum < partner) {
                int send_tag = 40 + i;
                int recv_tag = 40 + i;

                // Invia la somma locale al processo partner
                MPI_Send(&sum, 1, MPI_INT, partner, send_tag, MPI_COMM_WORLD);

                // Ricevi la somma del processo partner
                MPI_Recv(&sumparz, 1, MPI_INT, partner, recv_tag, MPI_COMM_WORLD, &status);

                // Aggiorna la variabile 'sum' con la somma ricevuta
                sum += sumparz;
            } else {
                int send_tag = 40 + i;
                int recv_tag = 40 + i;

                // Ricevi la somma dal processo partner
                MPI_Recv(&sumparz, 1, MPI_INT, partner, recv_tag, MPI_COMM_WORLD, &status);

                // Invia la somma locale al processo partner
                MPI_Send(&sum, 1, MPI_INT, partner, send_tag, MPI_COMM_WORLD);
                sum += sumparz; 
            }
            t1 = MPI_Wtime();
        }
    }

    timeP = t1 - t0;
    printf("Il tempo impiegato da %d è di %e s\n", menum, timeP);

    MPI_Reduce(&timeP, &timetot, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);    

    // print result
    if(strategy == 1){
        if(menum == 0){
            printf("\nSomma totale = %d\n", sum);
            printf("Tempo totale impiegato per l'algoritmo %e\n", timetot);
        }
    }else{
        printf("\nSono il processo %d: e la somma totale = %d\n", menum, sum);
        if(menum == 0)
            printf("Tempo totale impiegato per l'algoritmo %e\n", timetot);
    }

    // freeing memory before program termination
    if(menum == 0){ 
        free(elements);
        free(values);
    }else{
        free(values);
    }
    MPI_Finalize();
    
    return 0;
}

int strategy_is_valid(int strategy){
    if(strategy < 1 && strategy > 3)
        return EXIT_FAILURE;

    // it's valid 
    return 0;
}
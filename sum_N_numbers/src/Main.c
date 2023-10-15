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
int strategy_is_valid(int strategy);

/****************************************************/

int main(int argc, char *argv[]){
    int menum;              // id del processore
    int nproc;              // numero processori
    int tag;                // identificativo del messaggio
    int N;                  // numero di elementi da sommare
    int sum;                // somma totale da stampare
    int logNproc;           // numero di passi da effettuare per la II, III strategia
    int *elements_loc;      // vettore di elementi locale
    int strategy;           // strategia con cui sommare 
    int *elements;          // array completo
    int nloc;               // numero di elementi che ciascun processore deve sommare
    int rest;               // resto della divisione 
    int sumparz = 0;        // somma parziale di ciascun processo
    double end_time;
    double start_time;
    double timetot = 0;
    MPI_Status status;      // per chiamate bloccanti

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

    if(menum == 0){
        if(check_if_inputs_are_valid(argc, N, strategy) != 0)
        return EXIT_FAILURE;

        // array malloc
        elements = (int *)malloc(sizeof(int) * N);
        if(elements == NULL){
            fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
            return EXIT_FAILURE;
        }
        fill_array(elements, N, argv);

        // Seleziona la strategia a seconda se il numero di elementi è potenza di 2, 
        // se lo e' seleziona la strategia sceltaa se sono la 2 o la 3 altrimenti passa alla 1
        if(((strategy == 2 || strategy == 3) && ((nproc & (nproc - 1)) != 0)) || (nproc == 1)){
            strategy = 1;
        }

        //per risparmiare tempo computazionale
        if(strategy == 2 || strategy == 3){
            logNproc = log2(nproc); 
        }
    }
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    nloc = N / nproc;
    rest = N % nproc;

    if(menum < rest){
        nloc = nloc + 1;
    }
    elements_loc = (int *)malloc(sizeof(int) * nloc);

    // invia elementi da sommare agli altri processori
    if(menum == 0){
        for(int i = 0; i < N; i++){
            elements_loc[i] = elements[i];
        }

        int tmp = nloc;
        int start = 0;
        for(int i = 0; i < nproc; i++){
            start += tmp;
            tag = 22 + i;
            if(i == rest)
                tmp -= 1;
            MPI_Send(&elements[start], tmp, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
    } else {
        tag = 22 + menum;
        MPI_Recv(elements_loc, nloc, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    }
    
    // attendiamo che i processi si sincronizzino
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    sum = 0;
    for(int i = 0; i < nloc; i++){
        sum = sum + elements_loc[i];
    }

    // check the strategy to apply
    if(strategy == 1){
        if(menum == 0){
            for(int i = 1; i < nproc; i++){
                tag = 80 + i;
                MPI_Recv(&sumparz, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
                sum = sum+sumparz;
            }
        }else{
            tag = menum + 80;
            MPI_Send(&sum, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
        }
        end_time = MPI_Wtime();
    }else if(strategy == 2){ //second_strategy
        for(int i = 0; i < logNproc; i++){
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
        }
        end_time = MPI_Wtime();
    } else{ // third_strategy
        for(int i = 0; i < logNproc; i++){
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
        }
        end_time = MPI_Wtime();
    }

    double timeP = end_time - start_time;
    printf("Il tempo impiegato da %d è di %e s\n", menum, );

    MPI_Reduce(&timeP, &timetot, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);    

    // print result
    if(strategy == 1){
        if(menum == 0){
            printf("\nSomma totale = %d\n", sum);
            printf("Tempo totale impiegato per l'algoritmo %e\n", timetot);
        }
    }else{
        printf("\nSono il processo %d e la somma totale = %d\n", menum, sum);
        if(menum == 0)
            printf("Tempo totale impiegato per l'algoritmo %e\n", timetot);
    }

    // freeing memory before program termination
    if(menum == 0){ 
        free(elements);
        free(elements_loc);
    }else{
        free(elements_loc);
    }
    MPI_Finalize();
    
    return 0;
}

int check_if_inputs_are_valid(int argc, int N, int strategy){
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


/*
Da risolvere:
-solo menum==0 deve avere tutti i dati
-capire come effettuare il passaggio utilizando xloc
-elements è diverso da n e viene distribuito n
-strategia 2 e 3 sono invertite?
-vengono creati file con backtrace
*/
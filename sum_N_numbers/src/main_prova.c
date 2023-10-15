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

    // MPI initialization
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if(menum == 0){
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

        fill_array(elements, N, argv);

        // Verifica se la strategia 2 (o 3) è applicabile: se il numero dei processori non è potenza di 2 applica la strategia 1
        if(!strategy_2_OR_3_are_applicable(strategy, nproc))
            strategy = STRATEGY_1;
    }
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&strategy, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //per risparmiare tempo computazionale
    if(strategy == 2 || strategy == 3){
        logNproc = log2(nproc); 
    }

    nloc = N / nproc;
    rest = N % nproc;

    if(menum < rest){
        nloc = nloc + 1;
    }

    elements_loc = (int *)malloc(sizeof(int) * nloc);
    if(elements_loc == NULL){
        fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
        return EXIT_FAILURE;
    }
    
    // invia elementi da sommare agli altri processori
    if(menum == 0){
        for(int i = 0; i < nloc; i++){
            elements_loc[i] = elements[i];
        }

        int tmp = nloc;
        int start = 0;
        for(int i = 1; i < nproc; i++){
            start += tmp;
            tag = 22 + i;
            if(i == rest) tmp -= 1;
            
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
    int nsum = 0;
    for(int i = 0; i < nloc; i++){
        sum = sum + elements_loc[i];
        nsum += 1;
    }

    nsum --;
    printf("Sono %d e ho fatto %d addizioni locali.\n", menum, nsum);

    // check the strategy to apply
    if(strategy == STRATEGY_1){
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
    }else if(strategy == STRATEGY_2){ 
        for(int i = 0; i < logNproc; i++){
            int partner;
            
            if((menum % (int)pow(2, i)) == 0){
                
                if (menum % (int)pow(2, i + 1) == 0){
                    partner = menum + pow(2, i);
                    tag = 60 + i;
                    MPI_Recv(&sumparz, 1, MPI_INT, partner, tag, MPI_COMM_WORLD, &status);
                    sum += sumparz;
                    nsum ++;
                }
                else{
                    partner = menum - pow(2, i);
                    tag = 60 + i;
                    MPI_Send(&sum, 1, MPI_INT, partner, tag, MPI_COMM_WORLD);
                }
            }
        }
        end_time = MPI_Wtime();
    } else{ // third_strategy
        for(int i = 0; i < logNproc; i++){
            int partner;

            if ((menum % (int)pow(2, i + 1)) < (int)pow(2, i)) {
                partner = menum + (int)pow(2, i);
                int send_tag = 40 + i;
                int recv_tag = 40 + i;

                // Invia la somma locale al processo partner
                MPI_Send(&sum, 1, MPI_INT, partner, send_tag, MPI_COMM_WORLD);

                // Ricevi la somma del processo partner
                MPI_Recv(&sumparz, 1, MPI_INT, partner, recv_tag, MPI_COMM_WORLD, &status);

                // Aggiorna la variabile 'sum' con la somma ricevuta
                sum += sumparz;
            } else {
                partner = menum - (int)pow(2, i);
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
    printf("Il tempo impiegato da %d è di %e s\n", menum, timeP);

    MPI_Reduce(&timeP, &timetot, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);    

    // print result
    if(strategy == 1){
        if(menum == 0){
            printf("\nSomma totale = %d\n", sum);
            printf("Tempo totale impiegato per l'algoritmo %e\n", timetot);
        }
    }else{
        printf("Sono %d e il numero di somme eseguite alla fine e' %d\n", menum, nsum);
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



/*
Da risolvere:
-solo menum==0 deve avere tutti i dati
-capire come effettuare il passaggio utilizando xloc
-elements è diverso da n e viene distribuito n
-strategia 2 e 3 sono invertite?
-vengono creati file con backtrace
*/
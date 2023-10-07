/**
 * @author Fabrizio Vitale
 * @author Giovanni Falcone
 * @author Luigi Mangiacapra
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "mpi.h"

/* ******************************************************* */
/*                      SUPPORT FUNCTION                   */
/* ******************************************************* */

static void fill_array_randomly(int *elements, int N);

static void fill_array_by_argv(int *elements, int N, char *argv[]);

/* ******************************************************* */

extern int menum;
extern int nproc;
extern int tag;
extern int sumparz;
extern MPI_Status status;
extern int sum;
extern int logNproc;

int strategy_is_valid(int strategy){
    if(strategy < 1 && strategy > 3)
        return EXIT_FAILURE;

    // it's valid 
    return 0;
}

void get_numbers(int *elements, int N, char *argv[]){
    if(N > 20)
        fill_array_randomly(elements, N);
    else
        fill_array_by_argv(elements, N, argv);
}

void fill_array_randomly(int *elements, int N){
    srand(time(NULL));

    printf("Generazione numeri randomici...\n");

    for(int i = 0; i < N; i++){
        elements[i] = rand() % 100;
        printf("Element %d: %d\n", i, elements[i]);
    }
}

void fill_array_by_argv(int *elements, int N, char *argv[]){
    printf("Inserimento dei numeri forniti da terminale...\n");

    for(int i = 0; i < N; i++){
        elements[i] = atoi(argv[i + 3]);    // 0: name src; 1: N; 2: strategy; starting from 3 we have all numbers
    }
}

void first_strategy(){
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
}

void second_strategy(){
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
        }
    }
}

void third_strategy(){
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
    }
}
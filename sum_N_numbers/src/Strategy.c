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

int menum;
int nproc;
int tag;
int sumparz;
MPI_Status status;
int sum;
int logNproc;

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
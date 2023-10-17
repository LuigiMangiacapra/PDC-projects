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


int first_strategy(int menum, int nproc, int sum){
    int sum_parz = 0;
    int tag;
    MPI_Status status;

    if(menum == 0){
        for(int i = 1; i < nproc; i++){
            tag = 80 + i;
            MPI_Recv(&sum_parz, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
            sum += sum_parz;
        }
    }else{
        tag = menum + 80;
        MPI_Send(&sum, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    }

    return sum;
}

int second_strategy(int menum, int logNproc, int *array, int sum){
    int sum_parz = 0;
    int tag;
    int partner;

    int power_for_partecipation;
    int does_processor_partecipate;

    int power_for_communication;
    int does_processor_receive;

    MPI_Status status;

    for(int i = 0; i < logNproc; i++){
        power_for_partecipation =  array[i];
        does_processor_partecipate = (menum % power_for_partecipation) == 0;

        if(does_processor_partecipate){
            power_for_communication = array[i + 1];
            does_processor_receive = (menum % power_for_communication) == 0;

            if (does_processor_receive){
                partner = menum + power_for_partecipation;
                tag = 60 + i;
                MPI_Recv(&sum_parz, 1, MPI_INT, partner, tag, MPI_COMM_WORLD, &status);
                sum += sum_parz;
            }
            else{
                partner = menum - power_for_partecipation;
                tag = 60 + i;
                MPI_Send(&sum, 1, MPI_INT, partner, tag, MPI_COMM_WORLD);
            }
        }
    }

    return sum;
}

int third_strategy(int menum, int logNproc, int *array, int sum){
    int partner;
    int send_tag;
    int recv_tag;
    int sum_parz;
    MPI_Status status;

    sum_parz = 0;
    for(int i = 0; i < logNproc; i++){
        if ((menum % array[i + 1]) < array[i]) {
            partner = menum + array[i];
            send_tag = 40 + i;
            recv_tag = 40 + i;

            // Invia la somma locale al processo partner
            MPI_Send(&sum, 1, MPI_INT, partner, send_tag, MPI_COMM_WORLD);

            // Ricevi la somma del processo partner
            MPI_Recv(&sum_parz, 1, MPI_INT, partner, recv_tag, MPI_COMM_WORLD, &status);

            // Aggiorna la variabile 'sum' con la somma ricevuta
            sum += sum_parz;
        } else {
            partner = menum - array[i];
            send_tag = 40 + i;
            recv_tag = 40 + i;

            // Ricevi la somma dal processo partner
            MPI_Recv(&sum_parz, 1, MPI_INT, partner, recv_tag, MPI_COMM_WORLD, &status);

            // Invia la somma locale al processo partner
            MPI_Send(&sum, 1, MPI_INT, partner, send_tag, MPI_COMM_WORLD);
            sum += sum_parz;
        }
    }

    return sum;
}
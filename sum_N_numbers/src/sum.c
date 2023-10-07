#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
//#include "mpi.h"

int main(int argc, char *argv[]){
    int menum, nproc, tag, i;
    int n, nloc, N, strategia, sum, logNproc, randomNumber;
    int *x, *xloc;
    MPI_Status status;
    int sumparz;

    //Parte argv e argc
    N = atoi(argv[1]);
    strategia = atoi(argv[2]);

    //Controllo input non validi
    if (N <= 0 || strategia < 1 || strategia > 3) {
        printf("Valori in input non validi.\n");
        return 1;
    }

    srand(time(NULL));

    //Se il numero di elementi è 20 o inferiore riempie l'array con i valori in input, altrimenti con valori randomici
    if(N <= 20){
        xloc = (int *)malloc(N * sizeof(int));
        if (xloc == NULL) {
            printf("Errore nell'allocazione della memoria per elements.\n");
            return 1;
        }
        for (int i = 0; i < N; i++) {
            xloc[i] = atoi(argv[i + 3]); // Argv[0] è il nome del programma, quindi iniziamo da argv[3].
        }
    }
    else{
        xloc = (int *)malloc(N * sizeof(int));
        if (xloc == NULL) {
            printf("Errore nell'allocazione della memoria per elements.\n");
            return 1;
        }
        for (int i = 0; i < N; i++) {
            xloc[i] = rand(); // Genera numeri casuali.
        }
    }

    //Seleziona la strategia a seconda se il numero di elementi è potenza di 2, se lo è seleziona la strategia sceltaa se sono la 2 o la 3 altrimenti passa alla 1
    if((strategia == 2 || strategia == 3) && ((N & (N-1)) != 0)){
        strategia = 1;
    }

    if(strategia == 2 || strategia == 3){
        logNproc = log2(nproc); //per risparmiare tempo computazionale
    }

    MPI_Init(&argv, &argc);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    //il processo 0 invia a tutti gli altri processi il valore con tag 10+i
    if(menum==0){
        //Lettura dei dati di input
        for(i=1; i<nproc; i++){
            tag = 10+i;
            MPI_Send(&n, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
    }else{//gli altri processi ricevono il valore dal processo 0 con tag 10+numero_processo
        tag=10+menum;
        MPI_Recv(&n, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    }
    

    sum=0;
    for(i=0; i<N; i++){
        sum=sum+xloc[i];
    }

    /*strategia 1*/
    if(strategia == 1){
        if(menum == 0){
            for(i=1; i<nproc; i++){
                tag = 80+i;
                MPI_Recv(&sumparz, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
                sum = sum+sumparz;
            }
        }else{
            tag = menum+80;
            MPI_Send(&sum, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
        }
    }else if(strategia == 2){
        for(i=0; i<logNproc; i++){
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
    }else{
        for(i=0; i<logNproc; i++){
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

    if(strategia == 1){
        if(menum == 0)
            printf("\nSomma totale=%d\n", sum);
    }else{
        printf("\nSono il processo %d: Somma totale= %d\n", menum, sum);
    }

    return 0;
}
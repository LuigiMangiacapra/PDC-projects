#include <stdio.h>
#include <math.h>
#include "mpi.h"

int main(int argc, char *argv[]){
    int menum, nproc, ...;
    int n, nloc, tag, i, ...;
    int N, strategia;
    int sum;
    int *x, *xloc;
    MPI_Status status;
    int sumparz;
    int logNproc;
    int randomNumber;
    int *elements;

    //Parte argv e argc
    N = atoi(argv[1]);
    strategia = atoi(argv[2]);

    if (N <= 0 || strategia < 1 || strategia > 3) {
        printf("Valori in input non validi.\n");
        return 1;
    }

    srand(time(NULL));

    if(N <= 20){
        elements = (int *)malloc(N * sizeof(int));
        if (elements == NULL) {
            printf("Errore nell'allocazione della memoria per elements.\n");
            return 1;
        }
        for (int i = 0; i < N; i++) {
            elements[i] = atoi(argv[i + 3]); // Argv[0] è il nome del programma, quindi iniziamo da argv[3].
        }
    }else{
        elements = (int *)malloc(N * sizeof(int));
        if (elements == NULL) {
            printf("Errore nell'allocazione della memoria per elements.\n");
            return 1;
        }
        for (int i = 0; i < N; i++) {
            elements[i] = rand(); // Genera numeri casuali.
        }
    }

    if((strategia == 2 || strategia == 3) && ((N & (N-1)) != 0)){
        strategia = 1;
    }

    if(strategia == 2 || strategia == 3){
        logNproc = log2(nproc); //per risparmiare tempo computazionale
    }

    MPI_Init(&argv, &argc);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if(menum==0){
        //Lettura dei dati di input
        for(i=1; i<nproc; i++){
            tag = 10+i;
            MPI_Send(&n, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
    }else{
        tag=10+menum;
        MPI_Recv(&n, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    }
    
    sum=0;
    for(i=0; i<nloc; i++){
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
            if((menum % pow(2, i)) == 0){
                if((menum % pow(2, i+1)) == 0){
                    tag = 40+i;
                    //MPI_Recv(&sumparz, 1, MPI_INT, ?, tag, MPI_COMM_WORLD, &status);
                    //Ricevi da menum+2
                    sum = sum+sumparz;
                }else{
                    tag = 40+i;
                    //MPI_Send(&sum, 1, MPI_INT, ? , tag, MPI_COMM_WORLD);
                    //Invia a menum-2
                }
            }
        }
    }else{
        for(i=0; i<logNproc; i++){
            if((menum % pow(2, i+1)) < 2^i){
                tag = 60+i;
                //Ricevi da menum+2^i
                //Invia a menum-2^i
                sum = sum+sumparz;
            }else{
                tag = 60+i;
                //Ricevi da menum+2^i
                //Invia a menum-2^i
                sum = sum+sumparz;
            }
        }
    }

    if(strategia == 1){
        if(menum == 0)
            printf("\nSomma totale=%d\n", sum);
    }else{
        printf("\nSono il processo %d: Somma totale= %d\n", menum, sum)
    }

    return 0;
}
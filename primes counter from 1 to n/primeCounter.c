#include <stdio.h>
#include <string.h>
#include "mpi.h"

#define ARRAY_SIZE 100005
#define MASTER_RANK 0
int prime[ARRAY_SIZE+5];

void initPrimeArray(){
    int i =0;
    for(i;i<ARRAY_SIZE;i++){
        prime[i] = 1;
    }
    prime[0] = prime[1] = 0;
    int p = 2;
    for(p; p*p <= ARRAY_SIZE; p++){
        if(prime[p] == 1){
            i = 0;
            for(i = p*p; i <= ARRAY_SIZE; i += p){
                prime[i] = 0;
            }
        }
    }
}
int isPrime(int number){
    return prime[number];
}



int main(int argc , char * argv[])
{
    int x = 0, y = 0;
    int my_rank;
    int p;
    int source;
    int tag = 0;
    int message = 0;
    int range = 0 ;
    int start = 0, end = 0;
    MPI_Status status;
    MPI_Init(&argc , &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == MASTER_RANK) {
        initPrimeArray(); /// preprocessing all prime values
        printf("Enter x: ");
        scanf("%d", &x);
        printf("Enter y: ");
        scanf("%d", &y);

        range = (y - x + 1) / (p - 1);
        if((p - 1) > (y - x + 1)){
            range = 1;
        }
        start = x;
        int i = 1;
        for(i; i < p; i++){
            end = start + range;
            if(i == p - 1){
                end = y + 1;
            }
            if(end > y + 1){
                end = -1;
            }
            MPI_Send(&start, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
            MPI_Send(&end, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
            MPI_Send(&prime, ARRAY_SIZE, MPI_INT, i, tag, MPI_COMM_WORLD);
            start += range;
        }
    }


    if (my_rank != MASTER_RANK) {
        MPI_Recv(&start, 1, MPI_INT, MASTER_RANK, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&end, 1, MPI_INT, MASTER_RANK, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&prime, ARRAY_SIZE, MPI_INT, MASTER_RANK, tag, MPI_COMM_WORLD, &status);
        int i = start;
        for(i;i<end;i++){
            message += isPrime(i);
        }
        printf("Total number of primes in P%d is %d\n", my_rank, message);
        MPI_Send(&message, 1, MPI_INT, MASTER_RANK, tag, MPI_COMM_WORLD);
    }
    else {
        int total = 0;
        for (source = 1; source < p; source++) {
            MPI_Recv(&message, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
            total += message;
        }
        printf("total number of prime numbers is:%d\n", total);
    }

    MPI_Finalize();
    return 0;
}

#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include <stdlib.h>
#include <time.h>

#define MASTER 0
#define MAX_LINE_LENGTH 100
#define HEADERS_SIZE 2


int** extractVoters(char *fileName, int candidatesCount, int votersCount,int startingLine, int endingLine, int rank) {
    FILE* file = fopen(fileName, "r");
    char line[MAX_LINE_LENGTH];
    int **frequency = (int **)malloc((votersCount + 5) * sizeof(int *));
    for (int i = 0; i < votersCount + 5; ++i) {
        frequency[i] = (int *)malloc((candidatesCount + 5) * sizeof(int));
    }
    for (int i = 0; i < votersCount + 5; ++i) {
        for (int j = 0; j < candidatesCount + 5; ++j) {
            frequency[i][j] = 0;
        }
    }

    if(file == NULL){
        printf("Error in parsing Voters file , Exiting process\n");
        for(int i = 0; i < votersCount; i++) {
            free(frequency[i]);
        }
        free(frequency);
        fclose(file);
        exit(0);
    }

    for (int i = 1; i <= startingLine; i++) {
        fgets(line, MAX_LINE_LENGTH, file);
    }

    int currentLine = startingLine ;
    int i = 0 ;
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL && currentLine < endingLine) {
        char *token = strtok(line, " ");
        int currentPart = 0;
        while (currentPart < candidatesCount) {
            int chosenCandidate = atoi(token);
            frequency[i][currentPart] = chosenCandidate;
            token = strtok(NULL, " ");
            currentPart++;
        }
        i++;
        currentLine++;
    }
    fclose(file);
    return  frequency;
}

int readLineAsInt(const char *fileName, int lineNum) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    int value;
    for(int i = 0; i < lineNum; i++){
        if (fscanf(file, "%d", &value) != 1) {
            fclose(file);
            perror("Error reading first line as integer");
            return -1;
        }
    }
    fclose(file);
    return value;
}


int main(int argc, char *argv[]) {
    int my_rank;
    int p;
    int tag = 0;
    char fileName[100];
    int candidatesCount;
    int votersCount;
    int sum = 0;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == MASTER) {
        int choice;
        printf("please select option\n"
               "1- Generate File\n"
               "2- Enter Existing File\n"
               ">>> ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                strcpy(fileName, "election.txt");
                FILE *file = fopen("election.txt", "w");
                printf("Enter candidates count: ");
                scanf("%d", &candidatesCount);
                printf("Enter voters count: ");
                scanf("%d", &votersCount);
                fprintf(file, "%d\n%d\n", candidatesCount, votersCount);
                for (int i = 0; i < votersCount; i++) {
                    printf("Enter voter number %d preferences: ", i + 1);
                    for (int j = 0; j < candidatesCount; j++) {
                        int preference;
                        scanf("%d", &preference);
                        fprintf(file, "%d ", preference);
                    }
                    fprintf(file, "\n");
                }
                fclose(file);
                break;
            }
            case 2: {
                printf("Enter the fileName: ");
                scanf("%s", fileName);
                FILE* file = fopen(fileName, "r");
                if (file == NULL) {
                    printf("File not found\n");
                    MPI_Finalize();
                    return 0;
                }
                candidatesCount = readLineAsInt(fileName, 1);
                votersCount = readLineAsInt(fileName, 2);
                fclose(file);
                break;
            }
            default:
                printf("wrong input");
                break;
        }
    }

    MPI_Bcast(&candidatesCount, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&votersCount, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&fileName, 100, MPI_CHAR, MASTER, MPI_COMM_WORLD);


    int isVotableCandidate[candidatesCount + 1];
    for(int i = 0; i <= candidatesCount; i++) {
        isVotableCandidate[i] = 1;
    }

    for(int currRound = 0; currRound < 2 ; currRound++){
        int candidatesVotes[candidatesCount + 1];
        for(int i = 0; i <= candidatesCount; i++)
            candidatesVotes[i] = 0;
        int currProcessVotersCnt = votersCount / p;
        if(my_rank == p - 1){
            currProcessVotersCnt = votersCount - (currProcessVotersCnt * (p - 1));
        }
        int start = HEADERS_SIZE + (my_rank * (votersCount / p));
        int end = start + currProcessVotersCnt;
        printf("process %d reads from voter %d to voter  %d at round %d\n", my_rank, start - HEADERS_SIZE + 1, end - HEADERS_SIZE + 1, currRound + 1);

        int** voters = extractVoters(fileName, candidatesCount, votersCount, start, end, my_rank);
        int i;
        for(i = 0; i < currProcessVotersCnt; i++){
            int j = 0;
            while (!isVotableCandidate[voters[i][j]]) {
                j++;
            }
            candidatesVotes[voters[i][j]]++;
        }
        if(my_rank != MASTER){
            MPI_Send(&candidatesVotes, candidatesCount + 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
        }
        else{
            int candidatesVotesReceive[candidatesCount + 1];
            int source;
            for(source = 1; source < p ; source++)
            {
                MPI_Recv(&candidatesVotesReceive, candidatesCount + 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
                for(i = 1; i <= candidatesCount; i++){
                    candidatesVotes[i] += candidatesVotesReceive[i];
                }
            }
            int firstPlaceVotes = 0;
            int firstPlaceCandidate = 0;
            double totalVotes = 0;
            for(i = 1; i <= candidatesCount; i++){
                if(firstPlaceVotes < candidatesVotes[i]) {
                    firstPlaceVotes = candidatesVotes[i];
                    firstPlaceCandidate = i;
                }
                if(isVotableCandidate[i] == 1)
                    totalVotes += candidatesVotes[i];
            }

            printf("===========round %d votes percentage===========\n", currRound + 1);
            for(i = 1; i <= candidatesCount; i++){
                if(isVotableCandidate[i] == 1){
                    printf("candidate number %d got %f%% of total votes\n", i, (candidatesVotes[i]/totalVotes)*100);
                }
            }

            if(currRound == 0 && firstPlaceVotes * 2 > votersCount){
                printf("candidate %d won in round 1\n", firstPlaceCandidate);
                exit(0);
            }
            int secondPlaceVotes = 0;
            int secondPlaceCandidate = 0;
            for(i = 1; i <= candidatesCount; i++){
                if(secondPlaceVotes < candidatesVotes[i] && firstPlaceCandidate != i) {
                    secondPlaceVotes = candidatesVotes[i];
                    secondPlaceCandidate = i;
                }
            }

            for(i = 0; i <= candidatesCount; i++){
                if(i != firstPlaceCandidate && i != secondPlaceCandidate){
                    isVotableCandidate[i] = 0;
                }
            }

            if(currRound == 1){
                if(firstPlaceVotes == secondPlaceVotes){
                    printf("Draw, %d %d have the same votes\n", firstPlaceCandidate, secondPlaceCandidate);
                    exit(0);
                }
                printf("candidate %d won in round 2\n", firstPlaceCandidate);
                exit(0);
            }
            printf("====================================================\n");
        }
        MPI_Bcast(&isVotableCandidate, candidatesCount + 1, MPI_INT, MASTER, MPI_COMM_WORLD);
        for(i = 0; i < currProcessVotersCnt; i++) {
            free(voters[i]);
        }
        free(voters);
    }
    MPI_Finalize();
    return 0;
}

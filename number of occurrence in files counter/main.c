#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include <stdlib.h>
#include <time.h>

#define MASTER 0

#define FILES_COUNT 100
#define FILE_NAME_LENGTH 100
#define LINES_PER_FILE 100
#define NUMS_PER_FILE 100

#define MAX_NUM 100
#define MIN_NUM 1

// TODO
// NOTE to change numbers created in files change return value of this function for testing
int createRandomNumber() {
    return (rand() % MAX_NUM) + MIN_NUM;
//    return 10;
}


void createFileName(int fileNumber, char *directory, char *destination) {
    sprintf(destination, "%s/%d.txt", directory, fileNumber);
}

void createFiles(int files[], int filesCount, char *directoryName) {
    for (int i = 0; i < filesCount; i++) {
        files[i] = i+1;
        char fileName[FILE_NAME_LENGTH];
        createFileName(i+1,directoryName,fileName);
        FILE *filePointer = fopen(fileName, "w");
        if (filePointer == NULL) {
            printf("Unable to create %d files Terminating process , Try Again.\n", i);
            perror("Failed to open file\n");
            continue;
        }
        for (int j = 1; j <= LINES_PER_FILE; j++) {
            fprintf(filePointer, "%d\n", createRandomNumber());
        }
        fclose(filePointer);
    }
}

void extractNumbersFromFile(char *fileName, int* numbers) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    char line[LINES_PER_FILE];
    int count = 0;
    while (fgets(line, sizeof(line), file) != NULL && count < NUMS_PER_FILE) {
        numbers[count] = atoi(line);
        count++;
    }
    fclose(file);
}

int getNumOfOccurrencesInFile(int fileNumber, char* directoryName, int targetNumber){
    int cnt = 0;
    char fileName[FILE_NAME_LENGTH];
    createFileName(fileNumber, directoryName,fileName);
    int numbers [NUMS_PER_FILE];
    extractNumbersFromFile(fileName,numbers);
    for (int j = 0; j < NUMS_PER_FILE; j++) {
        if (numbers[j] == targetNumber)
            cnt++;
    }
    return cnt;
}


int main(int argc, char *argv[]) {
    int my_rank;
    int p;
    int tag = 0;
    char directoryName[100];
    int files[FILES_COUNT];
    int receiveCount = 0;
    int sendCount = 0;
    int localFiles[FILES_COUNT];
    int targetNumber;
    int frequency = 0;
    int sum = 0;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == MASTER) {
        printf("enter Directory name: ");
        scanf("%s", directoryName);
        printf("Enter Target Number: ");
        scanf("%d", &targetNumber);
        createFiles(files, FILES_COUNT, directoryName);
        sendCount = receiveCount = FILES_COUNT / p;
    }

    MPI_Bcast(&receiveCount, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&targetNumber, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&directoryName, 100, MPI_CHAR, MASTER, MPI_COMM_WORLD);
    MPI_Scatter(files, sendCount, MPI_INT, localFiles, receiveCount, MPI_INT, MASTER,
                MPI_COMM_WORLD);

    int cnt = 0;
    for (int i = 0; i < receiveCount; i++) {
        cnt += getNumOfOccurrencesInFile(localFiles[i], directoryName, targetNumber);
    }


    if (my_rank == MASTER) {
        int reminder = FILES_COUNT - (sendCount * p);
        for(int i = FILES_COUNT - reminder; i < FILES_COUNT; i++){
            cnt += getNumOfOccurrencesInFile(files[i], directoryName, targetNumber);

        }
    }
    printf("P%d: Total number of occurrences = %d\n", my_rank, cnt);
    MPI_Reduce(&cnt, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if(my_rank == MASTER){
        printf("Total number of occurrences in all 100 files = %d\n", sum);
    }
    MPI_Finalize();
    return 0;
}

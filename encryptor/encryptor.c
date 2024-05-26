#include <stdio.h>
#include <string.h>
#include "mpi.h"


#define MAX_LENGTH 10000
#define MASTER_RANK 0
#define SHIFT_CHAR_LEN 3
#define CONSOLE_READING 1
#define FILE_READING 2

void substring(const char *source, int start, int length, char *target) {
    strncpy(target, source + start, length);
    target[length] = '\0'; // Null-terminate the string
}

void encrypt(char *message){
    const unsigned int len = strlen(message);
    unsigned int i =0 ;
    for(i;i<len;i++){
        if((message[i] >= 'X' && message[i] <= 'Z') || (message[i] >= 'x' && message[i] <= 'z')){
            message[i] -= 26;
        }
        message[i] += SHIFT_CHAR_LEN;

    }
}

void decrypt(char *message){
    const unsigned int len = strlen(message);
    unsigned int i =0 ;
    for(i;i<len;i++){
        if((message[i] >= 'A' && message[i] <= 'C') || (message[i] >= 'a' && message[i] <= 'c')){
            message[i] += 26;
        }
        message[i] -= SHIFT_CHAR_LEN  ;
    }
}

void insertPartialString(const int senderRank, const int partialStringLength, const char* partialString ,char* originalString){
    int start = (senderRank - 1) * partialStringLength;
    int i = 0;
    for(; start < start + partialStringLength && i < strlen(originalString); start++, i++){
        originalString[start] = partialString[i];
    }
}

void readFromConsole(char* string){
    printf(">>>Enter original string to be run the operation on (WITHOUT SPACES): ");
    scanf("%s", string);
}

void readFromFile(char* string){
    char filePath[MAX_LENGTH];
    printf(">>>please enter the file path: ");
    scanf("%s", filePath);
    FILE *file;
    file = fopen(filePath, "r");
    if(file != NULL){
        if (fgets(string,  MAX_LENGTH, file) != NULL) {
            printf("String read from file is : %s\n", string);
        } else {
            perror("Error reading from file");
        }
        fclose(file);
    }
}

int main(int argc , char * argv[])
{
    char originalString[MAX_LENGTH];
    int myRank;
    int p;
    int source;
    int dataTag = 0;
    int encryptionOptionTag = 1;
    char partialData[MAX_LENGTH];
    int isDecryption = 0;
    int partialStringLength = 0 ;
    int numberOfUsedProcesses = 0;
    MPI_Status status;
    MPI_Init(&argc , &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (myRank == MASTER_RANK) {
        // send to all
        int readingOption;
        printf(">>>Enter Reading option: \n");
        printf("1- Console(std-in ,std-out)\n2- File\n>> ");
        scanf("%d", &readingOption);

        if(readingOption == CONSOLE_READING){
            readFromConsole(originalString);
        }
        else if(readingOption == FILE_READING){
            readFromFile(originalString);
        }
        if(strlen(originalString) == 0){
            printf("COULDN'T read string ABORTING APPLICATION............\n");
            return 0;
        }
        printf(">>>please choose encoding/decoding option: \n1- encryption\n2-decryption\n>> ");
        scanf("%d", &isDecryption);
        isDecryption--;

        partialStringLength = (strlen(originalString)) / (p - 1) ;
        int isProcessesMoreThanStringLength = (p > strlen(originalString));
        if(isProcessesMoreThanStringLength){
            partialStringLength = 1;
            numberOfUsedProcesses = strlen(originalString);
        }
        else {
            numberOfUsedProcesses = p - 1;
        }
        int i;
        for(i=1;i < p;i++){
            if(i > numberOfUsedProcesses){
                MPI_Send(NULL, 0, MPI_CHAR, i, dataTag, MPI_COMM_WORLD);
                MPI_Send(&isDecryption, 1, MPI_INT, i, encryptionOptionTag, MPI_COMM_WORLD);
                continue;
            }
            int start = (i-1) * partialStringLength;
            if(i == p - 1){
                int remainingLength = strlen(originalString) - (partialStringLength * (p - 2))  ;
                substring(originalString, start, remainingLength, partialData);
            }else{
                substring(originalString, start, partialStringLength, partialData);
            }
            MPI_Send(&partialData, strlen(partialData) + 1, MPI_CHAR, i, dataTag, MPI_COMM_WORLD);
            MPI_Send(&isDecryption, 1, MPI_INT, i, encryptionOptionTag, MPI_COMM_WORLD);
        }
    }

    else {
        MPI_Recv(&partialData, MAX_LENGTH, MPI_CHAR, MASTER_RANK, dataTag, MPI_COMM_WORLD, &status);
        MPI_Recv(&isDecryption, 1, MPI_INT, MASTER_RANK, encryptionOptionTag, MPI_COMM_WORLD, &status);
        if(strlen(partialData) != 0 ){
            if(!isDecryption){
                encrypt(partialData);
            }
            else{
                decrypt(partialData);
            }
            MPI_Send(&partialData, strlen(partialData)+1,MPI_CHAR,MASTER_RANK, dataTag, MPI_COMM_WORLD);
        }
    }

    if (myRank == MASTER_RANK) {
        int src;
        for (src = 1; src <= numberOfUsedProcesses; src++){
            MPI_Recv(&partialData, MAX_LENGTH, MPI_CHAR, src, dataTag, MPI_COMM_WORLD, &status);
            insertPartialString(src, partialStringLength, partialData, originalString);
        }
        printf("data after encoding/decoding (Output): %s\n",originalString);
    }
    MPI_Finalize();
    return 0;
}

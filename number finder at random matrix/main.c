#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int rows, cols, key;
    printf("enter rows count: ");
    scanf("%d", &rows);
    printf("enter cols count: ");
    scanf("%d", &cols);
    printf("enter the key: ");
    scanf("%d", &key);

    srand(time(NULL));

    int currentIndex = 0 ;
    int *idxArray = (int *)malloc(rows * cols * 2 * sizeof(int));
    int **matrix = (int **)malloc(rows * sizeof(int *));

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < rows; ++i) {
        matrix[i] = (int *)malloc(cols * sizeof(int));
    }

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = rand() % 10;
        }
    }

    #pragma omp parallel for schedule(dynamic)
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(matrix[i][j] == key){
                #pragma omp critical(idx_access)
                {
                    printf("found at row %d, col %d from process %d\n", i, j, omp_get_thread_num());
                    idxArray[currentIndex++] = i;
                    idxArray[currentIndex++] = j;
                }
            }
        }
    }

    printf("the matrix is\n");
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

    #pragma omp parallel for schedule(dynamic)
    for (int i = rows - 1; i >= 0; i--) {
        free(matrix[i]);
    }
    free(matrix);
    if(currentIndex == 0 ){
        printf("-1\n");
    }
    else{
        for(int i = 0; i < currentIndex; i+=2) {
            printf("row %d , col %d\n", idxArray[i], idxArray[i + 1]);
        }
    }

    free(idxArray);
    return 0;
}
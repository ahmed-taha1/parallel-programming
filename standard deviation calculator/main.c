#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// use
// gcc -o p3 problem3.c -fopenmp -lm
// to compile the code

int main(int argc, char *argv[]) {
    int n;
    int i;
    printf("enter n: ");
    scanf("%d", &n);
    printf("you entered %d\n", n);
    int threadsNum;
    float sum = 0;
    double globalSumDiff = 0;
//    int *arr = (int *)malloc(n * sizeof(int));
    srand(time(NULL));
    #pragma omp parallel shared(n, globalSumDiff) private(i, sum)
    {
        int *localArr = (int *)malloc(n * sizeof(int));
        # pragma for schedule(dynamic)
        for (i = 0; i < n; i++) {
            localArr[i] = rand() % 10;
        }
        printf("\narray values at process num %d are: \n", omp_get_thread_num());
        # pragma for schedule(dynamic)
        for (i = 0; i < n; i++) {
            printf("%d ", localArr[i]);
        }
        printf("\n");
        # pragma for schedule(dynamic) reduction(+:sum)
        for (i = 0; i < n; i++) {
            sum += localArr[i];
        }
        float mean = sum / n;
        printf("sum at process num %d is %d\n", omp_get_thread_num(), (int)sum);
        printf("mean at process num %d is %f\n", omp_get_thread_num(), mean);
        double localSumDiff = 0;
        # pragma for schedule(dynamic) reduction(+:localSumDiff)
        for (i = 0; i < n; i++) {
            localSumDiff += (localArr[i] - mean) * (localArr[i] - mean);
        }
        printf("local sum of squared differences at process num %d is %f\n", omp_get_thread_num(), localSumDiff);
        # pragma reduction(+:globalSumDiff)
        globalSumDiff += localSumDiff;
        threadsNum = omp_get_num_threads();
        free(localArr);
    }

    double standardDeviation = globalSumDiff / (n * threadsNum);
    standardDeviation = sqrt(standardDeviation);
    printf("\n\ntotal standard deviation is %f\n", standardDeviation);
//    int arrSize = n * threadsNum;
//    printf("threads num %d\n", threadsNum);
//
//    srand(time(NULL));
//    #pragma omp parallel for schedule(dynamic)
//    for (int i = 0; i < arrSize; ++i) {
//        arr[i] = rand() % 10;
//    }
//
//    printf("array values are: \n");
//    #pragma omp parallel for schedule(dynamic)
//    for (int i = 0; i < arrSize; ++i) {
//        printf("%d ", arr[i]);
//    }
//    printf("\n");
//
//    #pragma omp parallel for schedule(dynamic) reduction(+:sum)
//    for (int i = 0; i < arrSize; ++i) {
//        sum += arr[i];
//    }
//    float mean = sum / arrSize;
//    printf("sum is %d\n", (int)sum);
//    printf("mean is %f\n", mean);
//
//    #pragma omp parallel for schedule(dynamic) reduction(+:standardDeviationNominator)
//    for (int i = 0; i < arrSize; ++i) {
//        standardDeviationNominator += (arr[i] - mean) * (arr[i] - mean);
//    }
//
//    double standardDeviation = standardDeviationNominator / arrSize;
//    standardDeviation = sqrt(standardDeviation); // Add this line to calculate the square root
//    printf("standard deviation is %f\n", standardDeviation);
//    free(arr);
    return 0;
}
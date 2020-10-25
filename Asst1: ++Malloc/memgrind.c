#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "mymalloc.h"

#define REPEAT 50

struct timeval testA() {
    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 120; i++) {
        void* p = malloc(1);
        free(p);
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

struct timeval testB() {
    void* p[120];

    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 120; i++) {
        p[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(p[i]);
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

struct timeval testC() {
    void* p[120];
    int malloc_count = 0, free_count = 0;
    int doMalloc[240];
    srand((unsigned)time(0));
    for (int i = 0; i < 240; i++) doMalloc[i] = rand() % 2;

    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 240; i++) {
        if (malloc_count <= free_count || (doMalloc[i] && malloc_count < 120)) {
            p[malloc_count] = malloc(1);
            malloc_count++;
        }
        else {
            free(p[free_count]);
            free_count++;
        }
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

float avgTime(struct timeval* times) {
    float result = 0;
    for (int i = 0; i < REPEAT; i++) {
        result += (float)times[i].tv_sec*(float)1000;
        result += (float)times[i].tv_usec/(float)1000;
    }
    return result/(float)REPEAT;
}

int main(int argc, char**argv) {
    /* struct timeval resultA[REPEAT];
    struct timeval resultB[REPEAT];
    struct timeval resultC[REPEAT];
    for (int i = 0; i < REPEAT; i++) {
        resultA[i] = testA();
        resultB[i] = testB();
        resultC[i] = testC();
    }
    printf("Mean time for workload A: %.3fms\n", avgTime(resultA));
    printf("Mean time for workload B: %.3fms\n", avgTime(resultB));
    printf("Mean time for workload C: %.3fms\n", avgTime(resultC)); */
    char* ptr = malloc(1000);
    printf("Pointer 1 = %p\n", ptr);
    char* ptr1 = malloc(1000);
    printf("Pointer 2 = %p\n", ptr1);
    char* ptr2 = malloc(1000);
    printf("Pointer 3 = %p\n", ptr2);
    char*ptr3 = malloc(1097);
    return 0;
}
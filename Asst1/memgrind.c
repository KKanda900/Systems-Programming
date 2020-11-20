#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "mymalloc.h"

#define REPEAT 50

struct timeval testA()
{
    clean();
    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 120; i++)
    {
        if (DEBUG)
            printf("%d", i);
        void *p = malloc(1);
        if (DEBUG)
            printf("%d", i);
        free(p);
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

struct timeval testB()
{
    clean();
    void *p[120];

    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 120; i++)
    {
        if (DEBUG)
            printf("%d", i);
        p[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++)
    {
        if (DEBUG)
            printf("%d", i);
        free(p[i]);
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

struct timeval testC()
{
    clean();
    void *p[120];
    int malloc_count = 0, free_count = 0;
    int doMalloc[240];
    srand((unsigned)time(0));
    for (int i = 0; i < 240; i++)
        doMalloc[i] = rand() % 2;

    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 240; i++)
    {
        if (malloc_count <= free_count || (doMalloc[i] && malloc_count < 120))
        {
            if (DEBUG)
                printf("%d", malloc_count);
            p[malloc_count] = malloc(1);
            malloc_count++;
        }
        else
        {
            if (DEBUG)
                printf("%d", free_count);
            free(p[free_count]);
            free_count++;
        }
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

struct timeval testD()
{
    clean();
    void *p[120];
    int malloc_count = 0, free_count = 0;
    int doMalloc[240], mallocSize[120];
    srand((unsigned)time(0));
    for (int i = 0; i < 240; i++)
        doMalloc[i] = rand() % 2;
    srand((unsigned)time(0));
    for (int i = 0; i < 120; i++)
        mallocSize[i] = rand() % 1024;

    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 240; i++)
    {
        if (malloc_count <= free_count || (doMalloc[i] && malloc_count < 120))
        {
            if (DEBUG)
                printf("%d", malloc_count);
            p[malloc_count] = malloc(mallocSize[malloc_count]);
            malloc_count++;
        }
        else
        {
            if (DEBUG)
                printf("%d", free_count);
            free(p[free_count]);
            free_count++;
        }
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

struct timeval testE()
{
    clean();
    void *p1[60];
    void *p2[60];
    int malloc_count1 = 0, free_count1 = 0;
    int doMalloc[120];
    srand((unsigned)time(0));
    for (int i = 0; i < 120; i++)
        doMalloc[i] = rand() % 2;

    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 120; i++)
    {
        if (malloc_count1 <= free_count1 || (doMalloc[i] && malloc_count1 < 60))
        {
            if (DEBUG)
                printf("%d", malloc_count1);
            p1[malloc_count1] = malloc(120 - malloc_count1 * 2);
            malloc_count1++;
        }
        else
        {
            if (DEBUG)
                printf("%d", free_count1);
            free(p1[free_count1]);
            free_count1++;
        }
    }

    for (int i = 0; i < 60; i++)
    {
        if (DEBUG)
            printf("%d", i);
        p2[i] = malloc(45);
    }
    for (int i = 0; i < 60; i++)
    {
        if (DEBUG)
            printf("%d", i);
        free(p2[i]);
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

float avgTime(struct timeval *times)
{
    float result = 0;
    for (int i = 0; i < REPEAT; i++)
    {
        result += (float)times[i].tv_sec * (float)1000;
        result += (float)times[i].tv_usec / (float)1000;
    }
    return result / (float)REPEAT;
}

int main()
{
    struct timeval resultA[REPEAT];
    struct timeval resultB[REPEAT];
    struct timeval resultC[REPEAT];
    struct timeval resultD[REPEAT];
    struct timeval resultE[REPEAT];
    for (int i = 0; i < REPEAT; i++) {
        if (DEBUG) printf("round %d\n", i);
        resultA[i] = testA();
        resultB[i] = testB();
        resultC[i] = testC();
        resultD[i] = testD();
        resultE[i] = testE();
    }
    printf("\n");
    printf("------------------------------------\n");
    printf("| Mean time for workload A: %.3fms |\n", avgTime(resultA));
    printf("| Mean time for workload B: %.3fms |\n", avgTime(resultB));
    printf("| Mean time for workload C: %.3fms |\n", avgTime(resultC));
    printf("| Mean time for workload D: %.3fms |\n", avgTime(resultD));
    printf("| Mean time for workload E: %.3fms |\n", avgTime(resultE));
    printf("------------------------------------\n");
    return 0;
}

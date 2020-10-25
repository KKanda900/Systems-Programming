#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "mymalloc.h"

#define REPEAT 100000

struct timeval testA()
{
    clean();
    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 120; i++)
    {
        if (DEBUG) printf("%d", i);
        void* p = malloc(1);
        if (DEBUG) printf("%d", i);
        free(p);
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

struct timeval testB()
{
    clean();
    void* p[120];

    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 120; i++)
    {
        if (DEBUG) printf("%d", i);
        p[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++)
    {
        if (DEBUG) printf("%d", i);
        free(p[i]);
    }

    gettimeofday(&end_t, NULL);
    timersub(&end_t, &start_t, &res_t);
    return res_t;
}

struct timeval testC()
{
    clean();
    void* p[120];
    int malloc_count = 0, free_count = 0;
    int doMalloc[240];
    srand((unsigned)time(0));
    for (int i = 0; i < 240; i++) doMalloc[i] = rand() % 2;

    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 240; i++)
    {
        if (malloc_count <= free_count || (doMalloc[i] && malloc_count < 120))
        {
            if (DEBUG) printf("%d", malloc_count);
            p[malloc_count] = malloc(1);
            malloc_count++;
        }
        else
        {
            if (DEBUG) printf("%d", free_count);
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
    void* p[120];
    int malloc_count = 0, free_count = 0;
    int doMalloc[240], mallocSize[120];
    srand((unsigned)time(0));
    for (int i = 0; i < 240; i++) doMalloc[i] = rand() % 2;
    srand((unsigned)time(0));
    for (int i = 0; i < 120; i++) mallocSize[i] = rand() % 1024;

    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    for (int i = 0; i < 240; i++)
    {
        if (malloc_count <= free_count || (doMalloc[i] && malloc_count < 120))
        {
            if (DEBUG) printf("%d", malloc_count);
            p[malloc_count] = malloc(mallocSize[malloc_count]);
            malloc_count++;
        }
        else
        {
            if (DEBUG) printf("%d", free_count);
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
    struct timeval start_t;
    struct timeval end_t;
    struct timeval res_t;
    gettimeofday(&start_t, NULL);

    // Implement test D here

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

int main() {
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
//        resultE[i] = testE();
    }
    printf("Mean time for workload A: %.3fms\n", avgTime(resultA));
    printf("Mean time for workload B: %.3fms\n", avgTime(resultB));
    printf("Mean time for workload C: %.3fms\n", avgTime(resultC));
    printf("Mean time for workload D: %.3fms\n", avgTime(resultD));
//    printf("Mean time for workload E: %.3fms\n", avgTime(resultE));
    return 0;

/*    void* m[30];
    m[0] = malloc(491);
    free(m[0]);
    m[1] = malloc(828);
    free(m[1]);
    m[2] = malloc(955);
    m[3] = malloc(306);
    m[4] = malloc(260);
    free(m[2]);
    free(m[3]);
    free(m[4]);
    m[5] = malloc(399);
    m[6] = malloc(1023);
    free(m[5]);
    m[7] = malloc(20);
    free(m[6]);
    free(m[7]);
    m[8] = malloc(774);
    free(m[8]);
    m[9] = malloc(284);
    free(m[9]);
    m[10] = malloc(439);
    m[11] = malloc(237);
    free(m[10]);
    m[12] = malloc(746);
    m[13] = malloc(935);
    free(m[11]);
    free(m[12]);
    m[14] = malloc(734);
    m[15] = malloc(84);
    free(m[13]);
    free(m[14]);
    m[16] = malloc(389);
    free(m[15]);
    m[17] = malloc(410);
    m[18] = malloc(60);
    free(m[16]);
    m[19] = malloc(960);
    free(m[17]);
    free(m[18]);
    m[20] = malloc(122);
    free(m[19]);
    free(m[20]);
    m[21] = malloc(477);
    m[22] = malloc(692);
    m[23] = malloc(289);
    m[24] = malloc(443);
    m[25] = malloc(210);
    m[26] = malloc(864);
    m[27] = malloc(159);
    m[28] = malloc(781);*/
}

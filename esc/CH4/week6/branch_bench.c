#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define N (100000000)  // 1억

static inline uint64_t ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

int cmp_int(const void *a, const void *b) {
    int x = *(const int*)a;
    int y = *(const int*)b;
    return (x > y) - (x < y);
}

int main() {
    int *arr = aligned_alloc(64, sizeof(int) * N);
    if (!arr) {
        perror("alloc failed");
        return 1;
    }

    // 0~255 랜덤 값으로 채움
    for (size_t i = 0; i < N; i++) {
        arr[i] = rand() & 255;
    }

    // Case 1: 분기 예측이 "어려운" 경우 (랜덤 데이터)
    volatile long long sum1 = 0;
    uint64_t t1 = ns();

    for (size_t i = 0; i < N; i++) {
        if (arr[i] >= 128) sum1 += arr[i];
    }

    uint64_t t2 = ns();

    // Case 2: 분기 예측이 "쉬운" 경우 (정렬된 데이터)
    qsort(arr, N, sizeof(int), cmp_int);

    volatile long long sum2 = 0;
    uint64_t t3 = ns();

    for (size_t i = 0; i < N; i++) {
        if (arr[i] >= 128) sum2 += arr[i];
    }

    uint64_t t4 = ns();

    printf("Random data   : sum=%lld, time=%.3f sec\n", sum1, (t2 - t1) / 1e9);
    printf("Sorted data   : sum=%lld, time=%.3f sec\n", sum2, (t4 - t3) / 1e9);

    free(arr);
    return 0;
}

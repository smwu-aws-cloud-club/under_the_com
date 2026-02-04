#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define N (100000000)

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

static inline long long sum_branchy(const int *arr) {
    long long sum = 0;
    for (size_t i = 0; i < N; i++) {
        if (arr[i] >= 128) sum += arr[i];
    }
    return sum;
}

static inline long long sum_branchless(const int *arr) {
    long long sum = 0;
    for (size_t i = 0; i < N; i++) {
        int x = arr[i];
        sum += (long long)(x >= 128) * x;
    }
    return sum;
}

// branchless + dependency 완화 + 약간의 loop unrolling
static inline long long sum_branchless_unrolled(const int *arr) {
    long long s0=0, s1=0, s2=0, s3=0;

    for (size_t i = 0; i < N; i += 4) {
        int x0 = arr[i];
        int x1 = arr[i+1];
        int x2 = arr[i+2];
        int x3 = arr[i+3];

        s0 += (long long)(x0 >= 128) * x0;
        s1 += (long long)(x1 >= 128) * x1;
        s2 += (long long)(x2 >= 128) * x2;
        s3 += (long long)(x3 >= 128) * x3;
    }

    return s0 + s1 + s2 + s3;
}

static void bench(const char *name, long long (*fn)(const int *), const int *arr) {
    uint64_t t1 = ns();
    volatile long long sum = fn(arr);
    uint64_t t2 = ns();
    printf("%-22s sum=%lld, time=%.3f sec\n", name, sum, (t2 - t1) / 1e9);
}

int main() {
    int *arr = aligned_alloc(64, sizeof(int) * N);
    if (!arr) {
        perror("alloc failed");
        return 1;
    }

    // 랜덤 데이터
    for (size_t i = 0; i < N; i++) {
        arr[i] = rand() & 255;
    }

    puts("=== Random data ===");
    bench("branchy", sum_branchy, arr);
    bench("branchless", sum_branchless, arr);
    bench("branchless+unroll", sum_branchless_unrolled, arr);

    // 정렬 데이터
    qsort(arr, N, sizeof(int), cmp_int);

    puts("\n=== Sorted data ===");
    bench("branchy", sum_branchy, arr);
    bench("branchless", sum_branchless, arr);
    bench("branchless+unroll", sum_branchless_unrolled, arr);

    free(arr);
    return 0;
}

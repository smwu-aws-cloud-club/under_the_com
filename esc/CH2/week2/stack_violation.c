#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int *shared_ptr = NULL;

void* thread1(void* arg) {
    int local = 42;   // thread1의 스택 변수
    shared_ptr = &local;

    printf("[thread1] local address: %p, value: %d\n", &local, local);

    sleep(3); // thread2가 접근할 시간 확보

    printf("[thread1] after thread2 modification, local value: %d\n", local);
    return NULL;
}

void* thread2(void* arg) {
    sleep(1); // thread1이 포인터 설정할 때까지 대기

    if (shared_ptr) {
        printf("[thread2] writing to thread1 stack address: %p\n", shared_ptr);
        *shared_ptr = 999;   // 다른 스레드의 스택을 직접 수정
    }

    return NULL;
}

int main() {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}

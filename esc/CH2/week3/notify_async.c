#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

char buffer[128];
int done = 0;

void* worker_thread(void* arg) {
    sleep(2); // 파일 읽기 작업 가정

    pthread_mutex_lock(&mutex);
    snprintf(buffer, sizeof(buffer), "file content from worker thread");
    done = 1;
    pthread_cond_signal(&cond); // 작업 완료 알림
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    printf("[caller] thread id: %lu\n", pthread_self());

    pthread_t tid;
    pthread_create(&tid, NULL, worker_thread, NULL);

    printf("[caller] read_async returned immediately\n");

    pthread_mutex_lock(&mutex);
    while (!done) {
        pthread_cond_wait(&cond, &mutex); // 알림 대기
    }
    pthread_mutex_unlock(&mutex);

    // 결과 처리: 호출자 스레드
    printf("[caller] processing result in thread %lu\n", pthread_self());
    printf("[caller] content: %s\n", buffer);

    pthread_join(tid, NULL);
    return 0;
}

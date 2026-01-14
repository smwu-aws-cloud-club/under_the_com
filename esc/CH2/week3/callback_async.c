#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef void (*callback_t)(char*);

typedef struct {
    callback_t callback;
} task_t;

void* worker_thread(void* arg) {
    task_t* task = (task_t*)arg;

    sleep(2); // 파일 읽는 데 걸리는 시간

    char* buf = malloc(128);
    snprintf(buf, 128, "file content from worker thread");

    // 작업 완료 후 콜백 실행
    task->callback(buf);

    free(buf);
    free(task);
    return NULL;
}

void handler(char* buf) {
    printf("[callback] handled in thread %lu\n", pthread_self());
    printf("[callback] content: %s\n", buf);
}

int main() {
    printf("[caller] thread id: %lu\n", pthread_self());

    task_t* task = malloc(sizeof(task_t));
    task->callback = handler;

    pthread_t tid;
    pthread_create(&tid, NULL, worker_thread, task);

    printf("[caller] read_async returned immediately\n");

    pthread_join(tid, NULL);
    return 0;
}
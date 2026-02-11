# 실습

## 실습 개요

공유 카운터를 증가시키는 작업을 3가지 방식으로 구현하고, 실행 시간과 처리량(ops/sec) 을 비교하는 벤치마크

각 스레드는 worker() 함수에서 반복한다.

mutex 모드
```
pthread_mutex_lock(&g_mutex);
global_counter_plain++;
pthread_mutex_unlock(&g_mutex);
```

spinlock 모드
```
pthread_spin_lock(&g_spin);
global_counter_plain++;
pthread_spin_unlock(&g_spin);
```

atomic 모드
```
global_counter_atomic.fetch_add(1, std::memory_order_relaxed);
```

`counter++` 하나를 실행하기 위해 무엇을 지불하는가를 비교하는 실험

### 기대 관찰 포인트

- mutex: 임계구역에 동시에 1개 스레드만 들어가도록 하는 락. 락을 못 잡으면 보통 OS가 스레드를 sleep(block) 시킴
- spinlock: 락을 못 잡으면 스레드가 잠들지 않고, 계속 루프를 돌면서 락을 확인하는 락. 
- atomic은 lock-free, CPU가 제공하는 원자적 Read-Modify-Write(RMW) 명령으로 값을 증가시키는 방식.

<br>

### 실험 변수

- 스레드 수: 1, 2, 4, 8, 16…
- 작업량: 각 스레드가 1억 번 증가

<br>

## 실습 진행

```
sudo apt update
sudo apt install -y build-essential linux-tools-common linux-tools-generic
```
필요한 라이브러리 설치

```
g++ -O2 -pthread bench_lockfree_inc.cpp -o bench
```
코드 빌드

```
./bench mutex 2 1000000
./bench spin 2 1000000
./bench atomic 2 1000000
```

```
taskset -c 0,1 ./bench mutex 2 200000000
taskset -c 0,1 ./bench spin  2 200000000
taskset -c 0,1 ./bench atomic 2 200000000
```
코어 고정해서 실행

```
perf stat -d taskset -c 0,1 ./bench mutex 2 200000000
perf stat -d taskset -c 0,1 ./bench spin  2 200000000
perf stat -d taskset -c 0,1 ./bench atomic 2 200000000
```
perf으로 측정
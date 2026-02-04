## 파이프라인 / 분기 예측 실습

```
sudo apt update
sudo apt install -y build-essential
```
Ubuntu VM에서 컴파일러 설치

```
gcc --version
```
컴파일러 설치 확인

```
gcc -O3 -march=native branch_bench.c -o branch_bench
```
branch_bench.c 파일 컴파일
`-O3`: 컴파일러가 실행 속도를 위해 공격적으로 최적화(불필요한 코드 삭제, 루프 최적화 등)

### branch_bench.c 파일 내용
- static inline uint64_t ns(): 현재 시간을 나노초 단위로 읽음
- int cmp_int: 정수 오름차순 정렬
- aligned_alloc(64): 보통 CPU 캐시라인 크기인 64바이트를 할당해 분기예측 효과만 더 잘 보이게 함
- 배열에 랜덤으로 0~255 값을 채운 뒤 정렬된 배열/정렬되지 않은 배열에 대해 처리 시간 비교

<br>

```
esc@esc-ubuntu:~$ ./branch_bench
Random data   : sum=9576272166, time=0.427 sec
Sorted data   : sum=9576272166, time=0.077 sec
```
CPU는 파이프라인을 채우기 위해 보통 다음에 실행할 명령어를 미리 가져오고(fetch), 미리 디코드하고(decode), 미리 실행할 준비(execute)를 함
그런데 if가 있으면 다음 실행 경로가 2개로 갈라지는데, CPU는 둘 중 하나를 예측해서 먼저 실행함
예측이 틀리면 이미 실행해둔 경로가 무효가 되고, 파이프라인 flush가 나서 올바른 경로의 명령을 다시 가져와야 하는데 이게 큰 비용

정렬된 데이터는 초반에는 거의 false지만 중반부터 거의 true가 되어 분기 결과가 예측이 쉬움
랜덤이면 >= 128 결과가 거의 무작위이므로 직전 결과로 다음을 예측하기 어렵고 패턴 학습도 거의 불가능함

<br>

### branch_bench2.c 파일 내용
```
if (arr[i] >= 128) sum += arr[i];
```
if는 분기 명령어를 만드는데, 데이터가 랜덤이면 분기예측기가 맞추기 어렵다.
틀리면 pipeline flush가 발생해서 엄청 느려진다.

```
sum += (long long)(x >= 128) * x;
```
따라서 분기를 삭제해서 최적화를 한다.
조건이 true면 `sum += 1 * x`, false면 `sum += 0 * x`이 된다.

```
long long s0=0, s1=0, s2=0, s3=0;

for (i=0; i<N; i+=4) {
  ...
  s0 += ...
  s1 += ...
  s2 += ...
  s3 += ...
}

return s0+s1+s2+s3;
```
여기서 loop unrolling을 통해 한 번에 4개를 처리해서 루프 증가/비교/점프 같은 오버헤드가 감소한다.
따라서 컴파일러가 instruction scheduling 하기 쉬워진다.
그리고 기존에는 sum이 매번 이전 sum에 의존했지만 s0~s3로 분산해서 CPU가 s0, s1, s2, s3 연산을 더 병렬적으로 실행할 수 있게 되었고 파이프라인이 더 잘 채워진다.

```
esc@esc-ubuntu:~$ ./branch_bench2
=== Random data ===
branchy                sum=9576272166, time=0.066 sec
branchless             sum=9576272166, time=0.062 sec
branchless+unroll      sum=9576272166, time=0.018 sec

=== Sorted data ===
branchy                sum=9576272166, time=0.068 sec
branchless             sum=9576272166, time=0.089 sec
branchless+unroll      sum=9576272166, time=0.033 sec
```
실행 결과 오히려 정렬된 데이터가 랜덤보다 실행 시간이 더 늘어나게 되었다.
# 실습

```
echo "Hello async world" > test.txt
```
테스트를 위한 텍스트 파일 생성


## 경우 1

- 호출자가 실행 결과를 신경 쓰지 않는 경우 (콜백 방식)
- 파일 읽기 작업을 다른 스레드에서 수행
- 읽기 완료 후 콜백 함수가 작업자 스레드에서 실행
- 호출자 스레드는 결과를 받지 않음

<br>

비동기 콜백 방식의 c언어 코드 [`callback_async.c`](/esc/CH2/week3/callback_async.c) 작성

```
gcc callback_async.c -o callback_async -pthread
```
컴파일 

```
./callback_async
```
실행

[result](/esc/CH2/week3/images/practice1-1.png)<br>
- handler는 호출자 스레드가 아닌 작업자 스레드에서 실행
- 호출자 스레드는 결과를 전혀 처리하지 않음
- 콜백 = 결과 처리

<br>

## 경우 2

- 호출자가 실행 결과를 반드시 알아야 하는 경우 (알림 방식)
- 파일 읽기 작업을 다른 스레드에서 수행
- 작업 완료 시 조건 변수로 호출자 스레드에 알림

비동기 알림 방식의 c언어 코드 [`notify_async.c`](/esc/CH2/week3/notify_async.c) 작성

```
gcc notify_async.c -o notify_async -pthread
```
컴파일 

```
./notify_async
```
실행

[result](/esc/CH2/week3/images/practice1-2.png)<br>
- 작업자 스레드는 읽기만 수행
- 결과 처리는 호출자 스레드
- 호출자는 “언제 끝났는지”를 알림으로 인지

<br>
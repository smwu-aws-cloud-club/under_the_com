# 1. 모든 것은 CPU에서 시작된다

## CPU의 실행 과정

- 메모리에서 명령어를 가져와서 → 명령어 실행(execute) → 메모리에서 다음 명령어 인출
    - 이때 cpu는 Register(=PC)에서 명령어를 가져온다
    - 최초(시작점)의 PC regitser 값?
        1. source code를 가지고 compiler가 assembly language로 변환 후 실행 파일 생성
        2. 실행 파일이 disk에 저장됨
        3. disk에 저장된 실행 파일의 명령어가 memory에 저장
        4. 일반적인 program의 시작 지점 → main()
            - 프로그램 시작 → main 함수에 대응하는 첫 번째 기계 명령어를 찾고 → 메모리 주소를 PC register에 기록
- Register
    - Program Counter
    - 용량은 작고, 속도는 매우 빠른 메모리
    - cpu가 다음에 실행할 명령어 주소를 저장
        - default: +1
        - if-else, method call
            - jump register 사용 → PC regitser값이 동적으로 변경 → 실행해야 할 명령어로 jump

# 2. CPU에서 OS까지

- CPU가 프로그램을 실행하게 하려면 → 실행 파일을 수동으로 memory에 복사한 후 main 함수에 해당하는 첫 번째 기계 명령어를 메모리에서 찾음 → 이 주소를 pc register에 적재

## Problem

- 사용자가 직접 프로그램을 적재할 수 있는 적절한 크기의 메모리 영역을 찾아야 함
- cpu 레지스터 초기화 +method의  entry point를 찾아 PC register 설정 필요
- 멀티태스킹 지원 x
    - 한 번에 하나의 program만 실행 가능
- 모든 program은 사용할 hardware를 직접 특정 driver과 연결해야 함
    - program ↔ sound card driver 연결
    - program ↔ network card 연결 + TCP/IP stack source code 연결
    - etc..
- interactive interface 직접 구현해야 함
- library 직접 구현 필요

## Solution → OS 활용

- Process 관리 도구 + program 자동 적재 도구를 이용해서 multitasking 구현 ⇒ 이러한 기능들이 구현된 프로그램을 운영체제(OS)라고 부른다
- OS는 background에서 시스템에서 실행중인 process, cpu 개수, 물리 메모리의 용량 등을 관리하고, 사용자는 현재 사용 중인 프로그램이 cpu와 표준 크기의 memory를 독점하고 있다고 생각할 수 있다
- Process
    - program의 context가 저장된 structure
        - context: cpu 내부 register 값, cpu가 어떤 기계 명령어를 실행했는지 저장된 값
    - program이 일시 중지 되어도 저장된 context를 이용해 program 실행 재개 가능함
    - 프로그램은 실행된 후 process형태로 관리된다
- CPU가 process A 실행 → process A 실행 중지 → process B 실행 → process A 실행 재개(진행 상황은 process에 저장된 context로 복원)

# 3. Process의 불편함

## Problem

- process 주소 공간
    - code segment: 코드 컴파일 → 생성된 기계 명령어 저장
    - data segment: 전역 변수 저장
    - heap segment: 동적 메모리 할당(c → malloc), class(java)
    - stack segment: 함수의 실행 시간 스택
- 하지만 한 process에서 funA, funB의 함수가 독립적으로 실행 → 두 함수가 순차적으로 실행된다면(직렬) → 프로그램 전체 실행 시간이 늘어난다
- process 실행 → main함수의 첫 번째 기계 명령어 주소를 pc 레지스터에 기록 → 명령어 실행 흐름 형성
    - 하지만 entry function이 main 함수 하나밖에 없어 process의 기계 명령어를 한 번에 하나의 CPU에서만 실행 가능

## Solution

1. 다중 프로세스 프로그래밍
    - processA → funA 실행, processB → funB 실행
    - 프로세스 간 통신 문제가 발생할 수 있음
        - process마다 자체적인 주소 공간 가지고 있음 → 서로 통신하기 복잡함
    - 프로세스를 생성할 때 overhead
2. thread(경량 Process)
    - 실행 흐름. 같은 process에 속한 thread들은 같은 주소 공간을 공유함 → 따라서 thread는 Process보다 가볍고 생성 속도가 빠르며, thread간 통신을 위한 os 의 지원도 필요하지 않다
    - process를 시작하고 → thread 여러 개를 생성 → 다중 Core를 이용해 고성능, 높은 동시성 보장 가능
    - single-core에서도 thread 여러 개 생성 가능
        - thread가 운영 체제 계층에서 구현되며, core 개수와는 무관하기 때문
    - 처리하는 데 많은 시간이 필요한 event를 처리하는 별도의 thread를 생성해 병렬로 처리 → 효율성 굿
    - 공유 process 주소 공간에서 동일한 process에 속한 명령어들을 CPU 여러 개 에서 thread 단위로 동시에 실행한다
        - pc register은 main 함수 뿐만 아니라 다른 함수(funA,funB)도 가리키게 해 새로운 실행 흐름을 형성할 수 있음 → 이 실행 흐름은 동일한 프로세스 주소 공간 공유 → 프로세스 간 통신 필요 x
        - 따라서 하나의 process 안에 여러 실행 흐름이 존재할 수 있다 → 이것이 thread
    - thread를 활용한 실행 과정
        - funA와 funB 담당하는 thread를 2개 생성 → CPU PC register에 스레드의 진입 함수 주소 지정 →  2개의 thread를 CPU 코어 2개 에서 동시에 실행
        - 이때 funA, funB의 전역변수 resA, resB는 동일한 프로세스 주소 공간(data segment)에 존재
    - 하지만 다중 thread 사용 시 공유 리소스 접근 오류가 발생할 수 있음
        - 상호 배제, 동기화를 이용해 다중 스레드 공유 리소스 문제를 해결해야 함

# 4. 다중 thread와 메모리 구조

- 함수가 실행될 때 필요한 정보
    - 함수 매개변수, 지역변수, 반환주소 → stack frame에 저장
    - 모든 함수는 실행 시에 자신만의 stack frame을 가짐
        - stack frame은 LIFO순서로 증가, 감소
        - stack frame = 프로세스 주소 공간에서 스택 영역 형성
- 실행 흐름 여러 개를 가지는 process → 여러 개의 stack segment 필요 → 따라서 모든 thread는 자신만의 스택 영역을 가짐
    - 따라서 thread 생성 → 프로세스의 메모리 공간 소모

# 5. thread 활용 예시

## long task

- 전용 스레드를 생성하는 것이 적합
- 요청 마다 thread를 생성
    - 작업 요청이 들어옴 → 매번 thread 생성
- I/O 작업
- disk에 write 작업 발생 → disk write 전담 thread를 생성하자

## short task

- 네트워크 요청, querying
- 작업 처리에 필요한 시간이 짧고, 작업 수가 많음
- 하지만 요청당 스레드 방식을 사용한다면?
    - 스레드 생성, 종료에 많은 시간이 들고
    - 많은 요청이 들어옴 → 많은 스레드 생성 → 메모리, 리소스 많이 소비
    - 스레드 수가 많아지면 → 스레드 간 전환에 따른 부담 증가
- 따라서 thread pool을 사용해서 short task를 처리하는 것이 더 적절하다

## Thread Pool

### 동작 방식

- thread 여러 개를 미리 생성 → task 요청 → thread 할당
- 할당된 thread 재사용
- 자료구조 → queue 사용
    - 생산자-소비자 패턴 이용
        - 작업 전달 → producer, thread → consumer
- thread pool에 전달되는 작업 → 처리할 데이터, 데이터를 처리하는 함수
- thread pool을 활용한 task 처리 과정
    
    ```c
    struct task
    {
    	void* data; //작업이 처리할 데이터
    	handler handle; //작업 처리 함수
    }
    ```
    
    1. thread pool의 thread → 작업 대기열(jobs queue)에서 블로킹 상태로 대기
    2. 생산자가 작업 대기열에 데이터 기록 → thread pool의 thread wakeup
    3. thread는 작업 대기열에서 정의된 struct가져옴
        1. 이때 작업 대기열 → 여러 thread 간에 공유됨 → 동기화 상호배제 문제 처리 필요
    4. struct의 handle이 가리키는 처리함수(handler function) 실행

### Thread Pool의 thread 수

- thread 수가 너무 적다면
    - CPU 최대한 활용 x → 비효율적
- thread 수가 너무 많다면
    - 시스템 성능 저하, 메모리의 과다한 점유, 스레드 전환으로 생기는 부담
- thread pool에서 처리할 작업에 따라 thread pool의 thread 수를 결정하는 것이 best practice
    1. CPU 집약적인 작업
        - 외부 입출력에 의존할 필요 없이 처리할 수 있는 작업
            - 과학 연산, 행렬 연산
        - thread 수와 CPU 코어 수가 동일하다면 → 제일 효율적
    2. I/O 집약적인 작업
        - 대부분의 시간을 디스크 입출력, network 입출력 등에 소비하는 작업
        - 테스트를 통해 적절한 thread 수를 결정하기

# 6. 스레드 간 공유되는 프로세스 리소스

- 프로세스 → 운영 체제가 리소스를 할당하는 기본 단위
- 스레드 → 스케줄링의 기본 단위, process 리소스는 thread간에 공유됨

# 7. 스레드 전용 리소스

- 상태 변화 관점에서 보면, thread는 일종의 함수 실행이다
    - 함수 실행의 entry point → 진입 함수
    - CPU는 진입 함수에서 명령어 실행을 시작해 하나의 실행 흐름(thread)생성
    - 이때 thread는 자신만 사용할 수 있는 stack segment를 가진다 → 따라서 thread 여러 개가 있다면 여러 개의 스택 영역이 존재
- 함수 실행 시 존재하는 정보?
    - stack frame → 실행 시간 정보, 함수 반환값, 다른 함수를 호출할 때 전달되는 매개변수, 함수 내 지역변수, 레지스터 정보(레지스터 초기값, etc)
- thread context(스레드 상황 정보) → 오직 thread 전용. 다른 thread에서는 접근 x → 스레드 전용 리소스
    - stack frame
    - stack pointer: 스레드 스택 영역에서 stack top 위치 저장
    - PC register: 다음에 실행될 명령어 주소 저장
    - 내부 레지스터 값: 스레드의 현재 실행 상태(함수 실행 시 사용되는 레지스터 정보)
- 스레드 간에 공유되는 리소스
    - 스레드는 프로세스 주소 공간에서 stack segment를 제외한 나머지 영역을 모두 공유

# 8. 코드 영역

- 프로그래머가 작성한 코드 → 컴파일 → 실행 가능한 기계 명령어 저장하는 영역
- 기계 명령어는 실행 파일에 저장됨
- 프로그램 시작 → 기계 명령어는 프로세스 주소 공간에 적재
- 코드 영역은 thread 간에 공유됨
    - **모든 함수를 thread에 배치해서 실행할 수 있음**
    - 하지만 특정 함수를 특정 thread에서만 실행되도록 하는 것은 불가능
    - 따라서, code segment는 모든 thread가 공유하는 영역
    - 코드 영역은 read-only이므로 프로그램 실행 시 어떤 thread도 코드 영역 내용 변경 x → 프로세스 내 모든 thread가 코드 영역을 공유하고 있지만, 코드 영역에 관해서는 thread safty issue가 발생하지 x

# 9. 데이터 영역

- 데이터 영역에는 전역 변수가 저장됨
- 프로그램 실행 시 데이터 영역 내에 전역 변수의 instance는 1개 → 모든 thread는 전역 변수에 접근 가능
- 어떤 thread가 전역 변수 값을 변경 → 다른 thread에서도 전역 변수 값은 변경된 상태
- **따라서 모든 스레드가 데이터 영역의 변수에 접근할 수 있음**

# 10. 힙 영역

- malloc, new → heap에서 할당
- **thread는 pointer(변수의 주소)를 통해 pointer가 가리키는 데이터에 접근할 수 있음 → heap segment는 thread 간 공유 리소스**

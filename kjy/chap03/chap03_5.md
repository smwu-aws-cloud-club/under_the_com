# chap 03_5 정리 
25.05.08 update

## 03_5_1~2
* x86 CPU는 4가지 특권 단계(privilege level)
    * 숫자가 작을수록 특권이 커짐 
    * 특권: 일부 명령어를 실행할 수 있는지를 의미 
    * 일반적으로 시스템은 0, 3 두 단계만 사용함 
        * 3단계: 사용자 상태(user mode) 
        * 0단계: 커널 상태(kernel mode) 
* CPU가 운영체제의 코드를 실행할 때 -> kernel mode
* 프로그래머가 작성한 일반적인 코드, 일반적인 응용 프로그램을 실행할때 -> user mode 
* segmentation fault: user mode 코드가 특정 주소 공간에 접근하지 않도록 제한을 두지 않으면 발생 가능 

## 03_5_3~4
* system call: 운영체제에 서비스를 요청할 수 있음 (운영체제의 파일 읽기 쓰기, 네트워크 데이터 통신과 같은 작업을 처리해줄 수 있음)
    * 특정한 기계명령어로 구현됨
    * 명령어 실행 시 CPU는 user mode -> kernel mode로 전환되어 운영체제의 코드를 실행하는 방법으로 요청 수행 
* C의 표준 라이브러리(standard library)
    * user mode에서도 실행됨 
    * 프로그래머는 표준 라이브러리 호출하여 파일 읽고 쓰기 작업, 네트워크 통신 작업을 수행 -> 표준 라이브러리는 실행중인 운영체제에 따라 대응되는 시스템 호출 선택 
    * 표준 라이브러리는 시스템 호출로 운영체제와 소통 
    * malloc은 표준 라이브러리에 구현되어 있음 

## 03_5_5~6
* malloc() 시 메모리 할당자 안의 여유 메모리 조각이 부족해질때 brk 변수 값을 위로 이동하여 힙영역을 확장 (시스템 호출 필요)
    * brk: 힙 영역의 상단을 가리킴 
* brk system call: 힙 영역 크기를 늘리거나 줄일 수 있음 
    * mmap도 사용 가능(좀 더 구조가 유연함) 

## 03_5_7~
* 실제 물리 메모리는 언제 할당되는가? 
    * 실제로 할당한 메모리가 사용되는 순간에 물리 메모리를 할당하게됨 
    * 가상 메모리가 아직 실제 메모리와 연결되어 있지 않으면 내부적으로 page fault 오류 발생 가능 => 운영체제가 이 오류 감지 시, 페이지 테이블 수정하여 가상 메모리, 실제 물리 메모리 사상관계를 설정 -> 이것으로 실제 물리 메모리가 할당됨 



### 좀 더 찾아본 내용 
왜 brk system call보다 mmap 시스템 호출이 더 구조가 유연하다는걸까? 
* brk: 힙(Heap)의 끝을 늘려서 메모리를 확장하는 방식. 힙 영역은 연속적인 공간이어야 함.
    * 제한된 방향(힙 위쪽)으로만 증가 가능.
    * 메모리를 중간에서 해제하면 단편화(fragmentation)로 낭비 발생.
* mmap: 가상 메모리의 아무 위치에 매핑 가능.
    * 힙이나 스택과 무관한 독립된 공간 확보.
    * 원하는 크기, 주소 정렬, 접근 권한 등을 설정 가능.



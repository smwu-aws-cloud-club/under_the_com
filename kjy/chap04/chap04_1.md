# chap 04_1 정리 
25.05.11 update

## 04_1~3
* 논리곱, 논리합, 논리부정 
* `논리적 완전성`: 충분한 논리곱, 논리합, 논리부정 게이트가 있으면 어떠한 논리 함수도 구현 가능 (이외에는 필요 x)

## 04_1_4~5
* 배타적 논리합(Exclusive OR)
    * 논리합, 논리곱, 논리부정 게이트 3가지로 구성 가능
* `가산기(adder)`: 논리곱+배타적논리합 게이트 => 2진법 덧셈 구현 가능 
* ALU(Arithmetic Logic Unit): CPU에서 전문적으로 계산을 담당하는 모듈. 좀 더 복잡함 

* `부정 논리곱 게이트(non-conjunction gate, NAND)` : 논리곱 연산 처리후, 논리 부정 연산을 처리함 
    * 첫번째 부정 논리곱 게이트의 출력이 다른 부정 논리곱 게이트의 입력으로 들어감 
    * 2가지 상태만 존재 가능해짐 => 단자 a값을 회로의 출력으로 사용 
    * 회로에 정보를 저장할 수 있음 (1비트)

## 04_1_6~7
* 더 많은 비트 저장 -> 여러개 복제하여 붙이기 => `레지스터`
* 더 많은 정보를 저장하고 주소 지정 기능 제공을 하기 위해 -> 8비트를 1바이트로 규정, 각각의 바이트가 자신의 번호를 받음 
    * 부여된 번호 통해 회로에 저장된 정보 읽을 수 있음 
    * => `메모리`
* 모든 연산 논리를 반드시 회로 같은 하드웨어로 구현할 필요 없음 -> 하드웨어는 변하지 않지만, 소프트웨어는 변할 수 있기에 하드웨어에 서로 다른 소프트웨어를 제공하여 하드웨어가 완전히 새로운 기능 구현하도록 해야함 

## 04_1_8~9
* 기계 명령어는 조합회로를 이용하여 실행됨 
* `instruction set(명령어 집합)`: 실행할 수 있는 `명령어(opcode)`, 각 명령어에 필요한 `피연산자(operand)`를 묶은 것 
    * 서로 다른 유형의 CPU는 서로 다른 명령어 집합을 가지고 있음 
    * 예시: 16비트의 기계 명령어 
        * 처음 4비트: CPU에 수행할 작업을 알려줌(기계명령어는 총 2^4, 16개) 
        * 나머지 비트는 CPU에 어떻게 작업해야하는지 알려줌 
            * 레지스터 R6, R2의 값을 더한 후 -> 레지스터 R6에 기록하라 

## 04_1_10~11
* 예를 들어 가산기가 작동 시, 레지스터 R1, R2에 반드시 1을 저장하도록 어떻게 보장? -> 각 부분의 회로가 함께 작업할 수 있도록 조정 or 동기화하려면? 
    * `클럭 신호(clock signal)`: 클럭 신호가 전압을 변경할때마다 전체 회로의 각 레지스터, 즉 전체 회로 상태가 갱신됨 => 전체 회로가 함께 동작하도록 할 수 ㅣㅇㅆ음 
    * `클럭 주파수(clock rate)`: 높을 수록 CPU가 1초에 더 많은 작업을 할 수 있음 
* 프로세서(=CPU): 각종 계산이 가능한 산술 논리 장치 + 정보 저장 가능 레지스터 + 클럭신호 
* 실제 상용 CPU는 더 복잡함 

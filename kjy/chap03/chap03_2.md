# chap 03_2 정리 
26.01.21 update

## 03_2_1~2
![image](https://wayhome25.github.io/assets/post-img/cs/virtual_address_space2.png)
* 가상 메모리(virtual memory) 주소 != 실제 물리적인 메모리 주소 
    * 모든 프로세스의 코드 영역이 0x400000 에서 시작함 
    * 프로세스는 동일한 크기의 chunk로 나뉘어 물리 메모리에 저장됨 
    * 모든 조각은 물리 메모리 전체에 무작위로 흩어져 있음 
    * 가상 메모리 ~ 물리 메모리 사이의 `mapping` 관계 유지를 통해 유지 가능 -> `page table` 존재 이유 
        * 모든 가상 주소를 물리 주소에 Mapping하는 대신 프로세스의 주소 공간을 동일한 크기의 조각(`page`)로 나눔 
        * 사상은 `페이지` 단위로 이루어짐 
        * 각 프로세스에는 단 하나의 페이지 테이블만 있어야함 

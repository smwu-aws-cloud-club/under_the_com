# chap 06_4 정리 
26.02.24 update

## 06_4_1
* everything is file 
    * 디스크, 네트워크 데이터, 터미널, 프로세스 간 통신 도구인 파이프까지 모두 파일로 취급됨 
* file descriptor: 파일을 열 때 커널이 이를 반환 -> 파일 작업 실행 시 해당 file descriptor를 커널에 전달해줘야 함 -> 숫자임 
```c
char buffer[LEN];
int fd=open(file_name); // get file descriptor 
read(fd,buffer);
```

## 06_4_2~
* input/output multiplexing 
    * 프로그래머가 동시에 많은 수의 file descriptors를 다룰 수 있도록 함 
    * 관심 대상인 file descriptor를 커널에 알려주고, read/write가 가능해진 서술자가 있으면 응용 프로그램에게 알리는 방식 
* select 
    * 감시 가능한 서술자 묶음 제한: 1024개 
    * 호출 시 대응하는 프로세스/스레드는 감시 대상인 파일의 대기열에 추가됨 
    * 읽기/쓰기 가능 이벤트가 나타나면 해당 프로세스/스레드 깨어남 
    * but 깨어났을 떄 어떤 서술자가 읽고/쓰기 가능한지 알 수 없어서 처음부터 끝까지 확인해야함 
* poll
    * select와 유사하나 1024개 제한 해결함 
    * 마찬가지로 감시해야하는 서술자 수 늘어날수록 성능이 저하되는 문제가 있음 
* epoll
    * 커널에 필요한 데이터 구조를 생성 
        * 준비 완료된 파일 서술자 목록 
    * 감시되고 있는 파일 서술자에게 이벤트 발생 -> 해당 프로세스를 깨움 -> 준비 완료된 파일 서술자가 해당 자료 구조에 추가됨 
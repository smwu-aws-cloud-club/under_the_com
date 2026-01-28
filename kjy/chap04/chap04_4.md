# chap 04_4 정리 
26.01.28 update

## 04_4_1~4
* branch-misses: 분기 예측 실패율 
* cpu 자체를 메가팩토리로 볼 수 있음 
    * cpu는 기계 명령어를 실행함 
    * instruction fetch -> instruction decode -> execute -> writeback 4단계로 과정을 거침. 각 단계는 별도의 하드웨어로 처리됨 
    * 실제 cpu 내부에서는 기계 명령어 하나를 단계 수십개로 분해하여 실행 가능함 
* if 문을 만나면? -> 컴파일러가 조건부 점프 명령어로 변환함 (명령어는 분기 역할을 함)
    * 분기 점프 명령어가 실행 완료 전, 다음 명령어는 이미 파이프라인에 들어가 있어야함 => 미리 어디로 분기할 가능성이 있을지 예측 필요 
        * `분기예측(추측)`이 틀렸다면 안타깝게도 파이프라인에서 이미 실행 중이던 잘못된 분기 명령어 전부를 무효화함 -> cpu 추측이 틀리면 바로 성능 손실 발생 
    * likely, unlikely macro(c++20부터는 attribute로 편입됨)가 있는 이유: 컴파일러에 가능성이 더 높은 분기를 알려주게 됨 
* perf와 같은 분석 도구로 분기 예측이 성능 병목 원인인지 확인 가능 

### 추가로 찾아본 것 
likely, unlikely 사용 예제가 궁금함 
```c
#include <stdio.h>

int main(int argc, char *argv[]) {
    int x = argc;

    if (__builtin_expect(x == 1, 1)) { // likely
        printf("Likely case\n");
    } else {
        printf("Unlikely case\n");
    }

    return 0;
}
```
* gcc, clang에서는 expected가 1이면 condition이 자주 참, 0이면 자주 거짓임을 알려줄 수 있는 `__builtin_expect(condition, expected)`가 존재
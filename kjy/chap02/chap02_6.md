# chap 02_6 정리 
25.03.31 update

## 02_6_1~5
* 파일 읽기(read)와 같은 요청을 할때 system call 로 OS에 요청을 보냄 -> 파일 읽기 작업을 위해 호출 스레드를 중지시키고 -> 커널이 디스크 내용을 읽어오면 일시 중지되었던 스레드 깨움 -> `블로킹 입출력(blocking input/output)`
* 동기 호출은 호출자, 수신자가 같은 스레드에서 실행 중인지 여부와는 관련이 없음 
* 비동기 호출은 디스크의 파일 읽고 쓰기, 네트워크 데이터 송수신, 데이터베이스 작업처럼 시간이 많이 걸리는 입출력 작업을 백그라운드 형태로 실행함 
* 비동기 호출 처리의 2가지 상황
    1. 호출자가 실행 결과를 신경 쓰지 않을 때 -> 콜백으로 구현 
    2. 호출자가 실행 결과를 반드시 알아야할때 -> 알림(notification)으로 구현 
* 유휴 시간(idle time): 동기 처리 시, 주 스레드가 호출한 스레드의 처리를 완료될때까지 기다리는 시간 
* 비동기 처리는 작업을 동시에 실행할 수 있기 때문에 시스템 리소스를 최대한 활용할 수 있음, 시스템 응답 속도가 더 빨라짐
    * 그러나 동기 프로그래밍보다 이해하기 쉽지 않음
    * 시스템 유지 보수 측면에서 동기 프로그래밍보다 좋지 않음 

### 추가로 찾아본 콜백 예시 
```javascript
// 1. 데이터를 가져오는 함수 (비동기 작업을 시뮬레이션)
function fetchUserData(userId, callback) {
  console.log("사용자 데이터를 가져오는 중...");

  // 2초 뒤에 데이터를 가져왔다고 가정
  setTimeout(() => {
    const fakeUserData = {
      id: userId,
      name: "홍길동",
      age: 30,
    };

    console.log("데이터를 성공적으로 가져왔습니다.");
    // 데이터를 처리하기 위해 콜백 함수 실행
    callback(fakeUserData);
  }, 2000);
}

// 2. 데이터를 처리하는 함수 (콜백 함수)
function processUserData(userData) {
  console.log("사용자 데이터를 처리 중...");
  console.log(`ID: ${userData.id}`);
  console.log(`이름: ${userData.name}`);
  console.log(`나이: ${userData.age}`);
}

// 3. fetchUserData에 processUserData를 콜백으로 전달
fetchUserData(1, processUserData);
```
1. fetchUserData(1, processUserData)가 호출되면, fetchUserData 함수가 실행됩니다.
2. setTimeout으로 인해 2초 동안 대기한 뒤, 가짜 사용자 데이터를 생성합니다.
3. 2초 후, callback(fakeUserData)가 실행되며, processUserData 함수가 호출됩니다.
4. 최종적으로 사용자 데이터가 출력됩니다.

```
사용자 데이터를 가져오는 중...
데이터를 성공적으로 가져왔습니다.
사용자 데이터를 처리 중...
ID: 1
이름: 홍길동
나이: 30
```
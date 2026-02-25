# 보충: Long Polling과 WebSocket - HTTP부터 커널까지

> Inferable 팀의 ["WebSocket 대신 Long Polling을 선택한 이유"](https://www.inferable.ai/blog/posts/postgres-nodejs-longpolling.mdx) 아티클을 기반으로, CH6에서 배운 I/O 다중화 지식을 실제 네트워크 프로토콜 설계 결정에 연결해본다.

참고) 공부하면서 L7의 Polling, Long Polling, Websocket과 L4의 select, poll, epoll을 헷갈려서 이렇게 공부하게 되었습니다. 여러분은 혼동 없으시길 바랍니다.

<br>

## HTTP 동작 원리

### 요청/응답 구조

HTTP의 가장 근본적인 특성은 **클라이언트가 먼저 요청을 보내야만 서버가 응답할 수 있다**는 것이다.<br>
서버는 자발적으로 클라이언트에게 먼저 말을 걸 수 없다. 이 구조를 **단방향 요청/응답 모델**이라고 한다.

```
클라이언트                서버
    |                      |
    |--- GET /news ------->|
    |                      |  (서버가 응답을 준비)
    |<-- 200 OK + body ----|
    |                      |
    (연결 종료 또는 재사용)
```

이 구조가 "채팅", "실시간 알림", "주식 시세" 같은 기능을 구현하기 어렵게 만드는 근본 원인이다.<br>
서버에 새로운 데이터가 생겼을 때 클라이언트에게 즉시 알릴 방법이 HTTP 표준에 없다.

<br>

### Keep-Alive

HTTP/1.0에서는 요청 하나마다 TCP 연결을 새로 생성하고 응답 후 바로 종료했다.<br>
이는 매 요청마다 TCP 3-way handshake 비용이 발생하는 문제를 낳았다.

```
HTTP/1.0:
  [TCP 연결] → GET → 200 OK → [TCP 종료]
  [TCP 연결] → GET → 200 OK → [TCP 종료]
  [TCP 연결] → GET → 200 OK → [TCP 종료]

HTTP/1.1 (Keep-Alive):
  [TCP 연결]
      → GET → 200 OK
      → GET → 200 OK
      → GET → 200 OK
  [TCP 종료]
```

HTTP/1.1의 `Connection: keep-alive` 헤더로 동일한 TCP 연결을 재사용한다.<br>
그러나 핵심적인 제약은 바뀌지 않는다. **여전히 클라이언트가 요청해야만 서버가 응답할 수 있다.**<br>
TCP 연결이 유지되더라도 서버는 클라이언트의 요청 없이 데이터를 보낼 수 없다.

<br>

### Stateless

HTTP는 __무상태(stateless)__ 프로토콜이다. 서버는 이전 요청의 맥락을 기억하지 않는다.<br>
각 요청은 독립적으로 처리된다. 이것이 HTTP의 설계 철학이다.

Stateless의 장점과 단점:

| 장점 | 단점 |
|------|------|
| 서버를 여러 대로 수평 확장 쉬움 | 인증 정보를 매 요청에 포함해야 함 |
| 서버 재시작 시 클라이언트 영향 없음 | 연결 상태(세션) 관리가 복잡해짐 |
| 단순한 서버 구현 | 실시간 통신 구현이 어려움 |

이 세 가지 특성(단방향, Keep-Alive, Stateless)이 "실시간 통신을 HTTP 위에서 어떻게 구현할 것인가"라는 문제의 출발점이다.

<br>

## Long Polling

### 폴링 vs Long Polling

__폴링(Polling)__ : 클라이언트가 짧은 간격으로 계속 요청을 보내는 방식이다.

```
클라이언트                서버
    |--- GET /news? ------>|
    |<-- 200 (데이터 없음) -|  (즉시 응답)
    |   (1초 대기)
    |--- GET /news? ------>|
    |<-- 200 (데이터 없음) -|  (즉시 응답)
    |   (1초 대기)
    |--- GET /news? ------>|
    |<-- 200 (데이터 있음) -|  (즉시 응답)
```

데이터가 없을 때도 서버가 즉시 "없다"고 응답하므로 불필요한 요청이 많다.<br>
클라이언트가 1000개라면 초당 1000개의 빈 요청이 서버에 쏟아진다.

<br>

**Long Polling** : 서버가 데이터가 생길 때까지 응답을 보류하는 방식이다.

```
클라이언트                서버
    |--- GET /poll ------->|
    |                      |  (대기 중... 최대 30초)
    |                      |  (이벤트 발생!)
    |<-- 200 + 데이터 ------|  (이벤트 발생 시 응답)
    |--- GET /poll ------->|  (즉시 재연결)
    |                      |  (대기 중...)
    |                      |  (30초 초과)
    |<-- 200 (timeout) ----|  (타임아웃 응답)
    |--- GET /poll ------->|  (재연결)
```

데이터가 없으면 서버는 응답을 보류한다. 이벤트가 발생하면 즉시 응답하고, 클라이언트는 즉시 재연결한다.<br>
타임아웃(30초) 안에 이벤트가 없으면 빈 응답을 반환하고, 클라이언트는 재연결한다.

<br>

### 구현 핵심: waiters 배열

Long Polling 서버의 핵심은 **응답 객체를 메모리에 보관**하는 것이다.

```javascript
// 대기 중인 응답의 resolve 함수 목록
const waiters = [];

// GET /poll: 요청이 오면 바로 응답하지 않고 waiters에 등록
function handlePoll(req, res) {
  let resolved = false;

  // 30초 타임아웃 등록
  const timer = setTimeout(() => {
    resolved = true;
    const idx = waiters.indexOf(resolve);
    if (idx !== -1) waiters.splice(idx, 1);
    res.end(JSON.stringify({ type: 'timeout' }));
  }, 30_000);

  function resolve(data) {
    if (resolved) return;
    resolved = true;
    clearTimeout(timer);
    res.end(JSON.stringify({ type: 'event', data }));
  }

  waiters.push(resolve);

  // 클라이언트가 연결을 끊으면 clean-up (메모리 누수 방지)
  req.on('close', () => {
    if (!resolved) {
      resolved = true;
      clearTimeout(timer);
      const idx = waiters.indexOf(resolve);
      if (idx !== -1) waiters.splice(idx, 1);
    }
  });
}

// POST /event: 이벤트 발생 시 대기 중인 모든 클라이언트에 응답
function handleEvent(data) {
  const pending = waiters.splice(0);  // waiters 비우기
  pending.forEach(resolve => resolve(data));
}
```

실습 코드는 [week8/long-polling-server.js](week8/long-polling-server.js)에서 확인할 수 있다.

<br>

### 핵심 관찰

- **대기 중에도 FD 점유**: 30초 대기 중에 해당 TCP 연결의 파일 디스크립터가 유지된다
- **HTTP 헤더 파싱 반복**: 매 재연결마다 HTTP 헤더를 파싱하는 비용이 발생한다
- **메모리 유지**: `waiters` 배열이 응답 객체를 참조하므로 GC에 의해 해제되지 않는다
- **클린업 필수**: `req.on('close')` 처리 없이는 연결이 끊겨도 waiters에 계속 쌓인다

<br>

## WebSocket 기초

### HTTP Upgrade 핸드셰이크

WebSocket은 HTTP 연결로 시작해 프로토콜을 업그레이드하는 방식으로 동작한다.<br>
이 과정을 **WebSocket 핸드셰이크**라고 한다.

```
클라이언트                          서버
    |                               |
    |--- GET /ws HTTP/1.1 --------->|
    |    Upgrade: websocket         |
    |    Connection: Upgrade        |
    |    Sec-WebSocket-Key: ...     |
    |                               |
    |<-- HTTP/1.1 101 Switching ----|
    |    Upgrade: websocket         |
    |    Connection: Upgrade        |
    |    Sec-WebSocket-Accept: ...  |
    |                               |
    |<====== WebSocket 프레임 ======>|  (양방향 통신 시작)
    |<====== WebSocket 프레임 ======>|
    |<====== WebSocket 프레임 ======>|
```

`101 Switching Protocols` 응답 이후 동일한 TCP 연결에서 HTTP가 아닌 **WebSocket 프레임** 형식으로 데이터를 주고받는다.<br>
서버가 클라이언트에게 먼저 메시지를 보낼 수 있으며, HTTP 헤더 없이 작은 프레임으로 데이터를 전송한다.

<br>

### 구현 스택

| 스택 | 라이브러리/방법 | 의존성 |
|------|----------------|--------|
| Node.js | `ws` 라이브러리 | `npm install ws` |
| Python | `websockets` 라이브러리 | `pip install websockets` |
| Spring | `@EnableWebSocket` + `WebSocketHandler` | `spring-boot-starter-websocket` |

<br>

### Node.js 구현 구조

```javascript
const { WebSocketServer } = require('ws');
const clients = new Set();  // 연결된 클라이언트 목록

const wss = new WebSocketServer({ port: 8080 });

wss.on('connection', (ws) => {
  clients.add(ws);                          // 연결 등록
  ws.send('{"type":"welcome"}');            // 서버가 먼저 메시지 전송 가능

  ws.on('message', (raw) => {
    // 받은 메시지를 모든 클라이언트에 브로드캐스트
    for (const client of clients) {
      if (client.readyState === client.OPEN) {
        client.send(raw.toString());
      }
    }
  });

  ws.on('close', () => clients.delete(ws)); // 연결 종료 시 제거
});
```

<br>

### 브라우저 콘솔에서 테스트

브라우저 개발자 도구 콘솔에서 직접 WebSocket 연결을 테스트할 수 있다:

```javascript
// 연결
const ws = new WebSocket('ws://localhost:8080');

// 이벤트 핸들러 등록
ws.onopen    = () => console.log('연결됨');
ws.onmessage = (e) => console.log('수신:', e.data);
ws.onerror   = (e) => console.error('에러:', e);
ws.onclose   = () => console.log('종료됨');

// 메시지 전송 (연결된 후)
ws.send('안녕하세요');

// 연결 종료
ws.close();
```

실습 코드는 [week8/websocket-server.js](week8/websocket-server.js)와 [week8/index.html](week8/index.html)에서 확인할 수 있다.

<br>

## 커널 내부: TCP 연결과 epoll

[6.4절](/esc/CH6/README.md)에서 파일 디스크립터(FD)와 epoll의 동작 원리를 자세히 살펴봤다.<br>
여기서는 그 지식을 바탕으로 **WebSocket 서버가 수만 개의 연결을 어떻게 처리하는지** 살펴본다.

<br>

### TCP 연결 = 파일 디스크립터

클라이언트가 서버에 연결할 때마다 운영체제는 파일 디스크립터를 하나 할당한다.<br>
WebSocket이든 Long Polling이든, TCP 연결이 존재하는 한 FD는 점유된다.

```
클라이언트 10,000명 연결 시:
  → 서버 프로세스에 파일 디스크립터 10,000개 할당
  → lsof -p <pid> | wc -l 로 확인 가능
```

<br>

### WebSocket 서버의 epoll 루프

Node.js (`ws` 라이브러리)나 Python의 WebSocket 서버는 내부적으로 아래와 같은 구조로 동작한다:

```
1. epoll_create()
   → 커널에 epoll 인스턴스 생성

2. 클라이언트 연결 시:
   epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, ...)
   → 해당 FD를 감시 목록에 등록

3. 이벤트 루프:
   ready_fds = epoll_wait(epollfd, ...)
   → 이벤트가 발생할 때까지 블록 (잠든 상태)
   → 클라이언트 중 하나가 메시지를 보내면 깨어남
   → "준비된 FD 목록"만 반환

4. 준비된 FD에 대해서만 read/write 처리
   → 나머지 9,999개 연결은 계속 잠든 상태
```

**스레드 1개로 수만 연결을 처리할 수 있는 이유**:<br>
대부분의 연결은 `epoll_wait` 안에서 잠들어 있다.<br>
CPU는 실제로 데이터가 도착한 연결만 처리한다.<br>
6.4절에서 살펴본 것처럼, select/poll이 O(n)으로 모든 FD를 순회하는 것과 달리 epoll은 이벤트가 발생한 FD만 O(1)으로 받는다.

<br>

### Long Polling과 WebSocket의 FD 관점

```
Long Polling (30초 대기 중):
  client_fd_1 → epoll 감시 중 (타임아웃 타이머도 함께)
  client_fd_2 → epoll 감시 중
  client_fd_3 → epoll 감시 중
  ...
  (응답 후 연결 종료 → FD 해제 → 즉시 재연결 → 새 FD 할당)

WebSocket (영구 연결):
  client_fd_1 → epoll 감시 중 (연결이 끊기지 않는 한 유지)
  client_fd_2 → epoll 감시 중
  client_fd_3 → epoll 감시 중
  ...
  (메시지가 오면 처리, 오지 않으면 계속 잠든 상태)
```

두 방식 모두 epoll로 FD를 감시한다.<br>
핵심 차이는 FD 개수가 아니라 __HTTP 레이어를 계속 거치느냐(Long Polling) vs 거치지 않느냐(WebSocket)__이다.

<br>

## 운영 관점 비교: Long Polling vs WebSocket

### 핵심 비교

| 항목 | Long Polling | WebSocket |
|------|-------------|-----------|
| **인프라 호환성** | HTTP 기반, 기존 프록시/LB 그대로 사용 | WebSocket 지원 프록시 필요 (일부 구형 프록시 불가) |
| **서버 재시작** | 요청 단위로 재연결 → 자연스러운 graceful drain | 연결 강제 종료 → 클라이언트 재연결 로직 필요 |
| **스케일 아웃** | Stateless HTTP → 세션 없이 여러 서버에 분산 가능 | Sticky session 또는 pub/sub 중간 레이어 필요 |
| **디버깅** | curl, Postman으로 즉시 테스트 | 전용 WebSocket 클라이언트 필요 |
| **레이턴시** | 재연결마다 HTTP 헤더 파싱 오버헤드 | 프레임 단위 전송, 헤더 오버헤드 없음 |
| **와이파이 신뢰성** | HTTP이므로 일반 방화벽 통과 | 일부 기업/호텔 네트워크에서 WebSocket 차단 가능 |

<br>

### Inferable의 선택: Long Polling

Inferable이 WebSocket 대신 Long Polling을 선택한 실제 이유:

1. **기존 HTTP 모니터링 재사용**: Datadog, CloudWatch 등 HTTP 기반 모니터링 툴을 그대로 사용 가능
2. **로드밸런서 추가 설정 불필요**: WebSocket은 ALB에서 sticky session 설정이 필요
3. **서버 재시작 단순화**: 배포 시 연결 상태 관리 로직 불필요
4. **PostgreSQL과의 자연스러운 조합**: 500ms 간격으로 DB 조회, 적절한 인덱싱으로 비용 최소화

<br>

### 대안: Server-Sent Events (SSE)

Long Polling과 WebSocket 사이에는 **SSE(Server-Sent Events)**라는 선택지도 있다.

```
특성:
- HTTP/1.1 기반의 단방향(서버 → 클라이언트) 스트림
- 클라이언트: EventSource API 사용
- 연결이 끊기면 브라우저가 자동 재연결
- WebSocket과 달리 HTTP이므로 프록시 호환성 높음
- 단방향이므로 클라이언트 → 서버 전송은 별도 HTTP 요청 필요
```

<br>

### epoll과의 연결

세 가지 방식(일반 폴링, Long Polling, WebSocket) 모두 내부적으로 epoll을 사용한다.<br>
차이는 "언제 재연결을 허용하는가"의 트레이드오프다:

```
일반 폴링:
  빠른 재연결 주기 → 많은 HTTP 요청 → CPU 낭비

Long Polling:
  느린 재연결 주기 (이벤트 발생 시만) → HTTP 오버헤드 → epoll으로 타임아웃 관리

WebSocket:
  재연결 없음 → HTTP 오버헤드 없음 → epoll로 영구 연결 감시
```

**결론**: 6.4절에서 살펴본 "커널이 준비된 FD만 알려준다"는 epoll의 핵심 특성이,<br>
Long Polling의 30초 대기와 WebSocket의 수만 연결 유지를 모두 가능하게 하는 기반이다.

# 8주차 실습: Long Polling과 WebSocket 직접 구현

## 실습 개요

Long Polling과 WebSocket을 Node.js로 직접 구현하고,<br>
동일한 파일 디스크립터 + epoll 기반 위에서 두 방식이 어떻게 다르게 동작하는지 관찰한다.

이론 내용은 [CH6 Amendment.md](../Amendment.md)를 참고한다.

참고) 공부하면서 L7의 Polling, Long Polling, Websocket과 L4의 select, poll, epoll을 헷갈려서 이렇게 공부하게 되었습니다. 여러분은 혼동 없으시길 바랍니다.

<br>

## 파일 구성

```
week8/
├── long-polling-server.js   # Long Polling 서버 (Node.js 내장 모듈만 사용)
├── websocket-server.js      # WebSocket 서버 (ws 라이브러리 사용)
├── index.html               # 브라우저 테스트 UI
└── README.md                # 이 파일
```

<br>

## 준비

```bash
# ws 라이브러리 설치 (WebSocket 서버용)
npm install ws

# Node.js 버전 확인 (v14 이상 권장)
node --version
```

<br>

## 실행 및 테스트

### 실습 1: Long Polling

```bash
# 터미널 1: 서버 시작
node long-polling-server.js

# 터미널 2: 폴링 대기 (응답 없이 30초 대기하는 것을 관찰)
curl http://localhost:3000/poll

# 터미널 3: 이벤트 발생 (터미널 2에 즉시 응답되는 것을 관찰)
curl -X POST http://localhost:3000/event -d "안녕하세요"

# 타임아웃 확인: 30초 동안 이벤트 없이 기다리면
# {"type":"timeout"} 반환 후 연결 종료
```

**여러 클라이언트 동시 대기:**
```bash
# 터미널 여러 개에서 동시에 폴링 시작
curl http://localhost:3000/poll &
curl http://localhost:3000/poll &
curl http://localhost:3000/poll &

# 이벤트 발생 시 모든 클라이언트에 동시 응답되는 것을 확인
curl -X POST http://localhost:3000/event -d "브로드캐스트"
```

<br>

### 실습 2: WebSocket

```bash
# 터미널 1: WebSocket 서버 시작
node websocket-server.js

# 브라우저에서 index.html 열기
open index.html        # macOS
# 또는 브라우저에서 파일 직접 열기 (file:///경로/index.html)
```

**브라우저 개발자 도구 콘솔에서 직접 테스트:**
```javascript
// 연결
const ws = new WebSocket('ws://localhost:8080');
ws.onopen    = () => console.log('연결됨');
ws.onmessage = (e) => console.log('수신:', e.data);

// 메시지 전송 (연결 후)
ws.send('안녕하세요');

// 종료
ws.close();
```

<br>

## 관찰 포인트

### 1. 파일 디스크립터 확인

Long Polling 서버 또는 WebSocket 서버 실행 중에 열린 FD를 확인한다:

```bash
# 서버 프로세스 PID 확인
pgrep -f "node long-polling"
pgrep -f "node websocket"

# 해당 프로세스의 열린 FD 목록
lsof -p <PID> | grep -E "TCP|LISTEN"

# 클라이언트 연결 수에 따라 FD 수가 증가하는 것을 확인
```

### 2. Long Polling 관찰 포인트

- `/poll` 요청이 응답 없이 30초 동안 대기하는 것
- `/event` POST 직후 즉시 응답되는 것
- 응답 후 클라이언트가 즉시 재연결하는 것 (폴링 루프)
- 서버 재시작 시: 재연결 자연스럽게 이루어짐

### 3. WebSocket 관찰 포인트

- 연결 시 서버가 먼저 환영 메시지를 보내는 것 (Long Polling에서는 불가능)
- 메시지 전송 시 모든 클라이언트에 브로드캐스트되는 것
- 서버 재시작 시: 연결이 끊기고 클라이언트에서 재연결 로직 필요

<br>

## 핵심 연결: epoll

두 서버 모두 내부적으로 Node.js의 libuv를 통해 **Linux epoll** (macOS에서는 kqueue)을 사용한다.

```
Long Polling 서버 내부:
  epoll_wait() 블록
  → 클라이언트에서 HTTP 요청 도착 (FD 이벤트)
  → 깨어나 요청 파싱
  → waiters 배열에 resolve 등록
  → 다시 epoll_wait() 블록
  → 30초 타이머 이벤트 발생 시 깨어나 timeout 응답

WebSocket 서버 내부:
  epoll_wait() 블록
  → 클라이언트에서 WebSocket 프레임 도착 (FD 이벤트)
  → 깨어나 프레임 파싱
  → 모든 clients에 send
  → 다시 epoll_wait() 블록
```

두 방식의 실질적 차이는 FD 개수가 아니라, Long Polling은 이벤트마다 HTTP 레이어를 반복 거치고,<br>
WebSocket은 최초 핸드셰이크 이후 HTTP 없이 바로 프레임을 주고받는다는 것이다.

epoll 동작 원리는 [CH6 README 6.4절](../README.md#64-높은-동시성의-비결-입출력-다중화)을 참고한다.

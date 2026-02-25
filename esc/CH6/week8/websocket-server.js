/**
 * WebSocket 서버
 *
 * 학습 포인트:
 * - HTTP Upgrade 핸드셰이크 후 WebSocket 프레임으로 통신
 * - 서버가 클라이언트에게 먼저 메시지 전송 가능 (양방향)
 * - clients Set으로 연결된 클라이언트 관리
 * - 내부적으로 epoll을 사용해 FD 이벤트 감시 (Node.js libuv)
 *
 * 의존성: npm install ws
 * 실행: node websocket-server.js
 */

const { WebSocketServer } = require('ws');

// 연결된 클라이언트(WebSocket 객체) 목록
// 각 WebSocket 객체는 내부적으로 파일 디스크립터를 가리킨다
const clients = new Set();

const wss = new WebSocketServer({ port: 8080 });

wss.on('connection', (ws, req) => {
  clients.add(ws);

  const clientIp = req.socket.remoteAddress;
  console.log(`[연결] ${clientIp} (현재 연결 수: ${clients.size})`);

  // 서버가 먼저 메시지를 보낼 수 있다 (Long Polling에서는 불가능한 것)
  ws.send(JSON.stringify({
    type: 'welcome',
    message: '연결되었습니다.',
    connectedClients: clients.size,
  }));

  // 클라이언트에서 메시지 수신 시 모든 클라이언트에 브로드캐스트
  ws.on('message', (raw) => {
    const text = raw.toString();
    console.log(`[수신] "${text}"`);

    const payload = JSON.stringify({
      type: 'broadcast',
      message: text,
      from: clientIp,
    });

    // 연결된 모든 클라이언트에 전송
    let sent = 0;
    for (const client of clients) {
      // OPEN 상태인 클라이언트에만 전송
      if (client.readyState === client.OPEN) {
        client.send(payload);
        sent++;
      }
    }
    console.log(`[브로드캐스트] ${sent}개 클라이언트에 전달`);
  });

  // 연결 종료 시 clients Set에서 제거
  ws.on('close', (code, reason) => {
    clients.delete(ws);
    console.log(`[종료] ${clientIp} (코드: ${code}, 현재 연결 수: ${clients.size})`);
  });

  // 에러 처리
  ws.on('error', (err) => {
    console.error(`[에러] ${clientIp}: ${err.message}`);
    clients.delete(ws);
  });
});

console.log('WebSocket 서버 시작: ws://localhost:8080');
console.log('');
console.log('테스트 방법:');
console.log('  브라우저에서 index.html을 열거나,');
console.log('  개발자 도구 콘솔에서 아래 코드를 실행하세요:');
console.log('');
console.log('  const ws = new WebSocket("ws://localhost:8080");');
console.log('  ws.onmessage = (e) => console.log("수신:", e.data);');
console.log('  ws.onopen = () => ws.send("안녕하세요");');

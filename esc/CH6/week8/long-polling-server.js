/**
 * Long Polling 서버
 *
 * 학습 포인트:
 * - GET /poll: 응답을 보류하고 waiters 배열에 등록
 * - POST /event: 대기 중인 모든 클라이언트에 즉시 응답
 * - 30초 타임아웃: 이벤트 없으면 { type: 'timeout' } 반환
 * - req.on('close'): 클라이언트 연결 끊김 시 메모리 정리
 *
 * 의존성: 없음 (Node.js 기본 모듈만 사용)
 * 실행: node long-polling-server.js
 */

const http = require('http');

// 대기 중인 응답의 resolve 함수 목록
// 각 GET /poll 요청은 여기에 resolve 함수를 등록하고 대기한다
const waiters = [];

const TIMEOUT_MS = 30_000; // 30초

const server = http.createServer((req, res) => {
  const url = new URL(req.url, `http://${req.headers.host}`);

  // -----------------------------------------------
  // GET /poll - Long Polling 엔드포인트
  // -----------------------------------------------
  if (req.method === 'GET' && url.pathname === '/poll') {
    res.setHeader('Content-Type', 'application/json');
    // CORS 허용 (index.html에서 fetch 테스트를 위해)
    res.setHeader('Access-Control-Allow-Origin', '*');

    let resolved = false;

    // 30초 타임아웃: 이벤트 없으면 timeout 응답 반환
    const timer = setTimeout(() => {
      if (resolved) return;
      resolved = true;
      const idx = waiters.indexOf(resolve);
      if (idx !== -1) waiters.splice(idx, 1);
      console.log('[timeout] 클라이언트에 타임아웃 응답');
      res.end(JSON.stringify({ type: 'timeout' }));
    }, TIMEOUT_MS);

    // 이벤트 발생 시 호출될 함수
    function resolve(data) {
      if (resolved) return;
      resolved = true;
      clearTimeout(timer);
      console.log(`[event] 클라이언트에 응답: ${data}`);
      res.end(JSON.stringify({ type: 'event', data }));
    }

    // waiters 목록에 등록하고 대기
    waiters.push(resolve);
    console.log(`[poll] 대기 시작 (현재 대기 수: ${waiters.length})`);

    // 클라이언트가 연결을 끊으면 메모리 정리
    // 이 처리 없이는 waiters에 무효 함수가 계속 쌓인다
    req.on('close', () => {
      if (!resolved) {
        resolved = true;
        clearTimeout(timer);
        const idx = waiters.indexOf(resolve);
        if (idx !== -1) waiters.splice(idx, 1);
        console.log(`[close] 클라이언트 연결 끊김 (남은 대기 수: ${waiters.length})`);
      }
    });

    return;
  }

  // -----------------------------------------------
  // POST /event - 이벤트 발생 시뮬레이션
  // -----------------------------------------------
  if (req.method === 'POST' && url.pathname === '/event') {
    res.setHeader('Content-Type', 'application/json');
    res.setHeader('Access-Control-Allow-Origin', '*');

    let body = '';
    req.on('data', chunk => { body += chunk; });
    req.on('end', () => {
      const data = body || 'ping';

      // 대기 중인 모든 클라이언트에 즉시 응답
      const pending = waiters.splice(0); // waiters를 비우면서 가져옴
      console.log(`[event] 이벤트 발생: "${data}" → ${pending.length}개 클라이언트에 전달`);
      pending.forEach(resolve => resolve(data));

      res.end(JSON.stringify({ notified: pending.length, data }));
    });

    return;
  }

  // CORS preflight 처리
  if (req.method === 'OPTIONS') {
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST');
    res.writeHead(204);
    res.end();
    return;
  }

  res.writeHead(404);
  res.end(JSON.stringify({ error: 'Not Found' }));
});

server.listen(3000, () => {
  console.log('Long Polling 서버 시작: http://localhost:3000');
  console.log('');
  console.log('테스트 방법:');
  console.log('  # 터미널 1 - 폴링 대기');
  console.log('  curl http://localhost:3000/poll');
  console.log('');
  console.log('  # 터미널 2 - 이벤트 발생');
  console.log('  curl -X POST http://localhost:3000/event -d "hello"');
});

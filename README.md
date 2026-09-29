# ft_irc · 42 Seoul

> C++98로 작성한 kqueue 기반 단일 스레드 IRC 서버 `ircserv`

## 개요

RFC 1459/2812 계열 IRC 프로토콜의 일부를 구현한 서버로, irssi 같은 일반 IRC 클라이언트로 접속해 채널 입장, 메시지 전송, 채널 운영자 명령을 사용할 수 있습니다.
하나의 프로세스, 하나의 이벤트 루프에서 모든 클라이언트를 논블로킹 소켓으로 처리합니다.
3인 팀 프로젝트(ryuhajin, Seongjko, akth101)이며, 이 저장소는 팀 저장소의 서버 코드 스냅샷과 제가 개발 전에 정리한 학습 노트를 함께 담고 있습니다.

## 구현 포인트

- **kqueue 이벤트 루프**: 서버 소켓과 클라이언트 소켓을 `EVFILT_READ` / `EVFILT_WRITE`로 등록하고, `kevent()` 한 번의 호출로 변경 목록(`_newEvents`) 반영과 이벤트 대기를 함께 처리합니다. `EV_EOF` / `EV_ERROR`는 연결 종료로 처리합니다.
- **쓰기 이벤트 on/off**: 응답은 곧바로 `send()`하지 않고 클라이언트별 송신 버퍼에 쌓은 뒤 `EVFILT_WRITE`를 켭니다. 다 보내면 쓰기 이벤트를 끄고, 일부만 보내졌으면 남은 부분을 버퍼에 남겨 다음 이벤트에서 이어 보냅니다.
- **수신 버퍼와 파싱**: `recv()`로 읽은 데이터를 클라이언트별 버퍼에 이어 붙이고, 마지막 `\r\n`까지만 잘라 명령 단위로 나눕니다. 뒤에 남은 미완성 조각은 다음 수신 때 이어 붙입니다. 각 줄은 `prefix` / `cmd` / `params` / `trailing` 구조체(`request::msg`)로 파싱됩니다.
- **등록 절차**: `PASS` → `NICK` / `USER`가 모두 끝나야 등록된 것으로 보고 `001` 환영 메시지를 보냅니다. 첫 닉네임이 이미 쓰이고 있으면 뒤에 숫자를 붙여 고유한 닉네임을 만듭니다.
- **채널 모드**: `i`(초대 전용), `t`(운영자만 토픽 변경), `k`(채널 키), `o`(운영자 권한 부여/회수), `l`(인원 제한, 1~10)을 지원합니다. 채널의 기본 인원 상한은 10명입니다.
- **채널 수명 관리**: 첫 입장자가 채널 운영자가 되고, 마지막 사용자가 나가거나 강퇴되면 채널이 삭제됩니다. 입장 시 `helloBot`이 환영 메시지를 채널에 보냅니다.
- **종료 처리**: `SIGINT` / `SIGQUIT`을 받으면 루프를 빠져나와 모든 클라이언트에 `QUIT`을 보내고 채널과 클라이언트 객체를 정리합니다.

## 지원 명령어

| 명령어 | 동작 |
| --- | --- |
| `PASS` | 서버 비밀번호 확인. 틀리면 `464`, 재등록 시 `462` |
| `NICK` | 닉네임 설정/변경. 형식 검사(최대 12자), 중복 시 `433`, 변경 사실을 참여 채널에 전파 |
| `USER` | 사용자 이름 등록. NICK까지 끝나면 `001` 전송 |
| `JOIN` | 쉼표로 여러 채널 입장. 없으면 생성, `+i` / `+k` / `+l` 검사 후 `353` / `366` 전송 |
| `PART` | 쉼표로 여러 채널 퇴장, 마지막 사용자면 채널 삭제 |
| `KICK` | 운영자가 쉼표로 여러 사용자 강퇴 |
| `INVITE` | 사용자를 채널에 초대. `+i` 채널이면 운영자만 가능하고 초대 목록에 추가 |
| `TOPIC` | 토픽 조회/변경. `+t`면 운영자만 변경 가능 |
| `MODE` | 채널 모드 `i t k o l` 설정/해제, 인자 없으면 현재 모드 응답 (사용자 모드는 무시) |
| `PRIVMSG` | 채널 또는 닉네임으로 메시지 전송, 쉼표로 여러 대상 지정 |
| `PING` | `PONG`으로 응답 |
| `QUIT` | 연결 종료, 참여 중이던 채널에 `QUIT` 전파 |

그 밖의 명령은 `421 Unknown command`로 응답합니다. 명령별 응답 코드와 메시지는 `command.md`에 정리되어 있습니다.

## 빌드 & 실행

macOS(kqueue) 전용입니다. Linux에서는 `sys/event.h`가 없어 빌드되지 않습니다.

```sh
make
./ircserv <port> <password>      # 예: ./ircserv 6667 pass1234
```

```sh
# irssi
/connect localhost 6667 pass1234

# nc (줄 끝을 \r\n으로 보내야 하므로 -c 옵션 사용)
nc -c localhost 6667
PASS pass1234
NICK alice
USER alice 0 * :Alice
JOIN #test
PRIVMSG #test :hello
```

## 테스트

`flood.sh`는 10개의 백그라운드 작업이 각각 10번씩 `nc`로 접속해 PASS / NICK / USER / JOIN 후 채널에 메시지 50개를 보내고 QUIT하는 부하 테스트이며, 끝나면 연결 수와 메시지 수를 집계해 출력합니다. 스크립트 안의 포트(6668)와 비밀번호를 서버 실행 인자에 맞춰야 합니다.

## 학습 노트

| 파일 | 내용 |
| --- | --- |
| `notes/socket.md` | 소켓 통신 흐름을 전화 연결에 빗대어 정리한 서버/클라이언트 호출 순서 |
| `notes/function.md` | `sockaddr_in`, `socket`, `bind`, `listen`, `fcntl`, `kqueue`, `kevent`의 인자와 반환값 |
| `notes/MultiplexedIO.md` | 다중 입출력의 개념과 select / poll 계열 함수의 공통 동작 원리 |
| `notes/exam06.md` | select 기반 소켓 서버 예제, 흐름 도식, exam06 서브젝트 분석 |
| `notes/kqueue.md` | select와 kqueue의 차이, `kevent` 구조체와 `EV_SET`, kqueue 에코 서버 해설 |
| `notes/echo.cpp` | kqueue로 작성한 에코 서버 예제 (본 서버 이벤트 루프의 출발점) |
| `notes/rfc.md` | IRC RFC 읽기를 시작하며 남긴 짧은 메모 |

## 구조

```
.
├── main.cpp            # 인자 검사, 시그널, kevent 루프
├── server.{hpp,cpp}    # 소켓 설정, 이벤트 등록, 수신/송신, 연결 해제
├── client.{hpp,cpp}    # 클라이언트 상태와 송수신 버퍼
├── channel.{hpp,cpp}   # 채널 사용자/운영자/초대 목록과 모드
├── parser.{hpp,cpp}    # IRC 메시지 파싱
├── utils.{hpp,cpp}     # 포트/인자 검사
├── cmd/                # 명령어별 구현과 디스패처 (cmd.cpp)
├── command.md          # 명령별 응답 코드 정리
├── flood.sh            # 플러딩 테스트
└── notes/              # 학습 노트
```

42 Seoul 공통 과정 과제.

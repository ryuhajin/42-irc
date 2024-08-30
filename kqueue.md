# kqueue 그게 뭔데?

kqueue는 FreeBSD와 macOS에서 지원되는 다중입출력 함수이다.
이벤트 기반 I/O를 지원하며, 커널에서 직접 이벤트를 관리하므로 높은 성능을 발휘한다. 대규모 네트워크 서버에 적합하다.

## select 함수와 차이점
앞에서 배운 select 함수와 kqueue는 어떤 차이점이 있을까?

|항목|select()|kqueue()|
|---|---|---|
|지원 플랫폼|대부분의 운영체제 (리눅스, 윈도우, BSD 등)|BSD 계열 운영체제 (FreeBSD, macOS 등)|
|시간 복잡도|O(n)|O(1)|
|확장성|파일 디스크립터의 수에 제한이 있음 (FD_SETSIZE)|수천 개 이상의 파일 디스크립터를 효율적으로 처리 가능|
|유연성|소켓 관련 이벤트 감시에 적합|다양한 이벤트 감시 가능 (소켓, 파일, 타이머 등)|

## 우리는 왜 kqueue를 써야할까?
1. mac을 사용해야 하는 환경
2. 시간 복잡도 면에서 select보다 kqueue가 훨씬 우수.

### 시간 복잡도가 차이나는 이유
|항목|select()|kqueue()|
|---|---|---|
|데이터 구조|select 함수는 파일 디스크립터 집합(읽기, 쓰기, 예외 처리에 대한 각각의 FD 집합)을 검사하기 위해 비트 마스크 배열을 사용한다. 이 배열은 감시할 파일 디스크립터의 상태를 나타낸다.|kqueue는 커널 내부에서 이벤트 큐를 사용하여 비동기적으로 이벤트를 관리한다.|
|동작 방식|select는 호출될 때마다 모든 파일 디스크립터를 **처음부터 끝까지 순회**하여 어떤 파일 디스크립터가 지정된 이벤트(읽기, 쓰기, 예외 처리)를 준비한 상태인지 검사한다.|사용자가 감시할 이벤트를 정의하고 이를 kqueue에 등록하면, 커널은 이벤트가 발생할 때까지 감시한다. 이벤트가 발생하면, 커널은 해당 이벤트를 이벤트 큐에 추가하고 사용자에게 이 정보를 전달한다.|
|시간 복잡도|검사 과정에서 모든 파일 디스크립터를 순회하므로, 파일 디스크립터의 수가 n일 때 시간 복잡도는 O(n)이다.|이벤트가 발생한 파일 디스크립터의 수가 m이라면, kqueue의 시간 복잡도는 O(m)이 된다.|
||select는 검사 과정에서 각 파일 디스크립터에 대해 시스템 호출을 반복해야 하므로, 수백 개 이상의 디스크립터를 감시할 때 성능이 크게 떨어진다.|kqueue는 이벤트가 발생할 때만 관련 작업을 수행하므로, 이벤트가 발생한 파일 디스크립터의 수에 따라 작업량이 결정된다.|
||즉, 파일 디스크립터가 많아질수록 성능이 선형적으로 저하된다.|일반적으로 모든 파일 디스크립터에서 동시에 이벤트가 발생하는 경우는 드물기때문에 kqueue의 시간 복잡도는 사실상 O(1)로 간주된다.|

### 결론
> select는 **모든 파일 디스크립터를 반복적으로 검사**하는 데 반해, kqueue는 **이벤트가 발생한 파일 디스크립터만 처리**한다.<br>이로 인해 성능 차이가 발생하며 이러한 차이로 인해 대규모 네트워크 애플리케이션에서는 kqueue를 사용하는게 적합하다.

---

## kqueue 배우기

[kqueue 튜토리얼](https://wiki.netbsd.org/tutorials/kqueue_tutorial/)

### kevent 구조체 필드

```
struct kevent
{
    uintptr_t ident;   // 이벤트의 식별자 (파일 디스크립터, 프로세스 ID 등)
    int16_t filter;    // 이벤트 필터 (EVFILT_READ, EVFILT_WRITE 등)
    uint16_t flags;    // 이벤트 플래그 (EV_ADD, EV_DELETE 등)
    uint32_t fflags;    // 필터 특유의 플래그
    intptr_t data;    // 이벤트와 관련된 추가 데이터 (예: 읽거나 쓸 수 있는 바이트 수)
    void *udata;  // 사용자 정의 데이터 포인터 (콜백 또는 추가 정보 전달에 사용)
};
```

### EV_SET() 매크로 필드
> EV_SET() 매크로는 kevent 구조체를 초기화하고 설정하는 데 사용되는 유틸리티 매크로이다. kqueue 기반 애플리케이션에서 이벤트를 추가하거나 수정할 때 반복적으로 구조체의 필드를 설정해야 하는데, 이 과정을 간편하게 해준다.


EV_SET() 매크로는 총 7개의 매개변수를 가진다.

```
#define EV_SET(kevp, a, b, c, d, e, f) do
{ 
    (kevp)->ident = (a);                   
    (kevp)->filter = (b);                  
    (kevp)->flags = (c);                   
    (kevp)->fflags = (d);                  
    (kevp)->data = (e);                    
    (kevp)->udata = (f);                   
}
```

|위치|매개변수|동작|
|---|---|---|
|kevp|kevp|kevent 구조체 포인터. 초기화할 대상 구조체의 포인터를 전달한다.|
|a|ident|이벤트 식별자(ident 필드)로, fd라고 생각하면 된다. 이벤트를 모니터링할 대상(예: 파일 디스크립터, 소켓, 프로세스 ID 등)을 지정한다.|
|b|filter|이벤트 필터(filter 필드)로, 모니터링할 이벤트의 유형을 지정한다. (예: EVFILT_READ, EVFILT_WRITE 등).|
|c|flag|이벤트 플래그(flags 필드)로, 이벤트의 동작 방식을 지정한다. (예: EV_ADD, EV_DELETE, EV_ENABLE, EV_DISABLE 등).|
|d|fflags|필터 특유의 플래그(fflags 필드)로, 특정 필터에 대한 추가적인 설정을 지정한다.|
|e|data|이벤트와 관련된 추가 데이터(data 필드)로, 이벤트 발생 시 구조체가 반환할 데이터를 설정한다.|
|f|udata|사용자 정의 데이터 포인터(udata 필드)로, 이벤트와 관련된 추가 사용자 정보를 전달할 수 있다.|

## kqueue를 사용한 에코 서버 코드 예시
```c++
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <map>
#include <vector>

using namespace std;

void exit_with_perror(const string& msg)
{
    cerr << msg << endl;  // 에러 메시지를 출력하고
    exit(EXIT_FAILURE);   // 프로그램을 종료
}

void change_events(vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp_event;
    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata); // 이벤트 설정
    change_list.push_back(temp_event);  // 이벤트를 변경 목록에 추가
}

void disconnect_client(int client_fd, map<int, string>& clients)
{
    cout << "client disconnected: " << client_fd << endl;  // 클라이언트 연결 해제 시 출력
    close(client_fd);  // 클라이언트 소켓 닫기
    clients.erase(client_fd);  // 클라이언트 정보 삭제
}

int main()
{
    /* 서버 소켓 초기화 및 연결 설정 */
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        exit_with_perror("socket() error\n" + string(strerror(errno)));

    memset(&server_addr, 0, sizeof(server_addr));  // 서버 주소 구조체 초기화
    server_addr.sin_family = AF_INET;  // IPv4 주소체계 설정
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 모든 IP로부터의 연결 허용
    server_addr.sin_port = htons(8080);  // 포트 번호 8080으로 설정
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        exit_with_perror("bind() error\n" + string(strerror(errno)));

    if (listen(server_socket, 5) == -1)
        exit_with_perror("listen() error\n" + string(strerror(errno)));
    fcntl(server_socket, F_SETFL, O_NONBLOCK);  // 서버 소켓을 논블로킹 모드로 설정
    
    /* kqueue 초기화 */
    int kq;
    if ((kq = kqueue()) == -1)
        exit_with_perror("kqueue() error\n" + string(strerror(errno)));

    map<int, string> clients; // 클라이언트 소켓과 데이터를 저장하는 맵
    vector<struct kevent> change_list; // 이벤트 변경 목록을 저장하는 벡터
    struct kevent event_list[8]; // 이벤트 리스트를 저장하는 배열 (최대 8개의 이벤트 처리)

    /* 서버 소켓에 대한 이벤트 등록 (읽기 대기) */
    change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    cout << "echo server started" << endl;

    /* 메인 루프 */
    int new_events;
    struct kevent* curr_event;
    while (1)
    {
        /* 변경된 이벤트 적용 및 대기 중인 새로운 이벤트 반환 */
        new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
        if (new_events == -1)
            exit_with_perror("kevent() error\n" + string(strerror(errno)));

        change_list.clear(); // 새로운 변경을 위해 이벤트 변경 목록 초기화

        for (int i = 0; i < new_events; ++i)
        {
            curr_event = &event_list[i]; // 현재 이벤트

            /* 에러 이벤트 확인 */
            if (curr_event->flags & EV_ERROR)
            {
                if (curr_event->ident == server_socket)
                    exit_with_perror("server socket error"); // 서버 소켓에 오류 발생 시 종료
                else
                {
                    cerr << "client socket error" << endl;  // 클라이언트 소켓에 오류 발생 시
                    disconnect_client(curr_event->ident, clients);  // 클라이언트 연결 해제
                }
            }
            else if (curr_event->filter == EVFILT_READ) // 읽기 이벤트
            {
                if (curr_event->ident == server_socket) // 서버 소켓에서의 읽기 이벤트
                {
                    /* 새로운 클라이언트 연결 수락 */
                    int client_socket;
                    if ((client_socket = accept(server_socket, NULL, NULL)) == -1)
                        exit_with_perror("accept() error\n" + string(strerror(errno)));
                    cout << "accept new client: " << client_socket << endl;
                    fcntl(client_socket, F_SETFL, O_NONBLOCK);  // 새로운 클라이언트 소켓을 논블로킹 모드로 설정

                    /* 클라이언트 소켓에 대한 읽기 및 쓰기 이벤트 추가 */
                    change_events(change_list, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    change_events(change_list, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    clients[client_socket] = "";  // 클라이언트 목록에 추가
                }
                else if (clients.find(curr_event->ident)!= clients.end()) // 클라이언트 소켓에서의 읽기 이벤트
                {
                    /* 클라이언트로부터 데이터 읽기 */
                    char buf[1024];
                    int n = read(curr_event->ident, buf, sizeof(buf));

                    if (n <= 0)
                    {
                        if (n < 0)
                            cerr << "client read error!" << endl;  // 클라이언트 읽기 오류
                        disconnect_client(curr_event->ident, clients);  // 클라이언트 연결 해제
                    }
                    else
                    {
                        buf[n] = '\0';  // 문자열의 끝에 NULL 추가
                        clients[curr_event->ident] += buf;  // 클라이언트 데이터에 추가
                        cout << "received data from " << curr_event->ident << ": " << clients[curr_event->ident] << endl;
                    }
                }
            }
            else if (curr_event->filter == EVFILT_WRITE) // 쓰기 이벤트
            {
                /* 클라이언트로 데이터 전송 */
                map<int, string>::iterator it = clients.find(curr_event->ident);
                if (it != clients.end())
                {
                    if (clients[curr_event->ident] != "")
                    {
                        int n;
                        if ((n = write(curr_event->ident, clients[curr_event->ident].c_str(),
                                        clients[curr_event->ident].size()) == -1))
                        {
                            cerr << "client write error!" << endl;
                            disconnect_client(curr_event->ident, clients);  
                        }
                        else
                            clients[curr_event->ident].clear();  // 전송 후 데이터 초기화
                    }
                }
            }
        }

    }
    return (0);
}
```

## 내가 이해한 kqueue 에코 서버
1. change_list는 kqueue가 모니터링하게 할 fd list이다.
    * change_list는 kevent 구조체의 목록으로, kqueue가 모니터링할 파일 디스크립터(fd)와 관련된 이벤트 변경 사항들을 담고 있다.
    * 이 목록은 kevent() 호출 시에 kqueue에게 어떤 이벤트를 추가하거나 수정할지를 알려주는 데 사용된다.

2. change_events 함수의 EV_SET 매크로를 통해 kqueue가 모니터링할 fd들의 필터를 세팅하고 change_list에 추가한다.
   * EV_SET 매크로는 kevent 구조체를 초기화하는 데 사용된다.
    * change_events 함수는 이벤트(예: 읽기 이벤트)를 설정하고 이를 change_list에 추가하여 kevent()가 나중에 이 변경 사항을 kqueue에 적용할 수 있도록 한다.

3. event_list는 kqueue가 이벤트 발생을 감지한 fd list이다.
    * event_list는 kevent() 호출 시 kqueue가 감지한 이벤트를 저장하는 배열이다.
    * 이 배열에는 발생한 이벤트의 파일 디스크립터와 그 이벤트의 세부 정보가 포함된다.

4. change_list.clear() 를 하는 이유
    * change_list에 fd들을 추가하고 kevent 함수로 kqueue에 등록하면 프로그램이 종료되거나 fd가 닫히기 전까지 kqueue가 자동으로 모니터링 한다.
    * 즉 kqueue에 등록을 마친 fd들은 모니터링 목록에 다시 등록할 필요가 없으니 change_list.clear() 를 통해 이벤트 목록을 초기화 해준다.

5. kqueue는 change_list에 등록된 fd들을 모니터링하고 있다가 이벤트 발생을 감지하면 event_list에 이벤트가 발생한 fd를 추가한다.
    * 정확히는, kqueue는 kevent() 함수가 호출될 때마 다 change_list를 기반으로 모니터링할 이벤트를 설정하거나 수정한다. 
    * 이후, kqueue는 이들 이벤트를 지속적으로 모니터링하다가 이벤트가 발생하면 해당 이벤트를 event_list에 추가하여 반환한다.

6. kevent를 통해 이벤트가 발생한 fd가 몇 개인지 체크한다.
    * kevent() 함수의 반환값은 감지된(발생한) 이벤트의 개수를 나타낸다. 이 값을 통해 이벤트가 몇 개인지 확인할 수 있다.

7. 발생한 이벤트만큼 반복문을 돌며 kqueue가 event_list에 등록한 이벤트들의 요청을 처리한다.
    * kevent()의 반환값을 사용해 이벤트가 발생한 개수만큼 반복문을 실행하며, 각 이벤트를 처리한다.
    * 이 때, event_list의 각 항목을 참조하여 어떤 파일 디스크립터에서 어떤 종류의 이벤트가 발생했는지 확인하고, 이에 맞는 처리를 한다.

## kqueue 에코 서버 도식화
```
kqueue를 사용한 에코 서버의 흐름
1.서버 소켓 생성 및 초기화
    - 서버 소켓 생성 (AF_INET, SOCK_STREAM)
    - 서버 주소 구조체 초기화 (IPv4, INADDR_ANY, 포트 설정)
    - 서버 소켓을 특정 주소와 포트에 바인딩
    - 서버 소켓을 연결 대기 상태로 전환 (listen)
    - 서버 소켓을 논블로킹 모드로 설정 (fcntl)

2. kqueue 생성 및 초기화
    - kqueue 생성 (kqueue())
    - 이벤트 변경 목록 벡터(change_list) 초기화
    - 이벤트 리스트 배열(event_list) 초기화
    
3. 서버 소켓에 대한 읽기 이벤트 등록
    - EV_SET 매크로를 사용해 서버 소켓의 읽기 이벤트를 설정 (EVFILT_READ)
    - change_events 함수를 통해 change_list에 읽기 이벤트 추가
    
4. 메인 루프 진입
    - 변경 목록을 적용하고 새로운 이벤트 감지 (kevent 호출)
    - 이벤트 변경 목록 초기화 (change_list.clear())

5. 이벤트 처리
    - 발생한 이벤트 개수(new_events)만큼 반복 처리

    └─ 분기 1: 에러 이벤트 (EV_ERROR)
        ├─ 서버 소켓 에러: 프로그램 종료 
        └─ 클라이언트 소켓 에러: 클라이언트 연결 해제 
        
    └─ 분기 2: 읽기 이벤트 (EVFILT_READ) 
        └─ 2-1. 준비된 fd가 서버 소켓인 경우 (새로운 클라이언트 연결 요청)  
        ├─ 클라이언트 연결 요청을 수락 (accept)
        ├─ 새 클라이언트 소켓을 논블로킹 모드로 설정 (fcntl) 
        ├─ 클라이언트 소켓에 대한 읽기 및 쓰기 이벤트를 change_list에 추가  
        └─ 클라이언트 목록에 추가 
        
        └─ 2-2. 준비된 fd가 클라이언트 소켓인 경우 (데이터 수신) 
        ├─ 클라이언트로부터 데이터 수신 (read) 
        └─ 서브 분기: 수신된 데이터의 길이에 따른 처리 
            ├─ 데이터의 길이가 0이하인 경우 (연결 종료 또는 오류)
            ├─ 클라이언트 연결 해제
            └─ 클라이언트 목록에서 제거 
            │ 
            └─ 데이터가 있는 경우 (유효한 요청 처리)
            ├─ 수신된 메시지를 클라이언트 메시지 버퍼에 추가 
            └─ 수신된 메시지 출력
    └─ 분기 3: 쓰기 이벤트 (EVFILT_WRITE) 
        ├─ 클라이언트 버퍼에 데이터가 있을 때, 해당 데이터를 클라이언트에 전송 (write)
        └─ 전송 후 클라이언트 버퍼 초기화

6. 반복문을 통해 모니터링 작업을 계속 수행
    - 4번으로 돌아가 kevent 호출을 반복하며, 입출력 이벤트를 계속 감지하고 처리
```
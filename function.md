# 함수 사용법
socket 프로그래밍을 하면서 헷갈렸던 함수들의 매개변수, 옵션, 리턴값 정리

## struct sockaddr_in
```c++
#include <arpa/inet.h>

struct sockaddr_in {
    sa_family_t    sin_family;   // 주소 패밀리 (주소 체계, IPv4)
    in_port_t      sin_port;     // 포트 번호 (네트워크 바이트 순서)
    struct in_addr sin_addr;     // IPv4 주소 (네트워크 바이트 순서)
    char           sin_zero[8];  // 패딩용 필드 (미사용)
};
```

### sockaddr_in 초기화 예시
```c++
void settingServerAddr (sockaddr_in &serverAddr, char* port)
{
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
}
``` 
* [네트워크 소켓 프로그래밍 시 INADDR_ANY의 의미](https://limjunho.github.io/2021/05/20/INADDR_ANY.html)
* [소켓의 주소체계](https://m.blog.naver.com/pjok1122/221240952223) - 네트워크 바이트 순서로 저장 (htons, htonl)


## socket()
```c++
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```
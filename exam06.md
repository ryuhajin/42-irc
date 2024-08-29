## select 함수를 활용한 socket server 코드

irc를 시작하기 전 exam06 문제와 소스코드를 살짝 훑고 가면 도움이 될거라는 조언을 얻고 exam06을 찾아보는 시간을 가졌다.<br>
하지만 소켓 프로그래밍 지식이 없던 나에게는 exam06의 서브젝트와 이에 따른 코드 구현부를 이해하기 어려웠다.<br>
따라서 exam06에서 쓰이는 기초적인 함수 사용 방법부터 찾아보는게 옳다는 결론을 내렸다.<br>
아래는 c에서 사용하는 멀티플렉싱 함수인 select 함수를 사용한 기초적인 socket sever 코드이다.

* [select를 이용한 입출력 다중화](https://www.joinc.co.kr/w/Site/system_programing/File/select) - 입출력관리에 필요한 fd_set과 select 함수에 대해 자세히 설명해 놓은 블로그 링크

<br>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFSIZE 100
#define PORT 9897

int main(void)
{
    int serv_sock;
    struct sockaddr_in serv_addr;

    fd_set reads, temps;
    int fd_max;
    int binded;

    char message[BUFSIZE];
    struct timeval timeout;

    int fd, str_len;
    int clnt_sock, clnt_len;
    struct sockaddr_in clnt_addr;
    int opt = 1;

    // 1. 서버 소켓을 생성. IPv4 주소체계(AF_INET), TCP 프로토콜(SOCK_STREAM), 기본 프로토콜(0) 사용.
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);

    // 생성된 서버 소켓에 소켓 옵션 설정. SO_REUSEADDR와 SO_REUSEPORT는 서버 소켓이 바인딩에 실패하는 상황을 방지하기 위해 사용된다.
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    // 서버 소켓 주소 구조체 초기화.
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // IPv4 주소체계 사용
    serv_addr.sin_addr.s_addr = INADDR_ANY; // 모든 네트워크 인터페이스에서 접속 허용
    serv_addr.sin_port = htons(PORT); // 네트워크 바이트 오더로 포트 번호 설정

    // 2. 서버 소켓에 주소를 바인딩.
    binded = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(binded != 0)
    {
      perror("bind error");
      exit(-1);
    }

    // 3. 서버 소켓을 리슨 상태로 전환하여 클라이언트 연결을 대기.
    binded = listen(serv_sock, 5);
    if(binded != 0)
    {
      perror("listen error");
      exit(-1);
    }
    printf("server listening\n");

    // 4. 파일 디스크립터 집합 초기화. 모든 비트를 0으로 설정하여, 초기 상태에서는 어떤 파일 디스크립터도 포함되지 않도록 한다.
    FD_ZERO(&reads);

    // 5. 서버 소켓을 읽기 가능 상태로 설정하여, 클라이언트 연결 요청을 모니터링한다.
    FD_SET(serv_sock, &reads);

    // 6. 현재 모니터링할 파일 디스크립터 중 가장 큰 값으로 서버 소켓의 파일 디스크립터를 설정한다.
    fd_max = serv_sock;
    printf("serv_sock is : %d\n", serv_sock);

    while(1)
    {
      temps = reads; // select 호출 전 원본을 보존하기 위해 reads를 temps로 복사함.
      timeout.tv_sec = 5; // 타임아웃 5초로 설정.
      timeout.tv_usec = 0;

      // 7. select 함수를 사용하여 읽기 가능 상태의 파일 디스크립터가 있는지 확인한다.
      // fd_max + 1까지의 파일 디스크립터를 검사한다.
      // 최대 fd 번호에 1을 더하는 이유는 fd 번호가 0 부터 시작하기 때문이다.
      
      // fd_set 구조체는 fd에 변경된 데이터가 있다면 해당 fd의 비트값을 1로 설정해둔다.
      // select 함수는 데이터가 변경된 파일의 개수. 즉 fd_set에서 비트 값이 1인 fd들의 개수를 반환한다.
      // 데이터가 변경된 fd를 반환하지 않는 다는 것에 주의할 것!

      
      if(select(fd_max + 1, &temps, 0, 0, &timeout) < 0)
      {
        perror("select error");
        exit(1);
      }
 
      // 8. for문을 통해 모든 파일 디스크립터를 검사한다.
      // 위에서 말했다시피 select 함수는 어떤 파일 디스크립터에 변화가 있는지 알려주지 않으므로, 반복문을 통해 직접 확인해야 한다.
      for(fd = 0; fd < fd_max+1; fd++)
      {
        // 9. 현재 검사 중인 fd가 읽기 가능 상태인지 확인한다.
        if(FD_ISSET(fd, &temps))
        {
          // 10. 만약 읽기 가능 상태의 fd가 서버 소켓이라면, 클라이언트의 연결 요청을 처리한다.
          if(fd == serv_sock)
          {
            clnt_len = sizeof(clnt_addr);
            clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_len);
            // 새 클라이언트의 연결을 수락하고, 클라이언트 소켓을 생성.

            // 11. 새로 연결된 클라이언트 소켓을 파일 디스크립터 집합에 추가하여 모니터링한다.
            FD_SET(clnt_sock, &reads);
            if(fd_max < clnt_sock) // 새로 들어온 클라이언트의 소켓이 현재 최대 파일 디스크립터보다 큰 경우 갱신.
            {
              fd_max = clnt_sock;
            }
            printf("client access : socket descriptor %d\n", clnt_sock);
          }
          else{
            // 12. 이미 연결된 클라이언트로부터 데이터가 들어온 경우 데이터를 읽는다.
            str_len = read(fd, message, BUFSIZE);
            if(str_len == 0) // 13. 만약 데이터의 길이가 0이면 클라이언트가 연결 종료를 요청한 것이다.
            {
              // 14. 클라이언트 소켓을 파일 디스크립터 집합에서 제거하고, 해당 소켓을 닫습니다.
              FD_CLR(fd, &reads);
              close(fd);
              printf("client end : socket descriptor %d\n", fd);
            }
            else{ // 15. 클라이언트로부터 받은 메시지를 처리한다.
              message[str_len] = '\0'; // 문자열 끝을 표시하기 위해 널 문자 추가
              printf("client : %d message : %s\n", fd, message); // 클라이언트의 메시지를 출력
              write(fd, message, str_len); // 받은 메시지를 다시 클라이언트에게 에코한다.
            }
          }
        }
      }
    }
  return 0;
}
```
<br>

위의 흐름을 이해한다면 아래의 exam06의 서브젝트와 코드 흐름을 읽는데 큰 도움이 될 것이다.

## exam06 서브젝트
```
특정 포트(127.0.0.1의 포트)에 연결하려는 클라이언트를 대기하고, 클라이언트 간의 통신을 허용하는 프로그램을 작성하세요. 이 프로그램은 첫 번째 인자로 바인딩할 포트를 받습니다.

인자가 주어지지 않으면, 표준 오류(stderr)에 "Wrong number of arguments"를 출력하고 \n을 추가한 후, 상태 코드 1로 종료해야 합니다.

프로그램이 연결을 수락하기 전에 시스템 호출이 오류를 반환하면, 표준 오류(stderr)에 "Fatal error"를 출력하고 \n을 추가한 후, 상태 코드 1로 종료해야 합니다.

메모리를 할당할 수 없을 경우, 표준 오류(stderr)에 "Fatal error"를 출력하고 \n을 추가한 후, 상태 코드 1로 종료해야 합니다.


* 프로그램 요구사항

프로그램은 논블로킹(non-blocking)이어야 하며, 클라이언트가 메시지를 읽지 않아도 연결을 끊어서는 안 됩니다.

프로그램은 #define 전처리기를 포함하지 않아야 합니다.

프로그램은 오직 127.0.0.1에서만 연결을 대기해야 합니다.

수신하는 파일 디스크립터(fd)는 이미 'recv' 또는 'send' 호출 시 'select' 호출 없이 블록되도록 설정되어야 하지만, 그렇지 않으면 블록되지 않아야 합니다.


# 클라이언트가 서버에 연결될 때:

클라이언트는 ID를 부여받습니다.
첫 번째 클라이언트는 ID 0을 받고, 이후의 각 새로운 클라이언트는 마지막 클라이언트 ID + 1을 받습니다.

%d는 이 번호로 대체됩니다.

서버에 연결된 모든 클라이언트에게 다음 메시지가 전송됩니다: "server: client %d just arrived\n"

클라이언트는 프로그램에 메시지를 보낼 수 있어야 합니다.

메시지는 출력 가능한 문자로만 구성되며, 별도의 검사 없이 처리합니다.

하나의 메시지는 여러 개의 \n을 포함할 수 있습니다.
서버가 메시지를 수신하면, 모든 다른 클라이언트에게 "client %d: "를 각 줄 앞에 붙여서 다시 전송해야 합니다.


# 클라이언트가 서버에서 연결 해제될 때:

서버에 연결된 모든 클라이언트에게 다음 메시지가 전송됩니다: "server: client %d just left\n"

메모리 또는 파일 디스크립터(fd) 누수는 허용되지 않습니다.

```

<br>

exam06.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

// 클라이언트를 나타내는 구조체 정의
typedef struct s_client
{
    int     id;                 // 클라이언트 ID
    char    msg[300000];        // 클라이언트가 보낸 메시지 저장 버퍼
}   t_client;

t_client    clients[1024];          // 최대 1024명의 클라이언트를 관리하기 위한 배열
fd_set      read_set, write_set, current;  // fd_set은 소켓의 읽기, 쓰기 및 현재 상태를 추적하는 데 사용
int         maxfd = 0, gid = 0;     // maxfd는 현재 열린 가장 큰 파일 디스크립터, gid는 클라이언트 ID 생성에 사용
char        send_buffer[300000], recv_buffer[300000]; // 송수신 메시지 버퍼

// 오류 메시지를 출력하고 프로그램을 종료하는 함수
void    err(char  *msg)
{
    if (msg)
        write(2, msg, strlen(msg));     // 오류 메시지 출력
    else
        write(2, "Fatal error", 11);    // 기본 오류 메시지
    write(2, "\n", 1);                  // 개행 문자 추가
    exit(1);                            // 상태 코드 1로 프로그램 종료
}

// 모든 클라이언트에게 메시지를 전송하는 함수, except는 제외할 클라이언트의 fd
void    send_to_all(int except)
{
    for (int fd = 0; fd <= maxfd; fd++) // 모든 파일 디스크립터를 확인
    {
        if  (FD_ISSET(fd, &write_set) && fd != except) // 쓰기 가능한 소켓이고 제외할 fd가 아닐 경우
            if (send(fd, send_buffer, strlen(send_buffer), 0) == -1) // 메시지 전송
                err(NULL); // 전송 실패 시 오류 처리
    }
}

int     main(int ac, char **av)
{
    // 서버 프로그램 실행 시 포트 번호를 인자로 받아야 함
    if (ac != 2)
        err("Wrong number of arguments");

    struct sockaddr_in  serveraddr;  // 서버 주소 구조체
    socklen_t           len;
    int serverfd = socket(AF_INET, SOCK_STREAM, 0); // 서버 소켓 생성
    if (serverfd == -1) err(NULL);  // 소켓 생성 실패 시 오류
    maxfd = serverfd; // 현재 가장 큰 fd는 서버 소켓

    // 파일 디스크립터 집합 초기화
    FD_ZERO(&current); // current fd_set 초기화
    FD_SET(serverfd, &current); // 서버 소켓을 current fd_set에 추가
    bzero(clients, sizeof(clients)); // 클라이언트 배열 초기화
    bzero(&serveraddr, sizeof(serveraddr)); // 서버 주소 구조체 초기화

    // 서버 주소 설정
    serveraddr.sin_family = AF_INET; // IPv4 주소 체계 사용
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 인터페이스에서 수신
    serveraddr.sin_port = htons(atoi(av[1])); // 입력 받은 포트 번호로 설정

    // 소켓 바인드 및 연결 대기 설정
    if (bind(serverfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1 || listen(serverfd, 100) == -1)
        err(NULL); // 바인드 또는 연결 대기 실패 시 오류

    while (1)
    {
        read_set = write_set = current; // 현재 fd_set을 읽기 및 쓰기 세트에 복사
        if (select(maxfd + 1, &read_set, &write_set, 0, 0) == -1) continue; // select를 통해 준비된 소켓을 검사

        for (int fd = 0; fd <= maxfd; fd++) // 모든 파일 디스크립터를 순회하며 확인
        {
            if (FD_ISSET(fd, &read_set)) // fd가 읽기 가능한 상태인지 확인
            {
                if (fd == serverfd) // 서버 소켓이면 새 클라이언트 연결 수락
                {
                    int clientfd = accept(serverfd, (struct sockaddr *)&serveraddr, &len); // 새 클라이언트 연결 수락
                    if (clientfd == -1) continue; // 수락 실패 시 계속
                    if (clientfd > maxfd) maxfd = clientfd; // 최대 fd 업데이트
                    clients[clientfd].id = gid++; // 새로운 클라이언트에 ID 할당
                    FD_SET(clientfd, &current); // 클라이언트 소켓을 current fd_set에 추가
                    sprintf(send_buffer, "server: client %d just arrived\n", clients[clientfd].id); // 새로운 클라이언트 도착 메시지 작성
                    send_to_all(clientfd); // 모든 클라이언트에게 전송
                }
                else // 클라이언트 소켓이면 데이터 수신
                {
                    int ret = recv(fd, recv_buffer, sizeof(recv_buffer), 0); // 클라이언트로부터 메시지 수신
                    if (ret <= 0) // 연결 종료 또는 오류 발생 시
                    {
                        sprintf(send_buffer, "server: client %d just left\n", clients[fd].id); // 클라이언트 떠남 메시지 작성
                        send_to_all(fd); // 모든 클라이언트에게 전송
                        FD_CLR(fd, &current); // current fd_set에서 제거
                        close(fd); // 소켓 닫기
                        bzero(clients[fd].msg, strlen(clients[fd].msg)); // 메시지 버퍼 초기화
                    }
                    else // 메시지 수신 성공 시
                    {
                        for (int i = 0, j = strlen(clients[fd].msg); i < ret; i++, j++) // 수신한 데이터를 기존 메시지에 추가
                        {
                            clients[fd].msg[j] = recv_buffer[i];
                            if (clients[fd].msg[j] == '\n') // 줄바꿈 문자를 찾으면
                            {
                                clients[fd].msg[j] = '\0'; // null 문자로 변환하여 문자열 종료
                                sprintf(send_buffer, "client %d: %s\n", clients[fd].id, clients[fd].msg); // 클라이언트 메시지 작성
                                send_to_all(fd); // 모든 클라이언트에게 전송
                                bzero(clients[fd].msg, strlen(clients[fd].msg)); // 메시지 버퍼 초기화
                                j = -1; // 버퍼의 처음부터 다시 시작
                            }
                        }
                    }
                }
                break; // select 루프 탈출
            }
        }
    }
    return (0);
}

```
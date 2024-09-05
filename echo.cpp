#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <string>
#include <map>
#include <vector>

void exitPerror (const std::string& errMsg)
{
    std::cerr << errMsg << std::endl;
    exit(1);
}

void settingServerAddr (sockaddr_in &serverAddr, char* port)
{
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

void newEventRegister (std::vector<struct kevent>& newEvents,
uintptr_t fd, int16_t filter, uint16_t flags,
uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp;
    EV_SET(&temp, fd, filter, flags, fflags, data, udata);
    newEvents.push_back(temp);
}

void checkSocket (struct kevent& currEvent, int server)
{
    if (currEvent.flags & EV_ERROR)
    {
        if (currEvent.ident == (uintptr_t)server)
            exitPerror("client socket Error.");
        else
            exitPerror("client socket Error.");
    }
}

void disconnectClient(int clientFd, std::map<int, std::string>& clients)
{
    std::cout << "client disconnected: " << clientFd << std::endl;
    close(clientFd);  // 클라이언트 소켓 닫기
    clients.erase(clientFd);  // 클라이언트 정보 삭제
}

void newClientRegister (int server, std::vector<struct kevent>& newEvents, std::map<int, std::string>& clients)
{
    int newClientSocket;

    newClientSocket = accept(server, NULL, NULL);
    if (newClientSocket == -1)
        exitPerror("accept failed.");
    std::cout << "accept new client: " << newClientSocket << std::endl;
    fcntl(newClientSocket, F_SETFL, O_NONBLOCK);

    newEventRegister(newEvents, newClientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    newEventRegister(newEvents, newClientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
    
    // std::map<int, std::string> clients에 newClientSocket fd 추가
    clients[newClientSocket] = "";
}

void clientRead (struct kevent& currEvent, std::map<int, std::string>& clients)
{
    if (clients.find(currEvent.ident)!= clients.end())
    {
        char buffer[1024];
        int readSize = read(currEvent.ident, buffer, sizeof(buffer));

        if (readSize == -1)
        {
            std::cerr << "buffer read failed." << std::endl;
            disconnectClient(currEvent.ident, clients);
        }
        else if (readSize == 0)
            disconnectClient(currEvent.ident, clients);
        else
        {
            buffer[readSize] = '\0';  // 문자열의 끝에 NULL 추가
            clients[currEvent.ident] += buffer;  // 클라이언트 데이터에 추가
            std::cout << "client " << currEvent.ident << ": " << clients[currEvent.ident] << std::endl;
        }
    }
}


void readEventHandler(struct kevent& currEvent, int server, std::vector<struct kevent>& newEvents, std::map<int, std::string>& clients)
{
    if (currEvent.ident == (uintptr_t)server)
        newClientRegister(server, newEvents, clients);
    else
        clientRead(currEvent, clients); // 함수명 다시 생각해보기
}

void writeClinetSocket(struct kevent& currEvent, std::map<int, std::string>& clients)
{
     /* 클라이언트로 데이터 전송 */
    std::map<int, std::string>::iterator it = clients.find(currEvent.ident);
    if (it != clients.end())
    {
        if (clients[currEvent.ident] != "") // 버퍼에 쓸 문자열이 있다면
        {
            int writeSize;
            //ssize_t write(int fd, const void *buf, size_t count); 쓸 fd, 쓸 버퍼 포인터, 얼만큼 쓸건지 사이즈
            //c_str(): std::string을 C 스타일 문자열(const char *)로 변환하여 반환함.
            writeSize = write(currEvent.ident, clients[currEvent.ident].c_str(), clients[currEvent.ident].size());
            if (writeSize == -1)
            {
                std::cerr << "buffer write failed." << std::endl;
                disconnectClient(currEvent.ident, clients);  
            }
        else
            clients[currEvent.ident].clear();  // 전송 후 데이터 초기화. 키 값은 그대로 남고 값을 지운다. 즉 문자열 지움.
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc > 3)
    {
        std::cerr << "invaild input." << std::endl;
        return (1);
    }

    int server;
    struct sockaddr_in serverAddr; // <arpa/inet.h>

    if ((server = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        exitPerror("socket open failed."); // socket open
    memset(&serverAddr, 0, sizeof(serverAddr)); // socket 초기화
    settingServerAddr(serverAddr, argv[1]); // server addr init

    if (bind(server, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
        exitPerror("socket bind failed.");
    
    if (listen(server, 10) == -1) // listen backlog 개수 상의하기
        exitPerror("listen failed.");
    fcntl(server, F_SETFL, O_NONBLOCK); //socket fd nonblock setting

    int kq;
    if ((kq = kqueue()) == -1)
        exitPerror("kqueue failed.");

    std::map<int, std::string> clients; //int fd, std::string buffer
    std::vector<struct kevent> newEvents;
    struct kevent waitEvents[10];
    
    //fd, filter, flags, fflags, data, udata
    // flags (EV_ADD | EV_ENABLE)
    /*
        플래그는 특정 비트 위치를 갖고 있으며
        | 연산자를 사용하여 두 플래그의 비트를 모두 '1'로 설정하는 작업을 수행함.
    */
   // 서버 소켓을 kqueue가 감시할 수 있게 이벤트에 등록해줌 
    newEventRegister(newEvents, server, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    int eventsCount;
    struct kevent* currEvent;

    while (true)
    {
        //kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout
        eventsCount = kevent(kq, &newEvents[0], newEvents.size(), waitEvents, 10, NULL);
        if (eventsCount == -1)
            exitPerror("kevent failed.");
        newEvents.clear(); // kqueue에 등록한 이벤트들 지워주기 (벡터의 모든 요소를 지우고 size 0으로 만든다)

        for (int i = 0; i < eventsCount; i++)
        {
            currEvent = &waitEvents[i]; // kqueue가 감지한 이벤트들 하나씩 처리하기
            checkSocket(*currEvent, server); // socket check

            if (currEvent->filter == EVFILT_READ)
                readEventHandler(*currEvent, server, newEvents, clients);
            else if (currEvent->filter == EVFILT_WRITE) // 클라이언트 소켓이 쓰기 이벤트를 받을 준비가 되어있다 == 클라이언트의 쓰기 버퍼가 비어져있는 상태이다
                writeClinetSocket(*currEvent, clients);
        }
    }

    return (0);
}

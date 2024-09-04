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

int main(int argc, char* argv[])
{
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
    fcntl(server, F_SETFL, O_NONBLOCK);

    int kq;
    if ((kq = kqueue()) == -1)
        exitPerror("kqueue failed.");


    return (0);
}
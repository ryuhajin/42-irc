#ifndef SERVER_HPP
#define SERVER_HPP

#define EVENT_MAX 10
#define BACKLOG 10
#define BUFFER_SIZE 1024

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

#include "client.hpp"
#include "channel.hpp"
#include "parser.hpp"
#include "./cmd/cmd.hpp"

class Client;
class Channel;
class Server
{
private:
    struct sockaddr_in _serverAddr;
    socklen_t _addrSize; 
    std::vector<struct kevent> _newEvents;

    std::string _name;
    std::string _password;
    std::string _ip;

    int _fd;
    int _port;
    int _kq;

    std::map<int, Client*> _clients;
    std::map<std::string, Channel*> _channels;

    Server();
public:
    Server(int port, const std::string& pass);
    ~Server();

    //utils
    void exitPerror(const std::string& errMsg);
    void settingServerAddr(sockaddr_in &serverAddr, int port);
    void clearClients();
    void clearChannels();
    void disconnectAll (void);

    //get fuction
    const std::string getName() const; 
    const std::string getPassword() const; 
    const std::string getIp() const;
    int getFd() const;
    int getKq() const;
    std::vector<struct kevent>& getNewEvents();
    std::map<int, Client*>& getClients();
    Client* findClient(int fd);
    Client* findNick(const std::string& nick);
    std::map<std::string, Channel*>& getChannels();
    Channel* findChannel(std::string channelName);

    void addChannel(std::string channelName);
    void delChannel(std::string channelName);
    bool isChannelExists(std::string channelName);

    void newEventRegister(std::vector<struct kevent>& newEvents, uintptr_t fd, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
    void disableWriteEvent(std::vector<struct kevent>& newEvents, int fd);
    void enableWriteEvent(std::vector<struct kevent>& newEvents, int fd);
   
    void disconnectClient(int clientFd, std::map<int, Client*>& clients, const std::string& msg);
    void disconnectHandler(struct kevent& currEvent);
    void newClientRegister(int serverFd);
    void clientRequest(struct kevent& currEvent, Server& server, std::map<int, Client*>& clients);
    void readEventHandler(struct kevent& currEvent);
    void writeClinetSocket(struct kevent& currEvent, std::map<int, Client*>& clients);

    void selfSend(Server& server, Client& self, const std::string& msg);
    void otherSend(Server& server, Client& other, const std::string& msg);
    void broadcast(Server& server, Client& client, Channel& channel, const std::string& msg, bool includeSelf);
};


#endif
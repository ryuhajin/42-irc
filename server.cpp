#include "server.hpp"

Server::Server(int portNum, const std::string& pass)
{
    _name = "IRCsejong";
    _port = portNum;
    _password = pass;
    
    if ((_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        exitPerror("Error : socket open failed.");
    memset(&_serverAddr, 0, sizeof(_serverAddr));
    settingServerAddr(_serverAddr, _port);

    if (bind(_fd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) == -1)
        exitPerror("Error : socket bind failed.");
    
    if (listen(_fd, BACKLOG) == -1)
        exitPerror("Error : listen failed.");
    fcntl(_fd, F_SETFL, O_NONBLOCK);
    
    _addrSize = sizeof(_serverAddr);
    if (getsockname(_fd, (struct sockaddr*)&_serverAddr, &_addrSize) == -1)
    {
        close(_fd);
        exitPerror("Error : get sock name failed.");
    }

    std::string temp = inet_ntoa(_serverAddr.sin_addr);
    _ip = temp;

    if ((_kq = kqueue()) == -1)
        exitPerror("Error : kqueue failed.");

    newEventRegister(_newEvents, _fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
}

Server::~Server()
{
    this->disconnectAll();
    close(_fd);
    std::cout << "[destructor] server closed" << std::endl; 
}

void Server::exitPerror (const std::string& errMsg)
{
    std::cerr << errMsg << std::endl;
    exit(1);
}

void Server::settingServerAddr (sockaddr_in &serverAddr, int port)
{
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

const std::string Server::getIp() const
{
    return (_ip);
}

const std::string Server::getName() const
{
    return (_name);
}

const std::string Server::getPassword() const
{
    return (_password);
}
    
int Server::getFd() const
{
    return (_fd);
}

int Server::getKq() const
{
    return (_kq);
}

std::vector<struct kevent>& Server::getNewEvents()
{
    return (_newEvents);
}

std::map<int, Client*>& Server::getClients()
{
    return (_clients);
}

Client* Server::findClient(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end())
        return (it->second);
    return (NULL);
}

Client* Server::findNick(const std::string& nick)
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if(it->second->getNick() == nick)
            return (it->second);
    }
    return (NULL);
}

std::map<std::string, Channel*>& Server::getChannels()
{
    return (_channels);
}

Channel* Server::findChannel(std::string channelName)
{
    std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
    if (it != _channels.end())
        return (it->second);
    return (NULL);
}

void Server::addChannel(std::string channelName)
{
    bool hasChannel = isChannelExists(channelName);
    if (!hasChannel)
    {
        Channel* newChannel = new Channel(channelName);
        this->getChannels().insert(std::make_pair(channelName, newChannel));
        return ;
    }
}

void Server::delChannel(std::string channelName)
{
    Channel* channel = findChannel(channelName);
    if (channel)
    {
        if (channel->hasUsers() == true)
        {
            return ;
        }
        std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
        if (it != _channels.end())
        {
            delete it->second;
            _channels.erase(it);
        }
        return ;
    }
}

bool Server::isChannelExists(std::string channelName)
{
    if (findChannel(channelName) == NULL)
        return (false);
    return (true);
}

void Server::newEventRegister (std::vector<struct kevent>& newEvents,
uintptr_t fd, int16_t filter, uint16_t flags,
uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp;
    EV_SET(&temp, fd, filter, flags, fflags, data, udata);
    newEvents.push_back(temp);
}

void Server::disableWriteEvent(std::vector<struct kevent>& newEvents, int fd)
{
    struct kevent writeEvent;
    EV_SET(&writeEvent, fd, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
    newEvents.push_back(writeEvent);
}

void Server::enableWriteEvent(std::vector<struct kevent>& newEvents, int fd)
{
    struct kevent writeEvent;
    EV_SET(&writeEvent, fd, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
    newEvents.push_back(writeEvent);
}

void Server::selfSend(Server& server, Client& self, const std::string& msg)
{
    self.appendMsg(msg);
    server.enableWriteEvent(server.getNewEvents(), self.getFd());
}

void Server::otherSend(Server& server, Client& other, const std::string& msg)
{
    other.appendMsg(msg);
    server.enableWriteEvent(server.getNewEvents(), other.getFd());
}

void Server::broadcast(Server& server, Client& client, Channel& channel, const std::string& msg, bool includeSelf)
{
    if (includeSelf)
    {
        client.appendMsg(msg);
        server.enableWriteEvent(server.getNewEvents(), client.getFd());
    }

    std::vector<Client*> users = channel.getUsers();
    for (std::vector<Client*>::iterator it = users.begin(); it != users.end(); it++)
    {
        if ((*it)->getFd() == client.getFd())
            continue;
        (*it)->appendMsg(msg);
        server.enableWriteEvent(server.getNewEvents(), (*it)->getFd());
    }
}

void Server::clearClients()
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        Client& client = *it->second;
        this->selfSend(*this, client, Cmd::successMsg(Cmd::prefix(client.getNick(), client.getUserName(), client.getHost()),\
         "QUIT", "", "server closed"));
        send(client.getFd(), client.getMsg().c_str(), client.getMsg().size(), 0);
    
        if (client.getChannels().size() > 0)
        {
            std::vector<Channel*> channels = client.getChannels();
            for (size_t i = 0; i < channels.size(); i++)
            {
                client.quitChannel(channels[i]);
                if (channels[i]->isOp(client))
                    channels[i]->delOp(&client);
                channels[i]->delUser(&client);
            }
        }
        delete it->second;
    }
    _clients.clear();
}

void Server::clearChannels()
{
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        delete it->second;
    }
    _channels.clear();
}

void Server::disconnectAll (void)
{
    this->clearClients();
    this->clearChannels();
}

void Server::disconnectClient(int clientFd, std::map<int, Client*>& clients, const std::string& msg)
{
    std::map<int, Client*>::iterator it = clients.find(clientFd);

    if (it != clients.end())
    {
        Client& client = *it->second;
        std::vector<Channel*> channels = client.getChannels();
    
        if (channels.size() > 0)
        {
            std::string pre = Cmd::prefix(client.getNick(), client.getUserName(), client.getHost());
            for (size_t i = 0; i < channels.size(); i++)
            {
                client.quitChannel(channels[i]);
                if (channels[i]->isOp(client))
                    channels[i]->delOp(&client);
                channels[i]->delUser(&client);
                if (channels[i]->getUsers().empty())
                {
                    this->delChannel(channels[i]->getName());
                    continue ;
                }
                this->broadcast(*this, client, *channels[i], Cmd::successMsg(pre, "QUIT", "", msg), false);
            }
        }
        delete it->second;
        clients.erase(it);
    }
}

void Server::disconnectHandler (struct kevent& currEvent)
{
    if (currEvent.ident == (uintptr_t)this->getFd())
        exitPerror("server socket Error.");
    else
        disconnectClient(currEvent.ident, this->getClients(), " :leaving");
}

void Server::newClientRegister (int serverFd)
{
    std::cout << "[Server] new Client register" << std::endl;
    struct sockaddr_in newClientAdrr;
    socklen_t clientAddrSize = sizeof(newClientAdrr);
    
    int newClientFd = accept(serverFd, (struct sockaddr *)&newClientAdrr, &clientAddrSize);
    if (newClientFd == -1)
        return ;
    fcntl(newClientFd, F_SETFL, O_NONBLOCK);

    if (getsockname(newClientFd, (struct sockaddr*)&newClientAdrr, &clientAddrSize) == -1)
    {
        close(newClientFd);
        return ;
    }

    std::string host = inet_ntoa(newClientAdrr.sin_addr);
    
    newEventRegister(this->getNewEvents(), newClientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    newEventRegister(this->getNewEvents(), newClientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
    
    disableWriteEvent(this->getNewEvents(), newClientFd);

    Client* newClient = new Client(newClientFd, host);
    this->getClients().insert(std::make_pair(newClientFd, newClient));
}

std::vector<std::string> splitRequest (std::string request)
{
    std::vector<std::string> result;
    std::string delimiter = "\r\n";
    size_t start = 0;
    size_t end = request.find(delimiter);

    while (end != std::string::npos) {
        end += delimiter.size();
        std::string msg = request.substr(start, end - start) + "\0";
        result.push_back(msg);

        start = end;
        end = request.find(delimiter, start);
    }
    return (result);
}


void Server::clientRequest (struct kevent& currEvent, Server& server, std::map<int, Client*>& clients)
{
    char buff[BUFFER_SIZE];
    std::map<int, Client*>::iterator it = clients.find(currEvent.ident);
    if (it == clients.end())
        return ;

    Client& client = *(it->second);

    ssize_t readSize = recv(currEvent.ident, buff, BUFFER_SIZE - 1, 0);
    if (readSize == -1)
    {
        std::cerr << "Error : buffer read failed." << std::endl;
        disconnectClient(currEvent.ident, clients, " :leaving");
        return ;
    }

    buff[readSize] = '\0';
    std::string str = client.getBuffer() + std::string(buff);
    std::size_t lastCRLF = str.rfind("\r\n");
    if (lastCRLF != str.npos)
    {
        if (lastCRLF + 2 == str.length())
            client.setBuffer("");
        else
            client.setBuffer(str.substr(lastCRLF + 2));
    }
    else
    {
        client.setBuffer(str);
        return;
    }
    std::vector<std::string> request = splitRequest(str);

    for (size_t i = 0; i < request.size(); i++)
    {
        request::msg* msg = request::parser(request[i]);
        Cmd::execute(msg, server, client.getFd());
        delete msg;
    }
}

void Server::readEventHandler(struct kevent& currEvent)
{
    if (currEvent.ident == (uintptr_t)this->getFd())
        newClientRegister(this->getFd());
    else
        clientRequest(currEvent, *this, this->getClients());
}

void Server::writeClinetSocket(struct kevent& currEvent, std::map<int, Client*>& clients)
{
    std::map<int, Client*>::iterator it = clients.find(currEvent.ident);
    if (it != clients.end())
    {
        int clientFd = it->first;
        Client &client = *(it->second);
        ssize_t msgSize = client.getMsg().size();
        ssize_t writeSize = send(clientFd, client.getMsg().c_str(), msgSize, 0);
        if (writeSize == -1)
        {
            std::cerr << "Error : buffer write failed." << std::endl;
            this->disconnectClient(currEvent.ident, clients, " :leaving");
        }
        else
        {
            if (writeSize == msgSize)
            {
                client.clearSendMsg();
                this->disableWriteEvent(this->getNewEvents(), clientFd);
            }
            else
            {
                std::string rest = client.getMsg().substr(writeSize);
                client.setMsg(rest);
            }
        }
    }
}

#include "client.hpp"

Client::Client(int fd)
    :_fd(fd)
{
}

Client::Client(int fd, std::string host)
    :_fd(fd), _host(host)
{
    std::cout << "[constructor] Client fd: " << _fd << std::endl;
}

Client::~Client()
{
    close(_fd);
    std::cout << "[destructor] Client fd: " << _fd << std::endl;
}

void Client::setHost(const std::string& ip)
{
    _host = ip;
    _host = ip;
    
}

void Client::setPass(const std::string& pass)
{
    _pass = pass;
}
void Client::setUserName(const std::string& userName)
{
    _userName = userName;
}
void Client::setNick(const std::string& nick)
{
    _nick = nick;
}

void Client::appendMsg(const std::string& msg)
{
    _sendMsg += msg;
}

void Client::setMsg(const std::string& msg)
{
    _sendMsg = msg;
}

void Client::setBuffer(const std::string& str)
{
    _buffer = str;
}


const std::string Client::getBuffer() const
{   
    return (_buffer);
}

int Client::getFd() const
{
    return (_fd);
}

const std::string Client::getPass() const
{
    return (_pass);
}

const std::string Client::getUserName() const
{
    return (_userName);
}

const std::string Client::getNick() const
{
    return (_nick);
}

const std::string Client::getMsg() const
{
    return (_sendMsg);
}

const std::string Client::getHost() const
{
    return (_host);
}

void Client::clearSendMsg()
{
    this->_sendMsg.clear();
}

std::vector<Channel*>& Client::getChannels()
{
    return (_channels);
}

void Client::joinChannel(Channel* channel)
{
    _channels.push_back(channel);
}

void Client::quitChannel(Channel* channel)
{
    std::vector<Channel*>::iterator it = std::find(_channels.begin(), _channels.end(), channel);
    if (it != _channels.end())
        _channels.erase(it);
}
Channel* Client::findChannel(const std::string& channelName)
{
    for ( std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if ((*it)->getName() == channelName)
            return (*it);
    }
    return (NULL);
}

#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

#include "channel.hpp"

class Channel;
class Client
{
private:
    int _fd;
    std::string _pass;
    std::string _userName;
    std::string _nick;
    std::string _host; // IP
    std::string _buffer;
    std::vector<Channel*> _channels;
    std::string _sendMsg;

public:
    Client(int fd);
    Client(int fd, std::string host);
    ~Client();

    void setHost(const std::string& ip);
    void setPass(const std::string& pass);
    void setUserName(const std::string& userName);
    void setNick(const std::string& nick);
    void appendMsg(const std::string& msg);
    void setMsg(const std::string& msg);
    void setBuffer(const std::string& str);

    int getFd() const;
    const std::string getBuffer()const;
    const std::string getPass() const;
    const std::string getUserName() const;
    const std::string getNick() const;
    const std::string getMsg() const;
    const std::string getHost() const;
    std::vector<Channel*>& getChannels();

    void joinChannel(Channel* channel);
    void quitChannel(Channel* channel);
    Channel* findChannel(const std::string& channelName);

    void clearSendMsg();
};

#endif
#ifndef CMD_HPP
#define CMD_HPP

#include <iostream>
#include <string>
#include "../parser.hpp"
#include "../server.hpp"
#include "../client.hpp"
#include "../channel.hpp"

class Server;
class Client;
class Channel;

#define COMMAND_COUNT 12
#define NICK_LENGTH 12
#define CHANNEL_LENGTH 30

enum cmdNum {
    PASS = 0,
    NICK,
    USER,
    JOIN,
    PRIVMSG,
    MODE,
    KICK,
    QUIT,
    TOPIC,
    INVITE,
    PING,
    PART
};

namespace Cmd
{
    bool IsRegister(Client* client);
    bool IsVaildChar(char c);
    bool IsSpecial(char c);
    bool IsOp (Channel* channel, Client* client);
    bool IsUserMode(const std::string& param);
    bool IsVaildlimit (std::string params);
    bool IsVaildChannelName(const std::string& channelName);
    bool IsVaildNick(const std::string& nick);
    bool IsValidParams(request::msg*msg, Server& server, Client* client);

    void parseComma(const std::string &param, std::vector<std::string> &params);

    std::string prefix(const std::string& nick, const std::string& username, const std::string& host);
    std::string errorMsg(const std::string& prefix, const std::string& cmd, const std::string& target, const std::string& params);
    std::string successMsg(const std::string& prefix, const std::string& cmd, const std::string& target, const std::string& params);
    std::string failedMsg(const std::string& prefix, const std::string& cmd, const std::string& target, const std::string& option, const std::string& params);
    
    std::string modeFailedMsg(const std::string& prefix, const std::string& cmd, const std::string& target, const std::string& option, const std::string& params);
    std::string modeSuccessMsg(const std::string& prefix, const std::string& channel, const std::string& option, const std::string& params);

    void informChannelMode(Server& server, Channel& channel, Client& client);
    void sendUserList (Server& server, Client& client, Channel& channel);
    void sendEndList(Server& server, Client& client, Channel& channel);
    void sendTopic(Server& server, Client& client, Channel& channel);
    void isKey(Server& server, Client& client, Channel& channel);
    void isInvite(Server& server, Client& client, Channel& channel, const std::string& key);
    void channelJoin (Server& server, Channel& channel, Client& client);

    void firstNickRegi (Server& server, Client& Client, const std::string& nick);
    std::string addUniqueNumber (Server& server, const std::string& nick);
    
    void helloBot (Server& server, Client& client, Channel& channel);
    void toChannel (Server& server, Client& client, const std::string& channelName, const std::string& msg);
    void toUser (Server& server, Client& client, const std::string& user, const std::string& msg);

    int findCmdNum(request::msg* msg);

    std::string validatePartInput( Server& server, Client& client, request::msg* msg);

    void execute(request::msg* msg, Server& server, int clientFd);
    void pass (request::msg* msg, Server& server, int clientFd);
    void nick (request::msg* msg, Server& server, int clientFd);
    void user (request::msg* msg, Server& server, int clientFd);
    void join (request::msg* msg, Server& server, int clientFd);
    void part (request::msg* msg, Server& server, int clientFd);
    void mode(request::msg*msg, Server& server, int clientFd);
    void kick (request::msg* msg, Server& server, int clientFd);
    void quit (request::msg* msg, Server& server, int clientFd);
    void topic (request::msg* msg, Server& server, int clientFd);
    void invite (request::msg* msg, Server& server, int clientFd);
    void privmsg (request::msg* msg, Server& server, int clientFd);
    void ping (request::msg* msg, Server& server, int clientFd);

    bool invite (Channel* channel, bool flag);
    bool topic (Channel* channel, bool flag);
    bool key (Channel* channel, bool flag, const std::string& pass);
    bool op (Channel* channel, bool flag, Client* user);
    bool limit (Channel* channel, bool flag, const std::string& params);

    void unknown(request::msg* msg, Server& server, int clientFd);
};

#endif
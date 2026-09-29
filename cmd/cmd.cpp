#include "cmd.hpp"

bool Cmd::IsRegister(Client* client)
{
    if (client->getPass().size() == 0)
        return (false);
    else if (client->getNick().size() == 0)
        return (false);
    else if (client->getUserName().size() == 0)
        return (false);
    return (true);
}

bool Cmd::IsVaildChar(char c)
{
    if (isdigit(c) || isalpha(c))
        return (true);
    return (false);
}

bool Cmd::IsSpecial(char c)
{
    static const std::string special = "-[]\\`^{}";

    for (size_t i = 0; i < special.size(); i++)
    {
        if (c == special[i])
            return (true);
    }
    return (false);
}

bool Cmd::IsVaildChannelName(const std::string& channelName)
{
    if (channelName.size() > CHANNEL_LENGTH)
        return (false);

    if (channelName[0] == '#' || channelName[0] == '&')
    {
        for (size_t i = 1; i < channelName.size(); i++)
        {
            char c = channelName[i];
            if (c == ' ' || c == '\x07' || c == '\x00' || c == '\r' || c == '\n' || c == ',')
            return (false);
        }
        return (true);
    }
    return (false);
}

bool Cmd::IsVaildNick(const std::string& nick)
{
    if (nick.size() > NICK_LENGTH)
        return (false);
    
    if (!isalpha(nick[0]))
    {
        return (false);
    }

    for (size_t i = 1; i < nick.size(); i++)
    {
        if (!(IsVaildChar(nick[i]) || IsSpecial(nick[i])))
            return (false); //not char not special
    }
    return (true);
}

void Cmd::parseComma(const std::string &param, std::vector<std::string> &params)
{
    std::istringstream iss(param);
    std::string Token;
    while (std::getline(iss, Token, ','))
    {
        params.push_back(Token);
    }
}


std::string Cmd::prefix(const std::string& nick, const std::string& username, const std::string& host)
{
   std::string prefix = nick + "!" + username + "@" + host;
   return (prefix);
}

std::string Cmd::errorMsg(const std::string& prefix, const std::string& cmd, const std::string& target, const std::string& params)
{
    std::string msg = ":" + prefix + " " + cmd; 
    if (!target.empty())
        msg += " " + target;
    msg += " " + params + "\r\n";
   return (msg);
}

std::string Cmd::successMsg(const std::string& prefix, const std::string& cmd, const std::string& target, const std::string& params)
{
    std::string msg = ":" + prefix + " " + cmd; 
    if (!target.empty())
        msg += " " + target;
    msg += " " + params + "\r\n";

   return (msg);
}

std::string Cmd::failedMsg(const std::string& prefix, const std::string& cmd, const std::string& target, const std::string& option, const std::string& params)
{
    std::string msg = ":" + prefix + " " + cmd + " " + target + " " + option + params + "\r\n";
    return (msg);
}

std::string Cmd::modeSuccessMsg(const std::string& prefix, const std::string& channel, const std::string& option, const std::string& params)
{
    std::string msg = ":" + prefix + " " + "MODE" + " " + channel + " " + option + " " + params + "\r\n";
    return (msg);
}

int Cmd::findCmdNum(request::msg* msg)
{
    static const std::string commands[COMMAND_COUNT] =
    {"PASS", "NICK", "USER", "JOIN", "PRIVMSG", "MODE", "KICK", "QUIT", "TOPIC", "INVITE", "PING", "PART"};

    for (int i = 0; i < COMMAND_COUNT; i++)
    {
        if (msg->cmd == commands[i])
            return (i);
    }
    return (-1);
}

void Cmd::unknown(request::msg* msg, Server& server, int clientFd)
{
    Client* client = server.findClient(clientFd);
    std::string params = msg->cmd + " :Unknown command";

    server.selfSend(server, *client, errorMsg(server.getName(), "421", client->getNick(), params));
    return ;
}

void Cmd::execute(request::msg* msg, Server& server, int clientFd)
{
    int flag = Cmd::findCmdNum(msg);

    switch (flag)
    {
        case PASS:
            pass(msg, server, clientFd);
            break;
        case NICK:
            nick(msg, server, clientFd);
            break;
        case USER:
            user(msg, server, clientFd);
            break;
        case JOIN:
            join(msg, server, clientFd);
            break;
        case PRIVMSG:
            privmsg(msg, server, clientFd);
            break;
        case MODE:
            mode(msg, server, clientFd);
            break;
        case KICK:
            kick(msg, server, clientFd);
            break;
        case QUIT:
            quit(msg, server, clientFd);
            break;
        case TOPIC:
            topic(msg, server, clientFd);
            break;
        case INVITE:
            invite(msg, server, clientFd);
            break;
        case PING:
            ping(msg, server, clientFd);
            break;
        case PART:
            part(msg, server, clientFd);
            break;
        default:
            unknown(msg, server, clientFd);
            break;
    }
}

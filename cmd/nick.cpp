#include "cmd.hpp"

std::string Cmd::addUniqueNumber (Server& server, const std::string& nick)
{
    int num = 0;
    std::string newNick;

    while (true)
    {
        std::stringstream ss;
        ss << nick << num;
        newNick = ss.str();
        if (server.findNick(newNick))
            num++;
        else
            break;
    }

    return (newNick);
}

void Cmd::firstNickRegi (Server& server, Client& client, const std::string& nick)
{
    if (server.findNick(nick))
    {
        std::string UniqueNick = addUniqueNumber(server, nick);
        client.setNick(UniqueNick);
        return ;
    }
    client.setNick(nick);
}

void Cmd::nick (request::msg* msg, Server& server, int clientFd)
{
    Client& client = *server.findClient(clientFd);
    std::string target;
    std::string params;

    if (client.getNick().empty())
        target = "*";
    else
        target = client.getNick();

    if (msg->params.empty())
    {
        params = msg->cmd + " :Not enough parameters";
        server.selfSend(server, client, errorMsg(server.getName(), "461", target, params));
        return ;
    }
    else if (!IsVaildNick(msg->params[0]))
    {
        params = msg->params[0] + " :Erroneous nickname";
        server.selfSend(server, client, errorMsg(server.getName(), "432", target, params));
        return ;
    }

    if (client.getPass().empty())
        return ;

    if (client.getNick().empty())
    {
        firstNickRegi(server, client, msg->params[0]);
        if (IsRegister(&client))
        {
            params = ":Welcome to the Internet Relay Network " + prefix(client.getNick(), client.getUserName(), client.getHost());
            server.selfSend(server, client, successMsg(server.getName(), "001", client.getNick(), params));
        }

        return ;
    }

    if (server.findNick(msg->params[0]))
    {
        params = msg->params[0] + " :Nickname is already in use";
        server.selfSend(server, client, errorMsg(server.getName(), "433", client.getNick(), params));
        return ;
    }
    else
    {
        std::string oldNick = client.getNick();
        client.setNick(msg->params[0]);

        std::string pre = prefix(oldNick, client.getUserName(), client.getHost());
        params = ":" + client.getNick();
        
        server.selfSend(server, client, successMsg(pre, "NICK", "", params));
    
        std::vector<Channel*> channels = client.getChannels();
        for (size_t i = 0; i < channels.size(); i++)
        {
            server.broadcast(server, client, *channels[i], successMsg(pre, "NICK", "", params), false);
        }
        return ;
    }

    return ;
}

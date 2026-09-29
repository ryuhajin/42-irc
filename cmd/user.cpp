#include "cmd.hpp"

bool IsVaildUserParams (request::msg* msg)
{
    if (msg->params.size() == 3 && !msg->trailing.empty())
        return (true);
    else if (msg->params.size() == 4 && msg->trailing.empty())
        return (true);
    return (false);
}


void Cmd::user (request::msg* msg, Server& server, int clientFd)
{
    Client& client = *server.findClient(clientFd);
    std::string target;
    std::string params;
    
    if (client.getNick().empty())
        target = "*";
    else
        target = client.getNick();

    if (!IsVaildUserParams(msg))
    {
        params = msg->cmd + " :Not enough parameters";
        server.selfSend(server, client, errorMsg(server.getName(), "461", target, params));
        return ;
    }

    if (client.getPass().empty())
        return ;

    if (client.getUserName().empty())
    {
        client.setUserName(msg->params[0]);
        if (IsRegister(&client))
        {
            params = ":Welcome to the Internet Relay Network " + prefix(client.getNick(), client.getUserName(), client.getHost());
            server.selfSend(server, client, successMsg(server.getName(), "001", client.getNick(), params));
        }
    }
    else
    {
        params = ":You may not reregister";
        server.selfSend(server, client, errorMsg(server.getName(), "462", target, params));
    }

    return ;
}

#include "cmd.hpp"

void Cmd::pass (request::msg* msg, Server& server, int clientFd)
{
    Client& client = *server.findClient(clientFd);
    std::string target;
    std::string params;

    if (client.getNick().empty())
        target = "*";
    else
        target = client.getNick();

    if (msg->params.size() == 0)
    {
        params = msg->cmd + " :Not enough parameters";
        server.selfSend(server, client, errorMsg(server.getName(), "461", target, params));
        return ;
    }

    if (client.getPass().empty())
    {
        if (msg->params[0] != server.getPassword())
        {
            params = ":Password incorrect";
            server.selfSend(server, client, errorMsg(server.getName(), "464", target, params));
            return ;
        }
        client.setPass(msg->params[0]);
    }
    else 
    {
        params = ":You may not reregister";
        server.selfSend(server, client, errorMsg(server.getName(), "462", target, params));
    }

    return ;
}

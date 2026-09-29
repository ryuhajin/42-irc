#include "cmd.hpp"

void Cmd::quit (request::msg* msg, Server& server, int clientFd)
{
    Client& client = *server.findClient(clientFd);
    std::string params;

    if (msg->params.size() == 0)
    {
        server.disconnectClient(client.getFd(), server.getClients(), " :leaving");
    }
    else
    {
        for (size_t i = 0; i < msg->params.size(); i++)
        {
            params += msg->params[i] + " ";
        }
        if (msg->trailing.size() > 0)
            params += msg->trailing;
        server.disconnectClient(client.getFd(), server.getClients(), params);
    }

    return ;
}

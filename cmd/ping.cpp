#include "cmd.hpp"

void Cmd::ping (request::msg* msg, Server& server, int clientFd)
{
    Client& client = *server.findClient(clientFd);
    std::string params;

    if (msg->params.size() == 0)
    {
        params = msg->cmd + " :Not enough parameters" + "\r\n";
        server.selfSend(server, client, errorMsg(server.getName(), "461", client.getNick(), params));
        return ;
    }
    else if (!IsRegister(&client))
	{
        params = msg->cmd + " :You have not registered" + "\r\n";
        server.selfSend(server, client, errorMsg(server.getName(), "451", client.getNick(), params));
		return ;
	}

    params = ":" + msg->params[0]; 
    server.selfSend(server, client, successMsg(server.getName(), "PONG", server.getName(), params));
    return ;

}
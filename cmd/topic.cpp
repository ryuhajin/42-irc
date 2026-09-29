#include "cmd.hpp"

void Cmd::topic (request::msg* msg, Server& server, int clientFd)
{
    Client& client = *server.findClient(clientFd);
    std::string params;

    if (!IsRegister(&client))
	{
        std::string target;
        if (client.getNick().empty())
            target = "*";
        else
            target = client.getNick();
        params = msg->cmd + " :You have not registered";
        server.selfSend(server, client, errorMsg(server.getName(), "451", target, params));
		return ;
	}

    if (msg->params.size() == 0)
    {
        params = msg->cmd + " :Not enough parameters";
        server.selfSend(server, client, errorMsg(server.getName(), "461", client.getNick(), params));
        return ;
    }

    Channel* channel = server.findChannel(msg->params[0]);
    if (channel == NULL)
    {
        params = msg->params[0] + " :No such channel";
        server.selfSend(server, client, errorMsg(server.getName(), "403", client.getNick(), params));
        return ;
    }
    else if (!channel->findUser(client.getFd()))
    {
        params = msg->params[0] + " :You're not on that channel";
        server.selfSend(server, client, errorMsg(server.getName(), "442", client.getNick(), params));
        return ;
    }

    if (msg->trailing.empty())
    {
        if (channel->getTopic().empty())
        {
            params = channel->getName() + " :No topic is set";
            server.selfSend(server, client, errorMsg(server.getName(), "331", client.getNick(), params));
            return ;
        }
        // :server_name 332 <nickname> <#channel> :<topic>
        params = channel->getName() + " " + channel->getTopic();
        server.selfSend(server, client, errorMsg(server.getName(), "332", client.getNick(), params));
            return ;
    }

    if (channel->getOpTopic())
    {
        if (!channel->isOp(client))
        {
            params = msg->params[0] + " :You're not channel operator";
            server.selfSend(server, client, errorMsg(server.getName(), "482", client.getNick(), params));
            return ;
        } 
        channel->setTopic(msg->trailing);

        server.broadcast(server, client, *channel, successMsg(prefix(client.getNick(), client.getUserName(), client.getHost())\
        , "TOPIC", channel->getName(), msg->trailing), true);
        return ;
    }
    else
    {
        channel->setTopic(msg->trailing);

        server.broadcast(server, client, *channel, successMsg(prefix(client.getNick(), client.getUserName(), client.getHost())\
            , "TOPIC", channel->getName(), msg->trailing), true);
    }

    return ;
}

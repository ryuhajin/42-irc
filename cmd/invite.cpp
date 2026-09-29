#include "cmd.hpp"

void Cmd::invite (request::msg* msg, Server& server, int clientFd)
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

    if (msg->params.size() < 2)
    {
        params = msg->cmd + " :Not enough parameters";
        server.selfSend(server, client, errorMsg(server.getName(), "461", client.getNick(), params));
        return ;
    }

    Client* inviteUser = server.findNick(msg->params[0]);
    if (inviteUser == NULL)
    {   
        params = msg->params[0] +  " :No such nick";
        server.selfSend(server, client, errorMsg(server.getName(), "401", client.getNick(), params));
        return ;
    }

    Channel* channel = server.findChannel(msg->params[1]);

    if (channel == NULL)
    {
        params = msg->params[1] + " :No such channel";
        server.selfSend(server, client, errorMsg(server.getName(), "403", client.getNick(), params));
        return ;
    }
    else if (!channel->findUser(client.getFd()))
    {
        params = msg->params[1] + " :You're not on that channel";
        server.selfSend(server, client, errorMsg(server.getName(), "442", client.getNick(), params));
        return ;
    }
    else if (channel->findUser(inviteUser->getFd()))
    {
        params = msg->params[0] + " " + msg->params[1] + " :is already on channel";
        server.selfSend(server, client, errorMsg(server.getName(), "443", client.getNick(), params));
        return ;
    }
    else if (channel->getInvite() && !channel->isOp(client))
    {
        params = msg->params[1] + " :You're not channel operator";
        server.selfSend(server, client, errorMsg(server.getName(), "482", client.getNick(), params));
        return ;
    }
    else
    {
        if (channel->getInvite())
        {
            if (!channel->isInviteUser(inviteUser))
                channel->addInviteUser(inviteUser);
        }
        params = msg->params[0] + " " + msg->params[1];
        server.selfSend(server, client, successMsg(server.getName(), "341", client.getNick(), params));

        std::string params2 = msg->params[0] + " " + msg->params[1];
        server.otherSend(server, *inviteUser, successMsg(prefix(client.getNick(), client.getUserName(), client.getHost())\
        , "INVITE", client.getNick(), params2));
    }

    return ;
}

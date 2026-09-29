#include "cmd.hpp"

void Cmd::kick (request::msg* msg, Server& server, int clientFd)
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
    else if (!channel->isOp(client))
    {
        params = msg->params[0] + " :You're not channel operator";
        server.selfSend(server, client, errorMsg(server.getName(), "482", client.getNick(), params));
        return ;
    }
    else
    {
        std::vector<std::string> kicks;
        parseComma(msg->params[1], kicks);

        for (size_t i = 0; i < kicks.size(); i++)
        {
            Client* user = channel->findUser(kicks[i]);
            if (user == NULL)
            {
                params = " :No such nick or channel name";
                server.selfSend(server, client, failedMsg(server.getName(), "401", client.getNick(), kicks[i], params));
                continue ;
            }
            else
            {
                params = kicks[i];
                if (!msg->trailing.empty())
                    params += " " + msg->trailing;
                else
                    params += " :";
                server.broadcast(server, client, *channel, successMsg(prefix(client.getNick(), client.getUserName(), client.getHost()),
                "KICK", channel->getName(), params), true);
                user->quitChannel(channel);
                if (channel->isOp(user))
                    channel->delOp(user);
                channel->delUser(user);
                if (channel->getUserCount() == 0)
                {
                    server.delChannel(channel->getName());
                }
            }
        }
    }

    return ;
}

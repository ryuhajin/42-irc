#include "cmd.hpp"

std::string Cmd::validatePartInput(Server& server, Client& client, request::msg* msg)
{
    std::string errorMessage;

    if (!IsRegister(&client))
	{
        std::string target;
        if (client.getNick().empty())
            target = "*";
        else
            target = client.getNick();
        errorMessage = msg->cmd + " :You have not registered";
        return errorMsg(server.getName(), "461", target, errorMessage);
	}
    
    if (msg->params.size() < 1)
    {
        errorMessage = msg->cmd + " :Not enough parameters";
        return errorMsg(server.getName(), "461", client.getNick(), errorMessage);
    }

    return "";
}

void Cmd::part(request::msg* msg, Server& server, int clientFd)
{
    Client& client = *server.findClient(clientFd);

    std::string errorMessage = Cmd::validatePartInput(server, client, msg);
    if (!errorMessage.empty())
    {
        server.selfSend(server, client, errorMessage);
        return ;
    }

    std::vector<std::string> channelName;
    parseComma(msg->params[0], channelName);
    std::string params;

    for (size_t i = 0; i < channelName.size(); i++)
    {
        Channel* channel = server.findChannel(channelName[i]);

        if (channel == NULL)
        {
            params = channelName[i] + " :No such channel";
            std::string msg = errorMsg(server.getName(), "403", client.getNick(), params);
            server.selfSend(server, client, msg);
        }
        else
        {
            if (msg->trailing.empty())
                msg->trailing = ":";

            if (channel->getUsers().size() == 1 && channel->findUser(client.getNick()) != NULL)
            {
                client.quitChannel(channel);
                channel->delUser(&client);
                server.delChannel(channel->getName());
                server.selfSend(server, client, successMsg(prefix(client.getNick(), client.getUserName(), client.getHost()),
                "PART", channelName[i], msg->trailing));
            }
            else
            {
                if (channel->findUser(client.getNick()) != NULL)
                {
                    server.broadcast(server, client, *channel, successMsg(prefix(client.getNick(), client.getUserName(), client.getHost()),
                    "PART", channelName[i], msg->trailing), true);
                    client.quitChannel(channel);
                    if (channel->isOp(&client))
                        channel->delOp(&client);
                    channel->delUser(&client);
                }
            }
        }
    }
    return ;
}

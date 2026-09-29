#include "cmd.hpp"

void Cmd::toUser (Server& server, Client& client, const std::string& user, const std::string& msg)
{
    Client* to = server.findNick(user);
    std::string params;

    if (!to)
    {
        params = user + " :No such nick or channel name";
        server.selfSend(server, client, errorMsg(server.getName(), "401", client.getNick(), params));
        return ;
    }
    else
    {
        server.otherSend(server, *to, successMsg(prefix(client.getNick(), client.getUserName(), client.getHost())\
        , "PRIVMSG", to->getNick(), msg));
    }
}


void Cmd::toChannel (Server& server, Client& client, const std::string& channelName, const std::string& msg)
{
    Channel* channel = server.findChannel(channelName);
    std::string params;

    if (!IsVaildChannelName(channelName) || channel == NULL)
    {
        params = channelName + " :No such channel";
        server.selfSend(server, client, errorMsg(server.getName(), "461", client.getNick(), params));
        return;
    } 
    else
    {
        params = channelName + " " + msg;
        server.broadcast(server, client, *channel, successMsg(prefix(client.getNick(), client.getUserName(), client.getHost())\
        , "PRIVMSG", channel->getName(), msg), false);
    }
}

void Cmd::privmsg (request::msg* msg, Server& server, int clientFd)
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

    if (msg->params.size() < 1)
    {
        params = msg->cmd + " :Not enough parameters";
        server.selfSend(server, client, errorMsg(server.getName(), "461", client.getNick(), params));
        return ;
    }

    if (msg->trailing.empty())
    {
        params = ":No text to send";
        server.selfSend(server, client, errorMsg(server.getName(), "412", client.getNick(), params));
        return ;
    }

    std::vector<std::string> targets;
    parseComma(msg->params[0], targets);

    for (size_t i = 0; i < targets.size(); i++)
    {
        if (targets[i][0] == '#' || targets[i][0] == '&')
        {
            toChannel(server, client, targets[i], msg->trailing);
        }
        else
        {
            toUser(server, client, targets[i], msg->trailing);
        }
    }

    return ;
}

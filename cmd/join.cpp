#include "cmd.hpp"

void Cmd::sendUserList (Server& server, Client& client, Channel& channel)
{
    std::string params;
    std::string user;

    std::vector<Client*> users = channel.getUsers();
    for (size_t i = 0; i < users.size(); i++)
    {
        if (channel.isOp(users[i]))
            user = "@" + users[i]->getNick();
        else
            user = users[i]->getNick();
        params += user + " ";
    }
    params.erase(params.size()-1);

    std::string target = "= " + channel.getName() + " :";
    server.selfSend(server, client, successMsg(server.getName(), "353", client.getNick(), (target + params)));
    return ;
}

void Cmd::sendEndList(Server& server, Client& client, Channel& channel)
{
    std::string params = channel.getName() + " :End of NAMES list";
    server.selfSend(server,client, successMsg(server.getName(), "366", client.getNick(), params));
    return ;
}

void Cmd::sendTopic(Server& server, Client& client, Channel& channel)
{
    if (channel.getTopic().size() > 0)
    {
        std::string target = channel.getName() + " :";
        std::string params = channel.getTopic();
        server.selfSend(server, client, successMsg(server.getName(), "332", client.getNick(), (target + params)));
    }

    return ;
}

void Cmd::isInvite(Server& server, Client& client, Channel& channel, const std::string& key)
{
    std::string params;

    if (channel.getPassword().size() > 0 && channel.getPassword() != key)
    {
        params = channel.getName() + " :Cannot join channel (+k)";
        server.selfSend(server, client, errorMsg(server.getName(), "475", client.getNick(), params));
        return ;
    }
    channel.delInviteUser(&client);
    channelJoin (server, channel, client);
    return ;
}

void Cmd::isKey(Server& server, Client& client, Channel& channel)
{
    channelJoin (server, channel, client);
    return ;
}

void Cmd::helloBot (Server& server, Client& client, Channel& channel)
{
    std::string pre = prefix("helloBot", "bot", server.getIp());
    std::string msg = channel.getName() + " :welcome " + client.getNick() + ", What a small world!";
    server.broadcast(server, client, channel, successMsg(pre, "PRIVMSG", "", msg), true);
}

void Cmd::channelJoin (Server& server, Channel& channel, Client& client)
{
    channel.addUser(&client);
    client.joinChannel(&channel);

    server.broadcast(server, client, channel, successMsg(prefix(client.getNick(), client.getUserName(), client.getHost()),
            "JOIN", "", channel.getName()), true);
    sendUserList(server, client, channel);
    sendEndList(server, client, channel);
    helloBot(server, client, channel);
}

void Cmd::join (request::msg* msg, Server& server, int clientFd)
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

    std::vector<std::string> channelName;
    parseComma(msg->params[0], channelName);

    std::vector<std::string> keys;
    parseComma(msg->params[1], keys);

    for (size_t i = 0; i < channelName.size(); i++)
    {
        Channel* channel = server.findChannel(channelName[i]);
        
        if (!IsVaildChannelName(channelName[i]))
        {
            params = channelName[i] + " :No such channel";
            server.selfSend(server, client, errorMsg(server.getName(), "461", client.getNick(), params));
            continue;
        }
    
        std::string key = i < keys.size() ? keys[i] : "";
    
        if (channel == NULL)
        {
            server.addChannel(channelName[i]);
            Channel& newChannel = *server.findChannel(channelName[i]);
            newChannel.addOp(&client);
            channelJoin (server, newChannel, client);
            continue;
        }
        else if (channel->findUser(client.getFd()))
        {
            continue;
        }
        else if (channel->getUsers().size() >= (size_t)channel->getlimit())
        {
            params = channelName[i] + " :Cannot join channel (+l)";
            server.selfSend(server, client, errorMsg(server.getName(), "471", client.getNick(), params));
            continue ;
        }
        else if (channel->getInvite())
        {
            if (channel->isInviteUser(&client))
                isInvite(server, client, *channel, key);
            else
            {
                params =  channel->getName() + " :Cannot join channel (+i)";
                server.selfSend(server, client, errorMsg(server.getName(), "473", client.getNick(), params));
            }
            continue;
        }
        else if (channel->getPassword().size() > 0)
        {
            if (channel->getPassword() == key)
                isKey(server, client, *channel);
            else
            {
                params = channel->getName() + " :Cannot join channel (+k)";
                server.selfSend(server, client, errorMsg(server.getName(), "475", client.getNick(), params));
            }
            continue;
        }
        else
        { 
            channelJoin (server, *channel, client);
        }
    }

    return ;
}

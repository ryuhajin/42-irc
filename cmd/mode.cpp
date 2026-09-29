#include "cmd.hpp"

bool Cmd::IsVaildlimit (std::string params)
{
    int limit;
    std::stringstream ss(params);
    
    if (ss >> limit)
    {
        if (limit < 1 || limit > CHANNEL_LIMIT)
            return (false);
    }
    else
        return (false);
    return (true);
}

bool Cmd::invite (Channel* channel, bool flag)
{
	if (channel->getInvite() == false)
	{
		if (flag == true)
		{
			channel->setInvite(true);
			return (true);
		}
	}
	else
	{
		if (flag == false)
		{
			channel->setInvite(false);
			return (true);
		}
	}
	return (false);
}

bool Cmd::topic (Channel* channel, bool flag)
{
	if (channel->getOpTopic() == false)
	{
		if (flag == true)
		{
			channel->setOpTopic(true);
			return (true);
		}
	}
	else
	{
		if (flag == false)
		{
			channel->setOpTopic(false);
			return (true);
		}
	}
	return (false);
}

bool Cmd::key (Channel* channel, bool flag, const std::string& pass)
{
    if (channel->getPassword().size() == 0)
    {
        if (flag == true)
        {
            channel->setPassword(pass);
            return (true);
        }
    }
    else
    {
        if (flag == false)
        {
            channel->delPassword();
            return (true);
        }
    }
    return (false);
}

bool Cmd::op (Channel* channel, bool flag, Client* user)
{
    if (flag == true)
    {
        if (channel->isOp(user))
            return (false);
        channel->addOp(user);
        return (true);
    }
    else if (flag == false)
    {
        if (!channel->isOp(user))
            return (false);
        channel->delOp(user);
        return (true);
    }
    return (false);
}

bool Cmd::limit (Channel* channel, bool flag, const std::string& params)
{
    int limit;
    std::stringstream ss(params);
    ss >> limit;

    if (flag == true)
    {
        if (channel->getLflag() == false)
        {
            channel->setLflag(true);
            channel->setlimit(limit);
            return (true);
        }
        channel->setlimit(limit);
        return (true);
    }
    else
    {
        if (channel->getLflag() == true)
        {
            channel->setLflag(false);
            channel->setlimit(CHANNEL_LIMIT);
            return (true);
        }
    }
    return (false);
}

bool Cmd::IsUserMode(const std::string& param)
{
    if (param[0] == '#' || param[0] == '&')
        return (false);
    return (true);
}

void Cmd::informChannelMode(Server& server, Channel& channel, Client& client)
{
    std::string mode;

    if (channel.getInvite())
        mode += "+i";
    if (channel.getOpTopic())
        mode += "+t";
    if (channel.getPassword().size() > 0)
        mode += "+k";
    if (channel.getLflag())
        mode += "+l";

    server.selfSend(server, client, successMsg(prefix(client.getNick(),client.getUserName(),client.getHost()),\
    "MODE", channel.getName(), mode));
    return ;
}

void Cmd::mode(request::msg* msg, Server& server, int clientFd)
{
    Client* client = server.findClient(clientFd);
    std::string params;

    if (!IsRegister(client))
	{
        std::string target;
        if (client->getNick().empty())
            target = "*";
        else
            target = client->getNick();
        params = msg->cmd + " :You have not registered";
        server.selfSend(server, *client, errorMsg(server.getName(), "451", target, params));
		return ;
	}

    if (msg->params.empty())
    {
        params = msg->cmd + " :Not enough parameters";
        server.selfSend(server, *client, errorMsg(server.getName(), "461", client->getNick(), params));
        return ;
    }

    if (IsUserMode(msg->params[0]))
        return ;

    Channel* channel = server.findChannel(msg->params[0]);
    if (channel == NULL)
    {
        params = msg->params[0] + " :No such channel";
        server.selfSend(server, *client, errorMsg(server.getName(), "403", client->getNick(), params));
        return ;
    }

    if (msg->params.size() < 2)
    {
        informChannelMode(server, *channel, *client);
        return ;
    }
    else if (!channel->isOp(client))
	{
		params = msg->params[0] + " :You're not channel operator";
        server.selfSend(server, *client, errorMsg(server.getName(), "482", client->getNick(), params));
        return ;
	}

    std::string mode;
    std::string success;
    std::string fail;
	std::string param;

    size_t j = 2;
	bool flag = true;

    for (size_t i = 0; i < msg->params[1].size(); i++)
    {
        char c = msg->params[1][i];
        
        if (c == '-')
        {	
            flag = false;
            mode = "-";
            continue;
        }
        else if (c == '+')
        {
            flag = true;
            mode = "+";
            continue;
        }
        else if (c == 'i')
        {
            bool set = invite(channel, flag);
            if (set == true)
                success += mode + "i";
            continue;
        }
        else if (c == 't')
        {
            bool set = topic(channel, flag);
            if (set == true)
                success += mode + "t";
            continue;
        }
        else if (c == 'k')
        {
            if (flag)
            {
                if (msg->params.size() <= j || msg->params[j].empty())
                {    
                    fail += failedMsg(server.getName(), "461", client->getNick(), "MODE", " :Syntax error");
                    continue;
                }
            }
            bool set = key(channel, flag, msg->params[j]);
            if (set)
            {
                if (flag)
                {
                    param += msg->params[j] + " ";
                    j++;
                }
                success += mode + "k";
            }
            continue;
        }
        else if (c == 'o')
        {
            if (msg->params.size() <= j || msg->params[j].empty())
            {
                fail += failedMsg(server.getName(), "461", client->getNick(), "MODE", " :Syntax error");
                continue;
            }
            Client* user = channel->findUser(msg->params[j]);
            if (user == NULL)
            {
                fail += failedMsg(server.getName(), "401", client->getNick(), "o", " :No such nick or channel");
                continue;
            }
            bool set = op (channel, flag, user);
            if (set)
            {
                param += msg->params[j] + " ";
                success += mode + "o";
                j++;
            }
            continue;
        }
        else if (c == 'l')
        {
            if (flag)
            {
                if (msg->params.size() <= j || msg->params[j].empty())
                {
                    fail += failedMsg(server.getName(), "461", client->getNick(), "MODE", " :Syntax error");
                    continue;
                }
                else if (!IsVaildlimit(msg->params[j]))
                {
                    fail += failedMsg(server.getName(), "696", client->getNick(), channel->getName(), " l * :Invalid mode parameter");
                    continue;
                }
            }
            bool set = limit(channel, flag, msg->params[j]);
            if (set)
            {
                if (flag)
                {
                    param += msg->params[j] + " ";
                    j++;
                }
                success += mode + "l";
            }
            continue;
        }
        else
        {
            std::string option (1, c);
            std::string param = " :is unknown mode char for " + channel->getName();
            fail += failedMsg(server.getName(), "472", client->getNick(), option, param);
        }
    }
    
    if (!param.empty() && param[param.size() -1] == ' ')
        param.erase(param.size() -1);

    server.selfSend(server, *client, fail);

    if (!success.empty())
    {
        std::string result = modeSuccessMsg(prefix(client->getNick(),client->getUserName(),client->getHost()),\
        channel->getName(), success, param);
        server.broadcast(server, *client, *channel, result, true);
    }

    return ;
}

#include "channel.hpp"

Channel::Channel(std::string name)
    :_name(name), _limit(CHANNEL_LIMIT), _lflag(false), _inviteChannel(false), _opTopic(false)
{
    std::cout << "[constructor] Channel : " << _name << std::endl;
}

Channel::~Channel()
{
    std::cout << "[destructor] Channel : " << _name << std::endl;
}

const std::string Channel::getName() const
{
    return (_name);
}
const std::string Channel::getTopic() const
{
    return (_topic);
}

std::vector<Client*>& Channel::getOps()
{
    return (_ops);
}
std::vector<Client*>& Channel::getUsers()
{
    return (_users);
}
const std::string Channel::getPassword() const
{
    return (_password);
}

int Channel::getlimit() const
{
	return (_limit);
}

bool Channel::getLflag() const
{
    return (_lflag);
}

bool Channel::getInvite() const
{
	return (_inviteChannel);
}

bool Channel::getOpTopic() const
{
	return (_opTopic);
}

std::vector<Client*>& Channel::getInviteUsers()
{
    return (_inviteUsers);
}

void Channel::delPassword()
{
    _password.erase();
}

void Channel::setPassword(const std::string& pass)
{
    _password = pass;
}

void Channel::setName(const std::string& name)
{
    _name = name;
}
void Channel::setTopic(const std::string& topic)
{
    _topic = topic;
}

void Channel::setlimit(int num)
{
	_limit = num;
}


void Channel::setLflag(bool flag)
{
    _lflag = flag;
}

void Channel::setInvite (bool flag)
{
	_inviteChannel = flag;
}

void Channel::setOpTopic (bool flag)
{
	_opTopic = flag;
}

void Channel::addOp(Client* user)
{
    _ops.push_back(user);
}

bool Channel::isOp(Client* user)
{
    std::vector<Client*>::iterator it = std::find(_ops.begin(), _ops.end(), user);
    if (it != _ops.end())
        return (true);
    return (false);
}

bool Channel::isOp(Client& user)
{
    for ( std::vector<Client*>::iterator it = _ops.begin(); it != _ops.end(); it++)
    {
        if ((*it)->getFd() == user.getFd())
            return (true);
    }
    return (false);
}

void Channel::delOp(Client* user)
{
    std::vector<Client*>::iterator it = std::find(_ops.begin(), _ops.end(), user);
    if (it != _ops.end())
         _ops.erase(it);
}

void Channel::addUser(Client* user)
{
    _users.push_back(user);
}

void Channel::delUser(Client* user)
{
    std::vector<Client*>::iterator it = std::find(_users.begin(), _users.end(), user);
    if (it != _users.end())
        _users.erase(it);
}

void Channel::addInviteUser(Client *user)
{
    _inviteUsers.push_back(user);
}

void Channel::delInviteUser(Client *user)
{
    std::vector<Client*>::iterator it = std::find(_inviteUsers.begin(), _inviteUsers.end(), user);
    if (it != _inviteUsers.end())
         _inviteUsers.erase(it);
}

bool Channel::isInviteUser(Client* user)
{
    std::vector<Client*>::iterator it = std::find(_inviteUsers.begin(), _inviteUsers.end(), user);
    if (it != _inviteUsers.end())
        return (true);
    return (false);
}

Client* Channel::findUser(int fd)
{
    for ( std::vector<Client*>::iterator it = _users.begin(); it != _users.end(); it++)
    {
        if ((*it)->getFd() == fd)
            return (*it);
    }
    return (NULL);
}

Client* Channel::findUser(const std::string& nick)
{
    for ( std::vector<Client*>::iterator it = _users.begin(); it != _users.end(); it++)
    {
        if ((*it)->getNick() == nick)
            return (*it);
    }
    return (NULL);
}

bool Channel::hasUsers() const
{
    return (!_users.empty());
}

size_t Channel::getUserCount() const
{
    return (_users.size());
}

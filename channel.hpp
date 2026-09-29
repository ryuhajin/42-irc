#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include "client.hpp"

#define CHANNEL_LIMIT 10

class Client;
class Channel
{
private:
    std::string _name;
    std::string _topic;
    std::vector<Client *> _ops;
    std::vector<Client *> _users;
    std::vector<Client *> _inviteUsers;
	std::string _password;
	int _limit;
    bool _lflag;
	bool _inviteChannel;
	bool _opTopic;

    Channel();
public:
    Channel(std::string name);
    ~Channel();

    const std::string getName() const;
    const std::string getTopic() const;
    std::vector<std::string>& getBans();
    std::vector<Client*>& getOps();
    std::vector<Client*>& getUsers();
    std::vector<Client*>& getInviteUsers();
    const std::string getPassword() const;
	int getlimit() const;
    bool getLflag() const;
	bool getInvite() const;
	bool getOpTopic() const;

    void setPassword(const std::string& pass);
    void delPassword();

	void setlimit(int num);
    void setName(const std::string& name);
    void setTopic(const std::string& topic);
	
	void setInvite (bool flag);
	void setOpTopic (bool flag);
    void setLflag(bool flag);

    void addOp(Client *user);
    void delOp(Client *user);
    bool isOp(Client* user);
    bool isOp(Client& user);

    void addUser(Client *user);
    void delUser(Client *user);

    void addInviteUser(Client *user);
    void delInviteUser(Client *user);
    bool isInviteUser(Client* user);

    Client* findUser(int fd);
    Client* findUser(const std::string& nick);

    bool hasUsers() const;
    size_t getUserCount() const;
};

#endif
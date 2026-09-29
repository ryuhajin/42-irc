#include "server.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <csignal>

bool running = true;

void signalHandler(int signum)
{
    if (signum == SIGINT || signum == SIGQUIT)
    {
        running = false;
    }
}

void printClients(Server& server)
{
    std::cout << "Client count : " << server.getClients().size() << std::endl;
    for (std::map<int, Client*>::iterator it = server.getClients().begin(); it != server.getClients().end(); it++)
    {
        std::cout << "[Client]\n" << "fd: " << it->second->getFd() << "\n" << "nick: " << \
        it->second->getNick() << "\n" << "userName: " << it->second->getUserName() << "\n" << \
        "host: " << it->second->getHost() << "\n" << std::endl;
        std::cout << "in channles: ";
        for (size_t i = 0; i < it->second->getChannels().size(); i++)
        {
            std::cout << it->second->getChannels()[i]->getName() << " ";
        }
        std::cout << "\n" << std::endl;
    }
}

void printChannels(Server& server)
{
    std::cout << "#Channel count : " << server.getChannels().size() << std::endl;
    for (std::map<std::string, Channel*>::iterator it = server.getChannels().begin(); it != server.getChannels().end(); it++)
    {
        std::cout << "[Channel]\n" << "name: " << it->first << "\n";
        std::cout << "Users size: " << it->second->getUserCount() << "\n";
        std::vector<Client*> op = it->second->getOps();
        std::cout << "(OPs)\n";
        for (size_t i = 0; i < op.size(); i++)
        {
            std::cout << "nick: " << op[i]->getNick() << "/ fd: " << op[i]->getFd() << std::endl;
        }
        std::cout << "\n" << std::endl;
        std::vector<Client*> user = it->second->getUsers();
        std::cout << "(Users)\n";
        for (size_t i = 0; i < user.size(); i++)
        {
            std::cout << "nick: " << user[i]->getNick() << "/ fd: " << user[i]->getFd() << std::endl;
        }
        std::cout << "\n" << std::endl;
        std::vector<Client*> invite = it->second->getInviteUsers();
        std::cout << "invite flag: " << it->second->getInvite() << std::endl;
        std::cout << "(Invites)\n";
        for (size_t i = 0; i < invite.size(); i++)
        {
            std::cout << "nick: " << invite[i]->getNick() << "/ fd: " << invite[i]->getFd() << std::endl;
        }
        std::cout << "\n" << std::endl;
        std::cout << "limit flag: " <<  it->second->getLflag() << "/ limit:" << it->second->getlimit() << std::endl;
        std::cout << "topic flag: " << it->second->getOpTopic() << "/ topic:" << it->second->getTopic() << std::endl;
        std::cout << "password flag: " << it->second->getPassword().size() << "/ password:" << it->second->getPassword() << "\n" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    int port;
    std::string password;

    if (!argInit(argc, argv, port, password))
        exit(1);

    signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);

    Server server(port, password);

    struct kevent waitEvents[EVENT_MAX];
    struct kevent* currEvent;

    int eventsCount;
    while (running)
    {
        eventsCount = kevent(server.getKq(), &server.getNewEvents()[0], server.getNewEvents().size(), waitEvents, EVENT_MAX, NULL);
        if (eventsCount == -1)
        {
            if (running == false)
                break ;
            server.exitPerror("Error : kevent failed.");
        }
        server.getNewEvents().clear();
        for (int i = 0; i < eventsCount; i++)
        {
            currEvent = &waitEvents[i];
            if (currEvent->flags & (EV_EOF | EV_ERROR))
                server.disconnectHandler(*currEvent);
            if (currEvent->filter == EVFILT_READ)
                server.readEventHandler(*currEvent);
            else if (currEvent->filter == EVFILT_WRITE)
                server.writeClinetSocket(*currEvent, server.getClients());
            // printChannels(server);
            // printClients(server);
        }
    }

    return (0);
}

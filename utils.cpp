#include "utils.hpp"
#include <iostream>

bool IsVaildArg(int argc)
{
    if (argc != 3)
    {
        std::cout << "Error : invailde input. Recommend: ./irserve <portnum> <password>" << std::endl;
        return (false);
    }

    return (true);
}

bool IsVaildPort(char *argv[])
{
    for (int i = 0; argv[1][i] != '\0'; i++)
    {
        if (argv[1][i] < '0' || argv[1][i] > '9')
        {
            std::cout << "Error : invaild port." << std::endl;
            return (false);
        }
    }

    int portNum = std::atoi(argv[1]);

    if (portNum < 1 || portNum > 65535)
    {
        std::cout << "Error : invaild port." << std::endl;
        return (false);
    }

    return (true);
}

bool argInit(int argc, char *argv[], int& port, std::string& pass)
{
    if (!IsVaildArg(argc))
        return (false);
    else if (!IsVaildPort(argv))
        return (false);
    port = std::atoi(argv[1]);
    pass = argv[2];

    return (true);
}

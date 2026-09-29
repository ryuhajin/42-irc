#include "parser.hpp"

request::msg* request::init(void)
{
    return (new request::msg);
}

std::string request::eraseCRLF (const std::string& str)
{
	size_t i;
	i = str.find("\r\n");
    std::string result(str);

    result.erase(i, 2);
    return (result);
}

bool request::startsWith (const std::string& buffer)
{
    if (buffer[0] == ':')
        return (true);
    return (false);
}

std::string request::makeTrailing (const std::string& buffer, std::istringstream& iss)
{
    std::string line = buffer;

    std::string restOfStream;
    getline(iss, restOfStream);
    line += " " + restOfStream;

    return (line);
}

// void request::printData (request::msg* data)
// {
//      std::string prefix;
//         std::string cmd;
//         std::vector<std::string> params;
//         std::string trailing;
//     std::cout << "[struct msg]\n" << std::endl;
//     std::cout << "prefix: " << data->prefix << std::endl;
//     std::cout << "cmd: " << data->cmd << std::endl;
    
//     int i = 0;
//     for (std::vector<std::string>::iterator it = data->params.begin(); it != data->params.end(); it++)
//     {
//         std::cout << "params" << "[" << i << "]: " << *it << std::endl;
//         i++;
//     }
//     std::cout << "trailing: " << data->trailing << std::endl;
//     std::cout << std::endl;
// }

request::msg* request::parser(std::string request)
{
    request::msg* data = init();

    std::istringstream iss(request::eraseCRLF(request));
    std::string buffer;

    getline(iss, buffer, ' ');
    bool hasPrefix = request::startsWith(buffer);
    if (hasPrefix)
    {
        data->prefix = buffer;
        getline(iss, buffer, ' ');
        data->cmd = buffer;
    }
    else
    {
        data->prefix = "";
        data->cmd = buffer;
    }
    
    while (getline(iss, buffer, ' '))
    {
        if (buffer[0] != ':')
            data->params.push_back(buffer);
        else
        {
            data->trailing = makeTrailing(buffer, iss);
            break;
        }
    }

    // request::printData(data);
    return (data);
}

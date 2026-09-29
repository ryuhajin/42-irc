#ifndef PARSER_HPP
# define PARSER_HPP

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

namespace request {
    struct msg
    {
        std::string prefix;
        std::string cmd;
        std::vector<std::string> params;
        std::string trailing;
    };

    msg* init(void);
    std::string eraseCRLF (const std::string& str);
    bool startsWith (const std::string& buffer);
    std::string makeTrailing (const std::string& buffer, std::istringstream& iss);
    void printData (request::msg* data);
    msg* parser(std::string request);
}

#endif


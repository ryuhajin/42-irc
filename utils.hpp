#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>

bool IsVaildArg(int argc);
bool IsVaildPort(char *argv[]);
bool argInit(int argc, char *argv[], int& port, std::string& pass);

#endif
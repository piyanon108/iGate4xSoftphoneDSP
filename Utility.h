#ifndef UTILITY_H
#define UTILITY_H

#include <string>

#include <iostream>
#include <fstream>
#include <unistd.h>

class Utility
{
public:

    static std::string hexToString(char const* input, unsigned int length);

    static std::string getMacAddress();

    static std::string getIpAddress();

    static std::string nowStr();

    static std::string getTime();

    static double now();

    static double nowMilliSeconds();

    static std::string getmemoryusage();

};

#endif

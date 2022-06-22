#include <iostream>
#include <string>

#include <memory>
#include <string>
#include <stdexcept>

using namespace std;

bool logEnabled = false;

void log(string to_log)
{
    if(logEnabled == true)
    {
        std::cout << to_log << std::endl;
    }
}
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <mutex>

using namespace std;

void log(string message);

void waitMutex(mutex* exampleMutex);

enum goSleepCondition
{
    None,
    powerButton,
    halSensor   
};

#endif
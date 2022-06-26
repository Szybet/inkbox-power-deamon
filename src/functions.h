#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <mutex>

using namespace std;

void log(string message);

void waitMutex(mutex* exampleMutex);

void prepareVariables();

string readConfigString(string path);

void writeFileString(string path, string stringToWrite);

void Screenshot();

enum goSleepCondition
{
    None,
    powerButton,
    halSensor   
};

enum sleepBool
{
    Nothing,
    Prepare,
    GoingSleep,
    After,   
};

#endif
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <mutex>
#include <thread>

using namespace std;

void log(string message);

void waitMutex(mutex* exampleMutex);

void prepareVariables();

void ManageConfig();

string readConfigString(string path);

void writeFileString(string path, string stringToWrite);

string readFile(string path);

bool is_file_exist(string fileName);

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
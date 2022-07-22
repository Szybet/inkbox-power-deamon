#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <mutex>
#include <thread>

using namespace std;

void log(string message);

void waitMutex(mutex* exampleMutex);

void prepareVariables();

string readConfigString(string path);

void writeFileString(string path, string stringToWrite);

string readFile(string path);

bool fileExists(string fileName);

bool dirExists(string path);

string executeCommand(string command);

bool getChargerStatus();

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
    Skip,
};

#endif
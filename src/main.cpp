#include <iostream>
#include <string>
#include <thread>

#include "monitorEvents.h"
#include "functions.h"
#include "watchdog.h"
#include "fbinkFunctions.h"
#include "AppsFreeze.h"

extern bool logEnabled;
extern int fbfd;

using namespace std;

int main()
{
    const char* tmp = std:: getenv("DEBUG");
    std::string envVar;

    if(tmp != NULL)
    {
        envVar = tmp;
        if(envVar == "true")
        {
            logEnabled = true;
            log("Debug mode is activated");
        }
    }

    prepareVariables();
    ManageConfig();
    initFbink();

    thread monitorDev(startMonitoringDev);
    // https://stackoverflow.com/questions/7381757/c-terminate-called-without-an-active-exception
    thread watchdogThread(startWatchdog);

    monitorDev.join();
    watchdogThread.join();
    return 0;
}
#include <iostream>
#include <string>
#include <thread>

#include "monitorEvents.h"
#include "functions.h"
#include "watchdog.h"
#include "fbinkFunctions.h"
#include "AppsFreeze.h"
#include "configUpdate.h"

extern bool logEnabled;
extern int fbfd;

using namespace std;

int main()
{
    std::cout << "Starting..." << std::endl;
    
    const char* tmp = std:: getenv("DEBUG");
    std::string envVar;

    if(tmp != NULL)
    {
        envVar = tmp;
        if(envVar == "true")
        {
            logEnabled = true;
            log("Debug mode is activated");
            log("Saving logs to /tmp/PowerDaemonLogs.txt");
        }
    }


    prepareVariables();
    initFbink();

    thread monitorDev(startMonitoringDev);
    thread watchdogThread(startWatchdog);
    thread watchConfig(startMonitoringConfig);

    // https://stackoverflow.com/questions/7381757/c-terminate-called-without-an-active-exception
    monitorDev.join();
    watchdogThread.join();
    watchConfig.join();
    log("How did this ended");
    return 0;
}
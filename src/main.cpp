#include <iostream>
#include <string>
#include <thread>

#include "main.h"
#include "monitorEvents.h"

bool logEnabled = false;

void log(string to_log)
{
    if(logEnabled == true)
    {
        cout << to_log << endl;

    }
}

int main()
{
    const char* tmp = getenv("DEBUG");
    string envVar;

    if(tmp != NULL)
    {
        envVar = tmp;
        if(envVar == "true")
        {
            logEnabled = true;
            log("Debug mode is activated");
        }
    }

    std::thread monitor_dev(startMonitoringDev);

    // https://stackoverflow.com/questions/7381757/c-terminate-called-without-an-active-exception
    monitor_dev.join();

    return 0;
}
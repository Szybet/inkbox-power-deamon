#include <iostream>
#include <cstring>
#include <thread>

#include "monitorEvents.h"

using namespace std;

bool log_enabled = false;

void log(string to_log)
{
    if(log_enabled == true)
    {
        cout << to_log << endl;

    }
}

int main()
{
    cout << "WTFFFF" << endl;
    const char* tmp = getenv("DEBUG"); //
    string env_var;

    if(tmp != NULL)
    {
        env_var = tmp;
        if(env_var == "true")
        {
            log_enabled = true;
            log("Debug mode is activated");
        }
    }

    //std::thread monitor_dev(start_monitoring_dev);

    start_monitoring_dev();

    return 0;
}
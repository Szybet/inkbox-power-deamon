#include <iostream>
#include <string>
#include <memory>
#include <string>
#include <stdexcept>
#include <mutex>
#include <chrono>
#include <thread>

#include "functions.h"

using namespace std;

bool logEnabled = false;

// Mutex variables

bool watchdogStartJob = false;
mutex watchdogStartJob_mtx;

goSleepCondition newSleepCondition = None;
mutex newSleepCondition_mtx;

sleepBool sleepJob = Nothing;
mutex sleep_mtx;

//

void log(string to_log)
{
    if(logEnabled == true)
    {
        std::cout << to_log << std::endl;
    }
}

void waitMutex(mutex* exampleMutex)
{
    bool continueBool = false;
        std::chrono::milliseconds timespan(150);

    while(continueBool == false)
    {
        // https://en.cppreference.com/w/cpp/thread/mutex/try_lock
        if(exampleMutex->try_lock() == false)
        {
            std::this_thread::sleep_for(timespan);
        } else {
            continueBool = true;
        }
    }
}
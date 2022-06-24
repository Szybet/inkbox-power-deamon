#include "afterSleep.h"
#include "functions.h"

#include <exception>
#include <mutex>
#include <string>
#include <thread>

extern sleepBool sleepJob;
extern mutex sleep_mtx;

// Explanation why this code looks garbage
// threads in cpp cant be killed from outside, so its needed to check every step
// for a variable change. -Use another library! no. thats such a simple program
// that it doesnt need it + other libraries do the same, for example
// boost::thread does exactly what i have described above, just in the background
// ~Szybet

// void checkExitAfter()
void CEA()
{
    waitMutex(&sleep_mtx);
    if(sleepJob != After)
    {
        sleep_mtx.unlock();
        log("Terminating afterSleep");
        terminate();
    }
    sleep_mtx.unlock();
}

void afterSleep() {
  log("Launching afterSleep");
  CEA();
  
  log("Exiting afterSleep");
}
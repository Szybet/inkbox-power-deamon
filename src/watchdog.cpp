#include <mutex>
#include <string>
#include <thread>

#include "functions.h"
#include "monitorEvents.h"

using namespace std;

extern bool watchdogStartJob;
extern mutex watchdogStartJob_mtx;

extern goSleepCondition newSleepCondition;
extern mutex newSleepCondition_mtx;

void startWatchdog() {
  std::chrono::milliseconds timespan(150);
  while (true) {
    bool saveWatchdogState;
    waitMutex(&watchdogStartJob_mtx);
    saveWatchdogState = watchdogStartJob;
    watchdogStartJob_mtx.unlock();

    if(saveWatchdogState == true) {
      log("Watchdog event received");
    }

    std::this_thread::sleep_for(timespan);
  }
}
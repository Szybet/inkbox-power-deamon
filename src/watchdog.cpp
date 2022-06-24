#include <mutex>
#include <string>
#include <thread>

#include "afterSleep.h"
#include "functions.h"
#include "monitorEvents.h"
#include "prepareSleep.h"

using namespace std;

extern bool watchdogStartJob;
extern mutex watchdogStartJob_mtx;

extern goSleepCondition newSleepCondition;
extern mutex newSleepCondition_mtx;

extern sleepBool sleepJob;
extern mutex sleep_mtx;

void startWatchdog() {
  std::chrono::milliseconds timespan(150);

  thread prepareThread;
  thread afterThread;

  while (true) {
    bool saveWatchdogState = false;
    waitMutex(&watchdogStartJob_mtx);
    saveWatchdogState = watchdogStartJob;
    watchdogStartJob = false;
    watchdogStartJob_mtx.unlock();

    if (saveWatchdogState == true) {
      log("Watchdog event received");
      saveWatchdogState = false;

      waitMutex(&sleep_mtx);

      if (sleepJob == Nothing) {
        sleepJob = Prepare;
        sleep_mtx.unlock();
        prepareThread = thread(prepareSleep);
      } else if (sleepJob == Prepare) {
        sleepJob = After;
        sleep_mtx.unlock();
        prepareThread.join();
        afterThread = thread(afterSleep);
      } else if (sleepJob == After) {
        sleepJob = Prepare;
        sleep_mtx.unlock();
        afterThread.join();
        prepareThread = thread(prepareSleep);
      } else if (sleepJob == GoingSleep) {
        sleepJob = After;
        sleep_mtx.unlock();
        prepareThread.join();
        afterThread = thread(afterSleep);
      }
    }
  }

  std::this_thread::sleep_for(timespan);
  prepareThread.join();
  afterThread.join();
}

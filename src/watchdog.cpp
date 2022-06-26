#include <mutex>
#include <string>
#include <thread>

#include "afterSleep.h"
#include "functions.h"
#include "goingSleep.h"
#include "monitorEvents.h"
#include "prepareSleep.h"

using namespace std;

extern bool watchdogStartJob;
extern mutex watchdogStartJob_mtx;

extern goSleepCondition newSleepCondition;
extern mutex newSleepCondition_mtx;

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern sleepBool watchdogNextStep;

void startWatchdog() {
  std::chrono::milliseconds timespan(150);

  thread prepareThread;
  thread afterThread;
  thread goingThread;

  while (true) {
    bool saveWatchdogState = false;
    waitMutex(&watchdogStartJob_mtx);
    saveWatchdogState = watchdogStartJob;
    watchdogStartJob = false;
    watchdogStartJob_mtx.unlock();

    // this here takes signals from monitorEvents and assigns them to do things
    if (saveWatchdogState == true) {
      log("Watchdog event received");
      saveWatchdogState = false;

      // Proritise user events over next steps
      watchdogNextStep = Nothing;

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
        // device here what to do with a config
        sleepJob = After;
        sleep_mtx.unlock();
        prepareThread.join();
        afterThread = thread(afterSleep);
      }
    }
    if (watchdogNextStep != Nothing) {
      // Make sure all jobs exit. they propably already are, becouse they called
      // it
      waitMutex(&sleep_mtx);
      sleepJob = Nothing;
      sleep_mtx.unlock();
      log("im here testing");
      // its here dying. make a function based on .joinable to joing a thread
      // and use that - note for tommorow me
      prepareThread.join();
      afterThread.join();
      goingThread.join();
      log("im here testing dead");
      if (watchdogNextStep == After) {
        waitMutex(&sleep_mtx);
        sleepJob = After;
        sleep_mtx.unlock();
        afterThread = thread(afterSleep);
      } else if (watchdogNextStep == GoingSleep) {
        waitMutex(&sleep_mtx);
        sleepJob = GoingSleep;
        sleep_mtx.unlock();
        goingThread = thread(goSleep);
      } else {
        log("Its impossible. you will never see this log");
        exit(EXIT_FAILURE);
      }
    }
    std::this_thread::sleep_for(timespan);
  }

  prepareThread.join();
  afterThread.join();
}

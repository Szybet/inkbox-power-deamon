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
        prepareThread.detach();

        //
      } else if (sleepJob == Prepare) {
        sleepJob = After;
        sleep_mtx.unlock();
        if (prepareThread.joinable() == true) {
          prepareThread.join();
        }
        afterThread = thread(afterSleep);
        afterThread.detach();

        //
      } else if (sleepJob == After) {
        sleepJob = Prepare;
        sleep_mtx.unlock();

        if (prepareThread.joinable() == true) {
          prepareThread.join();
        }
        if (goingThread.joinable() == true) {
          goingThread.join();
        }

        prepareThread = thread(prepareSleep);
        prepareThread.detach();

        //
      } else if (sleepJob == GoingSleep) {
        log("Watchdog goes next with Goingsleep");
        sleepJob = After;
        sleep_mtx.unlock();

        log("trying to join goingThread");
        if (goingThread.joinable() == true) {
          goingThread.join();
        }

        log("Launching afterSleep thread from GoingSleep watchdog state");
        afterThread = thread(afterSleep);
        afterThread.detach();

        //
      }
    }
    if (watchdogNextStep != Nothing) {
      // Make sure all jobs exit. they propably already are, becouse they called
      // it
      waitMutex(&sleep_mtx);
      sleepJob = Nothing;
      sleep_mtx.unlock();

      // I wanted to write a function for it:
      /*
      void join_smarter(thread threadArg)
      {
        if(threadArg.joinable() == true)
        {
          threadArg.join();
        }
      }
      */
      // but it doesnt work, some weird error so...
      if (prepareThread.joinable() == true) {
        prepareThread.join();
      }
      if (afterThread.joinable() == true) {
        afterThread.join();
      }
      if (goingThread.joinable() == true) {
        goingThread.join();
      }

      if (watchdogNextStep == After) {
        waitMutex(&sleep_mtx);
        sleepJob = After;
        sleep_mtx.unlock();
        afterThread = thread(afterSleep);
        afterThread.detach();
      } else if (watchdogNextStep == GoingSleep) {
        waitMutex(&sleep_mtx);
        sleepJob = GoingSleep;
        sleep_mtx.unlock();
        goingThread = thread(goSleep);
        goingThread.detach();
      } else {
        log("Its impossible. you will never see this log");
        exit(EXIT_FAILURE);
      }
      watchdogNextStep = Nothing;
    }
    std::this_thread::sleep_for(timespan);
  }

  prepareThread.join();
  afterThread.join();
  goingThread.join();
}

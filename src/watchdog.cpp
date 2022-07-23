#include <mutex>
#include <string>
#include <thread>

#include "afterSleep.h"
#include "functions.h"
#include "goingSleep.h"
#include "monitorEvents.h"
#include "prepareSleep.h"
#include "devices.h"

using namespace std;

// var

extern bool WhenChargerSleep;

//
extern bool watchdogStartJob;
extern mutex watchdogStartJob_mtx;

extern goSleepCondition newSleepCondition;
extern mutex newSleepCondition_mtx;

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern sleepBool watchdogNextStep;

// this variable says what thread is currently active, to know which to kill
extern sleepBool CurrentActiveThread;
extern mutex CurrentActiveThread_mtx;

// I wanted to write a function for all those joins:
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

      // Handling 3-WhenChargerSleep
      if (WhenChargerSleep == false) {
        if (getChargerStatus() == true) {
          log("Skipping watchdog event becouse of 3-WhenChargerSleep");
          sleepJob = Skip;
        }
      }

      // Proritise user events over next steps - actually no. maybe
      // watchdogNextStep = Nothing;

      waitMutex(&sleep_mtx);

      if (sleepJob == Nothing) {
        log("Launching prepare thread becouse of nothing sleep job");
        // This is here to avoid waiting too long after
        waitMutex(&CurrentActiveThread_mtx);
        sleepJob = Prepare;
        sleep_mtx.unlock();

        if (CurrentActiveThread != Nothing) {
          bool check = false;
          CurrentActiveThread_mtx.unlock();
          while (check == false) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            waitMutex(&CurrentActiveThread_mtx);
            if (CurrentActiveThread == Nothing) {
              check = true;
            }
            CurrentActiveThread_mtx.unlock();
          }
        } else {
          CurrentActiveThread_mtx.unlock();
        }

        waitMutex(&CurrentActiveThread_mtx);
        CurrentActiveThread = Prepare;
        CurrentActiveThread_mtx.unlock();
        prepareThread = thread(prepareSleep);
        prepareThread.detach();

        //
      } else if (sleepJob == Prepare) {
        log("Launching after thread becouse of prepare sleep job");
        waitMutex(&CurrentActiveThread_mtx);
        sleepJob = After;
        sleep_mtx.unlock();

        if (CurrentActiveThread != Nothing) {
          bool check = false;
          CurrentActiveThread_mtx.unlock();
          while (check == false) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            waitMutex(&CurrentActiveThread_mtx);
            if (CurrentActiveThread == Nothing) {
              check = true;
            }
            CurrentActiveThread_mtx.unlock();
          }
        } else {
          CurrentActiveThread_mtx.unlock();
        }

        waitMutex(&CurrentActiveThread_mtx);
        CurrentActiveThread = After;
        CurrentActiveThread_mtx.unlock();

        afterThread = thread(afterSleep);
        afterThread.detach();

        //
      } else if (sleepJob == After) {
        log("Launching prepare thread becouse of after sleep job");
        waitMutex(&CurrentActiveThread_mtx);
        sleepJob = Prepare;
        sleep_mtx.unlock();

        if (CurrentActiveThread != Nothing) {
          bool check = false;
          CurrentActiveThread_mtx.unlock();
          while (check == false) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            waitMutex(&CurrentActiveThread_mtx);
            if (CurrentActiveThread == Nothing) {
              check = true;
            }
            CurrentActiveThread_mtx.unlock();
          }
        } else {
          CurrentActiveThread_mtx.unlock();
        }

        waitMutex(&CurrentActiveThread_mtx);
        CurrentActiveThread = Prepare;
        CurrentActiveThread_mtx.unlock();

        prepareThread = thread(prepareSleep);
        prepareThread.detach();

        //
      } else if (sleepJob == GoingSleep) {
        log("Launching after thread becouse of goingsleep sleep job");
        // To be sure a new thread isin't launching anyway
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        if (watchdogNextStep != After) {
          waitMutex(&CurrentActiveThread_mtx);
          sleepJob = After;
          sleep_mtx.unlock();

          if (CurrentActiveThread != Nothing) {
            bool check = false;
            CurrentActiveThread_mtx.unlock();
            while (check == false) {
              std::this_thread::sleep_for(std::chrono::milliseconds(50));
              waitMutex(&CurrentActiveThread_mtx);
              if (CurrentActiveThread == Nothing) {
                check = true;
              }
              CurrentActiveThread_mtx.unlock();
            }
          } else {
            CurrentActiveThread_mtx.unlock();
          }

          waitMutex(&CurrentActiveThread_mtx);
          CurrentActiveThread = After;
          CurrentActiveThread_mtx.unlock();

          afterThread = thread(afterSleep);
          afterThread.detach();

          //
        } else {
          log("A event from monitorevents requested after thread, but watchdogNextStep already wanted it, so skipping the monitorevent request");
          sleep_mtx.unlock();
        }
      } else {
        log("This will never happen: watchdog");
        sleep_mtx.unlock();
      }
    }
    if (watchdogNextStep != Nothing) {
      log("launching watchdogNextStep request");
      // Make sure all jobs exit. they propably already are, becouse they called
      // it
      waitMutex(&sleep_mtx);
      sleepJob = Nothing;
      sleep_mtx.unlock();

      waitMutex(&CurrentActiveThread_mtx);
      if (CurrentActiveThread != Nothing) {
        bool check = false;
        CurrentActiveThread_mtx.unlock();
        while (check == false) {
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
          waitMutex(&CurrentActiveThread_mtx);
          if (CurrentActiveThread == Nothing) {
            check = true;
          }
          CurrentActiveThread_mtx.unlock();
        }
      } else {
        CurrentActiveThread_mtx.unlock();
      }

      if (watchdogNextStep == After) {
        log("Launching after thread becouse of a request of watchdogNextStep");
        waitMutex(&sleep_mtx);
        sleepJob = After;
        sleep_mtx.unlock();

        waitMutex(&CurrentActiveThread_mtx);
        CurrentActiveThread = After;
        CurrentActiveThread_mtx.unlock();

        afterThread = thread(afterSleep);
        afterThread.detach();
      } else if (watchdogNextStep == GoingSleep) {
        log("Launching goingsleep thread becouse of a request of "
            "watchdogNextStep");
        waitMutex(&sleep_mtx);
        sleepJob = GoingSleep;
        sleep_mtx.unlock();

        waitMutex(&CurrentActiveThread_mtx);
        CurrentActiveThread = GoingSleep;
        CurrentActiveThread_mtx.unlock();

        goingThread = thread(goSleep);
        goingThread.detach();
      } else {
        log("Its impossible. you will never see this log");
        exit(EXIT_FAILURE);
      }
      watchdogNextStep = Nothing;
    }
    ledManager();
    std::this_thread::sleep_for(timespan);
  }

  prepareThread.join();
  afterThread.join();
  goingThread.join();
}
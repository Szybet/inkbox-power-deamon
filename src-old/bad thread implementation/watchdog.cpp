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

// those mutexes is to check if a thread wants to exit, to call .join before it
extern bool exitPreparethread;
extern mutex exitPreparethread_mtx;

extern bool exitGoingThread;
extern mutex exitGoingThread_mtx;

extern bool exitAfterThread;
extern mutex exitAfterThread_mtx;
//

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

      // Proritise user events over next steps
      watchdogNextStep = Nothing;

      waitMutex(&sleep_mtx);

      if (sleepJob == Nothing) {
        log("Watchdog: Launching normal prepare thread from user event");
        sleepJob = Prepare;
        sleep_mtx.unlock();

// make sure all threads are not wanting to die
        waitMutex(&exitAfterThread_mtx);
        if (exitAfterThread == true) {
          exitAfterThread = false;
          exitAfterThread_mtx.unlock();
          while (afterThread.joinable() == false) {
          };
          afterThread.join();
        } else {
          exitAfterThread_mtx.unlock();
        }

        waitMutex(&exitGoingThread_mtx);
        if (exitGoingThread == true) {
          exitGoingThread = false;
          exitGoingThread_mtx.unlock();
          while (goingThread.joinable() == false) {
          };
          goingThread.join();
        } else {
          exitGoingThread_mtx.unlock();
        }

        waitMutex(&exitPreparethread_mtx);
        if (exitPreparethread == true) {
          exitPreparethread = false;
          exitPreparethread_mtx.unlock();
          while (prepareThread.joinable() == false) {
          };
          prepareThread.join();
        } else {
          exitPreparethread_mtx.unlock();
        }
        //

        if (prepareThread.joinable() == true) {
          prepareThread.join();
        }
        prepareThread = thread(prepareSleep);
        //
      } else if (sleepJob == Prepare) {
        log("Watchdog: stopping Prepare and entering after thread");
        sleepJob = After;
        sleep_mtx.unlock();
        
        // make sure all threads are not wanting to die
        waitMutex(&exitAfterThread_mtx);
        if (exitAfterThread == true) {
          exitAfterThread = false;
          exitAfterThread_mtx.unlock();
          while (afterThread.joinable() == false) {
          };
          afterThread.join();
        } else {
          exitAfterThread_mtx.unlock();
        }

        waitMutex(&exitGoingThread_mtx);
        if (exitGoingThread == true) {
          exitGoingThread = false;
          exitGoingThread_mtx.unlock();
          while (goingThread.joinable() == false) {
          };
          goingThread.join();
        } else {
          exitGoingThread_mtx.unlock();
        }

        waitMutex(&exitPreparethread_mtx);
        if (exitPreparethread == true) {
          exitPreparethread = false;
          exitPreparethread_mtx.unlock();
          while (prepareThread.joinable() == false) {
          };
          prepareThread.join();
        } else {
          exitPreparethread_mtx.unlock();
        }
        //

        if (afterThread.joinable() == true) {
          afterThread.join();
        }
        afterThread = thread(afterSleep);
        //
      } else if (sleepJob == After) {
        log("Watchdog: stopping after and entering prepare thread");
        sleepJob = Prepare;
        sleep_mtx.unlock();

// make sure all threads are not wanting to die
        waitMutex(&exitAfterThread_mtx);
        if (exitAfterThread == true) {
          exitAfterThread = false;
          exitAfterThread_mtx.unlock();
          while (afterThread.joinable() == false) {
          };
          afterThread.join();
        } else {
          exitAfterThread_mtx.unlock();
        }

        waitMutex(&exitGoingThread_mtx);
        if (exitGoingThread == true) {
          exitGoingThread = false;
          exitGoingThread_mtx.unlock();
          while (goingThread.joinable() == false) {
          };
          goingThread.join();
        } else {
          exitGoingThread_mtx.unlock();
        }

        waitMutex(&exitPreparethread_mtx);
        if (exitPreparethread == true) {
          exitPreparethread = false;
          exitPreparethread_mtx.unlock();
          while (prepareThread.joinable() == false) {
          };
          prepareThread.join();
        } else {
          exitPreparethread_mtx.unlock();
        }
        //
        
        if (prepareThread.joinable() == true) {
          prepareThread.join();
        }
        prepareThread = thread(prepareSleep);
        //
      } else if (sleepJob == GoingSleep) {
        log("Watchdog: stopping going and entering after thread");
        sleepJob = After;
        sleep_mtx.unlock();

        // make sure all threads are not wanting to die
        waitMutex(&exitAfterThread_mtx);
        if (exitAfterThread == true) {
          exitAfterThread = false;
          exitAfterThread_mtx.unlock();
          while (afterThread.joinable() == false) {
          };
          afterThread.join();
        } else {
          exitAfterThread_mtx.unlock();
        }

        waitMutex(&exitGoingThread_mtx);
        if (exitGoingThread == true) {
          exitGoingThread = false;
          exitGoingThread_mtx.unlock();
          while (goingThread.joinable() == false) {
          };
          goingThread.join();
        } else {
          exitGoingThread_mtx.unlock();
        }

        waitMutex(&exitPreparethread_mtx);
        if (exitPreparethread == true) {
          exitPreparethread = false;
          exitPreparethread_mtx.unlock();
          while (prepareThread.joinable() == false) {
          };
          prepareThread.join();
        } else {
          exitPreparethread_mtx.unlock();
        }
        //
        
        if (afterThread.joinable() == true) {
          afterThread.join();
        }
        afterThread = thread(afterSleep);
        //
      } else {
        log("This shouldnt happen: watchdog");
        sleep_mtx.unlock();
      }
    }
    if (watchdogNextStep != Nothing) {
      log("Requested next watchdog state by a thread");
      // Make sure all jobs exit. they propably already are, becouse they called
      // it
      waitMutex(&sleep_mtx);
      sleepJob = Nothing;
      sleep_mtx.unlock();

      waitMutex(&exitPreparethread_mtx);
      if (exitPreparethread == true) {
        exitPreparethread = false;
        exitPreparethread_mtx.unlock();
        while (prepareThread.joinable() == false) {
        };
        prepareThread.join();
      } else {
        exitPreparethread_mtx.unlock();
      }

      waitMutex(&exitAfterThread_mtx);
      if (exitAfterThread == true) {
        exitAfterThread = false;
        exitAfterThread_mtx.unlock();
        while (afterThread.joinable() == false) {
        };
        afterThread.join();
      } else {
        exitAfterThread_mtx.unlock();
      }

      waitMutex(&exitGoingThread_mtx);
      if (exitGoingThread == true) {
        exitGoingThread = false;
        exitGoingThread_mtx.unlock();
        while (goingThread.joinable() == false) {
        };
        goingThread.join();
      } else {
        exitGoingThread_mtx.unlock();
      }

      if (watchdogNextStep == After) {
        log("After thread was requested");
        waitMutex(&sleep_mtx);
        sleepJob = After;
        sleep_mtx.unlock();
        if (afterThread.joinable() == true) {
          afterThread.join();
        }
        afterThread = thread(afterSleep);
      } else if (watchdogNextStep == GoingSleep) {
        log("Going sleep thread was requested");
        waitMutex(&sleep_mtx);
        sleepJob = GoingSleep;
        sleep_mtx.unlock();
        if (goingThread.joinable() == true) {
          goingThread.join();
        }
        goingThread = thread(goSleep);
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

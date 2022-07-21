#include "prepareSleep.h"
#include "AppsFreeze.h"
#include "Wifi.h"
#include "cinematicBrightness.h"
#include "fbink.h"
#include "fbinkFunctions.h"
#include "functions.h"
#include "pipeHandler.h"

#include <exception>
#include <mutex>
#include <string>
#include <thread>

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern FBInkDump dump;

extern sleepBool watchdogNextStep;

extern bool darkmode;

extern bool exitPreparethread;
extern mutex exitPreparethread_mtx;

// Explanation why this code looks garbage
// threads in cpp cant be killed from outside, so its needed to check every step
// for a variable change. -Use another library! no. thats such a simple program
// that it doesnt need it + other libraries do the same, for example
// boost::thread does exactly what i have described above, just in the
// background ~Szybet

// checkExitPrepare
void CEP() {
  waitMutex(&sleep_mtx);
  if (sleepJob != Prepare) {
    sleep_mtx.unlock();
    waitMutex(&exitPreparethread_mtx);
    if (exitPreparethread == true) {
      // will never happen.
      log("HOW, JUST HOW? this happened in aftersleep CEA");
      exit(-1);
    } else {
      exitPreparethread = true;
      exitPreparethread_mtx.unlock();
      // wait until it can die. safely.
      log("Entering loop for waiting to die: preparesleep");
      while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        waitMutex(&exitPreparethread_mtx);
        if (exitPreparethread == false) {
          exitPreparethread_mtx.unlock();
          // just to be sureee
          std::this_thread::sleep_for(std::chrono::milliseconds(40));
          terminate();
        } else {
          exitPreparethread_mtx.unlock();
        }
      }
    }
  } else {
    sleep_mtx.unlock();
  }
}

void prepareSleep() {
  log("Launching prepareSleep");
  CEP();
  screenshotFbink();
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  CEP();
  sleepPipeSend();
  CEP();
  // for every app to go to sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  CEP();
  freezeApps();
  CEP();
  clearScreen(darkmode);
  sleepScreen();
  CEP();
  writeFileString("/tmp/sleep_standby", "true");
  writeFileString("/tmp/sleep_mode", "true");

  CEP();
  saveBrightness(getBrightness());
  setBrightnessCin(0, getBrightness());
  CEP();
  turnOffWifi();
  CEP();
  writeFileString("/kobo/inkbox/remount", "false");

  system("/sbin/hwclock --systohc -u"); // why not?
  watchdogNextStep = GoingSleep;
  log("Exiting prepareSleep");
}

// Show a text Sleeping, but also enable with a config a screensaver, and
// writing Sleeping anyway with background
void sleepScreen() {
  // printImage("/image.jpg");
  fbinkWriteCenter("Sleeping", darkmode);
}

// first send a message to a fifo pipe for qt sleeping next read app list from
// /data/config/20-sleep_daemon and freeze them. user apps: ???
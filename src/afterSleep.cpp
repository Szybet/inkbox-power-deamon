#include "afterSleep.h"
#include "AppsFreeze.h"
#include "Wifi.h"
#include "cinematicBrightness.h"
#include "devices.h"
#include "fbinkFunctions.h"
#include "functions.h"
#include "pipeHandler.h"
#include "usbnet.h"

#include <chrono>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <mtd/mtd-user.h>
#include <mutex>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

// var

extern bool recconectWifi;

extern bool deepSleep;
extern bool deepSleepPermission;

extern string cpuGovernorToSet;

// idle count, to reset it
extern int countIdle;

//

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern FBInkDump dump;

extern bool darkmode;

extern sleepBool CurrentActiveThread;
extern mutex CurrentActiveThread_mtx;

extern mutex OccupyLed;

// there is no way to stop the threat... so i will use this bool
bool dieAfter;

// Explanation why this code looks garbage
// threads in cpp cant be killed from outside, so its needed to check every step
// for a variable change. -Use another library! no. thats such a simple program
// that it doesnt need it + other libraries do the same, for example
// boost::thread does exactly what i have described above, just in the
// background ~Szybet

// void checkExitAfter()
void CEA() {
  if (dieAfter == false) {
    manageChangeLedState();
    waitMutex(&sleep_mtx);
    if (sleepJob != After) {
      sleep_mtx.unlock();
      log("log: Terminating afterSleep");
      dieAfter = true;
    }
    sleep_mtx.unlock();
  }
}

void afterSleep() {
  log("Launching afterSleep");
  dieAfter = false;
  waitMutex(&OccupyLed);

  // Dont put CEA here to avoid locking deepSleepPermission forever
  returnDeepSleep();

  // very important.
  int fd = open("/sys/power/state-extended", O_RDWR);
  write(fd, "0", 1);
  close(fd);

  CEA();
  if (dieAfter == false) {
    writeFileString("/tmp/sleep_mode", "false");
    // initFbink();
    restoreFbDepth();
  }

  CEA();
  if (dieAfter == false) {
    system("/sbin/hwclock --hctosys -u");
    clearScreen(darkmode);
  }

  // how do i manage the lockscreen? the apps are freezen now
  // Don't

  CEA();
  if (dieAfter == false) {
    unfreezeApps();
    std::this_thread::sleep_for(std::chrono::milliseconds(650));
    restorePipeSend();
  }

  CEA();
  if (dieAfter == false) {
    restoreFbink(darkmode);
  }

  CEA();
  if (dieAfter == false) {
    setBrightnessCin(restoreBrightness(), 0);
    remove("/tmp/savedBrightness");
  }

  if (recconectWifi == true) {
    log("Recconecting to wifi becouse of 5-WifiRecconect");
    CEA();
    if (dieAfter == false) {
      turnOnWifi();
    }
  } else {
    log("Not Recconecting to wifi becouse of 5-WifiRecconect");
  }

  CEA();
  if (dieAfter == false) {
    startUsbNet();
  }

  CEA();
  if (dieAfter == false) {
    waitMutex(&sleep_mtx);
    sleepJob = Nothing;
    sleep_mtx.unlock();
  }

  OccupyLed.unlock();
  waitMutex(&CurrentActiveThread_mtx);
  CurrentActiveThread = Nothing;
  CurrentActiveThread_mtx.unlock();
  countIdle = 0;
  log("Exiting afterSleep");
}

void returnDeepSleep() {
  log("Returning from deep sleep");
  if (deepSleep == true) {
    remove("/data/config/20-sleep_daemon/SleepCall");
    setCpuGovernor(cpuGovernorToSet);
  }
  deepSleep = false;
  deepSleepPermission = true;
}
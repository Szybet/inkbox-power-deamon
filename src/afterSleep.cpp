#include "afterSleep.h"
#include "AppsFreeze.h"
#include "Wifi.h"
#include "cinematicBrightness.h"
#include "fbinkFunctions.h"
#include "functions.h"

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

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern bool darkmode;

// Explanation why this code looks garbage
// threads in cpp cant be killed from outside, so its needed to check every step
// for a variable change. -Use another library! no. thats such a simple program
// that it doesnt need it + other libraries do the same, for example
// boost::thread does exactly what i have described above, just in the
// background ~Szybet

// void checkExitAfter()
void CEA() {
  waitMutex(&sleep_mtx);
  if (sleepJob != After) {
    sleep_mtx.unlock();
    log("Terminating afterSleep");
    terminate();
  }
  sleep_mtx.unlock();
}

void afterSleep() {
  log("Launching afterSleep");
  // very important.
  int fd = open("/sys/power/state-extended", O_RDWR);
  write(fd, "0", 1);
  close(fd);

  CEA();
  writeFileString("/tmp/sleep_mode", "false");

  initFbink();

  if (darkmode == true) {
    clearScreen(true);
  } else {
    clearScreen(false);
  }

  CEA();

  // how do i manage the lockscreen? the apps are freezen now
  // Don't

  restoreFbink();
  CEA();
  unfreezeApps();
  CEA();
  setBrightnessCin(restoreBrightness(), 0);
  remove("/tmp/savedBrightness");

  CEA();
  // the longest step, propably
  turnOnWifi();
  CEA();

  waitMutex(&sleep_mtx);
  sleepJob = Nothing;
  sleep_mtx.unlock();
  log("Exiting afterSleep");
}
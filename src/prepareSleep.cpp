#include "prepareSleep.h"
#include "cinematicBrightness.h"
#include "fbinkFunctions.h"
#include "functions.h"
#include "fbink.h"
#include "AppsFreeze.h"
#include "Wifi.h"

#include <exception>
#include <mutex>
#include <string>
#include <thread>

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern FBInkDump dump;

extern sleepBool watchdogNextStep;

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
    log("Terminating prepareSleep");
    terminate();
  }
  sleep_mtx.unlock();
}

void prepareSleep() {
  log("Launching prepareSleep");
  CEP();
  screenshotFbink();
  CEP();
  clearScreen(false);
  sleepScreen();
  CEP();
  writeFileString("/tmp/sleep_standby", "true");
  writeFileString("/tmp/sleep_mode", "true");
  freezeApps();
  CEP();
  setBrightnessCin(0, getBrightness());
  saveBrightness(0);
  CEP();
  // actually we should check if modules of wifi are actually loaded before this
  turnOffWifi();
  CEP();
  writeFileString("/kobo/inkbox/remount", "false");
  watchdogNextStep = GoingSleep;
  log("Exiting prepareSleep");
}

// Show a text Sleeping, but also enable with a config a screensaver, and
// writing Sleeping anyway with background
void sleepScreen() {
  //printImage("/image.jpg");
  fbinkWriteCenter("Sleeping");
}

// first send a message to a fifo pipe for qt sleeping next read app list from
// /data/config/20-sleep_daemon and freeze them. user apps: ???
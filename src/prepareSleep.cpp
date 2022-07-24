#include "prepareSleep.h"
#include "AppsFreeze.h"
#include "Wifi.h"
#include "cinematicBrightness.h"
#include "devices.h"
#include "fbink.h"
#include "fbinkFunctions.h"
#include "functions.h"
#include "pipeHandler.h"
#include "usbnet.h"

#include <cstdlib>
#include <exception>
#include <experimental/filesystem>
#include <iostream>
#include <mutex>
#include <stdlib.h>
#include <string>
#include <thread>

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern FBInkDump dump;

extern sleepBool watchdogNextStep;

extern bool darkmode;

extern sleepBool CurrentActiveThread;
extern mutex CurrentActiveThread_mtx;

extern mutex OccupyLed;

extern bool deepSleep;

// there is no way to stop the threat... so i will use this bool
bool diePrepare;

// Explanation why this code looks garbage
// threads in cpp cant be killed from outside, so its needed to check every step
// for a variable change. -Use another library! no. thats such a simple program
// that it doesnt need it + other libraries do the same, for example
// boost::thread does exactly what i have described above, just in the
// background ~Szybet

// checkExitPrepare
void CEP() {
  if (diePrepare == false) {
    manageChangeLedState();
    waitMutex(&sleep_mtx);
    if (sleepJob != Prepare) {
      sleep_mtx.unlock();
      log("log: Terminating prepareSleep");
      diePrepare = true;
    }
    sleep_mtx.unlock();
  }
}

void prepareSleep() {
  waitMutex(&OccupyLed);
  log("Launching prepareSleep");
  diePrepare = false;

  CEP();
  if (diePrepare == false) {
    screenshotFbink();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }

  CEP();
  if (diePrepare == false) {
    sleepPipeSend();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  CEP();
  if (diePrepare == false) {
    freezeApps();
  }

  CEP();
  if (diePrepare == false) {
    clearScreen(darkmode);
    sleepScreen();
  }

  CEP();
  if (diePrepare == false) {
    writeFileString("/tmp/sleep_standby", "true");
    writeFileString("/tmp/sleep_mode", "true");
  }

  CEP();
  if (diePrepare == false) {
    saveBrightness(getBrightness());
    setBrightnessCin(0, getBrightness());
  }

  CEP();
  if (diePrepare == false) {
    turnOffWifi();
  }

  CEP();
  if (diePrepare == false) {
    writeFileString("/kobo/inkbox/remount", "false");
    system("/sbin/hwclock --systohc -u"); // why not?
  }

  CEP();
  if (diePrepare == false) {
    disableUsbNet();
  }

  CEP();
  if (diePrepare == false) {
    watchdogNextStep = GoingSleep;
  }

  CEP();
  if (diePrepare == false) {
    if (deepSleep == true) {
      deepSleepGo();
    }
  }

  // and yes, this will be always executed
  OccupyLed.unlock();

  waitMutex(&CurrentActiveThread_mtx);
  CurrentActiveThread = Nothing;
  CurrentActiveThread_mtx.unlock();
  log("Exiting prepareSleep");
}

// Show a text Sleeping, but also enable with a config a screensaver, and
// writing Sleeping anyway with background
void sleepScreen() {
  // printImage("/image.jpg");
  string screenSaverPath = "/data/onboard/.screensaver";
  if (dirExists(screenSaverPath) == true) {
    vector<string> imageList;
    for (const auto &entry :
         experimental::filesystem::directory_iterator(screenSaverPath)) {
      if (string(entry.path()).find(".png") != std::string::npos) {
        imageList.push_back(string(entry.path()));
        log("Found screensaver image: " + string(entry.path()));
      }
    }
    if (imageList.empty() == false) {
      string choosedScreensaver = imageList.at(rand() % imageList.size());
      if (fileExists(choosedScreensaver) == true) {
        log("Writing image to fbink: " + choosedScreensaver);
        int status = printImage(choosedScreensaver);
        if (status < 0) {
          log("Failed to decode the screensaver image, propably");
          fbinkWriteCenter("Sleeping", darkmode);
        }
        return;
      }
    }
  }
  log("Something went wrong with screensaver, writing normal message");
  fbinkWriteCenter("Sleeping", darkmode);
}

void deepSleepGo() {
  log("Going to deep sleep");
  // cpu governor power save
  setCpuGovernor("powersave");

  // for now only this
}
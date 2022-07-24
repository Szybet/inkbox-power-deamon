#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio.hpp>
#include <cerrno>
#include <exception>
#include <experimental/filesystem>
#include <fcntl.h>
#include <iostream>
#include <mtd/mtd-user.h>
#include <sstream>
#include <stdio.h>
#include <streambuf>
#include <string>
#include <sys/ioctl.h>
#include <sys/klog.h>
#include <thread>

#include "fbinkFunctions.h"
#include "functions.h"
#include "goingSleep.h"
#include "devices.h"

// var

// 4-ChargerWakeUp
extern bool ChargerWakeUp;
bool savedChargerState;

//

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern sleepBool CurrentActiveThread;
extern mutex CurrentActiveThread_mtx;

extern sleepBool watchdogNextStep;

extern mutex OccupyLed;

// there is no way to stop the threat... so i will use this bool
bool dieGoing;

// Some notes
/*

to be able to go so fast in time without delays, before setting 1 to
state-extended, there needs to be a 0. In other words, if there is a 1 full time
it wont work. for the nia it works like that. adjust it for other devices if it
doesnt work

also code explanation:
this thread doesnt set watchdogNextStep or sleepJob becouse monitor events sends
a signal after being waked up, so watchdog knows what to do;

*/

// checkExitGoing
void CEG() {
  if (dieGoing == false) {
    manageChangeLedState();
    waitMutex(&sleep_mtx);
    if (sleepJob != GoingSleep) {
      sleep_mtx.unlock();
      log("log: Terminating goSleep");
      dieGoing = true;
    }
    sleep_mtx.unlock();
  }
}

void goSleep() {
  log("Started goSleep");
  dieGoing = false;
  waitMutex(&OccupyLed);

  system("/bin/sync");

  CEG();
  if (dieGoing == false) {
    int fd = open("/sys/power/state-extended", O_RDWR);
    write(fd, "1", 1);
    close(fd);
  }

  smartWait(1000);

  CEG();
  log("Going to sleep now!");
  smartWait(1000);

  bool continueSleeping = true;
  int count = 0;
  while (continueSleeping == true and dieGoing == false) {
    // 4-ChargerWakeUp. Actually we need this variable anyway becouse to know if
    // to wakeup the device after it or not
    // if (ChargerWakeUp == true) {
    savedChargerState = getChargerStatus();
    // }

    // https://linux.die.net/man/3/klogctl
    klogctl(5, NULL, 0);

    // Manage the led here
    OccupyLed.unlock();
    ledManager();
    waitMutex(&OccupyLed);

    log("Trying sleep");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int fd2 = open("/sys/power/state", O_RDWR);
    int status = write(fd2, "mem", 3);
    close(fd2);

    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 500

    log("Got back from suspend");

    // get dmesg, and then only lines containing <3>
    char *logs_data;
    ssize_t len = klogctl(10, NULL, 0);
    logs_data = (char *)malloc(len);
    klogctl(3, logs_data, len);
    vector<string> dmesgErrorsVec;
    boost::split(dmesgErrorsVec, logs_data, boost::is_any_of("\n"),
                 boost::token_compress_on);

    // to show whole dmesg
    // log("dmesg: " + (string)logs_data);

    free(logs_data);
    string dmesgErrors;
    for (string line : dmesgErrorsVec) {
      if (line.find("<3>") != std::string::npos) {
        // tesdt
        dmesgErrors.append(line);
        dmesgErrors.append("\n");
      }
    }
    dmesgErrorsVec.clear();
    if (status == -1 or
        dmesgErrors.find("PM: Some devices failed to suspend") !=
            std::string::npos) {
      log("Failed to suspend, dmesg errors:\n" + dmesgErrors);
      log("\nstatus of writing to /sys/power/state: " + to_string(status));
      CEG();
      count = count + 1;
      if (count == 5) {
        log("5 failed attemts at suspending, sleep a little longer...");
        smartWait(10000);
      } else if (count == 15) {
        log("15 failed attempts at sleeping...");
        // Write to fbink here a sad message
      } else {
        smartWait(3000);
      }
    } else {
      // Exiting this sleeping hell
      log("Sleeping finished. Tryied going to sleep " + to_string(count) +
          " times");
      continueSleeping = false;

      // 4-ChargerWakeUp
      if (savedChargerState != getChargerStatus()) {
        if (ChargerWakeUp == true) {
          log("4-ChargerWakeUp option is enabled, and the charger state is "
              "diffrent. going to sleep one more time");
          count = 0;
          continueSleeping = true;
        } else {
          // Becouse the charger doesnt trigger a anything in monitorEvents
          log("The device waked up becouse of a charger, but 4-ChargerWakeUp "
              "is disabled so it will continue to wake up");
        }
      }
    }
  }

  OccupyLed.unlock();
  watchdogNextStep = After;
  waitMutex(&CurrentActiveThread_mtx);
  CurrentActiveThread = Nothing;
  CurrentActiveThread_mtx.unlock();
  log("Exiting going to sleep");
}

// Sometimes i regret using such a simple multi threading, but then i remember
// that this is safe
void smartWait(int timeToWait) {
  int time = timeToWait / 20;
  int count = 0;
  while (count < 20 and dieGoing == false) {
    count = count + 1;
    // Now that im thinking, the ceg isin't here needed that much. but the LED flickers more so its cool
    CEG();
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
  }
}
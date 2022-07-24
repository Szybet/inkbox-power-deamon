#include "configUpdate.h"
#include "functions.h"

//

#include <iostream>

// https://developer.ibm.com/tutorials/l-ubuntu-inotify/
#include "sys/inotify.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// https://pubs.opengroup.org/onlinepubs/009604599/functions/read.html
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

extern bool watchdogStartJob;
extern mutex watchdogStartJob_mtx;

extern goSleepCondition newSleepCondition;
extern mutex newSleepCondition_mtx;

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern sleepBool CurrentActiveThread;
extern mutex CurrentActiveThread_mtx;

extern bool deepSleep;
extern bool deepSleepPermission;

// this isin't the best implementation, and i dont understand some things

void startMonitoringConfig() {
  log("Starting monitoring for config updates");

  int fd;
  int wd;
  char buffer[BUF_LEN];

  fd = inotify_init();
  if (fd < 0) {
    log("inotify_init failed (old kernel?)");
  }

  wd = inotify_add_watch(fd, "/data/config/20-sleep_daemon",
                         IN_MODIFY | IN_CREATE | IN_DELETE);

  while (true) {
    int length, i = 0;
    length = read(fd, buffer, BUF_LEN);
    log("inotify readed up");

    if (length < 0) {
      log("failed to read from buffer");
    }

    /*
    while ( i < length ) {
        struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
        if(event->len) {
            if ( event->mask & IN_MODIFY ) {
                log("/tmp/power was modified");
            }
        }
    }
    */
    // this loop goes through all changes
    while (i < length) {
      log("Inotify loop executed");
      struct inotify_event *event = (struct inotify_event *)&buffer[i];
      if (event->len) {
        if (event->mask & IN_CREATE) {
          string evenNameString = event->name;
          log("Inotify: detected a create event of name: " + evenNameString);

          if (evenNameString == "updateConfig") {
            checkUpdateFile();
          }
          if (evenNameString == "SleepCall") {
            sleepInotifyCall();
          }
        } else if (event->mask & IN_DELETE) {
          string message = "What are you doing? this file / dir was deleted:";
          message.append(event->name);
          log(message);
        } else if (event->mask & IN_MODIFY) {
          string evenNameString = event->name;
          log("Inotify: detected a modification event of name: " +
              evenNameString);
          if (evenNameString == "updateConfig") {
            checkUpdateFile();
          }
          if (evenNameString == "SleepCall") {
            sleepInotifyCall();
          }
        }
      }
      i += EVENT_SIZE + event->len;
    }
    log("All events readed");
  }
  log("inotify crashed");
  (void)inotify_rm_watch(fd, wd);
  (void)close(fd);
}

void checkUpdateFile() {
  if (readConfigString("/data/config/20-sleep_daemon/updateConfig") == "true") {
    log("Updating config for request");
    prepareVariables();
    writeFileString("/data/config/20-sleep_daemon/updateConfig", "false");
  } else {
    log("updateConfig is false, not updating anything");
  }
}

void sleepInotifyCall() {
  log("sleepInotifyCall() called, going to sleep, propably");
  if (deepSleepPermission == true) {
    string deepSleepFile =
        readConfigString("/data/config/20-sleep_daemon/SleepCall");
    bool go = false;
    if (deepSleepFile == "deepsleep") {
      deepSleep = true;
      go = true;
    } else if (deepSleepFile == "sleep") {
      go = true;
    }
    if (go == true) {
      deepSleepPermission = false;
      log("Going to sleep becouse of inotify call");
      CurrentActiveThread_mtx.unlock();

      waitMutex(&watchdogStartJob_mtx);
      watchdogStartJob = true;
      watchdogStartJob_mtx.unlock();

      waitMutex(&newSleepCondition_mtx);
      newSleepCondition = Inotify;
      newSleepCondition_mtx.unlock();
    }
  }
}
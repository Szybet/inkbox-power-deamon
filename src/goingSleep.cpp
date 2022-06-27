#include "goingSleep.h"
#include "functions.h"
#include <fcntl.h>
#include <mtd/mtd-user.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

#include "fbinkFunctions.h"

extern sleepBool sleepJob;
extern mutex sleep_mtx;

extern int fbfd; // not sure if needed

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
  waitMutex(&sleep_mtx);
  if (sleepJob != GoingSleep) {
    sleep_mtx.unlock();
    log("Terminating goSleep");
    terminate();
  }
  sleep_mtx.unlock();
}

void goSleep() {
  log("Going to sleep now!");
  CEG();
  closeFbink();
  CEG();

  system("/bin/sync");
  std::this_thread::sleep_for(std::chrono::milliseconds(4500));

  int fd = open("/sys/power/state-extended", O_RDWR);
  write(fd, "1", 1);
  close(fd);

  std::this_thread::sleep_for(std::chrono::milliseconds(4500));

  // checking here CEG(); breaks the fast time thingy, dont add it to nia

  int fd2 = open("/sys/power/state", O_RDWR);
  int sleepStatus = write(fd2, "mem", 3);
  close(fd2);
  std::this_thread::sleep_for(std::chrono::milliseconds(750));
  if (sleepStatus < 0) {
    log("Failed to go to sleep");
  }

  log("Sleep finished, Exiting going to sleep");
}
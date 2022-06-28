#include "goingSleep.h"
#include "functions.h"
#include <boost/asio.hpp>
#include <fcntl.h>
#include <iostream>
#include <mtd/mtd-user.h>
#include <sstream>
#include <stdio.h>
#include <streambuf>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

// not all of this is propabyl needed
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

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
  // closeFbink();
  // CEG();

  system("/bin/sync");
  std::this_thread::sleep_for(std::chrono::milliseconds(9000));

  int fd = open("/sys/power/state-extended", O_RDWR);
  write(fd, "1", 1);
  close(fd);

  std::this_thread::sleep_for(std::chrono::milliseconds(4000));

  // checking here CEG(); breaks the fast time thingy, dont add it to nia

  CEG();
  std::this_thread::sleep_for(std::chrono::milliseconds(4000));

  // https://stackoverflow.com/questions/2513312/nunit-does-not-capture-output-of-stdcerr
  // https://itecnote.com/tecnote/c-copy-a-streambufs-contents-to-a-string/
  // https://stackoverflow.com/questions/10150468/how-to-redirect-cin-and-cout-to-files
  // https://stackoverflow.com/questions/877652/copy-a-streambufs-contents-to-a-string
  /*
  streambuf *buf = std::cerr.rdbuf();
  std::istream is(buf);
  std::string line;
  std::getline(is, line);
  */
  // I have tryied many things. nothing worked
  // https://stackoverflow.com/questions/45892551/c-program-of-accessing-device-file-dont-work

  char byte[1000];
  int stderrdevice = open("/dev/stderr", O_RDONLY | O_NOCTTY);

  int fd2 = open("/sys/power/state", O_RDWR);
  write(fd2, "mem", 3);

  close(fd2);

  ssize_t size = read(stderrdevice, &byte, 1000);

  printf("Read byte %s\n", byte);

  log("Sleep finished, Exiting going to sleep");
}
#include "cinematicBrightness.h"
#include "functions.h"
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <cstdlib>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern string model;

extern int CinematicBrightnessdelayMs;

void setBrightnessCin(int levelToSet, int currentLevel) {
  int device;
  if ((device = open("/dev/ntx_io", O_RDWR)) == -1) {
    log("Error on opening ntx device");
    exit(EXIT_FAILURE);
  }
  chrono::milliseconds timespan(CinematicBrightnessdelayMs);
  while (currentLevel != levelToSet) {
    if (currentLevel < levelToSet) {
      currentLevel = currentLevel + 1;
    } else {
      currentLevel = currentLevel - 1;
    }

    setBrightness(device, currentLevel);
    this_thread::sleep_for(timespan);
  }
  close(device);
}

void saveBrightness(int level) {
  writeFileString("/data/config/03-brightness/config", to_string(level));
  writeFileString("/tmp/savedBrightness", to_string(level));
}

int restoreBrightness() {
  return stoi(readConfigString("/tmp/savedBrightness"));
}

void setBrightness(int device, int level) { ioctl(device, 241, level); }

// bugs?
int getBrightness() {
  if (model == "n613") {
    return stoi(readConfigString("/opt/config/03-brightness/config"));
  } else if (model == "n236" or model == "n437") {
    return stoi(
        readConfigString("/sys/class/backlight/mxc_msp430_fl.0/brightness"));
  } else {
    return stoi(readConfigString(
        "/sys/class/backlight/mxc_msp430.0/actual_brightness"));
  }
}

/*
sleep 1
if [ "${DEVICE}" != "n613" ]; then
        # /tmp/savedBrightness is deleted in after_sleep.sh. The if is for
avoiding restoring wrong brightness in after_sleep.sh if [[ ! -f
"/tmp/savedBrightness" ]]; then CURRENT_BRIGHTNESS=$(cat
/kobo/var/run/brightness) echo "${CURRENT_BRIGHTNESS}" > /tmp/savedBrightness fi
else
        if [[ ! -f "/tmp/savedBrightness" ]]; then
                CURRENT_BRIGHTNESS=$(cat /opt/config/03-brightness/config)
                echo "${CURRENT_BRIGHTNESS}" > /tmp/savedBrightness
        fi
fi
/opt/bin/cinematic-brightness.sh 0 1

        if [ "${DEVICE}" == "n613" ]; then
                BRIGHTNESS=$(cat /opt/config/03-brightness/config)
        elif [ "${DEVICE}" == "n236" ] || [ "${DEVICE}" == "n437" ]; then
                BRIGHTNESS=$(cat
/sys/class/backlight/mxc_msp430_fl.0/brightness) else BRIGHTNESS=$(cat
/sys/class/backlight/mxc_msp430.0/brightness) fi

*/
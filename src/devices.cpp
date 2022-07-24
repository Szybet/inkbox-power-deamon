#include "devices.h"
#include "functions.h"
#include <fcntl.h>
#include <unistd.h>

extern string model;

extern bool LedUsage;

extern mutex OccupyLed;

extern bool ledstate;

// yes, simply and clear
void manageChangeLedState() {
  if (LedUsage == true) {
    changeLedState();
  }
}

void changeLedState() {
  if (model == "n306") {
    // here its checking the real one, if some other app wants to control the
    // led, fine
    string path = "/sys/devices/platform/leds/leds/GLED/brightness";
    string state = readConfigStringNoLog(path);
    int dev = open(path.c_str(), O_RDWR);
    if (state == "1") {
      write(dev, "0", 1);
      ledstate = 0;
    } else {
      write(dev, "1", 1);
      ledstate = 1;
    }
    close(dev);
  }
}

void setLedState(bool on) {
  if (model == "n306") {
    string path = "/sys/devices/platform/leds/leds/GLED/brightness";
    int dev = open(path.c_str(), O_RDWR);
    if (on == true) {
      write(dev, "1", 1);
      ledstate = 1;
    } else {
      write(dev, "0", 1);
      ledstate = 0;
    }
    close(dev);
  }
}

void ledManager() {
  if (LedUsage == true) {
    if (OccupyLed.try_lock() == true) {
      OccupyLed.unlock();
      if (getAccurateChargerStatus() == true) {
        if (ledstate == 0) {
          setLedState(true);
        }
      } else {
        if (ledstate == 1) {
          setLedState(false);
        }
      }
    }
  }
}

bool getChargerStatus() {
  string chargerStatus;
  if (model == "kt") {
    chargerStatus = readFile(
        "/sys/devices/system/yoshi_battery/yoshi_battery0/battery_status");
  } else {
    chargerStatus = readFile(
        "/sys/devices/platform/pmic_battery.1/power_supply/mc13892_bat/status");
  }
  chargerStatus = normalReplace(chargerStatus, "\n", "");
  if (chargerStatus == "Discharging") {
    // log("So the device is not charging");
    return false;
  } else {
    // log("So the device is charging");
    return true;
  }
}

bool isDeviceChargerBug() {
  if (model == "n905c" or model == "n905b" or model == "n705" or
      model == "n613" or model == "n236" or model == "kt") {
    return true;
  } else {
    return false;
  }
}

/*
Let me explain this function
there are 3 states ( for kobo nia though )
1 Charging
2 Discharging
3 Not charging

getChargerStatus() connects 2 and 3 in one, and its fine for some functions and
applications when we need to know if the charger is still connected

but now we need to know if the diode should be on or off

so here we are connecting 2 and 3

*/
bool getAccurateChargerStatus() {
  string chargerStatus;
  if (model == "kt") {
    chargerStatus = readFile(
        "/sys/devices/system/yoshi_battery/yoshi_battery0/battery_status");
  } else {
    chargerStatus = readFile(
        "/sys/devices/platform/pmic_battery.1/power_supply/mc13892_bat/status");
  }
  chargerStatus = normalReplace(chargerStatus, "\n", "");
  // log("charger status is: " + chargerStatus);
  if (chargerStatus == "Discharging" or chargerStatus == "Not charging") {
    // log("So the device is not charging, or is charged in 100%");
    return false;
  } else {
    // log("So the device is charging");
    return true;
  }
}

void setCpuGovernor(string cpuGovernor) {
  log("Setting cpu freq governor to " + cpuGovernor);
  int dev =
      open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", O_RDWR);
  int writeStatus =
      write(dev, cpuGovernor.c_str(), cpuGovernor.length());
  close(dev);
  log("Write status writing to scaling_governor is: " +
      std::to_string(writeStatus));
}
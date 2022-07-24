#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <experimental/filesystem>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <regex>

#include "devices.h"
#include "fbink.h"
#include "functions.h"

using namespace std;

// Variables ( that there is no risk that they will be readed at the same tame
// by many threads ). Used by fbink, internal things

bool logEnabled = false;

string model;

int fbfd;

FBInkDump dump;

vector<int> AppsPids;

bool wasUsbNetOn;

// Config var

int CinematicBrightnessdelayMs;

bool lockscreen;

bool darkmode;

string cpuGovernorToSet;

bool WhenChargerSleep;

bool ChargerWakeUp;

bool recconectWifi;

bool LedUsage;

int idleSleepTime;

bool customCase;
// When 2 it resets to 0 and triggers the watchdog. so 1 is ignored
int customCaseCount = 0;

bool deepSleep;
bool deepSleepPermission = true; // Becouse inotify is weird, if false it will ignore the call. its called true in after sleep

// Internal variables used by watchdog and threads

// im not sure if this one doesnt need a mutex. will leave it for now
sleepBool watchdogNextStep = Nothing;

// Mutex variables

bool watchdogStartJob = false;
mutex watchdogStartJob_mtx;

goSleepCondition newSleepCondition = None;
mutex newSleepCondition_mtx;

sleepBool sleepJob = Nothing;
mutex sleep_mtx;

sleepBool CurrentActiveThread;
mutex CurrentActiveThread_mtx;

// To avoid confusion with the led control ( if this is locked, then it other
// things are done beside charging indicator, like flickering)
mutex OccupyLed;

// Avoid writing to the led if not needed
bool ledstate = false;

// Count for idle
int countIdle = 0;

//

void log(string to_log) {
  if (logEnabled == true) {
    std::cout << to_log << std::endl;

    // I wonder if its better to not close it every time
    ofstream logFile("/tmp/PowerDaemonLogs.txt", ios::app);
    logFile << to_log << std::endl;
    logFile.close();
  }
}

void waitMutex(mutex *exampleMutex) {
  // Ok, now here look how stupid i was when i started writing this
  /*
  bool continueBool = false;
  std::chrono::milliseconds timespan(150);

  while (continueBool == false) {
    // https://en.cppreference.com/w/cpp/thread/mutex/try_lock
    if (exampleMutex->try_lock() == false) {
      std::this_thread::sleep_for(timespan);
    } else {
      continueBool = true;
    }
  }
  */
  // All this can be, and should be made like this:
  exampleMutex->lock();
  // now this function doesnt have any sense, maybe looks better
  // could be replaced in the future, but maybe adding some logging which thread
  // occupies which mutex would be cool leave for now
}

void prepareVariables() {
  log("Reading variables");
  model = readConfigString("/opt/inkbox_device");
  log("Running on: " + model);

  // Lockscreen
  string stringRead1 = readConfigString("/opt/config/12-lockscreen/config");
  if (stringRead1 == "true") {
    lockscreen = true;
  } else {
    lockscreen = false;
  }
  log("lockscreen is: " + stringRead1);

  // Simply it, for fbink dump
  dump = {0};

  // dark mode
  string stringRead2 = readConfigString("/opt/config/10-dark_mode/config");
  if (stringRead2 == "true") {
    log("darkmode is: true");
    darkmode = true;
  } else {
    log("darkmode is: false");
    darkmode = false;
  }

  // usb net
  string commandOutput = executeCommand("service usbnet status");
  if (commandOutput.find("status: started") != std::string::npos) {
    log("Usb net is started");
    wasUsbNetOn = true;
  } else {
    log("Usb net isin't started");
    wasUsbNetOn = false;
  }

  // Specific daemon configs:
  // in the future set it through config file

  // /data/config/20-sleep_daemon
  string mainPath = "/data/config/20-sleep_daemon";
  if (dirExists(mainPath) == false) {
    log("Creating basic config");
    experimental::filesystem::create_directory(mainPath);
    // /data/config/20-sleep_daemon/appList.txt
    writeFileString("/data/config/20-sleep_daemon/appList.txt",
                    "inkbox-bin\noobe-inkbox-bin\nlockscreen-bin\ncalculator-"
                    "bin\nqreversi-bin\n2048-bin\nscribble\nlightmaps\nexec");
    writeFileString("/data/config/20-sleep_daemon/updateConfig", "false");
  }

  // 1-CinematicBrightnessdelayMs
  string cinematicPath =
      "/data/config/20-sleep_daemon/1-CinematicBrightnessdelayMs";
  if (fileExists(cinematicPath) == true) {
    CinematicBrightnessdelayMs = stoi(readConfigString(cinematicPath));
  } else {
    writeFileString(cinematicPath, "50");
    CinematicBrightnessdelayMs = 50;
  }

  // 2-cpuGovernor
  string cpuGovernorPath = "/data/config/20-sleep_daemon/2-cpuGovernor";
  if (fileExists(cpuGovernorPath) == true) {
    cpuGovernorToSet = readConfigString(cpuGovernorPath);
    setCpuGovernor(cpuGovernorToSet);
  } else {
    writeFileString(cpuGovernorPath, "ondemand");
    cpuGovernorToSet = "ondemand";
  }

  // 3-WhenChargerSleep
  string WhenChargerSleepPath =
      "/data/config/20-sleep_daemon/3-WhenChargerSleep";
  if (fileExists(WhenChargerSleepPath) == true) {
    string boolToConvert = readConfigString(WhenChargerSleepPath);
    if (boolToConvert == "true") {
      WhenChargerSleep = true;
    } else {
      WhenChargerSleep = false;
    }
  } else {
    // those devices have problems when going to sleep with a charger
    if (isDeviceChargerBug() == true) {
      writeFileString(WhenChargerSleepPath, "false");
      WhenChargerSleep = false;
    } else {
      writeFileString(WhenChargerSleepPath, "true");
      WhenChargerSleep = true;
    }
  }

  // 4-ChargerWakeUp
  string ChargerWakeUpPath = "/data/config/20-sleep_daemon/4-ChargerWakeUp";
  if (fileExists(ChargerWakeUpPath) == true) {
    // welp a function for this would be cool
    string boolToConvert = readConfigString(ChargerWakeUpPath);
    if (boolToConvert == "true") {
      ChargerWakeUp = true;
    } else {
      ChargerWakeUp = false;
    }
  } else {
    writeFileString(ChargerWakeUpPath, "false");
    ChargerWakeUp = false;
  }

  // 5-WifiRecconect
  string recconectWifiPath = "/data/config/20-sleep_daemon/5-WifiRecconect";
  if (fileExists(recconectWifiPath) == true) {
    string boolToConvert = readConfigString(recconectWifiPath);
    if (boolToConvert == "true") {
      recconectWifi = true;
    } else {
      recconectWifi = false;
    }
  } else {
    writeFileString(recconectWifiPath, "true");
    recconectWifi = true;
  }

  // 6-LedUsage
  string LedUsagePath = "/data/config/20-sleep_daemon/6-LedUsage";
  if (fileExists(LedUsagePath) == true) {
    string boolToConvert = readConfigString(LedUsagePath);
    if (boolToConvert == "true") {
      LedUsage = true;
    } else {
      LedUsage = false;
    }
  } else {
    writeFileString(LedUsagePath, "false");
    recconectWifi = false;
  }
  setLedState(false);

  // 7-IdleSleep
  string idleSleepTimePath = "/data/config/20-sleep_daemon/7-IdleSleep";
  if (fileExists(idleSleepTimePath) == true) {
    string intToConvert = readConfigString(idleSleepTimePath);
    intToConvert = normalReplace(intToConvert, "\n", ""); // i like to be sure about some things.
    idleSleepTime = stoi(intToConvert);
  } else {
    writeFileString(idleSleepTimePath, "60");
    idleSleepTime = 60;
  }

  // 8-CustomCase
  string customCasePath = "/data/config/20-sleep_daemon/8-CustomCase";
  if (fileExists(customCasePath) == true) {
    string boolToConvert = readConfigString(customCasePath);
    if (boolToConvert == "true") {
      customCase = true;
    } else {
      customCase = false;
    }
  } else {
    writeFileString(customCasePath, "false");
    customCase = false;
  }

  // 9-DeepSleep
  string DeepSleepPath = "/data/config/20-sleep_daemon/9-DeepSleep";
  if (fileExists(DeepSleepPath) == false) {
    writeFileString(DeepSleepPath, "false");
  }
}

string readConfigString(string path) {
  ifstream indata;
  string returnData;
  indata.open(path);
  if (!indata) {
    log("couldn't read config file: " + path);
    return "none";
  }
  indata >> returnData;
  while (!indata.eof()) {
    indata >> returnData;
  }
  indata.close();
  log(path + " is: " + returnData);
  return returnData;
}

void writeFileString(string path, string stringToWrite) {
  fstream File;
  File.open(path, ios::out);
  if (!File) {
    string message = "File could not be created at path: ";
    message.append(path);
    log(message);
    exit(EXIT_FAILURE);
  } else {
    File << stringToWrite;
    File.close();
    log("Writed: \"" + stringToWrite + "\" to: " + path);
  }
}

string readFile(string path) {
  ifstream input_file(path);
  if (!input_file.is_open()) {
    string message = "Could not open file: ";
    message.append(path);
    log(message);
    exit(EXIT_FAILURE);
  }
  return string((std::istreambuf_iterator<char>(input_file)),
                std::istreambuf_iterator<char>());
}

bool fileExists(string fileName) {
  std::ifstream infile(fileName);
  return infile.good();
}

bool dirExists(string path) {
  struct stat info;

  if (stat(path.c_str(), &info) != 0)
    return false;
  else if (info.st_mode & S_IFDIR)
    return true;
  else
    return false;
}

string executeCommand(string command) {
  char buffer[128]; // must be enough
  string result = "";

  // Open pipe to file
  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    return "popen failed!";
  }

  // read till end of process:
  while (!feof(pipe)) {

    // use buffer to read and add to result
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }

  pclose(pipe);

  log("Output of command: \" " + command + " \"" + "is: \" " + result + " \"");
  return result;
}

// why this isin't a standard
string normalReplace(string MainString, string strToLookFor, string replacement)
{
  return std::regex_replace(MainString, std::regex(strToLookFor), replacement);
}

string readConfigStringNoLog(string path) {
  ifstream indata;
  string returnData;
  indata.open(path);
  if (!indata) {
    log("couldn't read config file: " + path);
    return "none";
  }
  indata >> returnData;
  while (!indata.eof()) {
    indata >> returnData;
  }
  indata.close();
  return returnData;
}
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <experimental/filesystem>
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
#include <vector>

#include "fbink.h"
#include "functions.h"

using namespace std;

// Variables ( that there is no risk that they will be readed at the same tame
// by many threads )

bool logEnabled = false;

string model;
bool lockscreen;

int CinematicBrightnessdelayMs;

int fbfd;

FBInkDump dump;

vector<int> AppsPids;

// im not sure if this one doesnt need a mutex. will leave it for now
sleepBool watchdogNextStep = Nothing;

bool darkmode;

// Mutex variables

bool watchdogStartJob = false;
mutex watchdogStartJob_mtx;

goSleepCondition newSleepCondition = None;
mutex newSleepCondition_mtx;

sleepBool sleepJob = Nothing;
mutex sleep_mtx;

// those mutexes is to check if a thread wants to exit, to call .join before it
bool exitPreparethread = false;
mutex exitPreparethread_mtx;

bool exitGoingThread = false;
mutex exitGoingThread_mtx;

bool exitAfterThread = false;
mutex exitAfterThread_mtx;
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

  // Specific daemon configs:
  // in the future set it through config file

  // /data/config/20-sleep_daemon
  string mainPath = "/data/config/20-sleep_daemon";
  if (dirExists(mainPath) == false) {
    experimental::filesystem::create_directory(mainPath);
    // /data/config/20-sleep_daemon/appList.txt
    writeFileString("/data/config/20-sleep_daemon/appList.txt",
                    "inkbox-bin\noobe-inkbox-bin\nlockscreen-bin\ncalculator-"
                    "bin\nqreversi-bin\n2048-bin\nscribble\nlightmaps\nexec");
    log("Created /data/config/20-sleep_daemon/appList.txt directory and appList.txt in it");
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
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
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

//

void log(string to_log) {
  if (logEnabled == true) {
    std::cout << to_log << std::endl;
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

  string stringRead1 = readConfigString("/opt/config/12-lockscreen/config");
  if (stringRead1 == "true") {
    lockscreen = true;
  } else {
    lockscreen = false;
  }
  log("lockscreen is: " + stringRead1);

  // in the future set it through config file
  CinematicBrightnessdelayMs = 50;

  dump = {0};

  string stringRead2 = readConfigString("/opt/config/12-lockscreen/config");
  if(stringRead2 == "true")
  {
    darkmode = true;
  } else {
    darkmode = false;
  }
}

void ManageConfig() {
  // /data/config/20-sleep_daemon
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

bool is_file_exist(string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}
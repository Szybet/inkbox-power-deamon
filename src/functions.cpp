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

#include "functions.h"

using namespace std;

// Variables ( only readed )

bool logEnabled = false;

string model;
bool lockscreen;

int CinematicBrightnessdelayMs;

int fbfd;

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
    model = readConfigString("/opt/inkbox_device");
    log("Running on: " + model);


    string stringRead = readConfigString("/opt/config/12-lockscreen/config");
    if(stringRead == "true")
    {
        lockscreen = true;
    } else {
        lockscreen = false;
    }
    log("lockscreen is: " + stringRead);

  // in the future set it through config file
  CinematicBrightnessdelayMs = 300;
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

void writeFileString(string path, string stringToWrite)
{
    fstream File;
	File.open(path, ios::out);
	if (!File) {
		log("File not created");
        exit(EXIT_FAILURE);
	}
	else {
		File << stringToWrite;
		File.close();
        log("Writed: \"" + stringToWrite + "\" to: " + path);
	}
}

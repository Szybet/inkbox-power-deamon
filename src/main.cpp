#include <iostream>
#include <pthread.h>
#include <string>
#include <thread>

#include "AppsFreeze.h"
#include "configUpdate.h"
#include "fbinkFunctions.h"
#include "functions.h"
#include "goingSleep.h"
#include "monitorEvents.h"
#include "pipeHandler.h"
#include "watchdog.h"
#include "idleSleep.h"

extern bool logEnabled;
extern int fbfd;

using namespace std;

int main() {
  static_cast<void>(pthread_create);
  static_cast<void>(pthread_cancel);

  std::cout << "Starting..." << std::endl;

  const char *tmp = std::getenv("DEBUG");
  std::string envVar;

  if (tmp != NULL) {
    envVar = tmp;
    if (envVar == "true") {
      logEnabled = true;
      log("Debug mode is activated");
      log("Saving logs to /tmp/PowerDaemonLogs.txt");
    }
  }

  prepareVariables();
  initFbink();
  startPipeServer();

  thread monitorDev(startMonitoringDev);
  thread watchdogThread(startWatchdog);
  thread watchConfig(startMonitoringConfig);
  thread idleSleep(startIdleSleep);

  // https://stackoverflow.com/questions/7381757/c-terminate-called-without-an-active-exception
  monitorDev.join();
  watchdogThread.join();
  watchConfig.join();
  idleSleep.join();
  
  log("How did this ended");
  return -1;
}
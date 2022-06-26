#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <vector>

#include "AppsFreeze.h"
#include "functions.h"

extern vector<int> AppsPids;

// https://ofstack.com/C++/9293/linux-gets-pid-based-on-pid-process-name-and-pid-of-c.html
int getPidByName(string task_name) {
  struct dirent *entry = nullptr;
  DIR *dp = nullptr;

  string proc = "/proc/";
  dp = opendir(proc.c_str());
  while ((entry = readdir(dp))) {
    ifstream file(proc + entry->d_name + "/status");
    string firstLine;
    getline(file, firstLine);
    // https://stackoverflow.com/questions/2340281/check-if-a-string-contains-a-string-in-c
    if (firstLine.find(task_name) != std::string::npos) {
      log("Found pid of " + task_name + ",its: " + entry->d_name);
      return stoi(entry->d_name);
    }
  }
  closedir(dp);
  return EXIT_FAILURE;
}

// /data/config/20-sleep_daemon/appList.txt
vector<string> getBuiltAppsList(string path) {
  vector<string> vectorToReturn;
  vector<string> vectorToParse;

  string configfile = readFile(path);
  log("appList.txt is: " + configfile);
  boost::split(vectorToParse, configfile, boost::is_any_of("\n"),
               boost::token_compress_on);

  for (string &app : vectorToParse) {
    if (app.empty() == false) {
      log("In app vector: " + app);
      vectorToReturn.push_back(app);
    }
  }
  return vectorToReturn;
}

string getRunningUserApp() {
  return readConfigString("/kobo/tmp/currentlyRunningUserApplication");
}

void freezeApps() {
  vector<int> Pidvec;

  vector<string> BuiltInApps =
      getBuiltAppsList("/data/config/20-sleep_daemon/appList.txt");

  for (string &app : BuiltInApps) {
    Pidvec.push_back(getPidByName(app));
  }
  if (is_file_exist("/kobo/tmp/currentlyRunningUserApplication")) {
    Pidvec.push_back(stoi(getRunningUserApp()));
  }

  AppsPids = Pidvec;

  // SIGCONT - continue
  // SIGSTOP - force freeze
  for (int &pid : AppsPids) {
    kill(pid, SIGSTOP);
  }
}

void unfreezeApps() {
  for (int &pid : AppsPids) {
    kill(pid, SIGCONT);
  }
}

void tellAppsToFreeze() {}

void killProcess(string name) { kill(getPidByName(name), 9); }
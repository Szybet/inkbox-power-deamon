#ifndef APPS_FREEZE_H
#define APPS_FREEZE_H

#include <vector>
#include <string>

using namespace std;

vector<string> getBuiltAppsList(string path);

void freezeApps();

string getRunningUserApp();

int getPidByName(string task_name);

void unfreezeApps();

void tellAppsToFreeze();

void killProcess(string name);

#endif
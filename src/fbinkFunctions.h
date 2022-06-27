#ifndef FBINK_FUNCTIONS_H
#define FBINK_FUNCTIONS_H

#include <string>
#include "fbink.h"

using namespace std;

void initFbink();

int fbinkWriteCenter(string stringToWrite);

void clearScreen(bool darkmodeset);

void printImage(string path);

void screenshotFbink();

void restoreFbink();

void closeFbink();

#endif
#ifndef FBINK_FUNCTIONS_H
#define FBINK_FUNCTIONS_H

#include <string>
#include "fbink.h"

using namespace std;

void initFbink();

int fbinkWriteCenter(string stringToWrite, bool darkmode);

void clearScreen(bool darkmodeset);

int printImage(string path);

void screenshotFbink();

void restoreFbink(bool darkmode);

void closeFbink();

void restoreFbDepth();

#endif
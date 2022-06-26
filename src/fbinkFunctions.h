#ifndef FBINK_FUNCTIONS_H
#define FBINK_FUNCTIONS_H

#include <string>

using namespace std;

void initFbink();

int fbinkWriteCenter(string stringToWrite);

void clearScreen();

void printImage(string path);

#endif
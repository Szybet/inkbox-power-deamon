#ifndef CINEMATICBRIGHTNESS_H
#define CINEMATICBRIGHTNESS_H

void setBrightnessCin(int levelToSet, int currentLevel);

void saveBrightness(int level);

int restoreBrightness();

void setBrightness(int device, int level);

int getBrightness();

#endif
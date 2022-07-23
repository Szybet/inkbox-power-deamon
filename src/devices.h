#ifndef DEVICES_H
#define DEVICES_H

void manageChangeLedState();

void changeLedState();

void setLedState(bool on);

void ledManager();

bool getChargerStatus();

bool isDeviceChargerBug();

bool getAccurateChargerStatus();

#endif
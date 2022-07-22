#include "usbnet.h"
#include "Wifi.h" // for loading modules
#include "functions.h"

#include <fcntl.h>
#include <iostream>
#include <linux/module.h>
#include <stdexcept>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define delete_module(name, flags) syscall(__NR_delete_module, name, flags)

extern bool wasUsbNetOn;

extern string model;
// Idk if there is a better way to stop those services. I dont care, i dont want
// to do anything with openrc ever again

void disableUsbNet() {
  if (wasUsbNetOn == true) {
    log("Disabling usbnet");
    int timestamp = 200;
    if (model == "kt") {
      executeCommand("modprobe -r g_ether");
    } else {
      //
      if (delete_module(string("g_ether").c_str(), O_NONBLOCK) != 0) {
        log("Cant unload module: g_ether");
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(timestamp));
      //
      if (delete_module(string("usb_f_rndis").c_str(), O_NONBLOCK) != 0) {
        log("Cant unload module: usb_f_rndis");
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(timestamp));
      //
      if (delete_module(string("usb_f_ecm_subset").c_str(), O_NONBLOCK) != 0) {
        log("Cant unload module: usb_f_ecm_subset");
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(timestamp));
      //
      if (delete_module(string("usb_f_eem").c_str(), O_NONBLOCK) != 0) {
        log("Cant unload module: usb_f_eem");
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(timestamp));
      //
      if (delete_module(string("usb_f_ecm").c_str(), O_NONBLOCK) != 0) {
        log("Cant unload module: usb_f_ecm");
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(timestamp));
      //
      if (delete_module(string("u_ether").c_str(), O_NONBLOCK) != 0) {
        log("Cant unload module: u_ether");
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(timestamp));
      //
      if (delete_module(string("libcomposite").c_str(), O_NONBLOCK) != 0) {
        log("Cant unload module: libcomposite");
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(timestamp));
      //
      if (delete_module(string("configfs").c_str(), O_NONBLOCK) != 0) {
        log("Cant unload module: configfs");
      }
    }
  }
}

void startUsbNet() {
    if(wasUsbNetOn == true)
    {
        log("restoring usbnet");
        executeCommand("service usbnet restart");
    }
}
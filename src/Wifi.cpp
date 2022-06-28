#include "Wifi.h"
#include "AppsFreeze.h"
#include "functions.h"

#include <chrono>
#include <fcntl.h>
#include <linux/module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

// https://stackoverflow.com/questions/5947286/how-to-load-linux-kernel-modules-from-c-code
#define delete_module(name, flags) syscall(__NR_delete_module, name, flags)
#define init_module(module_image, len, param_values)                           \
  syscall(__NR_init_module, module_image, len, param_values)

extern string model;

void turnOffWifi() {
  string WIFI_MODULE;
  string SDIO_WIFI_PWR_MODULE;
  string WIFI_DEV;

  // if [ -d "/sys/class/net/${WIFI_DEV}" ]; then
  // nope

  if (model == "n873" or model == "n236" or model == "n306") {
    WIFI_MODULE = "8189fs";
    SDIO_WIFI_PWR_MODULE = "sdio_wifi_pwr";
    WIFI_DEV = "eth0";
  } else if (model == "n705" or model == "n905b" or model == "n905c" or
             model == "n613") {
    WIFI_MODULE = "dhd";
    SDIO_WIFI_PWR_MODULE = "sdio_wifi_pwr";
    WIFI_DEV = "eth0";
  } else if (model == "n437") {
    WIFI_MODULE = "bcmdhd";
    SDIO_WIFI_PWR_MODULE = "sdio_wifi_pwr";
    WIFI_DEV = "wlan0";
  } else {
    WIFI_MODULE = "dhd";
    SDIO_WIFI_PWR_MODULE = "sdio_wifi_pwr";
    WIFI_DEV = "eth0";
  }

  if (readConfigString("/sys/class/net/" + WIFI_DEV + "/operstate") == "up") {
    writeFileString("/run/was_connected_to_wifi", "true");

    killProcess("dhcpcd");
    killProcess("wpa_supplicant");
    killProcess("udhcpc");

    string turnOffInterface = "/sbin/ifconfig " + WIFI_DEV + " down";
    system(turnOffInterface.c_str());

    if (model == "n705" or model == "n905b" or model == "n905c" or
        model == "n613" or model == "n437") {
      system("wlarm_le down");
    }

    if (delete_module(WIFI_MODULE.c_str(), O_NONBLOCK) != 0) {
      log("Cant unload module: " + WIFI_MODULE);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    if (delete_module(SDIO_WIFI_PWR_MODULE.c_str(), O_NONBLOCK) != 0) {
      log("Cant unload module: " + SDIO_WIFI_PWR_MODULE);
    }
    // sleeping has problems with wifi, so wait additional time only when it is
    system("/bin/sync");
    // std::this_thread::sleep_for(std::chrono::milliseconds(15000));
  }
}

void turnOnWifi() {
  string WIFI_MODULE;
  string SDIO_WIFI_PWR_MODULE;
  string WIFI_DEV;

  if (model == "n873" or model == "n236" or model == "n306") {
    WIFI_MODULE = "/modules/wifi/8189fs.ko";
    SDIO_WIFI_PWR_MODULE = "/modules/drivers/mmc/card/sdio_wifi_pwr.ko";
    WIFI_DEV = "eth0";
  } else if (model == "n705" or model == "n905b" or model == "n905c" or
             model == "n613") {
    WIFI_MODULE = "/modules/dhd.ko";
    SDIO_WIFI_PWR_MODULE = "/modules/sdio_wifi_pwr.ko";
    WIFI_DEV = "eth0";
  } else if (model == "n437") {
    WIFI_MODULE = "/modules/wifi/bcmdhd.ko";
    SDIO_WIFI_PWR_MODULE = "/modules/drivers/mmc/card/sdio_wifi_pwr.ko";
    WIFI_DEV = "wlan0";
  } else {
    WIFI_MODULE = "/modules/dhd.ko";
    SDIO_WIFI_PWR_MODULE = "/modules/sdio_wifi_pwr.ko";
    WIFI_DEV = "eth0";
  }
  if (readConfigString("/run/was_connected_to_wifi") == "true") {
    load_module(WIFI_MODULE);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    load_module(SDIO_WIFI_PWR_MODULE);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    string wifiInterfaceUp = "/sbin/ifconfig " + WIFI_DEV + " up";
    system(wifiInterfaceUp.c_str());

    if (model == "n705" or model == "n905b" or model == "n905c" or
        model == "n613" or model == "n437") {
      system("/usr/bin/wlarm_le up");
    }

    if (is_file_exist("/data/config/17-wifi_connection_information/essid") ==
            true and
        is_file_exist(
            "/data/config/17-wifi_connection_information/passphrase") == true) {
      string ESSID =
          readConfigString("/data/config/17-wifi_connection_information/essid");
      string PASSPHRASE = readConfigString(
          "/data/config/17-wifi_connection_information/passphrase");
      string recconection = "/usr/local/bin/wifi/connect_to_network.sh " +
                            ESSID + " " + PASSPHRASE;
      system(recconection.c_str());
    }
    remove("/run/was_connected_to_wifi");
  }
}

void load_module(string path) {
  size_t image_size;
  struct stat st;
  void *image;
  const char *params = "";
  int fd = open(path.c_str(), O_RDONLY);
  puts("init");
  fstat(fd, &st);
  image_size = st.st_size;
  image = malloc(image_size);
  read(fd, image, image_size);
  close(fd);
  if (init_module(image, image_size, params) != 0) {
    log("init_module");
    exit(EXIT_FAILURE);
  }
  free(image);
}
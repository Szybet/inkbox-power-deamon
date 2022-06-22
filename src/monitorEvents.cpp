#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "functions.h"
#include "main.h"
#include "monitorEvents.h"

// libevdev
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libevdev/config.h"
#include "libevdev/libevdev.h"

using namespace std;

void startMonitoringDev() {
  log("Starting monitoring events");

  struct libevdev *dev = NULL;

  int fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
  int rc = libevdev_new_from_fd(fd, &dev);

  if (rc < 0) {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
    exit(1);
  }

  log("Input device name: " + (string)libevdev_get_name(dev));
  log("Input device ID: bus " + to_string(libevdev_get_id_bustype(dev)) +
      " ,vendor" + to_string(libevdev_get_id_vendor(dev)) + " ,product" +
      to_string(libevdev_get_id_product(dev)));
  do {

    struct input_event ev;
    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc == 0) {
      string codeName = (string)libevdev_event_code_get_name(ev.type, ev.code);
      log("Input event received: " +
          (string)libevdev_event_type_get_name(ev.type) + codeName +
          to_string(ev.value));

      if (codeName == "KEY_POWER" and ev.value == 1) {
        log("Sending message to sleep");
      }
    }

    std::chrono::milliseconds timespan(150);
    std::this_thread::sleep_for(timespan);

  } while (rc == 1 || rc == 0 || rc == -EAGAIN);
}
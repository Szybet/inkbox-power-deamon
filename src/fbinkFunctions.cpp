#include "fbinkFunctions.h"
#include "functions.h"

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "fbink.h"

extern int fbfd;

extern FBInkDump dump;

extern string model;

void initFbink() {
  fbfd = fbink_open();
  if (fbfd == -1) {
    log("Failed to open the framebuffer");
    exit(EXIT_FAILURE);
  }
  log("Loaded FBInk version: " + (string)fbink_version());
  /*
    if (fbink_close(fbfd) < 0) {
    printf("Failed to close the framebuffer, aborting . . .\n");
    return EXIT_FAILURE;
  }
  */
}

int fbinkWriteCenter(string stringToWrite, bool darkmode) {

  FBInkConfig fbink_cfg = {0};
  fbink_cfg.is_centered = true;
  fbink_cfg.is_halfway = true;
  if (darkmode == true) {
    // doesnt work?
    //fbink_cfg.bg_color = BG_BLACK;
    //fbink_cfg.fg_color = FG_WHITE;
    fbink_cfg.is_inverted = true;
  }

  if (fbink_init(fbfd, &fbink_cfg) < 0) {
    log("Failed to initialize FBInk, aborting");
    return EXIT_FAILURE;
  }

  fbink_add_ot_font("/etc/init.d/splash.d/fonts/resources/inter-b.ttf",
                    FNT_REGULAR);

  FBInkOTConfig fbinkOT_cfg = {0};
  fbinkOT_cfg.is_centered = true;
  fbinkOT_cfg.is_formatted = true;
  fbinkOT_cfg.size_pt = 20;

  FBInkOTFit ot_fit = {0};

  if (fbink_print_ot(fbfd, stringToWrite.c_str(), &fbinkOT_cfg, &fbink_cfg,
                     &ot_fit) < 0) {
    log("Failed to print to fbink");
  }

  return EXIT_SUCCESS;
}

void clearScreen(bool darkmodeset) {
  FBInkConfig fbink_cfg = {0};
  fbink_cfg.is_inverted = darkmodeset;

  if (fbink_init(fbfd, &fbink_cfg) < 0) {
    log("Failed to initialize FBInk, aborting");
    exit(EXIT_FAILURE);
  }

  // clear_screen(fbfd, fbink_cfg.is_inverted ? penBGColor ^ 0xFFu : penBGColor,
  // fbink_cfg.is_flashing);
  FBInkRect cls_rect = {0};
  cls_rect.left = (unsigned short int)0;
  cls_rect.top = (unsigned short int)0;
  cls_rect.width = (unsigned short int)0;
  cls_rect.height = (unsigned short int)0;
  fbink_cls(fbfd, &fbink_cfg, &cls_rect, false);
}

int printImage(string path) {
  FBInkConfig fbink_cfg = {0};

  return fbink_print_image(fbfd, path.c_str(), 0, 0, &fbink_cfg);
}

void screenshotFbink() {
  /*
  string mainString = "/usr/bin/chroot /kobo /usr/bin/fbgrab ";
  mainString = mainString + "\"" + path + "\"";
  system(mainString.c_str());
  */
  FBInkConfig fbink_cfg = {0};

  if (fbink_init(fbfd, &fbink_cfg) < 0) {
    log("Failed to initialize FBInk, aborting");
  }

  log("Inited fbink in screenshotFbink");

  if (fbink_dump(fbfd, &dump) < 0) {
    log("something went wrong with bump");
  };
  log("screenshot done");
}

void restoreFbink(bool darkmode) {
  FBInkConfig fbink_cfg = {0};
  if (darkmode == true) {
    // doesnt work
    //fbink_cfg.is_inverted = true;
    fbink_cfg.is_nightmode = true;
  }

  if (fbink_init(fbfd, &fbink_cfg) < 0) {
    log("Failed to initialize FBInk, aborting");
  }

  fbink_restore(fbfd, &fbink_cfg, &dump);
  // free(dump.data);
}

void closeFbink() { fbink_close(fbfd); }

void restoreFbDepth() {
  // fbdepth.c is a pretty complicated file so
  if (model == "n437" or model == "kt") {
    if (fileExists("/kobo/tmp/inkbox_running") == true) {
      system("/opt/bin/fbink/fbdepth -d 8");
    } else {
      // X11 is running, elsewise there is something wrong ...
      if (model == "kt") {
        system("/opt/bin/fbink/fbdepth -d 32");
      } else {
        system("/opt/bin/fbink/fbdepth -d 16");
      }
    }
  }
}
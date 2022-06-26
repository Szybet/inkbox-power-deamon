#include "fbinkFunctions.h"
#include "functions.h"

#include <cstdlib>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "fbink.h"

extern int fbfd;

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

int fbinkWriteCenter(string stringToWrite) {

  FBInkConfig fbink_cfg = {0};
  fbink_cfg.is_centered = true;
  fbink_cfg.is_halfway = true;

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

void clearScreen() {
  FBInkConfig fbink_cfg = {0};

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

void printImage(string path)
{
    FBInkConfig fbink_cfg = {0};

    fbink_print_image(fbfd, path.c_str(), 0, 0, &fbink_cfg);
}
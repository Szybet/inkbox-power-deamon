#include "monitorEvents.h"
#include "main.h"

#include <iostream>

// https://developer.ibm.com/tutorials/l-ubuntu-inotify/
#include "sys/inotify.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// https://pubs.opengroup.org/onlinepubs/009604599/functions/read.html
#include <unistd.h>


#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

void startMonitoringDev() {
  log("Starting monitoring events");

  int length, i = 0;
  int fd;
  int wd;
  char buffer[BUF_LEN];

  fd = inotify_init();
  if (fd < 0) {
    log("inotify_init failed (old kernel?)");
  }

  wd = inotify_add_watch(fd, "/tmp", IN_MODIFY | IN_CREATE | IN_DELETE );

  length = read( fd, buffer, BUF_LEN );

  if ( length < 0 ) {
    log("failed to read from buffer");
  }  

log("inotify setted up");
/*
while ( i < length ) {
    struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
    if(event->len) {
        if ( event->mask & IN_MODIFY ) {
            log("/tmp/power was modified");
        }
    }
}
*/
while ( i < length ) {
    struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
    if ( event->len ) {
      if ( event->mask & IN_CREATE ) {
        if ( event->mask & IN_ISDIR ) {
          printf( "The directory %s was created.\n", event->name );       
        }
        else {
          printf( "The file %s was created.\n", event->name );
        }
      }
      else if ( event->mask & IN_DELETE ) {
        if ( event->mask & IN_ISDIR ) {
          printf( "The directory %s was deleted.\n", event->name );       
        }
        else {
          printf( "The file %s was deleted.\n", event->name );
        }
      }
      else if ( event->mask & IN_MODIFY ) {
        if ( event->mask & IN_ISDIR ) {
          printf( "The directory %s was modified.\n", event->name );
        }
        else {
          printf( "The file %s was modified.\n", event->name );
        }
      }
    }
    i += EVENT_SIZE + event->len;
  }

  ( void ) inotify_rm_watch( fd, wd );
  ( void ) close( fd );
}
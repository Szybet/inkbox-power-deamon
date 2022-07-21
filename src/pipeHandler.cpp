#include "pipeHandler.h"
#include "functions.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <experimental/filesystem>
#include <fcntl.h>

void startPipeServer() {
  log("Starting pipe server");

  if (dirExists("/run/ipd") == false) {
    log("Creating /run/ipd");
    experimental::filesystem::create_directory("/run/ipd");
    experimental::filesystem::create_directory("/kobo/run/ipd");
    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    const char * myfifo = "/run/ipd/fifo";
    mkfifo(myfifo, 0666);

    system("/bin/mount --bind /run/ipd /kobo/run/ipd");
  } else if (dirExists("/kobo/run/ipd") == false)
  {
    log("Creating /kobo/run/ipd ( this is weird )");
    experimental::filesystem::create_directory("/kobo/run/ipd");
    system("/bin/mount --bind /run/ipd /kobo/run/ipd");
  }
  
}

void sleepPipeSend() {
  log("sending message");
  const char * myfifo = "/run/ipd/fifo";
  int fd = open(myfifo, O_RDWR); // O_WRONLY // https://stackoverflow.com/questions/24099693/c-linux-named-pipe-hanging-on-open-with-o-wronly

  string testString = "start";
  write(fd, testString.c_str(), 5);
  close(fd);
}

void restorePipeSend() {
  log("sending message");
  const char * myfifo = "/run/ipd/fifo";
  int fd = open(myfifo, O_RDWR); // O_WRONLY // https://stackoverflow.com/questions/24099693/c-linux-named-pipe-hanging-on-open-with-o-wronly

  string testString = "stop0";
  write(fd, testString.c_str(), 5);
  close(fd);
}
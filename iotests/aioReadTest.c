#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "aioReads.h"
#include "utils.h"
#include "logSpeed.h"

int    keepRunning = 1;       // have we been interrupted
int    exitAfterSeconds = 30;
int    qd = 32;
char   *path = NULL;
int    seqFiles = 0;
double maxSizeGB = 0;

void handle_args(int argc, char *argv[]) {
  int opt;
  
  while ((opt = getopt(argc, argv, "dDr:t:k:o:q:f:s:G:")) != -1) {
    switch (opt) {
    case 't':
      exitAfterSeconds = atoi(optarg);
      break;
    case 'q':
      qd = atoi(optarg);
      break;
    case 's':
      seqFiles = atoi(optarg);
      break;
    case 'G':
      maxSizeGB = atof(optarg);
      break;
    case 'f':
      path = optarg;
      break;
    default:
      exit(-1);
    }
  }
  if (path == NULL) {
    fprintf(stderr,"./aioReadTest [-s sequentialFiles] [-q queueDepth] [-t 30 secs] [-G 32] -f blockdevice\n");
    exit(1);
  }
}



int main(int argc, char *argv[]) {
  handle_args(argc, argv);

  size_t seed = (size_t) timedouble();
  srand(seed);
  fprintf(stderr,"path: %s, seed %zd, blocksize: %d", path, seed, 65536);
  int fd = open(path, O_RDONLY | O_DIRECT);
  if (fd < 0) {perror(path);return -1; }

  size_t bdSize = blockDeviceSize(path);
  if (maxSizeGB >0) {
    bdSize = (size_t) (maxSizeGB * 1024L * 1024 * 1024);
  }
  fprintf(stderr,", bdSize %.1lf GB\n", bdSize/1024.0/1024/1024);

  
  size_t num = 10*1000*1000;
  size_t *positions = malloc(num * sizeof(size_t));

  if (seqFiles == 0) {
    for (size_t i = 0; i < num; i++) {
      positions[i] = ((size_t)((rand() % bdSize) / 65536)) * 65536;
      if (i < 10) {
	fprintf(stderr,"[%zd] pos %zd\n", i, positions[i]);
      }
    }
  } else {
    for (size_t i = 0; i < num; i++) {
      size_t gap = bdSize / (seqFiles + 1);
      positions[i] = ((i % seqFiles) * gap) + ((i / seqFiles) * 65536);
      if (i < 10) {
	fprintf(stderr,"[%zd] pos %zd\n", i, positions[i]);
      }
    }
  }
    
  
  readMultiplePositions(fd, positions, num, 65536, exitAfterSeconds, qd);
  
  return 0;
}

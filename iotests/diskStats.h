#ifndef _DISKSTATS_H
#define _DISKSTATS_H

#include <unistd.h>

typedef struct {
  size_t startSecRead;
  size_t finishSecRead;
  size_t startSecWrite;
  size_t finishSecWrite;
  size_t numDrives;
  size_t allocDrives;
  int *majorArray;
  int *minorArray;
} diskStatType;

void diskStatSetup(diskStatType *d);
void diskStatAddStart(diskStatType *d, size_t readSectors, size_t writeSectors);
void diskStatAddFinish(diskStatType *d, size_t readSectors, size_t writeSectors);
void diskStatSummary(diskStatType *d, size_t *totalReadBytes, size_t *totalWriteBytes, size_t shouldReadBytes, size_t shouldWriteBytes, int verbose);
void diskStatAddDrive(diskStatType *d, int fd);
void diskStatSectorUsage(diskStatType *d, size_t *sread, size_t *swritten, int verbose);
void diskStatFromFilelist(diskStatType *d, const char *path);
void diskStatStart(diskStatType *d);
void diskStatFinish(diskStatType *d);

#endif


#ifndef _UTILS_H
#define _UTILS_H

#include "logSpeed.h"

double timedouble();

void writeChunks(int fd, char *label, int *chunkSizes, int numChunks, size_t maxTime, logSpeedType *l, size_t maxBufSize, size_t outputEvery, int seq, int direct, int verifyWrites, float flushEverySecs);
void readChunks(int fd, char *label, int *chunkSizes, int numChunks, size_t maxTime, logSpeedType *l, size_t maxBufSize, size_t outputEvery, int seq, int direct, float limitGBToProcess);

size_t blockDeviceSize(char *fd);
int isBlockDevice(char *name);

void dropCaches();
char *username();

void checkContents(char *label, char *charbuf, size_t size, const size_t checksum, float checkpercentage, size_t stopatbytes);
void shmemUnlink();

double loadAverage();

#endif


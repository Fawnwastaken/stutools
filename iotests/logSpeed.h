#ifndef _LOGSPEED_H
#define _LOGSPEED_H

#include <unistd.h>

#define OUTPUTINTERVAL 1

typedef struct {
  double starttime;
  size_t num;
  size_t alloc;
  size_t total;
  double *values;
  double lasttime;
} logSpeedType;

void   logSpeedInit(volatile logSpeedType *l);
void   logSpeedFree(logSpeedType *l);
void   logSpeedReset(logSpeedType *l);

int    logSpeedAdd(logSpeedType *l, size_t value);

double logSpeedTime(logSpeedType *l);

double logSpeedMedian(logSpeedType *l);
double logSpeedMean(logSpeedType *l);
size_t logSpeedN(logSpeedType *l);
size_t logSpeedTotal(logSpeedType *l);
double logSpeedRank(logSpeedType *l, const float rank); // between [0...1)
double logSpeedMax(logSpeedType *l);

void   logSpeedDump(logSpeedType *l, const char *fn);

#endif


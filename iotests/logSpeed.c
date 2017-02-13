#include <stdlib.h>
#include <stdio.h>

#include "logSpeed.h"
#include "utils.h"


void logSpeedInit(volatile logSpeedType *l) {
  l->starttime = timedouble();
  l->lasttime = l->starttime;
  l->num = 0;
  l->alloc = 10000;
  l->total = 0;
  l->values = calloc(l->alloc, sizeof(double)); if (!l->values) {fprintf(stderr,"OOM!\n");exit(1);}
}

void logSpeedReset(logSpeedType *l) {
  l->starttime = timedouble();
  l->lasttime = l->starttime;
  l->num = 0;
  l->total = 0;
}

double logSpeedTime(logSpeedType *l) {
  return l->lasttime - l->starttime;
}



int logSpeedAdd(logSpeedType *l, size_t value) {
  const double thistime = timedouble();
  const double timegap = thistime - l->lasttime;

  if (l->num >= l->alloc) {
    l->alloc = l->alloc * 2 + 1;
    l->values = realloc(l->values, l->alloc * sizeof(double)); if (!l->values) {fprintf(stderr,"OOM! realloc failed\n");exit(1);}
    //    fprintf(stderr,"skipping...\n"); sleep(1);
  } 
     // fprintf(stderr,"it's been %zd bytes in %lf time, is %lf, total %zd, %lf,  %lf sec\n", value, timegap, value/timegap, l->total, l->total/logSpeedTime(l), logSpeedTime(l));
  l->values[l->num] = value / timegap;
  l->lasttime = thistime;
  l->total += value;
  l->num++;
  
 

  return l->num;
}


static int comparisonFunction(const void *p1, const void *p2) {
  double *d1 = (double*)p1;
  double *d2 = (double*)p2;

  if (*d1 < *d2) return -1;
  else if (*d1 > *d2) return 1;
  else return 0;
}

size_t logSpeedTotal(logSpeedType *l) {
  return l->total;
}

   
double logSpeedMedian(logSpeedType *l) {
  qsort(l->values, l->num, sizeof(size_t), comparisonFunction);
  const double med = l->values[l->num / 2];
  //  fprintf(stderr,"values: %zd, median %f\n", l->num, med);

  return med;
}

double logSpeedMean(logSpeedType *l) {
  return l->total / logSpeedTime(l);
}

double logSpeed1(logSpeedType *l) {
  qsort(l->values, l->num, sizeof(size_t), comparisonFunction);
  return l->values[(size_t)(l->num * 0.01)];
}

double logSpeed5(logSpeedType *l) {
  qsort(l->values, l->num, sizeof(size_t), comparisonFunction);
  return l->values[(size_t)(l->num * 0.95)];
}

double logSpeed95(logSpeedType *l) {
  qsort(l->values, l->num, sizeof(size_t), comparisonFunction);
  return l->values[(size_t)(l->num * 0.95)];
}

double logSpeed99(logSpeedType *l) {
  qsort(l->values, l->num, sizeof(size_t), comparisonFunction);
  return l->values[(size_t)(l->num * 0.99)];
}

double logSpeedRank(logSpeedType *l, const float rank) {
  if (rank < 0 || rank > 1) {
    fprintf(stderr,"rand needs to be in the range [0..1]\n");
    exit(1);
  }
  qsort(l->values, l->num, sizeof(size_t), comparisonFunction);
  return l->values[(size_t)(l->num * rank)];
}

double logSpeedMax(logSpeedType *l) {
  qsort(l->values, l->num, sizeof(size_t), comparisonFunction);
  return l->values[l->num - 1];
}

size_t logSpeedN(logSpeedType *l) {
  return l->num;
}

  

void logSpeedFree(logSpeedType *l) {
  free(l->values);
  l->values = NULL;
}


void logSpeedDump(logSpeedType *l, const char *fn) {
  logSpeedMedian(l);
  FILE *fp = fopen(fn, "wt");
  for (size_t i = 0; i < l->num; i++) {
    fprintf(fp, "%lf\n", l->values[i]);
  }
  fclose(fp);
}


#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>

#include "logSpeed.h"
#include "utils.h"


double logSpeedInit(volatile logSpeedType *l) {
  l->starttime = timedouble();
  l->lasttime = l->starttime;
  l->num = 0;
  l->alloc = 10000;
  CALLOC(l->rawtime, l->alloc, sizeof(double));
  CALLOC(l->rawvalues, l->alloc, sizeof(double));
  CALLOC(l->rawcount, l->alloc, sizeof(size_t));
  return l->starttime;
}

void logSpeedReset(logSpeedType *l) {
  if (l) {
    l->starttime = timedouble();
    l->lasttime = l->starttime;
    l->num = 0;
  }
}


double logSpeedTime(logSpeedType *l) {
  return l->lasttime - l->starttime;
}


// if no count
int logSpeedAdd(logSpeedType *l, double value) {
  return logSpeedAdd2(l, value, 0);
}


int logSpeedAdd2(logSpeedType *l, double value, size_t count) {
  assert(l);
    
  if (l->num >= l->alloc) {
    l->alloc = l->alloc * 2 + 1;
    l->rawtime = realloc(l->rawtime, l->alloc * sizeof(double));
    l->rawvalues = realloc(l->rawvalues, l->alloc * sizeof(double));
    l->rawcount = realloc(l->rawcount, l->alloc * sizeof(size_t));
    //    fprintf(stderr,"skipping...\n"); sleep(1);
    //    fprintf(stderr,"new size %zd\n", l->num);
  } 
  // fprintf(stderr,"it's been %zd bytes in %lf time, is %lf, total %zd, %lf,  %lf sec\n", value, timegap, value/timegap, l->total, l->total/logSpeedTime(l), logSpeedTime(l));
  l->rawtime[l->num] = timedouble();
  l->lasttime = l->rawtime[l->num];
  l->rawvalues[l->num] = value;
  l->rawcount[l->num] = count;
  l->num++;
  //  } else {
  //    fprintf(stderr,"skipping dupli\n");

  return l->num;
}

double logSpeedMean(logSpeedType *l) {
  if (l->num <= 1) {
    return 0;
  } else {
    return (l->rawvalues[l->num -1] / (l->rawtime[l->num - 1] - l->starttime));
  }
}

size_t logSpeedN(logSpeedType *l) {
  return l->num;
}
  

void logSpeedFree(logSpeedType *l) {
  free(l->rawtime);
  free(l->rawvalues);
  free(l->rawcount);
  l->rawtime = NULL;
  l->rawvalues = NULL;
  l->rawcount = NULL;
}


void logSpeedDump(logSpeedType *l, const char *fn, const int format, const char *description, size_t bdSize, size_t origBdSize, const char *cli) {
  //  logSpeedMedian(l);
  //  if (format) {
  //    fprintf(stderr, "*info* logSpeedDump format %d\n", format);
  //  }
  double valuetotal = 0;
  size_t counttotal = 0;

  FILE *fp = fopen(fn, "wt");
  if (!fp) {perror(fn); exit(-1);}

  if (format == MYSQL) {
    // first dump to MySQL format
    // dump to file, mysql -D database -h hostname <file.mysql
    //    char *filename = malloc(100);
    //    sprintf(filename, "/tmp/stutoosmysqlXXXXXX");
    //    int fd = mkstemp(filename);
    //    if (fd < 0) {perror(filename);exit(-1);}
    fprintf(fp, "create table if not exists runs (id int auto_increment, primary key(id), time datetime, hostname text, domainname text, RAM int, threads int, bdSize float, origBdSize float, cli text, description text);\n"
		"create table if not exists rawvalues (id int, time double, globaltime double, MiB int, SumMiB int, IOs int, SumIOs int);\n");

    char hostname[1000], domainname[1000];
    int w = gethostname(hostname, 1000); if (w != 0) {hostname[0] = 0;}
    w = getdomainname(domainname, 1000); if (w != 0) {domainname[0] = 0;}
    fprintf(fp, "start transaction; insert into runs(description, time, hostname, domainname, RAM, threads, bdSize, origBdSize, cli) values (\"%s\", NOW(), \"%s\", \"%s\", %zd, %zd, %.1lf, %.1lf, \"%s\");\n", (description==NULL)?"":description, hostname, domainname, (size_t)(TOGiB(totalRAM()) + 0.5), numThreads(), TOGiB(bdSize), TOGiB(origBdSize), cli);

    for (size_t i = 0; i < l->num; i++) {
      valuetotal += l->rawvalues[i];
      counttotal += l->rawcount[i];
      fprintf(fp, "insert into rawvalues(id, time,globaltime,MiB,SumMiB,IOs,SumIOs) VALUES(last_insert_id(), %.6lf,%.6lf,%.2lf,%.2lf,%zd,%zd);\n", l->rawtime[i] - l->starttime, l->rawtime[i], l->rawvalues[i], valuetotal, l->rawcount[i], counttotal);
    }
    fprintf(fp,"commit;\n");
  } else if (format != JSON) {// if plain format
    for (size_t i = 0; i < l->num; i++) {
      valuetotal += l->rawvalues[i];
      counttotal += l->rawcount[i];
      fprintf(fp, "%.6lf\t%.6lf\t%.2lf\t%.2lf\t%zd\t%zd\n", l->rawtime[i] - l->starttime, l->rawtime[i], l->rawvalues[i], valuetotal, l->rawcount[i], counttotal);
    }
  } else if (format == JSON) {
    fprintf(fp,"[\n");
    for (size_t i = 0; i < l->num; i++) {
      valuetotal += l->rawvalues[i];
      counttotal += l->rawcount[i];
      fprintf(fp, "{\"time\":\"%.6lf\", \"globaltime\":\"%.6lf\", \"MiB\":\"%.2lf\", \"SumMiB\":\"%.2lf\", \"IOs\":\"%zd\", \"SumIOs\":\"%zd\"}", l->rawtime[i] - l->starttime, l->rawtime[i], l->rawvalues[i], valuetotal, l->rawcount[i], counttotal);
      if (i < l->num-1) fprintf(fp,",");
      fprintf(fp,"\n");
    }
    fprintf(fp,"]\n");
  } else {
    assert(0); // no idea
  }
  fclose(fp);
}



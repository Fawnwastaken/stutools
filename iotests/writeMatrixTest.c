#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

int keeprunning = 1;
int useDirect = 0;
float benchmarkTime = 1;

typedef struct {
  int threadid;
  char *path;
  size_t total;
} threadInfoType;
struct timeval gettm()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return now;
}
double timedouble() {
  struct timeval now = gettm();
  double tm = (now.tv_sec * 1000000 + now.tv_usec);
  return tm/1000000.0;
}

void intHandler(int d) {
  fprintf(stderr,"got signal\n");
  keeprunning = 0;
}
static void *runThread(void *arg) {
  threadInfoType *threadContext = (threadInfoType*)arg; // grab the thread threadContext args
  //  fprintf(stderr,"opening... '%s'\n", threadContext->path);
  int fd = open(threadContext->path, O_WRONLY | (useDirect ? O_DIRECT : 0));
  if (fd < 0) {
    perror(threadContext->path);
    return NULL;
  }
  //  fprintf(stderr,"opened %s\n", threadContext->path);
#define BUFSIZE (1024*1024)
  void *buf = NULL;
  if (posix_memalign(&buf, 4096, BUFSIZE)) { // O_DIRECT requires aligned memory
	fprintf(stderr,"memory allocation failed\n");exit(1);
  }	
  memset(buf, 0x00, BUFSIZE);
  int wbytes = 0;
  size_t lastg = 0;
  double starttime = timedouble();
  while (((wbytes = write(fd, buf, BUFSIZE)) > 0) && keeprunning) {
    threadContext->total += wbytes;
    if (threadContext->total >> 30 != lastg) {
      lastg = threadContext->total >>30;
      //fprintf(stderr,"write to '%s': %zd GB, speed %.1f MB/s\n", threadContext->path, lastg, (threadContext->total >> 20) / (timedouble() - starttime));
    }
    if (timedouble() - starttime >= benchmarkTime) break;
  }
  if (wbytes < 0) {
    perror("weird problem");
  }
  //  fprintf(stderr,"finished. Total write from '%s': %zd bytes in %.1f seconds, %.2f MB/s\n", threadContext->path, threadContext->total, timedouble() - starttime, (threadContext->total >> 20) / (timedouble() - starttime));
  close(fd);
  free(buf);
  return NULL;
}

size_t benchmark(threadInfoType *threadContext, const int num, size_t running[]) {
    pthread_t *pt = calloc(num, sizeof(pthread_t));    if (pt==NULL) {fprintf(stderr, "OOM(pt): \n");exit(-1);}

    double starttime = timedouble();

    for (size_t i = 0; i < num; i++) {
      if (running[i]) {
	//	fprintf(stderr,"running=%zd\n",i);
	threadContext[i].total = 0;
	pthread_create(&(pt[i]), NULL, runThread, &(threadContext[i]));
      }
    }
    size_t allbytes = 0;
    for (size_t i = 0; i < num; i++) {
      if (running[i]) {
	pthread_join(pt[i], NULL);
	allbytes += threadContext[i].total;
      }
    }
    double elapsedtime = timedouble() - starttime;
    size_t speedmb =  (size_t) ((allbytes/1024.0/1024) / elapsedtime);
    free(pt);
    return speedmb;
}



threadInfoType *gatherDrives(int argc, char *argv[], int *num) {
    threadInfoType *threadContext = (threadInfoType*) calloc(argc, sizeof(threadInfoType));
    if (threadContext == NULL) {fprintf(stderr,"OOM(threadContext): \n");exit(-1);}
    threadInfoType *ret = threadContext;
    
    *num = 0;
    if (argc > 0) {
      
      for (size_t i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	  threadContext->threadid = *num;
	  *num = (*num) + 1;
	  threadContext->path = argv[i];
	  threadContext->total = 0;
	  fprintf(stderr,"drive %d: %s\n", *num, threadContext->path);
	  threadContext++;
	}
      }
    }
    return ret;
}

void handle_args(int argc, char *argv[]) {
  int opt;
  
  while ((opt = getopt(argc, argv, "dfS")) != -1) {
    switch (opt) {
    case 'd':
      fprintf(stderr,"USING DIRECT\n");
      useDirect = 1;
      break;
    case 'f':
      benchmarkTime = 0.1;
      fprintf(stderr,"timeout: %.1f seconds\n", benchmarkTime);
      break;
    case 'S':
      benchmarkTime = 10;
      fprintf(stderr,"timeout: %.1f seconds\n", benchmarkTime);
      break;
    } 
  }
}


void nSquareTest(threadInfoType *t, const int num) {

  size_t values[num][num];
  size_t *running = calloc(100, sizeof(size_t));

  for (size_t j = 0; j < num; j++) {
    for (size_t i = 0; i < num; i++) {
      memset(running, 0, sizeof(size_t));
      running[i] = 1;
      running[j] = 1;

      // do test
      size_t speedmb = 0;
      if (i >= j) {
	speedmb = benchmark(t, num, running);
	values[j][i] = speedmb;
      } else {
	values[j][i] = values[i][j];
      }
	
      fprintf(stdout,"%4zd ", values[j][i]);
      fflush(stdout);
    }
    fprintf(stdout,"\n");
  }

}

int main(int argc, char *argv[]) {
  handle_args(argc, argv);

  int num = 0;
  threadInfoType *t = gatherDrives(argc, argv, &num);

  nSquareTest(t, num);

  return 0;
}






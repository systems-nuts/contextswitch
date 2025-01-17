// Copyright (C) 2010  Benoit Sigoure
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include <string.h>
#include <sys/mman.h>
static inline unsigned long rdtsc(void)
{
        unsigned long low, high;

        asm volatile("rdtsc" : "=a" (low), "=d" (high));

        return ((low) | (high) << 32);
}

static inline long long unsigned time_ns(struct timespec* const ts) {
  if (clock_gettime(CLOCK_REALTIME, ts)) {
    exit(1);
  }
  return ((long long unsigned) ts->tv_sec) * 1000000000LLU
    + (long long unsigned) ts->tv_nsec;
}

static const int iterations = 500000;


static void* thread(void*ctx) {
  (void)ctx;
  for (int i = 0; i < iterations; i++)
      sched_yield();
  return NULL;
}

int main(void) {
  unsigned long long *results = malloc(sizeof(unsigned long long)*iterations);
  memset(results,0,sizeof(long long unsigned)*iterations);
  int ret= mlock(results,sizeof(long long unsigned)*iterations);
  double total=0.0;
  unsigned long long start, stop;
  struct sched_param param;
  param.sched_priority = 1;
  if (sched_setscheduler(getpid(), SCHED_FIFO, &param))
    fprintf(stderr, "sched_setscheduler(): %s\n", strerror(errno));

  struct timespec ts;
  pthread_t thd;
  if (pthread_create(&thd, NULL, thread, NULL)) {
    return 1;
  }
  
  long long unsigned start_ns = time_ns(&ts);
  start = rdtsc();
  for (int i = 0; i < iterations; i++)
  {
      sched_yield();
      stop = rdtsc();
      results[i]= stop-start;
      start = stop;
  }
  long long unsigned delta = time_ns(&ts) - start_ns;
    for (int i = 0; i < iterations; i++) {
	   printf("%lld\n",results[i]);
          //total+=results[i];
  }
  const int nswitches = iterations << 2;
//  printf("%i  thread context switches in %lluns (%.1fns/ctxsw)\n",
//         nswitches, delta, (delta / (float) nswitches));
//  printf("%i  thread context switches in %lfns (%.1fns/ctxsw)\n",
//         nswitches, total/2.1, ((total/2.1) / (float) nswitches));
  return 0;
}

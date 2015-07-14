#include <pthread.h>
#include <stdio.h>
#include <propeller.h>

//#define MAIN_SLEEP

pthread_t t1, t2;

void *
threadfunc(void *arg)
{
  char *msg = (char *)arg;

  for(;;) {
    printf("%s (cog %d)\n", msg, cogid());
    fflush(stdout);
#ifdef MAIN_SLEEP
    pthread_yield();
#else
    usleep(100000);
#endif
  }
}

void
busywait(unsigned int usecs)
{
  unsigned long cycles = usecs * (_clkfreq/1000000);
  unsigned long then = getcnt() + cycles;
  while ((long)(then - getcnt()) > 0)
    pthread_yield();
}

int
main()
{
  printf("ping pong demo\n");

  pthread_create(&t1, NULL, threadfunc, (void *)"ping");
  pthread_create(&t2, NULL, threadfunc, (void *)"pong");
  for(;;) {
#ifdef MAIN_SLEEP
    usleep(100000);
#else
    busywait(1000000);
#endif
    printf("main still running on cog %d\n", cogid());
  }
}

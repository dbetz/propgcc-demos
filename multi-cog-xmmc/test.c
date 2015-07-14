/*
  test1.c

*/
#include "propeller.h"
#include <unistd.h>
//#include <cog.h>
//#include <math.h>
//#include <stdlib.h>

//#ifdef __PROPELLER_USE_XMM__
//#define MAX_COGS 7
//#else
//#define MAX_COGS 8
//#endif

static long stacks[8][16 + EXTRA_STACK_LONGS];

void blink14();
void blink13();

void high(int pin);
void low(int pin);
void pause(int time);
//int *cog_run(void (*function)(void *par), int stacksize);


int main()
{
    DIRA |= 0xf << 12;
  // Add startup code here.
    //cog_run(blink14,128);
    // Start blink14() COG -> XMMC
    cogstart(blink14, NULL, &stacks[1], sizeof(stacks[1]));
    // Start blink13() COG -> XMMC
    cogstart(blink13, NULL, &stacks[2], sizeof(stacks[2]));
 
  while(1)
  {
    // Add main loop code here.
//    high(23);   // For C3 board test.
    //high(14); // For DNA-RTC board test.
//    sleep(1);
//    pause(500);
// low(23);
    //low(14);
    //sleep(1);
    pause(500);
  }  
}


void blink14()
{
 while(1)
  {
    //high(23);  // For C3 board test.
    high(14);  // For DNA-RTC board test.
    pause(1000);
    //sleep(1);
    //low(23);  // For C3 board test.
    low(14);  // For DNA-RTC board test.
    pause(1000);
    //sleep(1);
  }  
}

void blink13()
{
  while(1)
  {
    //high(22);  // For C3 board test.
    high(13);  // For DNA-RTC board test.
    pause(500);
    //sleep(1);
    //low(22);  // For C3 board test.
    low(13);  // For DNA-RTC board test.
    pause(500);
    //sleep(1);
  }
}

void high(int pin)
{
 int mask = 1 << pin;
 OUTA |= mask;
 DIRA |= mask;
}

void low(int pin)
{
 int mask = 1 << pin;
 OUTA &= ~mask;
 DIRA |= mask;
}

void pause(int time)
{
 // Not sure if this is correct, but it seems to work as expected.
 waitcnt(((CLKFREQ/1000)*time) + CNT);
}



#ifndef HELPERS_DEEPSLEEP_H
#define HELPERS_DEEPSLEEP_H





/**********************************************************
*                                                         *
* Deep Sleep related functions                            *
*                                                         *
**********************************************************/
int getDeepSleepMax();

bool isDeepSleepEnabled();

bool readyForSleep();

void prepare_deepSleep(int dsdelay);

void deepSleepStart(int dsdelay);


#endif // HELPERS_DEEPSLEEP_H
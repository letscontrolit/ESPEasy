#ifndef HELPERS_NETWORK_H
#define HELPERS_NETWORK_H

/********************************************************************************************\
   Status LED
 \*********************************************************************************************/
#define PWMRANGE_FULL 1023
#define STATUS_PWM_NORMALVALUE (PWMRANGE_FULL >> 2)
#define STATUS_PWM_NORMALFADE (PWMRANGE_FULL >> 8)
#define STATUS_PWM_TRAFFICRISE (PWMRANGE_FULL >> 1)

void statusLED(bool traffic);


#endif
#ifndef Arduino_h
#define Arduino_h

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cstdio>

#include "Print.h"

extern "C" {
typedef uint8_t byte;
typedef uint8_t boolean;

/* sketch */
extern void setup(void);
extern void loop(void);
unsigned long millis(void);
}

#define PROGMEM
#define strnlen_P strnlen
#define pgm_read_byte_near(x) *(x)

#define yield(x) {}

#pragma GCC system_header
#define ERROR_PSC_PRINTF(fmt, ...) printf(("PubSubClient error: " fmt), ##__VA_ARGS__)
#define ERROR_PSC_PRINTF_P(fmt, ...) printf(("PubSubClient error: " fmt), ##__VA_ARGS__)
#ifdef DEBUG_PUBSUBCLIENT
#define DEBUG_PSC_PRINTF(fmt, ...) printf(("PubSubClient: " fmt), ##__VA_ARGS__)
#endif

#define _UNUSED_ __attribute__((unused))

#endif  // Arduino_h

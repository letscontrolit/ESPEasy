#include "../Globals/ESPEasy_time.h"

ESPEasy_time node_time;

uint32_t getUnixTime() {
    return node_time.getUnixTime();
}
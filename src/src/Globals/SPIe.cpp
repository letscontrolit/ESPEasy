#include "../Globals/SPIe.h"

#ifdef ESP32
SPIClass SPIe(HSPI);
#endif // ifdef ESP32

#ifndef WEBSERVER_WEBSERVER_HARDWAREPAGE_H
#define WEBSERVER_WEBSERVER_HARDWAREPAGE_H

#include "../WebServer/common.h"

enum class SPI_Options_e {
  None = 0,
  Vspi,
  Hspi,
  UserDefined
};

#ifdef WEBSERVER_HARDWARE

// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_hardware();

#endif // ifdef WEBSERVER_HARDWARE

#endif
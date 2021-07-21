#ifndef WEBSERVER_WEBSERVER_METRICSPAGE_H
#define WEBSERVER_WEBSERVER_METRICSPAGE_H

#include "../WebServer/common.h"


#ifdef WEBSERVER_METRICS

void handle_metrics();


String handle_matrics_value_name(const String& valName);


String handle_matrics_value_value(const String& valValue);

String handle_metrics_devices();


#endif    // ifdef WEBSERVER_METRICS

#endif
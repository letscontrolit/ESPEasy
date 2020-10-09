#ifndef WEBSERVER_WEBSERVER_DEVICESPAGE_H
#define WEBSERVER_WEBSERVER_DEVICESPAGE_H

#include "WebServer_common.h"


#ifdef WEBSERVER_DEVICES

# include "src/Globals/Nodes.h"
# include "src/Globals/Device.h"
# include "src/Globals/CPlugins.h"
# include "src/Globals/Plugins.h"

# include "src/Static/WebStaticData.h"

# include "src/Helpers/_CPlugin_SensorTypeHelper.h"
# include "src/Helpers/StringGenerator_GPIO.h"

#include <ESPeasySerial.h>


void handle_devices();

// ********************************************************************************
// Add a device select dropdown list
// TODO TD-er: Add JavaScript filter:
//             https://www.w3schools.com/howto/howto_js_filter_dropdown.asp
// ********************************************************************************
void addDeviceSelect(const String& name,  int choice);

// ********************************************************************************
// Collect all submitted form data and store the task settings
// ********************************************************************************
void handle_devices_CopySubmittedSettings(taskIndex_t taskIndex, pluginID_t taskdevicenumber);

// ********************************************************************************
// Show table with all selected Tasks/Devices
// ********************************************************************************
void handle_devicess_ShowAllTasksTable(byte page);

void format_originating_node(byte remoteUnit);

void format_I2C_port_description(taskIndex_t x);

void format_SPI_port_description(int8_t spi_gpios[3]);

void format_I2C_pin_description();

void format_SPI_pin_description(int8_t spi_gpios[3], taskIndex_t x);



// ********************************************************************************
// Show the task settings page
// ********************************************************************************
void handle_devices_TaskSettingsPage(taskIndex_t taskIndex, byte page);

void devicePage_show_pin_config(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

void devicePage_show_serial_config(taskIndex_t taskIndex);

void devicePage_show_I2C_config(taskIndex_t taskIndex);

void devicePage_show_output_data_type(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

void devicePage_show_controller_config(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

void devicePage_show_interval_config(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

void devicePage_show_task_values(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

#endif // ifdef WEBSERVER_DEVICES



#endif
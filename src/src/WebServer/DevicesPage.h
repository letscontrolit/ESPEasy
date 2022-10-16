#ifndef WEBSERVER_WEBSERVER_DEVICESPAGE_H
#define WEBSERVER_WEBSERVER_DEVICESPAGE_H

#include "../WebServer/common.h"


#ifdef WEBSERVER_DEVICES

#include "../DataTypes/DeviceIndex.h"
#include "../DataTypes/TaskIndex.h"
#include "../DataTypes/PluginID.h"

#include "../Static/WebStaticData.h"


void handle_devices();

// ********************************************************************************
// Add a device select dropdown list
// TODO TD-er: Add JavaScript filter:
//             https://www.w3schools.com/howto/howto_js_filter_dropdown.asp
// ********************************************************************************
void addDeviceSelect(const __FlashStringHelper * name,  int choice);

// ********************************************************************************
// Collect all submitted form data and store the task settings
// ********************************************************************************
void handle_devices_CopySubmittedSettings(taskIndex_t taskIndex, pluginID_t taskdevicenumber);

// ********************************************************************************
// Show table with all selected Tasks/Devices
// ********************************************************************************
void handle_devicess_ShowAllTasksTable(uint8_t page);

#if FEATURE_ESPEASY_P2P
void format_originating_node(uint8_t remoteUnit);
#endif
void format_I2C_port_description(taskIndex_t x);

void format_SPI_port_description(int8_t spi_gpios[3]);

void format_I2C_pin_description(taskIndex_t x);

void format_SPI_pin_description(int8_t spi_gpios[3], taskIndex_t x, bool showCSpin = true);



// ********************************************************************************
// Show the task settings page
// ********************************************************************************
void handle_devices_TaskSettingsPage(taskIndex_t taskIndex, uint8_t page);

void devicePage_show_pin_config(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

void devicePage_show_serial_config(taskIndex_t taskIndex);

void devicePage_show_I2C_config(taskIndex_t taskIndex);

void devicePage_show_output_data_type(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

#if FEATURE_PLUGIN_STATS
void devicePage_show_task_statistics(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);
#endif // if FEATURE_PLUGIN_STATS

void devicePage_show_controller_config(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

void devicePage_show_interval_config(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

void devicePage_show_task_values(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

#endif // ifdef WEBSERVER_DEVICES



#endif
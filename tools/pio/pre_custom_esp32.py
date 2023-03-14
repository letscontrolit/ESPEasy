Import("env")
import os

# access to global construction environment
#print env

# Dump construction environment (for debug purpose)
#print env.Dump()

# append extra flags to global build environment
# which later will be used to build:
# - project source code
# - frameworks
# - dependent libraries
custom_defines = []
#env.Append(CPPDEFINES=[
  # ,"NO_HTTP_UPDATER"
  # ,("WEBSERVER_RULES_DEBUG", "0")
#])
if os.path.isfile('src/Custom.h'):
  custom_defines=["-DUSE_CUSTOM_H"]
else:
  custom_defines=[
    "-DPLUGIN_BUILD_MAX_ESP32",

    "-DFEATURE_ARDUINO_OTA=1",


    "-DFEATURE_MQTT_TLS=1",
    "-DFEATURE_EXT_RTC=1",
    "-DFEATURE_SD=1",
    "-DFEATURE_I2CMULTIPLEXER=1",
    "-DFEATURE_TRIGONOMETRIC_FUNCTIONS_RULES=1",

    "-DFEATURE_PLUGIN_STATS=1",
    "-DFEATURE_CHART_JS=1",

    "-DFEATURE_SETTINGS_ARCHIVE=1",
    "-DFEATURE_ESPEASY_P2P=1",
    "-DFEATURE_CUSTOM_PROVISIONING=1",
    "-DDEFAULT_FACTORY_DEFAULT_DEVICE_MODEL=0",
    "-DDEFAULT_PROVISIONING_FETCH_RULES1=true",
    "-DDEFAULT_PROVISIONING_FETCH_RULES2=true",
    "-DDEFAULT_PROVISIONING_FETCH_SECURITY=true",
    "-DDEFAULT_PROVISIONING_FETCH_CONFIG=true",
    "-DDEFAULT_PROVISIONING_FETCH_PROVISIONING=true",
    "-DDEFAULT_PROVISIONING_SAVE_URL=true",
    "-DDEFAULT_PROVISIONING_SAVE_CREDENTIALS=true",
    "-DDEFAULT_PROVISIONING_ALLOW_FETCH_COMMAND=true"

  ]



my_flags = env.ParseFlags(env['BUILD_FLAGS'])
my_defines = my_flags.get("CPPDEFINES")
env.Append(BUILD_FLAGS=custom_defines)
#defines = {k: v for (k, v) in my_defines}

print("\u001b[32m Custom PIO configuration check \u001b[0m")
# print the defines
print("\u001b[33m CPPDEFINES: \u001b[0m  {}".format(my_defines))
print("\u001b[33m Custom CPPDEFINES: \u001b[0m  {}".format(custom_defines))
print("\u001b[32m ------------------------------- \u001b[0m")


if (len(my_defines) == 0):
  print("\u001b[31m No defines are set, probably configuration error. \u001b[0m")
  raise ValueError

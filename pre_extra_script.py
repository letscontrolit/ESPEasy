Import("env")

# access to global construction environment
#print env

# Dump construction environment (for debug purpose)
#print env.Dump()

# append extra flags to global build environment
# which later will be used to build:
# - project source code
# - frameworks
# - dependent libraries
env.Append(CPPDEFINES=[
  "PIO_FRAMEWORK_ARDUINO_ESPRESSIF_SDK22x",
  "CONTROLLER_SET_ALL",
  "NOTIFIER_SET_NONE",
  "PLUGIN_SET_ONLY_SWITCH",
  "USES_P002",  # ADC
  "USES_P028",  # BME280
  "USES_P036",  # FrameOLED
  "USES_P049",  # MHZ19
  "USES_P052",  # SenseAir
  "USES_P056",  # SDS011-Dust
  "USES_P082",  # GPS
  "USES_P085",  # AcuDC24x

  ("WEBSERVER_RULES_DEBUG", "0")
])

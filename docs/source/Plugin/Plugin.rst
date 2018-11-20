.. ESP Easy colors (set in cutom.css)
.. role:: blue
.. role:: red
.. role:: green
.. role:: yellow
.. role:: orange
.. role:: cyan
.. role:: purple

Plugins
*******


List of official plugins
------------------------

There's three different released versions of ESP Easy:

:green:`NORMAL` is the stable release, you can consider these plugins reliable and you can use these in production.

:yellow:`TESTING` is the release with new plugins that have not yet been fully tested and proven stable.

:red:`DEVELOPMENT` is used for plugins that are still being developed and are not considered stable at all.

.. csv-table::
   :header: "Plugin number", "Plugin status", "Plugin type", "GitHub link", "Comment"
   :widths: 5, 8, 15, 20, 40

   "P001",":green:`NORMAL`","Switch Input","`P001_Switch.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P001_Switch.ino P001_Switch.ino>`_",""
   "P002",":green:`NORMAL`","Analog Input","`P002_ADC.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P002_ADC.ino P002_ADC.ino>`_",""
   "P003",":green:`NORMAL`","Generic","`P003_Pulse.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P003_Pulse.ino P003_Pulse.ino>`_",""
   "P004",":green:`NORMAL`","Environment","`P004_Dallas.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P004_Dallas.ino P004_Dallas.ino>`_",""
   "P005",":green:`NORMAL`","Environment","`P005_DHT.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P005_DHT.ino P005_DHT.ino>`_",""
   "P006",":green:`NORMAL`","Environment","`P006_BMP085.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P006_BMP085.ino P006_BMP085.ino>`_",""
   "P007",":green:`NORMAL`","Extra IO","`P007_PCF8591.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P007_PCF8591.ino P007_PCF8591.ino>`_",""
   "P008",":green:`NORMAL`","RFID","`P008_RFID.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P008_RFID.ino P008_RFID.ino>`_",""
   "P009",":green:`NORMAL`","Extra IO","`P009_MCP.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P009_MCP.ino P009_MCP.ino>`_",""
   "P010",":green:`NORMAL`","Light/Lux","`P010_BH1750.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P010_BH1750.ino P010_BH1750.ino>`_",""
   "P011",":green:`NORMAL`","Extra IO","`P011_PME.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P011_PME.ino P011_PME.ino>`_",""
   "P012",":green:`NORMAL`","Display","`P012_LCD.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P012_LCD.ino P012_LCD.ino>`_",""
   "P013",":green:`NORMAL`","Distance","`P013_HCSR04.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P013_HCSR04.ino P013_HCSR04.ino>`_",""
   "P014",":green:`NORMAL`","Environment","`P014_SI7021.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P014_SI7021.ino P014_SI7021.ino>`_",""
   "P015",":green:`NORMAL`","Light/Lux","`P015_TSL2561.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P015_TSL2561.ino P015_TSL2561.ino>`_",""
   "P016",":green:`NORMAL`","Communication","`P016_IR.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P016_IR.ino P016_IR.ino>`_",""
   "P017",":green:`NORMAL`","RFID","`P017_PN532.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P017_PN532.ino P017_PN532.ino>`_",""
   "P018",":green:`NORMAL`","Dust","`P018_Dust.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P018_Dust.ino P018_Dust.ino>`_",""
   "P019",":green:`NORMAL`","Switch input","`P019_PCF8574.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P019_PCF8574.ino P019_PCF8574.ino>`_",""
   "P020",":green:`NORMAL`","Communication","`P020_Ser2Net.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P020_Ser2Net.ino P020_Ser2Net.ino>`_",""
   "P021",":green:`NORMAL`","Regulator","`P021_Level.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P021_Level.ino P021_Level.ino>`_",""
   "P022",":green:`NORMAL`","Extra IO","`P022_PCA9685.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P022_PCA9685.ino P022_PCA9685.ino>`_",""
   "P023",":green:`NORMAL`","Display","`P023_OLED.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P023_OLED.ino P023_OLED.ino>`_",""
   "P024",":green:`NORMAL`","Environment","`P024_MLX90614.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P024_MLX90614.ino P024_MLX90614.ino>`_",""
   "P025",":green:`NORMAL`","Analog input","`P025_ADS1115.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P025_ADS1115.ino P025_ADS1115.ino>`_",""
   "P026",":green:`NORMAL`","Generic","`P026_Sysinfo.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P026_Sysinfo.ino P026_Sysinfo.ino>`_",""
   "P027",":green:`NORMAL`","Energy (DC)","`P027_INA219.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P027_INA219.ino P027_INA219.ino>`_",""
   "P028",":green:`NORMAL`","Environment","`P028_BME280.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P028_BME280.ino P028_BME280.ino>`_",""
   "P029",":green:`NORMAL`","Output","`P029_Output.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P029_Output.ino P029_Output.ino>`_",""
   "P030",":green:`NORMAL`","Environment","`P030_BMP280.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P030_BMP280.ino P030_BMP280.ino>`_",""
   "P031",":green:`NORMAL`","Environment","`P031_SHT1X.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P031_SHT1X.ino P031_SHT1X.ino>`_",""
   "P032",":green:`NORMAL`","Environment","`P032_MS5611.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P032_MS5611.ino P032_MS5611.ino>`_",""
   "P033",":green:`NORMAL`","Generic","`P033_Dummy.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P033_Dummy.ino P033_Dummy.ino>`_",""
   "P034",":green:`NORMAL`","Environment","`P034_DHT12.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P034_DHT12.ino P034_DHT12.ino>`_",""
   "P035",":green:`NORMAL`","Communication","`P035_IRTX.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P035_IRTX.ino P035_IRTX.ino>`_",""
   "P036",":green:`NORMAL`","Display","`P036_FrameOLED.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P036_FrameOLED.ino P036_FrameOLED.ino>`_",""
   "P037",":green:`NORMAL`","Generic","`P037_MQTTImport.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P037_MQTTImport.ino P037_MQTTImport.ino>`_",""
   "P038",":green:`NORMAL`","Output","`P038_NeoPixel.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P038_NeoPixel.ino P038_NeoPixel.ino>`_",""
   "P039",":green:`NORMAL`","Environment","`P039_Thermocouple.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P039_Thermocouple.ino P039_Thermocouple.ino>`_",""
   "P040",":green:`NORMAL`","RFID","`P040_ID12.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P040_ID12.ino P040_ID12.ino>`_",""
   "P041",":green:`NORMAL`","Output","`P041_NeoClock.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P041_NeoClock.ino P041_NeoClock.ino>`_",""
   "P042",":green:`NORMAL`","Output","`P042_Candle.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P042_Candle.ino P042_Candle.ino>`_",""
   "P043",":green:`NORMAL`","Output","`P043_ClkOutput.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P043_ClkOutput.ino P043_ClkOutput.ino>`_",""
   "P044",":green:`NORMAL`","Communication","`P044_P1WifiGateway.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P044_P1WifiGateway.ino P044_P1WifiGateway.ino>`_",""
   "P045",":green:`NORMAL`","Gyro","`P045_MPU6050.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P045_MPU6050.ino P045_MPU6050.ino>`_",""
   "P046",":yellow:`TESTING`","Hardware","`P046_VentusW266.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P046_VentusW266.ino P046_VentusW266.ino>`_","This one is suppressed by default, you need to compile your own version if you want to use it."
   "P047",":yellow:`TESTING`","Environment","`P047_i2c-soil-moisture-sensor.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P047_i2c-soil-moisture-sensor.ino P047_i2c-soil-moisture-sensor.ino>`_",""
   "P048",":yellow:`TESTING`","Motor","`P048_Motorshield_v2.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P048_Motorshield_v2.ino P048_Motorshield_v2.ino>`_","Adafruit Motorshield"
   "P049",":green:`NORMAL`","Gases","`P049_MHZ19.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P049_MHZ19.ino P049_MHZ19.ino>`_",""
   "P050",":red:`DEVELOPMENT`","Color","`P050_TCS34725.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P050_TCS34725.ino P050_TCS34725.ino>`_",""
   "P051",":yellow:`TESTING`","Environment","`P051_AM2320.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P051_AM2320.ino P051_AM2320.ino>`_",""
   "P052",":green:`NORMAL`","Gases","`P052_SenseAir.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P052_SenseAir.ino P052_SenseAir.ino>`_",""
   "P053",":yellow:`TESTING`","Dust","`P053_PMSx003.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P053_PMSx003.ino P053_PMSx003.ino>`_",""
   "P054",":yellow:`TESTING`","Communication","`P054_DMX512.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P054_DMX512.ino P054_DMX512.ino>`_",""
   "P055",":yellow:`TESTING`","Notify","`P055_Chiming.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P055_Chiming.ino P055_Chiming.ino>`_",""
   "P056",":red:`DEVELOPMENT`","Dust","`P056_SDS011-Dust.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P056_SDS011-Dust.ino P056_SDS011-Dust.ino>`_",""
   "P057",":yellow:`TESTING`","Display","`P057_HT16K33_LED.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P057_HT16K33_LED.ino P057_HT16K33_LED.ino>`_",""
   "P058",":yellow:`TESTING`","Keypad","`P058_HT16K33_KeyPad.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P058_HT16K33_KeyPad.ino P058_HT16K33_KeyPad.ino>`_",""
   "P059",":green:`NORMAL`","Switch input","`P059_Encoder.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P059_Encoder.ino P059_Encoder.ino>`_",""
   "P060",":yellow:`TESTING`","Analog input","`P060_MCP3221.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P060_MCP3221.ino P060_MCP3221.ino>`_",""
   "P061",":yellow:`TESTING`","Keypad","`P061_KeyPad.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P061_KeyPad.ino P061_KeyPad.ino>`_",""
   "P062",":yellow:`TESTING`","Keypad","`P062_MPR121_KeyPad.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P062_MPR121_KeyPad.ino P062_MPR121_KeyPad.ino>`_",""
   "P063",":green:`NORMAL`","Keypad","`P063_TTP229_KeyPad.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P063_TTP229_KeyPad.ino P063_TTP229_KeyPad.ino>`_",""
   "P064",":red:`DEVELOPMENT`","Gesture","`P064_APDS9960.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P064_APDS9960.ino P064_APDS9960.ino>`_",""
   "P065",":yellow:`TESTING`","Notify","`P065_DRF0299_MP3.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P065_DRF0299_MP3.ino P065_DRF0299_MP3.ino>`_",""
   "P066",":yellow:`TESTING`","Color","`P066_VEML6040.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P066_VEML6040.ino P066_VEML6040.ino>`_",""
   "P067",":yellow:`TESTING`","Weight","`P067_HX711_Load_Cell.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P067_HX711_Load_Cell.ino P067_HX711_Load_Cell.ino>`_",""
   "P068",":yellow:`TESTING`","Environment","`P068_SHT3x.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P068_SHT3x.ino P068_SHT3x.ino>`_",""
   "P069",":yellow:`TESTING`","Environment","`P069_LM75A.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P069_LM75A.ino P069_LM75A.ino>`_",""
   "P070",":yellow:`TESTING`","Output","`P070_NeoPixel_Clock.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P070_NeoPixel_Clock.ino P070_NeoPixel_Clock.ino>`_","This one is suppressed by default, you need to compile your own version if you want to use it."
   "P071",":yellow:`TESTING`","Communication","`P071_Kamstrup401.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P071_Kamstrup401.ino P071_Kamstrup401.ino>`_",""
   "P072",":yellow:`TESTING`","Environment","`P072_HDC1080.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P072_HDC1080.ino P072_HDC1080.ino>`_",""
   "P073",":yellow:`TESTING`","Display","`P073_7DGT.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P073_7DGT.ino P073_7DGT.ino>`_",""
   "P074",":yellow:`TESTING`","Light/Lux","`P074_TSL2591.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P074_TSL2591.ino P074_TSL2591.ino>`_",""
   "P075",":yellow:`TESTING`","Display","`P075_Nextion.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P075_Nextion.ino P075_Nextion.ino>`_","LCD Color Graphic Touch Screen. Plugin can update display and receive/send touch events."
   "P076",":yellow:`TESTING`","Energy (AC)","`P076_HLW8012.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P076_HLW8012.ino P076_HLW8012.ino>`_",""
   "P077",":yellow:`TESTING`","Energy (AC)","`P077_CSE7766.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P077_CSE7766.ino P077_CSE7766.ino>`_","This plugin is specifically made for Sonoff devices Sonoff S31 and Sonoff Pow R2"
   "P078",":yellow:`TESTING`","Energy (AC)","`P078_Eastron.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P078_Eastron.ino P078_Eastron.ino>`_",""
   "P079",":yellow:`TESTING`","Motor","`P079_Wemos_Motorshield.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P079_Wemos_Motorshield.ino P079_Wemos_Motorshield.ino>`_","Wemos Motorshield"
   "P080",":yellow:`TESTING`","Switch input","`P080_DallasIButton.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P080_DallasIButton.ino P080_DallasIButton.ino>`_",""
   "P081",":yellow:`TESTING`","Generic","`P081_Cron.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P081_Cron.ino P081_Cron.ino>`_",""

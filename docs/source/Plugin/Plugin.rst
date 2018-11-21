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
   :header: "Plugin number", "Plugin status", "Plugin name", "Used by", "GitHub link", "Comment"
   :widths: 5, 8, 30, 15, 30, 40

   "P001",":green:`NORMAL`",":cyan:`Switch Input - Switch`","","`P001_Switch.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P001_Switch.ino>`_",""
   "P002",":green:`NORMAL`",":cyan:`Analog Input - Internal`","","`P002_ADC.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P002_ADC.ino>`_",""
   "P003",":green:`NORMAL`",":cyan:`Generic - Pulse counter`","","`P003_Pulse.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P003_Pulse.ino>`_",""
   "P004",":green:`NORMAL`",":cyan:`Environment - DS18b20`","","`P004_Dallas.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P004_Dallas.ino>`_",""
   "P005",":green:`NORMAL`",":cyan:`Environment - DHT11/12/22/SONOFF2301/7021`","","`P005_DHT.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P005_DHT.ino>`_",""
   "P006",":green:`NORMAL`",":cyan:`Environment - BMP085/180`","","`P006_BMP085.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P006_BMP085.ino>`_",""
   "P007",":green:`NORMAL`",":cyan:`Analog input - PCF8591`","","`P007_PCF8591.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P007_PCF8591.ino>`_",""
   "P008",":green:`NORMAL`",":cyan:`RFID - Wiegand`","","`P008_RFID.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P008_RFID.ino>`_",""
   "P009",":green:`NORMAL`",":cyan:`Switch Input - MCP23017`","","`P009_MCP.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P009_MCP.ino>`_",""
   "P010",":green:`NORMAL`",":cyan:`Light/Lux - BH1750`","","`P010_BH1750.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P010_BH1750.ino>`_",""
   "P011",":green:`NORMAL`",":cyan:`Extra IO - ProMini Extender`","","`P011_PME.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P011_PME.ino>`_",""
   "P012",":green:`NORMAL`",":cyan:`Display - LCD2004`","","`P012_LCD.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P012_LCD.ino>`_",""
   "P013",":green:`NORMAL`",":cyan:`Distance - HC-SR04/RCW-0001`","","`P013_HCSR04.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P013_HCSR04.ino>`_",""
   "P014",":green:`NORMAL`",":cyan:`Environment - SI7021/HTU21D`","","`P014_SI7021.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P014_SI7021.ino>`_",""
   "P015",":green:`NORMAL`",":cyan:`Light/Lux - TSL2561`","","`P015_TSL2561.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P015_TSL2561.ino>`_",""
   "P016",":green:`NORMAL`",":cyan:`Communication - TSOP4838`","","`P016_IR.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P016_IR.ino>`_",""
   "P017",":green:`NORMAL`",":cyan:`RFID - PN532`","","`P017_PN532.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P017_PN532.ino>`_",""
   "P018",":green:`NORMAL`",":cyan:`Dust - Sharp GP2Y10`","","`P018_Dust.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P018_Dust.ino>`_",""
   "P019",":green:`NORMAL`",":cyan:`Switch input - PCF8574`","","`P019_PCF8574.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P019_PCF8574.ino>`_",""
   "P020",":green:`NORMAL`",":cyan:`Communication - Serial Server`","","`P020_Ser2Net.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P020_Ser2Net.ino>`_",""
   "P021",":green:`NORMAL`",":cyan:`Regulator - Level Control`","","`P021_Level.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P021_Level.ino>`_",""
   "P022",":green:`NORMAL`",":cyan:`Extra IO - PCA9685`","","`P022_PCA9685.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P022_PCA9685.ino>`_",""
   "P023",":green:`NORMAL`",":cyan:`Display - OLED SSD1306`","","`P023_OLED.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P023_OLED.ino>`_",""
   "P024",":green:`NORMAL`",":cyan:`Environment - MLX90614`","","`P024_MLX90614.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P024_MLX90614.ino>`_",""
   "P025",":green:`NORMAL`",":cyan:`Analog input - ADS1115`","","`P025_ADS1115.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P025_ADS1115.ino>`_",""
   "P026",":green:`NORMAL`",":cyan:`Generic - System Info`","","`P026_Sysinfo.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P026_Sysinfo.ino>`_",""
   "P027",":green:`NORMAL`",":cyan:`Energy (DC) - INA219`","","`P027_INA219.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P027_INA219.ino>`_",""
   "P028",":green:`NORMAL`",":cyan:`Environment - BMx280`","","`P028_BME280.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P028_BME280.ino>`_",""
   "P029",":green:`NORMAL`",":cyan:`Output - Domoticz MQTT Helper`","","`P029_Output.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P029_Output.ino>`_",""
   "P030",":green:`NORMAL`",":cyan:`Environment - BMP280`","","`P030_BMP280.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P030_BMP280.ino>`_",""
   "P031",":green:`NORMAL`",":cyan:`Environment - SHT1X`","","`P031_SHT1X.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P031_SHT1X.ino>`_",""
   "P032",":green:`NORMAL`",":cyan:`Environment - MS5611 (GY-63)`","","`P032_MS5611.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P032_MS5611.ino>`_",""
   "P033",":green:`NORMAL`",":cyan:`Generic - Dummy Device`","","`P033_Dummy.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P033_Dummy.ino>`_",""
   "P034",":green:`NORMAL`",":cyan:`Environment - DHT12 (I2C)`","","`P034_DHT12.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P034_DHT12.ino>`_",""
   "P035",":green:`NORMAL`",":cyan:`Communication - IR Transmit`","","`P035_IRTX.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P035_IRTX.ino>`_",""
   "P036",":green:`NORMAL`",":cyan:`Display - OLED SSD1306/SH1106 Framed`","","`P036_FrameOLED.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P036_FrameOLED.ino>`_",""
   "P037",":green:`NORMAL`",":cyan:`Generic - MQTT Import`","","`P037_MQTTImport.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P037_MQTTImport.ino>`_",""
   "P038",":green:`NORMAL`",":cyan:`Output - NeoPixel (Basic)`","","`P038_NeoPixel.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P038_NeoPixel.ino>`_",""
   "P039",":green:`NORMAL`",":cyan:`Environment - Thermocouple`","","`P039_Thermocouple.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P039_Thermocouple.ino>`_",""
   "P040",":green:`NORMAL`",":cyan:`RFID - ID12LA/RDM6300`","","`P040_ID12.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P040_ID12.ino>`_",""
   "P041",":green:`NORMAL`",":cyan:`Output - NeoPixel (Word Clock)`","","`P041_NeoClock.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P041_NeoClock.ino>`_",""
   "P042",":green:`NORMAL`",":cyan:`Output - NeoPixel (Candle)`","","`P042_Candle.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P042_Candle.ino>`_",""
   "P043",":green:`NORMAL`",":cyan:`Output - Clock`","","`P043_ClkOutput.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P043_ClkOutput.ino>`_",""
   "P044",":green:`NORMAL`",":cyan:`Communication - P1 Wifi Gateway`","","`P044_P1WifiGateway.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P044_P1WifiGateway.ino>`_",""
   "P045",":green:`TESTING`",":cyan:`Gyro - MPU 6050`","","`P045_MPU6050.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P045_MPU6050.ino>`_",""
   "P046",":yellow:`TESTING`",":cyan:`Hardware - Ventus W266`","","`P046_VentusW266.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P046_VentusW266.ino>`_","This one is suppressed by default, you need to compile your own version if you want to use it."
   "P047",":yellow:`TESTING`",":cyan:`Environment - Soil moisture sensor`","","`P047_i2c-soil-moisture-sensor.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P047_i2c-soil-moisture-sensor.ino>`_",""
   "P048",":yellow:`TESTING`",":cyan:`Motor - Adafruit Motorshield v2`","","`P048_Motorshield_v2.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P048_Motorshield_v2.ino>`_","Adafruit Motorshield"
   "P049",":green:`NORMAL`",":cyan:`Gases - CO2 MH-Z19`","","`P049_MHZ19.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P049_MHZ19.ino>`_",""
   "P050",":red:`DEVELOPMENT`",":cyan:`Color - TCS34725`","","`P050_TCS34725.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P050_TCS34725.ino>`_",""
   "P051",":yellow:`TESTING`",":cyan:`Environment - AM2320`","","`P051_AM2320.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P051_AM2320.ino>`_",""
   "P052",":green:`NORMAL`",":cyan:`Gases - CO2 Senseair`",":ref:`P052_S8_page`","`P052_SenseAir.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P052_SenseAir.ino>`_",""
   "P053",":yellow:`TESTING`",":cyan:`Dust - PMSx003`","","`P053_PMSx003.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P053_PMSx003.ino>`_",""
   "P054",":yellow:`TESTING`",":cyan:`Communication - DMX512 TX`","","`P054_DMX512.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P054_DMX512.ino>`_",""
   "P055",":yellow:`TESTING`",":cyan:`Notify - Chiming`","","`P055_Chiming.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P055_Chiming.ino>`_",""
   "P056",":red:`DEVELOPMENT`",":cyan:`Dust - SDS011/018/198`","","`P056_SDS011-Dust.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P056_SDS011-Dust.ino>`_",""
   "P057",":yellow:`TESTING`",":cyan:`Display - HT16K33`","","`P057_HT16K33_LED.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P057_HT16K33_LED.ino>`_",""
   "P058",":yellow:`TESTING`",":cyan:`Keypad - HT16K33`","","`P058_HT16K33_KeyPad.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P058_HT16K33_KeyPad.ino>`_",""
   "P059",":green:`NORMAL`",":cyan:`Switch input - Rotary Encoder`","","`P059_Encoder.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P059_Encoder.ino>`_",""
   "P060",":yellow:`TESTING`",":cyan:`Analog input - MCP3221`","","`P060_MCP3221.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P060_MCP3221.ino>`_",""
   "P061",":yellow:`TESTING`",":cyan:`Keypad`","","`P061_KeyPad.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P061_KeyPad.ino>`_",""
   "P062",":yellow:`TESTING`",":cyan:`Keypad`","","`P062_MPR121_KeyPad.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P062_MPR121_KeyPad.ino>`_",""
   "P063",":green:`NORMAL`",":cyan:`Keypad`","","`P063_TTP229_KeyPad.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P063_TTP229_KeyPad.ino>`_",""
   "P064",":red:`DEVELOPMENT`",":cyan:`Gesture`","","`P064_APDS9960.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P064_APDS9960.ino>`_",""
   "P065",":yellow:`TESTING`",":cyan:`Notify`","","`P065_DRF0299_MP3.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P065_DRF0299_MP3.ino>`_",""
   "P066",":yellow:`TESTING`",":cyan:`Color`","","`P066_VEML6040.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P066_VEML6040.ino>`_",""
   "P067",":yellow:`TESTING`",":cyan:`Weight`","","`P067_HX711_Load_Cell.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P067_HX711_Load_Cell.ino>`_",""
   "P068",":yellow:`TESTING`",":cyan:`Environment`","","`P068_SHT3x.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P068_SHT3x.ino>`_",""
   "P069",":yellow:`TESTING`",":cyan:`Environment`","","`P069_LM75A.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P069_LM75A.ino>`_",""
   "P070",":yellow:`TESTING`",":cyan:`Output`","","`P070_NeoPixel_Clock.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P070_NeoPixel_Clock.ino>`_","This one is suppressed by default, you need to compile your own version if you want to use it."
   "P071",":yellow:`TESTING`",":cyan:`Communication`","","`P071_Kamstrup401.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P071_Kamstrup401.ino>`_",""
   "P072",":yellow:`TESTING`",":cyan:`Environment`","","`P072_HDC1080.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P072_HDC1080.ino>`_",""
   "P073",":yellow:`TESTING`",":cyan:`Display`","","`P073_7DGT.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P073_7DGT.ino>`_",""
   "P074",":yellow:`TESTING`",":cyan:`Light/Lux`","","`P074_TSL2591.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P074_TSL2591.ino>`_",""
   "P075",":yellow:`TESTING`",":cyan:`Display`","","`P075_Nextion.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P075_Nextion.ino>`_","LCD Color Graphic Touch Screen. Plugin can update display and receive/send touch events."
   "P076",":yellow:`TESTING`",":cyan:`Energy (AC)`","","`P076_HLW8012.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P076_HLW8012.ino>`_",""
   "P077",":yellow:`TESTING`",":cyan:`Energy (AC)`","","`P077_CSE7766.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P077_CSE7766.ino>`_","This plugin is specifically made for Sonoff devices Sonoff S31 and Sonoff Pow R2"
   "P078",":yellow:`TESTING`",":cyan:`Energy (AC)`","","`P078_Eastron.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P078_Eastron.ino>`_",""
   "P079",":yellow:`TESTING`",":cyan:`Motor`","","`P079_Wemos_Motorshield.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P079_Wemos_Motorshield.ino>`_","Wemos Motorshield"
   "P080",":yellow:`TESTING`",":cyan:`Switch input`","","`P080_DallasIButton.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P080_DallasIButton.ino>`_",""
   "P081",":yellow:`TESTING`",":cyan:`Generic`","","`P081_Cron.ino <https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P081_Cron.ino>`_",""

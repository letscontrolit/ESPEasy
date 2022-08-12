By the physical design of these modules, using NeoPixel leds for display that can consume up to 60 mA each, the power consumption is dependent on the number of neopixels that are activated and the brightness that is set. On a set of NeoPixels, like consisting of 64 pixels, the max. power used 60 mA * 64 = 3840 mA @ 5V = 19.2 watt with all leds on at max brightness (255), so it requires an adequate power supply, preferably separate from the power supply used to power the ESP.

When reducing the brightness to 40% (0.4 * 255 = 102), that power is reduced to ca. 1.5 A @ 5V = ca. 7.5 watt. 40% is usually enough during daylight conditions, at night the brightness can often be reduced to 20%.

For a set of 300 pixels, the max. power would count up to 300 * 60 mA @ 5V = 90 W. At 40% brightness, that is reduced to 36 W.

Reasoning back from the available power supply max. load (f.e. 30 W), the max. allowed brightness can be calculated by dividing the max. load by the voltage (5V) and the number of pixels (300) = 20 mA, 60 / 20 = 3 = ~33% brightness = 85. That will still be bright enough, but will put quite a strain at the power supply.
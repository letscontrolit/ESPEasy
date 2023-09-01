// NeoPixelBusLg
//
// This example demonstrates the use of the NeoPixelBusLg
// with integrated luminance and gamma support
//
// There is serial output of the current state so you can
// confirm and follow along
//

#include <NeoPixelBusLg.h> // instead of NeoPixelBus.h

const uint16_t PixelCount = 96; // set to the number of pixels in your strip
const uint8_t PixelPin = 14;  // make sure to set this to the correct pin, ignored for Esp8266

RgbColor red(255, 0, 0);
RgbColor green(0, 255, 0);
RgbColor blue(0, 0, 255);
RgbColor black(0, 0, 0);
RgbColor white(255, 255, 255);

// Make sure to provide the correct color order feature
// for your NeoPixels
NeoPixelBusLg<NeoRgbFeature, NeoWs2812xMethod> strip(PixelCount, PixelPin);

// If speed is an issue and memory is not, then you can use the gamma table variant
// which is much faster but uses 256 bytes of RAM
// NeoPixelBusLg<NeoRgbFeature, NeoWs2812xMethod, NeoGammaTableMethod> strip(PixelCount, PixelPin);

// If you want to turn gamma correction off, then you can use the null gamma method
// NeoPixelBusLg<NeoRgbFeature, NeoWs2812xMethod, NeoGammaNullMethod> strip(PixelCount, PixelPin);

// If you use a LED driver between the NeoPixel chip and the LEDs that require the PWM range inverted
// NeoPixelBusLg<NeoRgbFeature, NeoWs2812xMethod, NeoGammaInvertMethod<NeoGammaNullMethod>> strip(PixelCount, PixelPin);

void setup()
{
    Serial.begin(115200);
    while (!Serial); // wait for serial attach

    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();

    // this resets all the neopixels to an off state
    strip.Begin();
    strip.SetLuminance(128); // (0-255) - initially at half brightness
    strip.Show();

    Serial.println();
    Serial.println("Running...");
}

void loop()
{
    static const uint8_t c_MinBrightness = 0;
    static const uint8_t c_MaxBrightness = 255;
    
    static int8_t direction = -1; // start with dimming

    uint8_t luminance = strip.GetLuminance();

    Serial.print(direction);
    Serial.print("  ");
    Serial.println(luminance);

    delay(200);

    // swap direction of luminance when limits are reached
    //
    if (direction < 0 && luminance <= c_MinBrightness)
    {
        direction = 1;
    }
    else if (direction > 0 && luminance >= c_MaxBrightness)
    {
        direction = -1;
    }
    else
    {
        luminance += direction;
    }

    strip.SetLuminance(luminance);

    // draw something
    //
    uint16_t half = strip.PixelCount() / 2;
    DrawGradient(green, black, 0, half - 1);
    DrawGradient(black, red, half, strip.PixelCount() - 1);

    // show the results
    strip.Show();
}

void DrawGradient(RgbColor startColor, 
        RgbColor finishColor, 
        uint16_t startIndex, 
        uint16_t finishIndex)
{
    uint16_t delta = finishIndex - startIndex;
    
    for (uint16_t index = startIndex; index < finishIndex; index++)
    {
        float progress = static_cast<float>(index - startIndex) / delta;
        RgbColor color = RgbColor::LinearBlend(startColor, finishColor, progress);
        strip.SetPixelColor(index, color);
    }
}
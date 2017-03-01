# esp8266-oled-ssd1306

This is a driver for the SSD1306 based 128x64 pixel OLED display running on the Arduino/ESP8266 platform.

You can either download this library as a zip file and unpack it to your Arduino/libraries folder or (once it has been added) choose it from the Arduino library manager.

## Credits
Many thanks go to Fabrice Weinberg (@FWeinb) for optimizing and refactoring the UI library.
The init sequence for the SSD1306 was inspired by Adafruits library for the same display.

## Usage

The SSD1306Demo is a very comprehensive example demonstrating the most important features of the library.

## Features

* Draw pixels at given coordinates
* Draw or fill a rectangle with given dimensions
* Draw Text at given coordinates:
 * Define Alignment: Left, Right and Center
 * Set the Fontface you want to use (see section Fonts below)
 * Limit the width of the text by an amount of pixels. Before this widths will be reached, the renderer will wrap the text to a new line if possible
* Display content in automatically side scrolling carousel
 * Define transition cycles
 * Define how long one frame will be displayed
 * Draw the different frames in callback methods
 * One indicator per frame will be automatically displayed. The active frame will be displayed from inactive once

## Fonts

Fonts are defined in a proprietary but open format. You can create new font files by choosing from a given list
of open sourced Fonts from this web app: http://oleddisplay.squix.ch
Choose the font family, style and size, check the preview image and if you like what you see click the "Create" button. This will create the font array in a text area form where you can copy and paste it into a new or existing header file.

![FontTool](https://github.com/squix78/esp8266-oled-ssd1306/raw/master/resources/FontTool.png)


## API

### Display Control

```C++
// Create the display object connected to pin sda and sdc
SSD1306(int i2cAddress, int sda, int sdc);

// Initialize the display
void init();

// Cycle through the initialization
void resetDisplay(void);

// Connect again to the display through I2C
void reconnect(void);

// Turn the display on
void displayOn(void);

// Turn the display offs
void displayOff(void);

// Clear the local pixel buffer
void clear(void);

// Write the buffer to the display memory
void display(void);

// Set display contrast
void setContrast(char contrast);

// Turn the display upside down
void flipScreenVertically();

// Send a command to the display (low level function)
void sendCommand(unsigned char com);

// Send all the init commands
void sendInitCommands(void);
```

## Pixel drawing

```C++
// Draw a pixel at given position
void setPixel(int x, int y);

// Draw 8 bits at the given position
void setChar(int x, int y, unsigned char data);

// Draw the border of a rectangle at the given location
void drawRect(int x, int y, int width, int height);

// Fill the rectangle
void fillRect(int x, int y, int width, int height);

// Draw a bitmap with the given dimensions
void drawBitmap(int x, int y, int width, int height, const char *bitmap);

// Draw an XBM image with the given dimensions
void drawXbm(int x, int y, int width, int height, const char *xbm);

// Sets the color of all pixel operations
void setColor(int color);
```

## Text operations

``` C++
// Draws a string at the given location
void drawString(int x, int y, String text);

// Draws a String with a maximum width at the given location.
// If the given String is wider than the specified width
// The text will be wrapped to the next line at a space or dash
void drawStringMaxWidth(int x, int y, int maxLineWidth, String text);

// Returns the width of the String with the current
// font settings
int getStringWidth(String text);

// Specifies relative to which anchor point
// the text is rendered. Available constants:
// TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER, TEXT_ALIGN_RIGHT
void setTextAlignment(int textAlignment);

// Sets the current font. Available default fonts
// defined in SSD1306Fonts.h:
// ArialMT_Plain_10, ArialMT_Plain_16, ArialMT_Plain_24
// Or create one with the font tool at http://oleddisplay.squix.ch
void setFont(const char *fontData);
```

## Frame Transition Functions

The Frame Transition functions are a set of functions on top of the basic library. They allow you to easily write frames which will be shifted in regular intervals. The frame animation (including the frame indicators) will only be activated if you define callback functions with setFrameCallacks(..). If no callback methods are defined no indicators will be displayed.

```C++
// Sets the callback methods of the format void method(x,y). As soon as you define the callbacks
// the library is in "frame mode" and indicators will be drawn.
void setFrameCallbacks(int frameCount, void (*frameCallbacks[])(SSD1306 *display, SSD1306UiState* state,int x, int y));

// Tells the framework to move to the next tick. The
// current visible frame callback will be called once
// per tick
void nextFrameTick(void);

// Draws the frame indicators. In a normal setup
// the framework does this for you
void drawIndicators(int frameCount, int activeFrame);

// defines how many ticks a frame should remain visible
// This does not include the transition
void setFrameWaitTicks(int frameWaitTicks);

// Defines how many ticks should be used for a transition
void setFrameTransitionTicks(int frameTransitionTicks);

// Returns the current state of the internal state machine
// Possible values: FRAME_STATE_FIX, FRAME_STATE_TRANSITION
// You can use this to detect when there is no transition
// on the way to execute operations that would
int getFrameState();
```

## Example: SSD1306Demo

### Frame 1
![DemoFrame1](https://github.com/squix78/esp8266-oled-ssd1306/raw/master/resources/DemoFrame1.jpg)

This frame shows three things:
 * How to draw an xbm image
 * How to draw a static text which is not moved by the frame transition
 * The active/inactive frame indicators

### Frame 2
![DemoFrame2](https://github.com/squix78/esp8266-oled-ssd1306/raw/master/resources/DemoFrame2.jpg)

Currently there are one fontface with three sizes included in the library: Arial 10, 16 and 24. Once the converter is published you will be able to convert any ttf font into the used format.

### Frame 3

![DemoFrame3](https://github.com/squix78/esp8266-oled-ssd1306/raw/master/resources/DemoFrame3.jpg)

This frame demonstrates the text alignment. The coordinates in the frame show relative to which position the texts have been rendered.

### Frame 4

![DemoFrame4](https://github.com/squix78/esp8266-oled-ssd1306/raw/master/resources/DemoFrame4.jpg)

This shows how to use define a maximum width after which the driver automatically wraps a word to the next line. This comes in very handy if you have longer texts to display.

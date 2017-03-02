/**The MIT License (MIT)

Copyright (c) 2015 by Fabrice Weinberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Credits for parts of this code go to Daniel Eichhorn
*/

#pragma once

#include <Arduino.h>
#include "../esp8266-oled-ssd1306/SSD1306.h"

enum AnimationDirection {
  SLIDE_UP,
  SLIDE_DOWN,
  SLIDE_LEFT,
  SLIDE_RIGHT
};

enum IndicatorPosition {
  TOP,
  RIGHT,
  BOTTOM,
  LEFT
};

enum IndicatorDirection {
  LEFT_RIGHT,
  RIGHT_LEFT
};

enum FrameState {
  IN_TRANSITION,
  FIXED
};

const char ANIMATION_activeSymbole[] PROGMEM = {
  0x00, 0x18, 0x3c, 0x7e, 0x7e, 0x3c, 0x18, 0x00
};

const char ANIMATION_inactiveSymbole[] PROGMEM = {
  0x00, 0x0, 0x0, 0x18, 0x18, 0x0, 0x0, 0x00
};


// Structure of the UiState
struct SSD1306UiState {
  int           lastUpdate                = 0;
  int           ticksSinceLastStateSwitch = 0;

  FrameState    frameState                = FIXED;
  int           currentFrame              = 0;
};

typedef bool (*FrameCallback)(SSD1306 *display,  SSD1306UiState* state, int x, int y);
typedef bool (*OverlayCallback)(SSD1306 *display,  SSD1306UiState* state);

class SSD1306Ui {
  private:
    SSD1306             *display;

    // Global dirty flag to indicate that the display needs to be redraw.
    bool                dirty                     = true;

    // Symboles for the Indicator
    IndicatorPosition   indicatorPosition         = BOTTOM;
    IndicatorDirection  indicatorDirection        = LEFT_RIGHT;

    const char*         activeSymbole             = ANIMATION_activeSymbole;
    const char*         inactiveSymbole           = ANIMATION_inactiveSymbole;

    // Values for the Frames
    AnimationDirection  frameAnimationDirection   = SLIDE_RIGHT;

    int                 frameTransitionDirection  = 1;

    int                 ticksPerFrame             = 151; // ~ 5000ms at 30 FPS
    int                 ticksPerTransition        = 15;  // ~  500ms at 30 FPS

    bool                autoTransition            = true;

    FrameCallback*      frameFunctions;
    int                 frameCount                = 0;

    // Values for Overlays
    OverlayCallback*    overlayFunctions;
    int                 overlayCount              = 0;

    // UI State
    SSD1306UiState      state;

    // Bookeeping for update
    int                 updateInterval            = 33;

    int                 getNextFrameNumber();
    void                drawIndicator();
    void                drawFrame();
    void                drawOverlays();
    void                tick();

  public:

    SSD1306Ui(SSD1306 *display);

    /**
     * Initialise the display
     */
    void init();

    /**
     * Configure the internal used target FPS
     */
    void setTargetFPS(byte fps);

    // Automatic Controll
    /**
     * Enable automatic transition to next frame after the some time can be configured with `setTimePerFrame` and `setTimePerTransition`.
     */
    void enableAutoTransition();

    /**
     * Disable automatic transition to next frame.
     */
    void disableAutoTransition();

    /**
     * Set the direction if the automatic transitioning
     */
    void setAutoTransitionForwards();
    void setAutoTransitionBackwards();

    /**
     *  Set the approx. time a frame is displayed
     */
    void setTimePerFrame(int time);

    /**
     * Set the approx. time a transition will take
     */
    void setTimePerTransition(int time);

    // Customize indicator position and style
    /**
     * Set the position of the indicator bar.
     */
    void setIndicatorPosition(IndicatorPosition pos);

    /**
     * Set the direction of the indicator bar. Defining the order of frames ASCENDING / DESCENDING
     */
    void setIndicatorDirection(IndicatorDirection dir);

    /**
     * Set the symbole to indicate an active frame in the indicator bar.
     */
    void setActiveSymbole(const char* symbole);

    /**
     * Set the symbole to indicate an inactive frame in the indicator bar.
     */
    void setInactiveSymbole(const char* symbole);

    // Frame settings

    /**
     * Configure what animation is used to transition from one frame to another
     */
    void setFrameAnimation(AnimationDirection dir);

    /**
     * Add frame drawing functions
     */
    void setFrames(FrameCallback* frameFunctions, int frameCount);

    // Overlay

    /**
     * Add overlays drawing functions that are draw independent of the Frames
     */
    void setOverlays(OverlayCallback* overlayFunctions, int overlayCount);

    // Manuell Controll
    void  nextFrame();
    void  previousFrame();

    // State Info
    SSD1306UiState getUiState();

    int update();
};


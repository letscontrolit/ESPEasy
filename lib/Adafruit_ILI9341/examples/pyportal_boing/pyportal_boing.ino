// 'Boing' ball demo for PyPortal.
// Requires Adafruit_GFX 1.4.5 or later and Adafruit_DMA

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#define SCREENWIDTH  ILI9341_TFTHEIGHT // Native display orientation is
#define SCREENHEIGHT ILI9341_TFTWIDTH  // vertical, so swap width/height
#include "graphics.h"

#define TFT_D0        34 // Data bit 0 pin (MUST be on PORT byte boundary)
#define TFT_WR        26 // Write-strobe pin (CCL-inverted timer output)
#define TFT_DC        10 // Data/command pin
#define TFT_CS        11 // Chip-select pin
#define TFT_RST       24 // Reset pin
#define TFT_RD         9 // Read-strobe pin
#define TFT_BACKLIGHT 25

// ILI9341 with 8-bit parallel interface:
Adafruit_ILI9341 tft(tft8bitbus, TFT_D0, TFT_WR, TFT_DC, TFT_CS, TFT_RST, TFT_RD);

#define BGCOLOR    0xAD75
#define GRIDCOLOR  0xA815
#define BGSHADOW   0x5285
#define GRIDSHADOW 0x600C
#define RED        0xF800
#define WHITE      0xFFFF

#define YBOTTOM   123  // Ball Y coord at bottom
 #define YBOUNCE -3.5  // Upward velocity on ball bounce

// Ball coordinates are stored floating-point because screen refresh
// is so quick, whole-pixel movements are just too fast!
float ballx     = 20.0, bally     = YBOTTOM, // Current ball position
      ballvx    =  0.8, ballvy    = YBOUNCE, // Ball velocity
      ballframe = 3;                         // Ball animation frame #
int   balloldx  = ballx, balloldy = bally;   // Prior ball position

// Working buffer for ball rendering...2 scanlines that alternate,
// one is rendered while the other is transferred via DMA.
uint16_t renderbuf[2][SCREENWIDTH];

uint16_t palette[16]; // Color table for ball rotation effect

uint32_t startTime, frame = 0; // For frames-per-second estimate

void setup() {
  Serial.begin(9600);
//  while(!Serial);

  // Turn on backlight (required on PyPortal)
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);

  tft.begin();
  tft.setRotation(3); // Landscape orientation, USB at bottom right

  // Draw initial framebuffer contents:
  tft.drawBitmap(0, 0, (uint8_t *)background,
    SCREENWIDTH, SCREENHEIGHT, GRIDCOLOR, BGCOLOR);

  startTime = millis();
}

void loop() {

  balloldx = (int16_t)ballx; // Save prior position
  balloldy = (int16_t)bally;
  ballx   += ballvx;         // Update position
  bally   += ballvy;
  ballvy  += 0.06;          // Update Y velocity
  if((ballx <= 15) || (ballx >= SCREENWIDTH - BALLWIDTH))
    ballvx *= -1;            // Left/right bounce
  if(bally >= YBOTTOM) {     // Hit ground?
    bally  = YBOTTOM;        // Clip and
    ballvy = YBOUNCE;        // bounce up
  }

  // Determine screen area to update.  This is the bounds of the ball's
  // prior and current positions, so the old ball is fully erased and new
  // ball is fully drawn.
  int16_t minx, miny, maxx, maxy, width, height;
  // Determine bounds of prior and new positions
  minx = ballx;
  if(balloldx < minx)                    minx = balloldx;
  miny = bally;
  if(balloldy < miny)                    miny = balloldy;
  maxx = ballx + BALLWIDTH  - 1;
  if((balloldx + BALLWIDTH  - 1) > maxx) maxx = balloldx + BALLWIDTH  - 1;
  maxy = bally + BALLHEIGHT - 1;
  if((balloldy + BALLHEIGHT - 1) > maxy) maxy = balloldy + BALLHEIGHT - 1;

  width  = maxx - minx + 1;
  height = maxy - miny + 1;

  // Ball animation frame # is incremented opposite the ball's X velocity
  ballframe -= ballvx * 0.5;
  if(ballframe < 0)        ballframe += 14; // Constrain from 0 to 13
  else if(ballframe >= 14) ballframe -= 14;

  // Set 7 palette entries to white, 7 to red, based on frame number.
  // This makes the ball spin
  for(uint8_t i=0; i<14; i++) {
    palette[i+2] = ((((int)ballframe + i) % 14) < 7) ? WHITE : RED;
    // Palette entries 0 and 1 aren't used (clear and shadow, respectively)
  }

  // Only the changed rectangle is drawn into the 'renderbuf' array...
  uint16_t c, *destPtr;
  int16_t  bx  = minx - (int)ballx, // X relative to ball bitmap (can be negative)
           by  = miny - (int)bally, // Y relative to ball bitmap (can be negative)
           bgx = minx,              // X relative to background bitmap (>= 0)
           bgy = miny,              // Y relative to background bitmap (>= 0)
           x, y, bx1, bgx1;         // Loop counters and working vars
  uint8_t  p;                       // 'packed' value of 2 ball pixels
  int8_t bufIdx = 0;

  tft.dmaWait();  // Wait for last line from prior call to complete
  tft.endWrite();

  tft.startWrite();
  tft.setAddrWindow(minx, miny, width, height);

  for(y=0; y<height; y++) { // For each row...
    destPtr = &renderbuf[bufIdx][0];
    bx1  = bx;  // Need to keep the original bx and bgx values,
    bgx1 = bgx; // so copies of them are made here (and changed in loop below)
    for(x=0; x<width; x++) {
      if((bx1 >= 0) && (bx1 < BALLWIDTH) &&  // Is current pixel row/column
         (by  >= 0) && (by  < BALLHEIGHT)) { // inside the ball bitmap area?
        // Yes, do ball compositing math...
        p = ball[by][bx1 / 2];                // Get packed value (2 pixels)
        c = (bx1 & 1) ? (p & 0xF) : (p >> 4); // Unpack high or low nybble
        if(c == 0) { // Outside ball - just draw grid
          c = background[bgy][bgx1 / 8] & (0x80 >> (bgx1 & 7)) ? GRIDCOLOR : BGCOLOR;
        } else if(c > 1) { // In ball area...
          c = palette[c];
        } else { // In shadow area...
          c = background[bgy][bgx1 / 8] & (0x80 >> (bgx1 & 7)) ? GRIDSHADOW : BGSHADOW;
        }
      } else { // Outside ball bitmap, just draw background bitmap...
        c = background[bgy][bgx1 / 8] & (0x80 >> (bgx1 & 7)) ? GRIDCOLOR : BGCOLOR;
      }
      *destPtr++ = c; // Store pixel color
      bx1++;  // Increment bitmap position counters (X axis)
      bgx1++;
    }
    tft.dmaWait(); // Wait for prior line to complete
    tft.writePixels(&renderbuf[bufIdx][0], width, false); // Non-blocking write
    bufIdx = 1 - bufIdx;
    by++; // Increment bitmap position counters (Y axis)
    bgy++;
  }

  // Show approximate frame rate
  if(!(++frame & 255)) { // Every 256 frames...
    uint32_t elapsed = (millis() - startTime) / 1000; // Seconds
    if(elapsed) {
      Serial.print(frame / elapsed);
      Serial.println(" fps");
    }
  }
}

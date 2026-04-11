/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 by Daniel Eichhorn
 * Copyright (c) 2016 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Credits for parts of this code go to Mike Rankin. Thank you so much for sharing!
 */

#include "OLEDDisplay.h"

#ifdef USE_SECOND_HEAP
  #include <umm_malloc/umm_heap_select.h>
#endif

OLEDDisplay::~OLEDDisplay() {
  end();
}

bool OLEDDisplay::init() {
  if (!this->connect()) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Can't establish connection to display\n");
    return false;
  }
  if(this->buffer==NULL) {
  {
    this->buffer = (uint8_t*) malloc(sizeof(uint8_t) * DISPLAY_BUFFER_SIZE);
  }
  if(!this->buffer) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not enough memory to create display\n");
    return false;
  }
  }

  #ifdef OLEDDISPLAY_DOUBLE_BUFFER
  if(this->buffer_back==NULL) {
  {
    # ifdef USE_SECOND_HEAP
    HeapSelectIram ephemeral;
    # endif // ifdef USE_SECOND_HEAP
    this->buffer_back = (uint8_t*) malloc(sizeof(uint8_t) * DISPLAY_BUFFER_SIZE);
  }
  if(!this->buffer_back) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not enough memory to create back buffer\n");
    free(this->buffer);
    return false;
  }
  }
  #endif

  sendInitCommands();
  resetDisplay();

  return true;
}

void OLEDDisplay::end() {
  if (this->buffer) { free(this->buffer); this->buffer = NULL; }
  #ifdef OLEDDISPLAY_DOUBLE_BUFFER
  if (this->buffer_back) { free(this->buffer_back); this->buffer_back = NULL; }
  #endif
  if (this->logBuffer != NULL) { free(this->logBuffer); this->logBuffer = NULL; }
}

void OLEDDisplay::resetDisplay(void) {
  clear();
  #ifdef OLEDDISPLAY_DOUBLE_BUFFER
  memset(buffer_back, 1, DISPLAY_BUFFER_SIZE);
  #endif
  display();
}

void OLEDDisplay::setColor(OLEDDISPLAY_COLOR color) {
  this->color = color;
}

void OLEDDisplay::setPixel(int16_t x, int16_t y) {
  if (x >= 0 && x < this->width() && y >= 0 && y < this->height()) {
    switch (color) {
      case WHITE:   buffer[x + (y / 8) * this->width()] |=  (1 << (y & 7)); break;
      case BLACK:   buffer[x + (y / 8) * this->width()] &= ~(1 << (y & 7)); break;
      case INVERSE: buffer[x + (y / 8) * this->width()] ^=  (1 << (y & 7)); break;
    }
  }
}

// Bresenham's algorithm - thx wikipedia and Adafruit_GFX
void OLEDDisplay::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      setPixel(y0, x0);
    } else {
      setPixel(x0, y0);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void OLEDDisplay::drawRect(int16_t x, int16_t y, int16_t width, int16_t height) {
  drawHorizontalLine(x, y, width);
  drawVerticalLine(x, y, height);
  drawVerticalLine(x + width - 1, y, height);
  drawHorizontalLine(x, y + height - 1, width);
}

void OLEDDisplay::fillRect(int16_t xMove, int16_t yMove, int16_t width, int16_t height) {
  for (int16_t x = xMove; x < xMove + width; x++) {
    drawVerticalLine(x, yMove, height);
  }
}

void OLEDDisplay::drawCircle(int16_t x0, int16_t y0, int16_t radius) {
  int16_t x = 0, y = radius;
	int16_t dp = 1 - radius;
	do {
		if (dp < 0)
			dp = dp + 2 * (++x) + 3;
		else
			dp = dp + 2 * (++x) - 2 * (--y) + 5;

		setPixel(x0 + x, y0 + y);     //For the 8 octants
		setPixel(x0 - x, y0 + y);
		setPixel(x0 + x, y0 - y);
		setPixel(x0 - x, y0 - y);
		setPixel(x0 + y, y0 + x);
		setPixel(x0 - y, y0 + x);
		setPixel(x0 + y, y0 - x);
		setPixel(x0 - y, y0 - x);

	} while (x < y);

  setPixel(x0 + radius, y0);
  setPixel(x0, y0 + radius);
  setPixel(x0 - radius, y0);
  setPixel(x0, y0 - radius);
}

void OLEDDisplay::drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads) {
  int16_t x = 0, y = radius;
  int16_t dp = 1 - radius;
  while (x < y) {
    if (dp < 0)
      dp = dp + 2 * (++x) + 3;
    else
      dp = dp + 2 * (++x) - 2 * (--y) + 5;
    if (quads & 0x1) {
      setPixel(x0 + x, y0 - y);
      setPixel(x0 + y, y0 - x);
    }
    if (quads & 0x2) {
      setPixel(x0 - y, y0 - x);
      setPixel(x0 - x, y0 - y);
    }
    if (quads & 0x4) {
      setPixel(x0 - y, y0 + x);
      setPixel(x0 - x, y0 + y);
    }
    if (quads & 0x8) {
      setPixel(x0 + x, y0 + y);
      setPixel(x0 + y, y0 + x);
    }
  }
  if (quads & 0x1 && quads & 0x8) {
    setPixel(x0 + radius, y0);
  }
  if (quads & 0x4 && quads & 0x8) {
    setPixel(x0, y0 + radius);
  }
  if (quads & 0x2 && quads & 0x4) {
    setPixel(x0 - radius, y0);
  }
  if (quads & 0x1 && quads & 0x2) {
    setPixel(x0, y0 - radius);
  }
}


void OLEDDisplay::fillCircle(int16_t x0, int16_t y0, int16_t radius) {
  int16_t x = 0, y = radius;
	int16_t dp = 1 - radius;
	do {
		if (dp < 0)
			dp = dp + 2 * (++x) + 3;
		else
			dp = dp + 2 * (++x) - 2 * (--y) + 5;

    drawHorizontalLine(x0 - x, y0 - y, 2*x);
    drawHorizontalLine(x0 - x, y0 + y, 2*x);
    drawHorizontalLine(x0 - y, y0 - x, 2*y);
    drawHorizontalLine(x0 - y, y0 + x, 2*y);


	} while (x < y);
  drawHorizontalLine(x0 - radius, y0, 2 * radius);

}

void OLEDDisplay::drawHorizontalLine(int16_t x, int16_t y, int16_t length) {
  if (y < 0 || y >= this->height()) { return; }

  if (x < 0) {
    length += x;
    x = 0;
  }

  if ( (x + length) > this->width()) {
    length = (this->width() - x);
  }

  if (length <= 0) { return; }

  uint8_t * bufferPtr = buffer;
  bufferPtr += (y >> 3) * this->width();
  bufferPtr += x;

  uint8_t drawBit = 1 << (y & 7);

  switch (color) {
    case WHITE:   while (length--) {
        *bufferPtr++ |= drawBit;
      }; break;
    case BLACK:   drawBit = ~drawBit;   while (length--) {
        *bufferPtr++ &= drawBit;
      }; break;
    case INVERSE: while (length--) {
        *bufferPtr++ ^= drawBit;
      }; break;
  }
}

void OLEDDisplay::drawVerticalLine(int16_t x, int16_t y, int16_t length) {
  if (x < 0 || x >= this->width()) return;

  if (y < 0) {
    length += y;
    y = 0;
  }

  if ( (y + length) > this->height()) {
    length = (this->height() - y);
  }

  if (length <= 0) return;


  uint8_t yOffset = y & 7;
  uint8_t drawBit;
  uint8_t *bufferPtr = buffer;

  bufferPtr += (y >> 3) * this->width();
  bufferPtr += x;

  if (yOffset) {
    yOffset = 8 - yOffset;
    drawBit = ~(0xFF >> (yOffset));

    if (length < yOffset) {
      drawBit &= (0xFF >> (yOffset - length));
    }

    switch (color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }

    if (length < yOffset) return;

    length -= yOffset;
    bufferPtr += this->width();
  }

  if (length >= 8) {
    switch (color) {
      case WHITE:
      case BLACK:
        drawBit = (color == WHITE) ? 0xFF : 0x00;
        do {
          *bufferPtr = drawBit;
          bufferPtr += this->width();
          length -= 8;
        } while (length >= 8);
        break;
      case INVERSE:
        do {
          *bufferPtr = ~(*bufferPtr);
          bufferPtr += this->width();
          length -= 8;
        } while (length >= 8);
        break;
    }
  }

  if (length > 0) {
    drawBit = (1 << (length & 7)) - 1;
    switch (color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }
  }
}

void OLEDDisplay::drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress) {
  uint16_t radius = height / 2;
  uint16_t xRadius = x + radius;
  uint16_t yRadius = y + radius;
  uint16_t doubleRadius = 2 * radius;
  uint16_t innerRadius = radius - 2;

  setColor(WHITE);
  drawCircleQuads(xRadius, yRadius, radius, 0b00000110);
  drawHorizontalLine(xRadius, y, width - doubleRadius + 1);
  drawHorizontalLine(xRadius, y + height, width - doubleRadius + 1);
  drawCircleQuads(x + width - radius, yRadius, radius, 0b00001001);

  uint16_t maxProgressWidth = (width - doubleRadius + 1) * progress / 100;

  fillCircle(xRadius, yRadius, innerRadius);
  fillRect(xRadius + 1, y + 2, maxProgressWidth, height - 3);
  fillCircle(xRadius + maxProgressWidth, yRadius, innerRadius);
}

void OLEDDisplay::drawFastImage(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *image) {
  drawInternal(xMove, yMove, width, height, image, 0, 0);
}

void OLEDDisplay::drawXbm(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *xbm) {
  int16_t widthInXbm = (width + 7) / 8;
  uint8_t data = 0;

  for(int16_t y = 0; y < height; y++) {
    for(int16_t x = 0; x < width; x++ ) {
      if (x & 7) {
        data >>= 1; // Move a bit
      } else {  // Read new data every 8 bit
        data = pgm_read_byte(xbm + (x / 8) + y * widthInXbm);
      }
      // if there is a bit draw it
      if (data & 0x01) {
        setPixel(xMove + x, yMove + y);
      }
    }
  }
}

void OLEDDisplay::drawStringInternal(int16_t xMove, int16_t yMove, char* text, uint16_t textLength, uint16_t textWidth) {
  if (fontData == nullptr) return;
  const uint8_t textHeight       = pgm_read_byte(fontData + HEIGHT_POS);
  const uint8_t firstChar        = pgm_read_byte(fontData + FIRST_CHAR_POS);
  const uint8_t numberOfChars    = pgm_read_byte(fontData + CHAR_NUM_POS);
  const uint16_t sizeOfJumpTable = static_cast<uint16_t>(numberOfChars)  * JUMPTABLE_BYTES;

  uint8_t cursorX         = 0;
  uint8_t cursorY         = 0;

  switch (textAlignment) {
    case TEXT_ALIGN_CENTER_BOTH:
      yMove -= textHeight >> 1;
    // Fallthrough
    case TEXT_ALIGN_CENTER:
      xMove -= textWidth >> 1; // divide by 2
      break;
    case TEXT_ALIGN_RIGHT:
      xMove -= textWidth;
      break;
    case TEXT_ALIGN_LEFT:
      break;
  }

  // Don't draw anything if it is not on the screen.
  if (xMove + textWidth  < 0 || xMove > this->width() ) {return;}
  if (yMove + textHeight < 0 || yMove > this->width() ) {return;}

  for (uint16_t j = 0; j < textLength; j++) {
    const int16_t xPos = xMove + cursorX;
    const int16_t yPos = yMove + cursorY;

    const uint8_t code = text[j];
    if (code >= firstChar) {
      const uint8_t charCode = code - firstChar;
      if (charCode < numberOfChars) {

        // 4 Bytes per char code
        const char* charOffset = fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES;
        const uint8_t msbJumpToChar    = pgm_read_byte( charOffset );                  // MSB  \ JumpAddress
        const uint8_t lsbJumpToChar    = pgm_read_byte( charOffset + JUMPTABLE_LSB);   // LSB /
        const uint8_t charByteSize     = pgm_read_byte( charOffset + JUMPTABLE_SIZE);  // Size
        const uint8_t currentCharWidth = pgm_read_byte( charOffset + JUMPTABLE_WIDTH); // Width

        // Test if the char is drawable
        if (!(msbJumpToChar == 255 && lsbJumpToChar == 255)) {
          // Get the position of the char data
          uint16_t charDataPosition = JUMPTABLE_START + sizeOfJumpTable + ((msbJumpToChar << 8) + lsbJumpToChar);
          drawInternal(xPos, yPos, currentCharWidth, textHeight, fontData, charDataPosition, charByteSize);
        }

        cursorX += currentCharWidth;
      }
    }
  }
}


void OLEDDisplay::drawString(int16_t xMove, int16_t yMove, const String& strUser) {
  if (fontData == nullptr) return;
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  // char* text must be freed!
  char* text = utf8ascii(strUser);

  uint16_t yOffset = 0;
  // If the string should be centered vertically too
  // we need to now how heigh the string is.
  if (textAlignment == TEXT_ALIGN_CENTER_BOTH) {
    uint16_t lb = 0;
    // Find number of linebreaks in text
    for (uint16_t i=0;text[i] != 0; i++) {
      lb += (text[i] == 10);
    }
    // Calculate center
    yOffset = (lb * lineHeight) / 2;
  }

  uint16_t line = 0;
  char* textPart = strtok(text,"\n");
  while (textPart != NULL) {
    uint16_t length = strlen(textPart);
    drawStringInternal(xMove, yMove - yOffset + (line++) * lineHeight, textPart, length, getStringWidth(textPart, length));
    textPart = strtok(NULL, "\n");
  }
  free(text);
}

void OLEDDisplay::drawStringMaxWidth(int16_t xMove, int16_t yMove, uint16_t maxLineWidth, const String& strUser) {
  if (fontData == nullptr) return;
  uint16_t firstChar  = pgm_read_byte(fontData + FIRST_CHAR_POS);
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  char* text = utf8ascii(strUser);

  uint16_t length = strlen(text);
  uint16_t lastDrawnPos = 0;
  uint16_t lineNumber = 0;
  uint16_t strWidth = 0;

  uint16_t preferredBreakpoint = 0;
  uint16_t widthAtBreakpoint = 0;

  for (uint16_t i = 0; i < length; i++) {
    strWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[i] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);

    // Always try to break on a space or dash
    if (text[i] == ' ' || text[i]== '-') {
      preferredBreakpoint = i;
      widthAtBreakpoint = strWidth;
    }

    if (strWidth >= maxLineWidth) {
      if (preferredBreakpoint == 0) {
        preferredBreakpoint = i;
        widthAtBreakpoint = strWidth;
      }
      drawStringInternal(xMove, yMove + (lineNumber++) * lineHeight , &text[lastDrawnPos], preferredBreakpoint - lastDrawnPos, widthAtBreakpoint);
      lastDrawnPos = preferredBreakpoint + 1;
      // It is possible that we did not draw all letters to i so we need
      // to account for the width of the chars from `i - preferredBreakpoint`
      // by calculating the width we did not draw yet.
      strWidth = strWidth - widthAtBreakpoint;
      preferredBreakpoint = 0;
    }
  }

  // Draw last part if needed
  if (lastDrawnPos < length) {
    drawStringInternal(xMove, yMove + lineNumber * lineHeight , &text[lastDrawnPos], length - lastDrawnPos, getStringWidth(&text[lastDrawnPos], length - lastDrawnPos));
  }

  free(text);
}

uint16_t OLEDDisplay::getStringWidth(const char* text, uint16_t length) {
  if (fontData == nullptr) return 0;
  uint16_t firstChar        = pgm_read_byte(fontData + FIRST_CHAR_POS);

  uint16_t stringWidth = 0;
  uint16_t maxWidth = 0;

  while (length--) {
    stringWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[length] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);
    if (text[length] == 10) {
      maxWidth = max(maxWidth, stringWidth);
      stringWidth = 0;
    }
  }

  return max(maxWidth, stringWidth);
}

uint16_t OLEDDisplay::getStringWidth(const String& strUser) {
  char* text = utf8ascii(strUser);
  uint16_t length = strlen(text);
  uint16_t width = getStringWidth(text, length);
  free(text);
  return width;
}

uint8_t OLEDDisplay::getCharWidth(const char c) {
  if (fontData == nullptr) return 0;
  uint8_t firstChar = pgm_read_byte(fontData + FIRST_CHAR_POS);
  if (utf8ascii(c) == 0)
    return 0;
  if (c < firstChar)
    return 0;
  return pgm_read_byte(fontData + JUMPTABLE_START + (c- firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);
}

void OLEDDisplay::setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment) {
  this->textAlignment = textAlignment;
}

void OLEDDisplay::setFont(const char *fontData) {
  this->fontData = fontData;
}

void OLEDDisplay::displayOn(void) {
  sendCommand(DISPLAYON);
}

void OLEDDisplay::displayOff(void) {
  sendCommand(DISPLAYOFF);
}

void OLEDDisplay::invertDisplay(void) {
  sendCommand(INVERTDISPLAY);
}

void OLEDDisplay::normalDisplay(void) {
  sendCommand(NORMALDISPLAY);
}

void OLEDDisplay::setContrast(char contrast, char precharge, char comdetect) {
  const uint8_t commands[] = {
    SETPRECHARGE,     //0xD9
    precharge,        //0xF1 default, to lower the contrast, put 1-1F
    SETCONTRAST,
    contrast,         // 0-255
    SETVCOMDETECT,    //0xDB, (additionally needed to lower the contrast)
    comdetect,        //0x40 default, to lower the contrast, put 0
    DISPLAYALLON_RESUME,
    NORMALDISPLAY,
    DISPLAYON
  };
  for (uint8_t i = 0; i < sizeof(commands); ++i) {
    sendCommand(commands[i]);
  }
}

void OLEDDisplay::flipScreenVertically() {
  sendCommand(SEGREMAP | 0x01);
  sendCommand(COMSCANDEC);           //Rotate screen 180 Deg
}

void OLEDDisplay::clear(void) {
  memset(buffer, 0, DISPLAY_BUFFER_SIZE);
}

void OLEDDisplay::drawLogBuffer(uint16_t xMove, uint16_t yMove) {
  if (fontData == nullptr) return;
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);
  // Always align left
  setTextAlignment(TEXT_ALIGN_LEFT);

  // State values
  uint16_t length   = 0;
  uint16_t line     = 0;
  uint16_t lastPos  = 0;

  for (uint16_t i=0;i<this->logBufferFilled;i++){
    // Everytime we have a \n print
    if (this->logBuffer[i] == 10) {
      length++;
      // Draw string on line `line` from lastPos to length
      // Passing 0 as the lenght because we are in TEXT_ALIGN_LEFT
      drawStringInternal(xMove, yMove + (line++) * lineHeight, &this->logBuffer[lastPos], length, 0);
      // Remember last pos
      lastPos = i;
      // Reset length
      length = 0;
    } else {
      // Count chars until next linebreak
      length++;
    }
  }
  // Draw the remaining string
  if (length > 0) {
    drawStringInternal(xMove, yMove + line * lineHeight, &this->logBuffer[lastPos], length, 0);
  }
}

bool OLEDDisplay::setLogBuffer(uint16_t lines, uint16_t chars){
  if (logBuffer != NULL) free(logBuffer);
  uint16_t size = lines * chars;
  if (size > 0) {
    this->logBufferLine     = 0;      // Lines printed
    this->logBufferFilled   = 0;      // Nothing stored yet
    this->logBufferMaxLines = lines;  // Lines max printable
    this->logBufferSize     = size;   // Total number of characters the buffer can hold
    {
      # ifdef USE_SECOND_HEAP
      HeapSelectIram ephemeral;
      # endif // ifdef USE_SECOND_HEAP
      this->logBuffer         = (char *) malloc(size * sizeof(uint8_t));
    }
    if(!this->logBuffer) {
      DEBUG_OLEDDISPLAY("[OLEDDISPLAY][setLogBuffer] Not enough memory to create log buffer\n");
      return false;
    }
  }
  return true;
}

size_t OLEDDisplay::write(uint8_t c) {
  if (this->logBufferSize > 0) {
    // Don't waste space on \r\n line endings, dropping \r
    if (c == 13) return 1;

    bool maxLineNotReached = this->logBufferLine < this->logBufferMaxLines;
    bool bufferNotFull = this->logBufferFilled < this->logBufferSize;

    // Can we write to the buffer?
    if (bufferNotFull && maxLineNotReached) {
      this->logBuffer[logBufferFilled] = utf8ascii(c);
      this->logBufferFilled++;
      // Keep track of lines written
      if (c == 10) this->logBufferLine++;
    } else {
      // Max line number is reached
      if (!maxLineNotReached) this->logBufferLine--;

      // Find the end of the first line
      uint16_t firstLineEnd = 0;
      for (uint16_t i=0;i<this->logBufferFilled;i++) {
        if (this->logBuffer[i] == 10){
          // Include last char too
          firstLineEnd = i + 1;
          break;
        }
      }
      // If there was a line ending
      if (firstLineEnd > 0) {
        // Calculate the new logBufferFilled value
        this->logBufferFilled = logBufferFilled - firstLineEnd;
        // Now we move the lines infront of the buffer
        memcpy(this->logBuffer, &this->logBuffer[firstLineEnd], logBufferFilled);
      } else {
        // Let's reuse the buffer if it was full
        if (!bufferNotFull) {
          this->logBufferFilled = 0;
        }// else {
        //  Nothing to do here
        //}
      }
      write(c);
    }
  }
  // We are always writing all uint8_t to the buffer
  return 1;
}

size_t OLEDDisplay::write(const char* str) {
  if (str == NULL) return 0;
  size_t length = strlen(str);
  for (size_t i = 0; i < length; i++) {
    write(str[i]);
  }
  return length;
}

#ifdef OLEDDISPLAY_DOUBLE_BUFFER
bool OLEDDisplay::getChangedBoundingBox(
  uint8_t& minBoundX, 
  uint8_t& minBoundY, 
  uint8_t& maxBoundX, 
  uint8_t& maxBoundY)
{
  minBoundY = ~0;
  maxBoundY = 0;

  minBoundX = ~0;
  maxBoundX = 0;
  // Calculate the Y bounding box of changes
  // and copy buffer[pos] to buffer_back[pos];
  const uint32_t* buf_32 = (const uint32_t*)((uintptr_t)buffer & ~(uintptr_t)3u);
  uint32_t* back_buf_32 = (uint32_t*)((uintptr_t)buffer_back & ~(uintptr_t)3u);

  const uint8_t y_maxindex = this->height() / 8;
  const uint8_t x_maxindex = this->width(); 

  for (uint8_t y = 0; y < y_maxindex; ++y) {
    for (uint8_t x = 0; x < x_maxindex; x += 4) {
      const uint16_t pos = (x + (y * this->width())) >> 2;

      uint32_t buf_val, back_buf_val;
      __builtin_memcpy(&buf_val, (buf_32 + pos), sizeof(uint32_t));
      __builtin_memcpy(&back_buf_val, (back_buf_32 + pos), sizeof(uint32_t));
      asm volatile ("" :"+r"(back_buf_val)); // inject 32-bit dependency

      if (buf_val != back_buf_val) {
        minBoundY = _min(minBoundY, y);
        maxBoundY = _max(maxBoundY, y);
        if ((x < minBoundX) || ((x+3) > maxBoundX)) {
          for (uint8_t i = 0; i < 4; ++i) {
            if (((buf_val >> (8*i)) & 0xFF) != ((back_buf_val >> (8*i)) & 0xFF))
            {
              minBoundX = _min(minBoundX, x + i);
              maxBoundX = _max(maxBoundX, x + i);
            }
          }
        }
        __builtin_memcpy((back_buf_32 + pos), &buf_val, sizeof(uint32_t));
      }
    }
    yield();
  }

  // If the minBoundY wasn't updated
  // we can savely assume that buffer_back[pos] == buffer[pos]
  // holdes true for all values of pos
  return (minBoundY != (uint8_t)(~0));
}
#endif


void OLEDDisplay::SetComPins(uint8_t _compins) {
  sendCommand(SETCOMPINS);
  sendCommand(_compins); // according to the adafruit lib, sometimes this may need to be 0x02
}

// Private functions
void OLEDDisplay::sendInitCommands(void) {
  const uint8_t commands[] = {
    DISPLAYOFF,
    SETDISPLAYCLOCKDIV,
    0xF0,             // Increase speed of the display max ~96Hz
    SETMULTIPLEX,
    static_cast<uint8_t>(this->height() - 1),  // FIXME TD-er: should add some checks here?
    SETDISPLAYOFFSET,
    0x00,
    SETSTARTLINE,
    CHARGEPUMP,
    0x14,
    MEMORYMODE,
    0x00,
    SEGREMAP,
    COMSCANINC,
    SETCOMPINS,
    0x12,             // according to the adafruit lib, sometimes this may need to be 0x02
    SETCONTRAST,
    0xCF,
    SETPRECHARGE,
    0xF1,
    SETVCOMDETECT,    //0xDB, (additionally needed to lower the contrast)
    0x40,             //0x40 default, to lower the contrast, put 0
    DISPLAYALLON_RESUME,
    NORMALDISPLAY,
    0x2e,             // stop scroll
    DISPLAYON};
  for (uint8_t i = 0; i < sizeof(commands); ++i) {
    sendCommand(commands[i]);
  }
}

void inline OLEDDisplay::drawInternal(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *data, uint16_t offset, uint16_t bytesInData) {
  if (width < 0 || height < 0) return;
  if (yMove + height < 0 || yMove > this->height())  return;
  if (xMove + width  < 0 || xMove > this->width())   return;

  uint8_t  rasterHeight = 1 + ((height - 1) >> 3); // fast ceil(height / 8.0)
  int8_t   yOffset      = yMove & 7;

  bytesInData = bytesInData == 0 ? width * rasterHeight : bytesInData;

  int16_t initYMove   = yMove;
  int8_t  initYOffset = yOffset;


  for (uint16_t i = 0; i < bytesInData; i++) {

    // Reset if next horizontal drawing phase is started.
    if ( i % rasterHeight == 0) {
      yMove   = initYMove;
      yOffset = initYOffset;
    }

    uint8_t currentByte = pgm_read_byte(data + offset + i);

    int16_t xPos = xMove + (i / rasterHeight);
    int16_t yPos = ((yMove >> 3) + (i % rasterHeight)) * this->width();

//    int16_t yScreenPos = yMove + yOffset;
    int16_t dataPos    = xPos  + yPos;

    if (dataPos >=  0  && dataPos < DISPLAY_BUFFER_SIZE &&
        xPos    >=  0  && xPos    < this->width() ) {

      if (yOffset >= 0) {
        switch (this->color) {
          case WHITE:   buffer[dataPos] |= currentByte << yOffset; break;
          case BLACK:   buffer[dataPos] &= ~(currentByte << yOffset); break;
          case INVERSE: buffer[dataPos] ^= currentByte << yOffset; break;
        }
        if (dataPos < (DISPLAY_BUFFER_SIZE - this->width())) {
          switch (this->color) {
            case WHITE:   buffer[dataPos + this->width()] |= currentByte >> (8 - yOffset); break;
            case BLACK:   buffer[dataPos + this->width()] &= ~(currentByte >> (8 - yOffset)); break;
            case INVERSE: buffer[dataPos + this->width()] ^= currentByte >> (8 - yOffset); break;
          }
        }
      } else {
        // Make new offset position
        yOffset = -yOffset;

        switch (this->color) {
          case WHITE:   buffer[dataPos] |= currentByte >> yOffset; break;
          case BLACK:   buffer[dataPos] &= ~(currentByte >> yOffset); break;
          case INVERSE: buffer[dataPos] ^= currentByte >> yOffset; break;
        }

        // Prepare for next iteration by moving one block up
        yMove -= 8;

        // and setting the new yOffset
        yOffset = 8 - yOffset;
      }

      yield();
    }
  }
}

// Code form http://playground.arduino.cc/Main/Utf8ascii
uint8_t OLEDDisplay::utf8ascii(uint8_t ascii) {
  static uint8_t LASTCHAR;

  if ( ascii < 128 ) { // Standard ASCII-set 0..0x7F handling
    LASTCHAR = 0;
    return ascii;
  }

  uint8_t last = LASTCHAR;   // get last char
  LASTCHAR = ascii;

  switch (last) {    // conversion depnding on first UTF8-character
    case 0xC2: return  (ascii);  break;
    case 0xC3: return  (ascii | 0xC0);  break;
    case 0x82: if (ascii == 0xAC) return (0x80);    // special case Euro-symbol
  }

  return  0; // otherwise: return zero, if character has to be ignored
}

// You need to free the char!
char* OLEDDisplay::utf8ascii(const String& str) {
  uint16_t k = 0;
  uint16_t length = str.length() + 1;

  // Copy the string into a char array
  char* s = (char*) malloc(length * sizeof(char));
  if(!s) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][utf8ascii] Can't allocate another char array. Drop support for UTF-8.\n");
    return (char*) str.c_str();
  }
  str.toCharArray(s, length);

  length--;

  for (uint16_t i=0; i < length; i++) {
    char c = utf8ascii(s[i]);
    if (c!=0) {
      s[k++]=c;
    }
  }

  s[k]=0;

  // This will leak 's' be sure to free it in the calling function.
  return s;
}

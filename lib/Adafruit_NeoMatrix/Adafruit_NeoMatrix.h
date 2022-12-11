/*!
 * @file Adafruit_NeoMatrix.h
 *
 * Arduino library to control single and tiled matrices of WS2811- and
 * WS2812-based RGB LED devices such as the Adafruit NeoPixel Shield or
 * displays assembled from NeoPixel strips, making them compatible with
 * the Adafruit_GFX graphics library.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.
 *
 * This file is part of the Adafruit NeoMatrix library.
 *
 * NeoMatrix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * NeoMatrix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with NeoMatrix.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _ADAFRUIT_NEOMATRIX_H_
#define _ADAFRUIT_NEOMATRIX_H_

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <pins_arduino.h>
#endif
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>

// Matrix layout information is passed in the 'matrixType' parameter for
// each constructor (the parameter immediately following is the LED type
// from NeoPixel.h).

// These define the layout for a single 'unified' matrix (e.g. one made
// from NeoPixel strips, or a single NeoPixel shield), or for the pixels
// within each matrix of a tiled display (e.g. multiple NeoPixel shields).

#define NEO_MATRIX_TOP 0x00         ///< Pixel 0 is at top of matrix
#define NEO_MATRIX_BOTTOM 0x01      ///< Pixel 0 is at bottom of matrix
#define NEO_MATRIX_LEFT 0x00        ///< Pixel 0 is at left of matrix
#define NEO_MATRIX_RIGHT 0x02       ///< Pixel 0 is at right of matrix
#define NEO_MATRIX_CORNER 0x03      ///< Bitmask for pixel 0 matrix corner
#define NEO_MATRIX_ROWS 0x00        ///< Matrix is row major (horizontal)
#define NEO_MATRIX_COLUMNS 0x04     ///< Matrix is column major (vertical)
#define NEO_MATRIX_AXIS 0x04        ///< Bitmask for row/column layout
#define NEO_MATRIX_PROGRESSIVE 0x00 ///< Same pixel order across each line
#define NEO_MATRIX_ZIGZAG 0x08      ///< Pixel order reverses between lines
#define NEO_MATRIX_SEQUENCE 0x08    ///< Bitmask for pixel line order

// These apply only to tiled displays (multiple matrices):

#define NEO_TILE_TOP 0x00         ///< First tile is at top of matrix
#define NEO_TILE_BOTTOM 0x10      ///< First tile is at bottom of matrix
#define NEO_TILE_LEFT 0x00        ///< First tile is at left of matrix
#define NEO_TILE_RIGHT 0x20       ///< First tile is at right of matrix
#define NEO_TILE_CORNER 0x30      ///< Bitmask for first tile corner
#define NEO_TILE_ROWS 0x00        ///< Tiles ordered in rows
#define NEO_TILE_COLUMNS 0x40     ///< Tiles ordered in columns
#define NEO_TILE_AXIS 0x40        ///< Bitmask for tile H/V orientation
#define NEO_TILE_PROGRESSIVE 0x00 ///< Same tile order across each line
#define NEO_TILE_ZIGZAG 0x80      ///< Tile order reverses between lines
#define NEO_TILE_SEQUENCE 0x80    ///< Bitmask for tile line order

/**
 * @brief Class for using NeoPixel matrices with the GFX graphics library.
 */
class Adafruit_NeoMatrix : public Adafruit_GFX, public Adafruit_NeoPixel {

public:
  /**
   * @brief Construct a single (non-tiled) matrix.
   * @param  w           Matrix width in pixels.
   * @param  h           Matrix height in pixels.
   * @param  pin         Arduino pin number for NeoPixel data out.
   * @param  matrixType  Matrix layout - add together NEO_MATRIX_* values
   *                     to declare orientation, rotation, etc.
   * @param  ledType     NeoPixel LED type, similar to Adafruit_NeoPixel
   *                     constructor (e.g. NEO_GRB).
   */
  Adafruit_NeoMatrix(int w, int h, uint8_t pin = 6,
                     uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
                                          NEO_MATRIX_ROWS,
                     neoPixelType ledType = NEO_GRB + NEO_KHZ800);

  /**
   * @brief Construct a tiled matrix.
   * @param  matrixW     Individual sub-matrix (tile) width in pixels.
   * @param  matrixH     Individual sub-matrix (tile) height in pixels.
   * @param  tX          Number of tiles on the X (horizontal) axis.
   * @param  tY          Number of tiles on the Y (vertical) axis.
   * @param  pin         Arduino pin number for NeoPixel data out.
   * @param  matrixType  Tiled matrix layout - add together NEO_MATRIX_* and
   *                     NEO_TILE_* values to declare orientation, rotation,
   *                     etc.
   * @param  ledType     NeoPixel LED type, similar to Adafruit_NeoPixel
   *                     constructor (e.g. NEO_GRB).
   */
  Adafruit_NeoMatrix(uint8_t matrixW, uint8_t matrixH, uint8_t tX, uint8_t tY,
                     uint8_t pin = 6,
                     uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
                                          NEO_MATRIX_ROWS + NEO_TILE_TOP +
                                          NEO_TILE_LEFT + NEO_TILE_ROWS,
                     neoPixelType ledType = NEO_GRB + NEO_KHZ800);

  virtual ~Adafruit_NeoMatrix() {
  } // Virtual destructor to allow better memory management

  /**
   * @brief  Pixel-drawing function for Adafruit_GFX.
   * @param  x      Pixel column (0 = left edge, unless rotation used).
   * @param  y      Pixel row (0 = top edge, unless rotation used).
   * @param  color  Pixel color in 16-bit '565' RGB format.
   */
  void drawPixel(int16_t x, int16_t y, uint16_t color);

  /**
   * @brief  Fill matrix with a single color.
   * @param  color  Pixel color in 16-bit '565' RGB format.
   */
  void fillScreen(uint16_t color);

  /**
   * @brief  Pass-through is a kludge that lets you override the current
   *         drawing color with a 'raw' RGB (or RGBW) value that's issued
   *         directly to pixel(s), side-stepping the 16-bit color limitation
   *         of Adafruit_GFX. This is not without some limitations of its
   *         own -- for example, it won't work in conjunction with the
   *         background color feature when drawing text or bitmaps (you'll
   *         just get a solid rect of color), only 'transparent'
   *         text/bitmaps.  Also, no gamma correction.
   *         Remember to UNSET the passthrough color immediately when done
   *         with it (call with no value)!
   * @param  c  Pixel color in packed 32-bit 0RGB or WRGB format.
   */
  void setPassThruColor(uint32_t c);

  /**
   * @brief  Stop using pass-through color, return to normal 16-bit color
   *         usage.
   */
  void setPassThruColor(void);

  /**
   * @brief  Register a function for mapping X/Y coordinates to absolute
   *         pixel indices (for unusual layouts if if NEO_MATRIX_* and
   *         NEO_TILE_* settings do not provide sufficient control).
   * @param  fn  Pointer to function that accepts two uint16_t arguments
   *             (column and row), returns absolute pixel index.
   */
  void setRemapFunction(uint16_t (*fn)(uint16_t, uint16_t));

  /**
   * @brief   Quantize a 24-bit RGB color value to 16-bit '565' format.
   * @param   r         Red component (0 to 255).
   * @param   g         Green component (0 to 255).
   * @param   b         Blue component (0 to 255).
   * @return  uint16_t  Quantized color for GFX drawing functions.
   */
  static uint16_t Color(uint8_t r, uint8_t g, uint8_t b);

private:
  const uint8_t type;
  const uint8_t matrixWidth, matrixHeight, tilesX, tilesY;
  uint16_t (*remapFn)(uint16_t x, uint16_t y);

  uint32_t passThruColor;
  boolean passThruFlag = false;
};

#endif // _ADAFRUIT_NEOMATRIX_H_

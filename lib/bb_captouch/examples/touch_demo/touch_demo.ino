#include <bb_captouch.h>
//#include <OneBitDisplay.h>
#include <bb_spi_lcd.h>
#include <bb_scd41.h>
#include <Wire.h>

//ONE_BIT_DISPLAY obd;
SCD41 co2;

//#define CYD_128C
//#define LILYGO_S3_PRO
#define LILYGO_S3_LONG

#ifdef LILYGO_S3_LONG
#define TOUCH_SDA 15
#define TOUCH_SCL 10
#define TOUCH_INT 11
// reset is 16, but it's shared with the LCD
#define TOUCH_RST -1
#define LCD DISPLAY_T_DISPLAY_S3_LONG
#endif

#ifdef LILYGO_S3_PRO
#define TOUCH_SDA 5
#define TOUCH_SCL 6
#define TOUCH_INT 7
#define TOUCH_RST 13
#define LCD DISPLAY_T_DISPLAY_S3_PRO
#endif

#ifdef CYD_28C
// These defines are for a low cost 2.8" ESP32 LCD board with the GT911 touch controller
#define TOUCH_SDA 33
#define TOUCH_SCL 32
#define TOUCH_INT 21
#define TOUCH_RST 25
#define LCD DISPLAY_CYD
#endif

#ifdef CYD_128C
// These defines are for a low cost 1.28" ESP32-C3 round LCD board with the CST816D touch controller
#define TOUCH_SDA 4
#define TOUCH_SCL 5
#define TOUCH_INT 0
#define TOUCH_RST 1
#define QWIIC_SDA 21
#define QWIIC_SCL 20
#define LCD DISPLAY_CYD_128
#endif

#ifdef ARDUINO_M5STACK_CORES3
#define TOUCH_SDA 12
#define TOUCH_SCL 11
#define TOUCH_INT -1
#define TOUCH_RST -1
#define LCD DISPLAY_M5STACK_CORES3
#endif // CORES3

BBCapTouch bbct;
BB_SPI_LCD lcd;
int iWidth, iHeight;

const char *szNames[] = {"Unknown", "FT6x36", "GT911", "CST820", "CST226", "AXS15231"};
void setup() {

  Serial.begin(115200);
  while (!Serial) {};
  lcd.begin(LCD);
  Serial.println("Starting...");
//  obd.setI2CPins(QWIIC_SDA, QWIIC_SCL);
//  obd.setBitBang(true);
//  obd.I2Cbegin(OLED_128x64);
//  obd.fillScreen(OBD_WHITE);
//  obd.setFont(FONT_12x16);
//  obd.println("QWIIC OLED");
  iWidth = lcd.width();
  iHeight = lcd.height();
  Serial.printf("LCD size = %dx%d\n", iWidth, iHeight);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.setFont(FONT_8x8);
  lcd.setCursor(0, 0);
  lcd.println("CYD Touch Test");
  delay(1000);
  Wire.end();
  //Wire1.end();
  bbct.init(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);
  int iType = bbct.sensorType();
  Serial.printf("Sensor type = %s\n", szNames[iType]);
#ifdef OLD_STUFF
if (co2.init(QWIIC_SDA, QWIIC_SCL, 1, 100000) == SCD41_SUCCESS) {
//    Serial.println("Found SCD41 sensor!");
    co2.start(); // start sampling mode
    lcd.println("SCD41 found!");
  } else { // can't find the sensor, stop
    lcd.println("SCD41 sensor not found");
    lcd.println("Check your connections");
    lcd.println("\nstopping...");
    while (1) {};
  } // no sensor connected or some error
#endif
} /* setup() */

void loop() {
#ifndef OLD_STUFF
 int i;
 TOUCHINFO ti;

while (1) {
  if (bbct.getSamples(&ti)) {
    for (int i=0; i<ti.count; i++){
      Serial.print("Touch ");Serial.print(i+1);Serial.print(": ");;
      Serial.print("  x: ");Serial.print(ti.x[i]);
      Serial.print("  y: ");Serial.print(ti.y[i]);
      Serial.print("  size: ");Serial.println(ti.area[i]);
      Serial.println(' ');
      lcd.fillCircle(iWidth-1-ti.y[i], ti.x[i], 3, (i == 0) ? TFT_BLUE : TFT_RED);
    } // for each touch point
  } // if touch event happened
} // while (1)
#else
char szTemp[64];
int i, iCO2;

    delay(5000); // 5 seconds per reading
    // The SCD41 takes 5 seconds to return a sample in this mode
    co2.getSample();
    iCO2 = co2.co2();
    lcd.setCursor(0, 100);
    lcd.println(iCO2);
    sprintf(szTemp, "Temperature %dC ", co2.temperature());
    lcd.println(co2.temperature());
    lcd.println(szTemp);
    sprintf(szTemp, "Humidity %d%%", co2.humidity());
    lcd.println(co2.humidity());
    lcd.println(szTemp);
    #endif
} /* loop() */

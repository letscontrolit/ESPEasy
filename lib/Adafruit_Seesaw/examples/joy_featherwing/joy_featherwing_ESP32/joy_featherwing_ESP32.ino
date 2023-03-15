// This code is only for use with ESP chips and demonstrates how to perform background operations against the I2C bus to
// communicate with the Joy Featherwing peripheral in a non-blocking fashion. As ESP's FreeRTOS does not play nicely with
// performing operations against I2C from within an ISR, a queue is used to signal to a worker task that a button has been
// pressed rather than reading from I2C within the ISR itself. To read from the analog controller, a separate task running
// in a loop polls for changes. It also attempts to calibrate the controller and deterine a center point while compensating
// for any controller drift. All operations over I2C are synchronized with a semaphore to prevent crashes or corruption
// resulting from concurrent operations as the Wire library is not thread safe.

#if !defined(ESP8266) && !defined(ESP32)
#error This sketch only supports ESP32 and ESP8266
#endif // ESP8266 / ESP32

#include "Adafruit_seesaw.h"

// This can be enabled to get extra logging about the position of the controller relative to the correction values
//#define JOY_CALIBRATION_MODE

// This sketch requires that one of the optional interrupt pins on the Joy Featherwing is soldered. This value should match the
// complimentary GPIO pin on the ESP device. See: https://learn.adafruit.com/joy-featherwing/pinouts
#define IRQ_PIN 14 // Pin 14 is the pin directly to the left of the SCL pin on an ESP32

Adafruit_seesaw ss;

// GPIO pins on the Joy Featherwing for reading button presses. These should not be changed.
#define BUTTON_RIGHT 6
#define BUTTON_DOWN 7
#define BUTTON_LEFT 9
#define BUTTON_UP 10
#define BUTTON_SEL 14

// GPIO Analog pins on the Joy Featherwing for reading the analog stick. These should not be changed.
#define STICK_H 3
#define STICK_V 2

// When the analog stick is moved and returns to its center point there may be a deviation from the true center of 512. A calibration will 
// occur when the analog stick read task begins. Even after this calibration there may be some drift on the stick that can make determining 
// the center point error prone. In order to compensate for this, values can be specified to determine a reasonable center point. If you have 
// a use case where you don't care about drift or the center point of the stick, this can all be ignored entirely.
#ifndef STICK_CENTER_POINT
#define STICK_CENTER_POINT 512 // Analog stick will read 0...1024 along each axis
#endif
#ifndef STICK_L_CORRECTION
#define STICK_L_CORRECTION -55
#endif
#ifndef STICK_R_CORRECTION
#define STICK_R_CORRECTION 50
#endif
#ifndef STICK_U_CORRECTION
#define STICK_U_CORRECTION 20
#endif
#ifndef STICK_D_CORRECTION
#define STICK_D_CORRECTION -20
#endif

// Every time the analog values are read they will be slightly different. In order
// to only detect movement when the stick is actually moved, these values can tune
// the minimum amount of movement + or - before it is considered as moved.
#ifndef MIN_STICK_H_MOVE
#define MIN_STICK_H_MOVE 5
#endif
#ifndef MIN_STICK_V_MOVE
#define MIN_STICK_V_MOVE 5
#endif

uint32_t button_mask = (1 << BUTTON_RIGHT) | (1 << BUTTON_DOWN) |
                       (1 << BUTTON_LEFT) | (1 << BUTTON_UP) | (1 << BUTTON_SEL);

QueueHandle_t buttonPressQueue;                      // Queue for notifying of button press changes
SemaphoreHandle_t i2cSem = xSemaphoreCreateBinary(); // This semaphore is used to synchronize calls to I2C to prevent concurrent operations

// ISR that gets triggered when a button is pressed.
void IRAM_ATTR onButtonPress()
{
    // The ISR just sends a signal to the queue. Value doesn't matter.
    uint8_t v = 0;
    if (!xQueueSend(buttonPressQueue, &v, portMAX_DELAY))
    {
        Serial.println("WARNING: Could not queue message because queue is full.");
    }
}

// Log the pressed buttons to the serial port
void outputPressedButtons(uint32_t mask)
{
#ifdef JOY_DEBUG
    Serial.print("Mask: ");
    Serial.println(mask, BIN);
#endif

    if (!(mask & (1 << BUTTON_RIGHT)))
    {
        Serial.println(F("Button A pressed"));
    }
    if (!(mask & (1 << BUTTON_DOWN)))
    {
        Serial.println(F("Button B pressed"));
    }
    if (!(mask & (1 << BUTTON_LEFT)))
    {
        Serial.println(F("Button Y pressed"));
    }
    if (!(mask & (1 << BUTTON_UP)))
    {
        Serial.println(F("Button X pressed"));
    }
    if (!(mask & (1 << BUTTON_SEL)))
    {
        Serial.println(F("Button SEL pressed"));
    }
}

// Queue consumer for responding to button presses
void buttonPressConsumer(void *)
{
    Serial.println(F("buttonPressConsumer() begin"));
    uint32_t lastValue = 0;
    while (true)
    {
        void *p; // Don't care about this value, only that we get queued
        // This will yield until the queue gets signalled
        xQueueReceive(buttonPressQueue, &p, portMAX_DELAY);
        xSemaphoreTake(i2cSem, portMAX_DELAY);
        uint32_t v = ss.digitalReadBulk(button_mask);

        // Debounce by discarding duplicate reads
        if (lastValue != v)
        {
            outputPressedButtons(v);
            lastValue = v;
        }

        xSemaphoreGive(i2cSem);
    }

    vTaskDelete(NULL);
}

void analogStickTask(void *)
{
    Serial.println(F("analogStickTask() begin"));

    int16_t x_ctr, y_ctr;
    xSemaphoreTake(i2cSem, portMAX_DELAY);
    x_ctr = ss.analogRead(STICK_H);
    y_ctr = ss.analogRead(STICK_V);
    xSemaphoreGive(i2cSem);

    Serial.printf("Initial center point x=%d y=%d\n", x_ctr, y_ctr);
    Serial.printf("Calibration values:\n\tCenter point: x=%d y=%d\n\tCorrections: u=%d d=%d l=%d r=%d\n\tCenter range: x=%d...%d y=%d...%d\n",
                  x_ctr,
                  y_ctr,
                  STICK_U_CORRECTION,
                  STICK_D_CORRECTION,
                  STICK_L_CORRECTION,
                  STICK_R_CORRECTION,
                  x_ctr + STICK_L_CORRECTION,
                  x_ctr + STICK_R_CORRECTION,
                  y_ctr + STICK_U_CORRECTION,
                  y_ctr + STICK_D_CORRECTION);

    int16_t x = -1;
    int16_t y = -1;
    bool isCentered = true;
    while (true)
    {
        xSemaphoreTake(i2cSem, portMAX_DELAY);
        int16_t new_x = ss.analogRead(STICK_H);
        int16_t new_y = ss.analogRead(STICK_V);
        xSemaphoreGive(i2cSem);

        // Ignore minute position changes as the values will change slightly with
        // every read. This can be tuned with MIN_STICK_H_MOVE and MIN_STICK_V_MOVE
        if (new_x <= x - MIN_STICK_H_MOVE ||
            new_x >= x + MIN_STICK_H_MOVE ||
            new_y <= y - MIN_STICK_V_MOVE ||
            new_y >= y + MIN_STICK_V_MOVE)
        {

#ifdef JOY_CALIBRATION_MODE
            Serial.printf("x=%d xc=%d x+c(L)=%d x+c(R)=%d <=%d >=%d\n",
                          new_x,
                          x_ctr,
                          new_x + STICK_L_CORRECTION,
                          new_x + STICK_R_CORRECTION,
                          x_ctr >= new_x + STICK_L_CORRECTION,
                          x_ctr <= new_x + STICK_R_CORRECTION);

            Serial.printf("y=%d yc=%d y+c(U)=%d y-c(D)=%d <=%d >=%d\n",
                          new_y,
                          y_ctr,
                          new_y + STICK_U_CORRECTION,
                          new_y + STICK_D_CORRECTION,
                          y_ctr <= new_y + STICK_U_CORRECTION,
                          y_ctr >= new_y + STICK_D_CORRECTION);
#endif

            // Make a best effort guess as to if the stick is centered or not based on
            // initial calibration and corrections
            isCentered = x_ctr >= max(0, new_x + STICK_L_CORRECTION) &&
                         x_ctr <= max(0, new_x + STICK_R_CORRECTION) &&
                         y_ctr <= max(0, new_y + STICK_U_CORRECTION) &&
                         y_ctr >= max(0, new_y + STICK_D_CORRECTION);

            // Ensure value is always 0...1024 and account for any corrections and/or calibrations to prevent over/underflows
            x = new_x < 0 ? 0 : new_x > 1024 ? 1024
                                             : new_x;
            y = new_y < 0 ? 0 : new_y > 1024 ? 1024
                                             : new_y;

            double x_rad = x / 4 - 128;
            double y_rad = y / 4 - 128;
            double angle = -atan2(-x_rad, y_rad) * (180.0 / PI);
            double velocity = sqrt(pow(x_rad, 2) + pow(y_rad, 2));

            // Log the position of the analog stick in various ways for different kinds of application
            Serial.printf("Analog stick position change!\n\tIs centered: %s\n\tPosition: X=%d Y=%d\n\tRadian: X=%f Y=%f\n\tDegrees: %f\n\tPosition from center: %f\n",
                          isCentered ? "true" : "false",
                          x, y,
                          x_rad, y_rad,
                          angle,
                          velocity);
        }

        // Tune this to be quick enough to read the controller position in a reasonable amount of time but not so fast that it 
        // saturates the I2C bus and delays or blocks other operations.
        delay(100);
    }

    vTaskDelete(NULL);
}

void setup()
{
    Serial.begin(115200);

    while (!Serial)
    {
        delay(10);
    }

    Serial.println("Joy FeatherWing example!");

    if (!ss.begin(0x49))
    {
        Serial.println("ERROR! seesaw not found");
        while (1)
        {
            delay(1);
        }
    }
    else
    {
        Serial.println("seesaw started");
        Serial.print("version: ");
        Serial.println(ss.getVersion(), HEX);
    }

    ss.pinModeBulk(button_mask, INPUT_PULLUP);
    ss.setGPIOInterrupts(button_mask, 1);
    pinMode(IRQ_PIN, INPUT);

    xSemaphoreGive(i2cSem); // Initialize the semaphore to 0 (default state is uninitialized which will cause a crash)
    buttonPressQueue = xQueueCreate(10, sizeof(uint8_t));

    // Task for listening to button presses
    xTaskCreate(
        buttonPressConsumer,
        "ButtonPressConsumer",
        10000, // Stack size -- too low and ESP will eventually crash within the task
        NULL,
        1,
        NULL);

    // Task for reading the analog stick value
    xTaskCreate(
        analogStickTask,
        "AnalogStickTask",
        10000, // Stack size -- too low and ESP will eventually crash within the task
        NULL,
        2,
        NULL);

    // Respond to changes from button presses
    attachInterrupt(IRQ_PIN, onButtonPress, FALLING);
}

void loop()
{
    // Do nothing. Everything we're doing here is in a Task
    delay(10000);
}

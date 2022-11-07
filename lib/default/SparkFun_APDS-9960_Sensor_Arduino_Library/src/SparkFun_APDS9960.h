/**
 * @file    SparkFun_APDS-9960.h
 * @brief   Library for the SparkFun APDS-9960 breakout board
 * @author  Shawn Hymel (SparkFun Electronics)
 *
 * @copyright	This code is public domain but you buy me a beer if you use
 * this and we meet someday (Beerware license).
 *
 * This library interfaces the Avago APDS-9960 to Arduino over I2C. The library
 * relies on the Arduino Wire (I2C) library. to use the library, instantiate an
 * APDS9960 object, call init(), and call the appropriate functions.
 */

#ifndef SparkFun_APDS9960_H
#define SparkFun_APDS9960_H

#include <Arduino.h>

/* Debug */
#define DEBUG                   0

/* APDS-9960 I2C address */
#define APDS9960_I2C_ADDR       0x39

/* Gesture parameters */
#define GESTURE_THRESHOLD_OUT   10
#define GESTURE_SENSITIVITY_1   50
#define GESTURE_SENSITIVITY_2   20

/* Error code for returned values */
#define ERROR                   0xFF

/* Acceptable device IDs */
#define APDS9960_ID_1           0xAB
#define APDS9960_ID_2           0x9C
#define APDS9960_ID_3           0xA8    // Add support for GY-9960LLC as requested in original library github

/* Misc parameters */
#define FIFO_PAUSE_TIME         30      // Wait period (ms) between FIFO reads

/* APDS-9960 register addresses */
#define APDS9960_ENABLE         0x80
#define APDS9960_ATIME          0x81
#define APDS9960_WTIME          0x83
#define APDS9960_AILTL          0x84
#define APDS9960_AILTH          0x85
#define APDS9960_AIHTL          0x86
#define APDS9960_AIHTH          0x87
#define APDS9960_PILT           0x89
#define APDS9960_PIHT           0x8B
#define APDS9960_PERS           0x8C
#define APDS9960_CONFIG1        0x8D
#define APDS9960_PPULSE         0x8E
#define APDS9960_CONTROL        0x8F
#define APDS9960_CONFIG2        0x90
#define APDS9960_ID             0x92
#define APDS9960_STATUS         0x93
#define APDS9960_CDATAL         0x94
#define APDS9960_CDATAH         0x95
#define APDS9960_RDATAL         0x96
#define APDS9960_RDATAH         0x97
#define APDS9960_GDATAL         0x98
#define APDS9960_GDATAH         0x99
#define APDS9960_BDATAL         0x9A
#define APDS9960_BDATAH         0x9B
#define APDS9960_PDATA          0x9C
#define APDS9960_POFFSET_UR     0x9D
#define APDS9960_POFFSET_DL     0x9E
#define APDS9960_CONFIG3        0x9F
#define APDS9960_GPENTH         0xA0
#define APDS9960_GEXTH          0xA1
#define APDS9960_GCONF1         0xA2
#define APDS9960_GCONF2         0xA3
#define APDS9960_GOFFSET_U      0xA4
#define APDS9960_GOFFSET_D      0xA5
#define APDS9960_GOFFSET_L      0xA7
#define APDS9960_GOFFSET_R      0xA9
#define APDS9960_GPULSE         0xA6
#define APDS9960_GCONF3         0xAA
#define APDS9960_GCONF4         0xAB
#define APDS9960_GFLVL          0xAE
#define APDS9960_GSTATUS        0xAF
#define APDS9960_IFORCE         0xE4
#define APDS9960_PICLEAR        0xE5
#define APDS9960_CICLEAR        0xE6
#define APDS9960_AICLEAR        0xE7
#define APDS9960_GFIFO_U        0xFC
#define APDS9960_GFIFO_D        0xFD
#define APDS9960_GFIFO_L        0xFE
#define APDS9960_GFIFO_R        0xFF

/* Bit fields */
#define APDS9960_PON            0b00000001
#define APDS9960_AEN            0b00000010
#define APDS9960_PEN            0b00000100
#define APDS9960_WEN            0b00001000
#define APSD9960_AIEN           0b00010000
#define APDS9960_PIEN           0b00100000
#define APDS9960_GEN            0b01000000
#define APDS9960_GVALID         0b00000001

/* On/Off definitions */
#define OFF                     0
#define ON                      1

/* Acceptable parameters for setMode */
#define POWER                   0
#define AMBIENT_LIGHT           1
#define PROXIMITY               2
#define WAIT                    3
#define AMBIENT_LIGHT_INT       4
#define PROXIMITY_INT           5
#define GESTURE                 6
#define ALL                     7

/* LED Drive values */
#define LED_DRIVE_100MA         0
#define LED_DRIVE_50MA          1
#define LED_DRIVE_25MA          2
#define LED_DRIVE_12_5MA        3

/* Proximity Gain (PGAIN) values */
#define PGAIN_1X                0
#define PGAIN_2X                1
#define PGAIN_4X                2
#define PGAIN_8X                3

/* ALS Gain (AGAIN) values */
#define AGAIN_1X                0
#define AGAIN_4X                1
#define AGAIN_16X               2
#define AGAIN_64X               3

/* Gesture Gain (GGAIN) values */
#define GGAIN_1X                0
#define GGAIN_2X                1
#define GGAIN_4X                2
#define GGAIN_8X                3

/* LED Boost values */
#define LED_BOOST_100           0
#define LED_BOOST_150           1
#define LED_BOOST_200           2
#define LED_BOOST_300           3

/* Gesture wait time values */
#define GWTIME_0MS              0
#define GWTIME_2_8MS            1
#define GWTIME_5_6MS            2
#define GWTIME_8_4MS            3
#define GWTIME_14_0MS           4
#define GWTIME_22_4MS           5
#define GWTIME_30_8MS           6
#define GWTIME_39_2MS           7

/* Default values */
#define DEFAULT_ATIME           219     // 103ms
#define DEFAULT_WTIME           246     // 27ms
#define DEFAULT_PROX_PPULSE     0x87    // 16us, 8 pulses
#define DEFAULT_GESTURE_PPULSE  0x89    // 16us, 10 pulses
#define DEFAULT_POFFSET_UR      0       // 0 offset
#define DEFAULT_POFFSET_DL      0       // 0 offset
#define DEFAULT_CONFIG1         0x60    // No 12x wait (WTIME) factor
#define DEFAULT_LDRIVE          LED_DRIVE_100MA
#define DEFAULT_PGAIN           PGAIN_4X
#define DEFAULT_AGAIN           AGAIN_4X
#define DEFAULT_PILT            0       // Low proximity threshold
#define DEFAULT_PIHT            50      // High proximity threshold
#define DEFAULT_AILT            0xFFFF  // Force interrupt for calibration
#define DEFAULT_AIHT            0
#define DEFAULT_PERS            0x11    // 2 consecutive prox or ALS for int.
#define DEFAULT_CONFIG2         0x01    // No saturation interrupts or LED boost
#define DEFAULT_CONFIG3         0       // Enable all photodiodes, no SAI
#define DEFAULT_GPENTH          40      // Threshold for entering gesture mode
#define DEFAULT_GEXTH           30      // Threshold for exiting gesture mode
#define DEFAULT_GCONF1          0x40    // 4 gesture events for int., 1 for exit
#define DEFAULT_GGAIN           GGAIN_4X
#define DEFAULT_GLDRIVE         LED_DRIVE_100MA
#define DEFAULT_GWTIME          GWTIME_2_8MS
#define DEFAULT_GOFFSET         0       // No offset scaling for gesture mode
#define DEFAULT_GPULSE          0xC9    // 32us, 10 pulses
#define DEFAULT_GCONF3          0       // All photodiodes active during gesture
#define DEFAULT_GIEN            0       // Disable gesture interrupts

/* Direction definitions */
enum {
  DIR_NONE,
  DIR_LEFT,
  DIR_RIGHT,
  DIR_UP,
  DIR_DOWN,
  DIR_NEAR,
  DIR_FAR,
  DIR_ALL
};

/* State definitions */

enum {
  N_A_STATE,   // NA_STATE - collision with ESP8266WiFi.h
  NEAR_STATE,
  FAR_STATE,
  ALL_STATE
};


/* Container for gesture data */
typedef struct gesture_data_type {
    uint8_t u_data[32];
    uint8_t d_data[32];
    uint8_t l_data[32];
    uint8_t r_data[32];
    uint8_t index;
    uint8_t total_gestures;
    uint8_t in_threshold;
    uint8_t out_threshold;
} gesture_data_type;

/* APDS9960 Class */
class SparkFun_APDS9960 {
public:

    /* Initialization methods */
    SparkFun_APDS9960();
    ~SparkFun_APDS9960();
    bool init();
    bool init(uint8_t ggain, uint8_t gldrive, uint8_t pgain, uint8_t again, uint8_t led_drive);
    uint8_t getMode();
    bool setMode(uint8_t mode, uint8_t enable);

    /* Turn the APDS-9960 on and off */
    bool enablePower();
    bool disablePower();

    /* Enable or disable specific sensors */
    bool enableLightSensor(bool interrupts = false);
    bool disableLightSensor();
    bool enableProximitySensor(bool interrupts = false);
    bool disableProximitySensor();
    bool enableGestureSensor(bool interrupts = true);
    bool enableGestureSensor(bool interrupts, uint8_t led_boost);
    bool disableGestureSensor();

    /* LED drive strength control */
    uint8_t getLEDDrive();
    bool setLEDDrive(uint8_t drive);
    uint8_t getGestureLEDDrive();
    bool setGestureLEDDrive(uint8_t drive);

    /* Gain control */
    uint8_t getAmbientLightGain();
    bool setAmbientLightGain(uint8_t gain);
    uint8_t getProximityGain();
    bool setProximityGain(uint8_t gain);
    uint8_t getGestureGain();
    bool setGestureGain(uint8_t gain);

    /* Get and set light interrupt thresholds */
    bool getLightIntLowThreshold(uint16_t &threshold);
    bool setLightIntLowThreshold(uint16_t threshold);
    bool getLightIntHighThreshold(uint16_t &threshold);
    bool setLightIntHighThreshold(uint16_t threshold);

    /* Get and set proximity interrupt thresholds */
    bool getProximityIntLowThreshold(uint8_t &threshold);
    bool setProximityIntLowThreshold(uint8_t threshold);
    bool getProximityIntHighThreshold(uint8_t &threshold);
    bool setProximityIntHighThreshold(uint8_t threshold);

    /* Get and set interrupt enables */
    uint8_t getAmbientLightIntEnable();
    bool setAmbientLightIntEnable(uint8_t enable);
    uint8_t getProximityIntEnable();
    bool setProximityIntEnable(uint8_t enable);
    uint8_t getGestureIntEnable();
    bool setGestureIntEnable(uint8_t enable);

    /* Clear interrupts */
    bool clearAmbientLightInt();
    bool clearProximityInt();

    /* Ambient light methods */
    bool readAmbientLight(uint16_t &val);
    bool readRedLight(uint16_t &val);
    bool readGreenLight(uint16_t &val);
    bool readBlueLight(uint16_t &val);

    /* Proximity methods */
    bool readProximity(uint8_t &val);

    /* Gesture methods */
    bool isGestureAvailable();
    int readGesture();
    int readGestureNonBlocking();

private:

    /* Gesture processing */
    void resetGestureParameters();
    bool processGestureData();
    bool decodeGesture();

    /* Proximity Interrupt Threshold */
    uint8_t getProxIntLowThresh();
    bool setProxIntLowThresh(uint8_t threshold);
    uint8_t getProxIntHighThresh();
    bool setProxIntHighThresh(uint8_t threshold);

    /* LED Boost Control */
    uint8_t getLEDBoost();
    bool setLEDBoost(uint8_t boost);

    /* Proximity photodiode select */
    uint8_t getProxGainCompEnable();
    bool setProxGainCompEnable(uint8_t enable);
    uint8_t getProxPhotoMask();
    bool setProxPhotoMask(uint8_t mask);

    /* Gesture threshold control */
    uint8_t getGestureEnterThresh();
    bool setGestureEnterThresh(uint8_t threshold);
    uint8_t getGestureExitThresh();
    bool setGestureExitThresh(uint8_t threshold);

    /* Gesture LED, gain, and time control */
    uint8_t getGestureWaitTime();
    bool setGestureWaitTime(uint8_t time);

    /* Gesture mode */
    uint8_t getGestureMode();
    bool setGestureMode(uint8_t mode);

    /* Raw I2C Commands */
    bool wireWriteByte(uint8_t val);
    bool wireWriteDataByte(uint8_t reg, uint8_t val);
    bool wireWriteDataBlock(uint8_t reg, uint8_t *val, unsigned int len);
    bool wireReadDataByte(uint8_t reg, uint8_t &val);
    int wireReadDataBlock(uint8_t reg, uint8_t *val, unsigned int len);

    /* Members */
    gesture_data_type gesture_data_;
    int gesture_ud_delta_;
    int gesture_lr_delta_;
    int gesture_ud_count_;
    int gesture_lr_count_;
    int gesture_near_count_;
    int gesture_far_count_;
    int gesture_state_;
    int gesture_motion_;

    uint8_t gesture_gain_;
    uint8_t proximity_gain_;
    uint8_t proximity_ldrive_;
    uint8_t ambient_gain_;
};

#endif

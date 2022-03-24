
/*
 * Board configuration (see examples below).
 */

  // Example configuration for Wio Terminal Board
  #define BOARD_BUTTON_PIN            WIO_KEY_A             // Pin where user button is attached
  #define BOARD_BUTTON_ACTIVE_LOW     true                  // true if button is "active-low"

  #define BOARD_LED_PIN               LED_BUILTIN           // Set LED pin - if you have a single-color LED attached
  //#define BOARD_LED_PIN_R           27                    // Set R,G,B pins - if your LED is PWM RGB
  //#define BOARD_LED_PIN_G           26
  //#define BOARD_LED_PIN_B           25
  //#define BOARD_LED_PIN_WS2812      33                    // Set if your LED is WS2812 RGB
  #define BOARD_LED_INVERSE           false                 // true if LED is common anode, false if common cathode
  #define BOARD_LED_BRIGHTNESS        255                   // 0..255 brightness control


/*
 * Advanced options
 */

#define BUTTON_HOLD_TIME_INDICATION   3000
#define BUTTON_HOLD_TIME_ACTION       10000

#define BOARD_PWM_MAX                 1023

#define CONFIG_AP_URL                 "blynk.setup"
#define CONFIG_DEFAULT_SERVER         "blynk.cloud"
#define CONFIG_DEFAULT_PORT           443

#define WIFI_NET_CONNECT_TIMEOUT      30000
#define WIFI_CLOUD_CONNECT_TIMEOUT    30000
#define WIFI_AP_IP                    IPAddress(192, 168, 4, 1)
#define WIFI_AP_Subnet                IPAddress(255, 255, 255, 0)
#define WIFI_CAPTIVE_PORTAL_ENABLE    0

//#define USE_TC3
//#define USE_TCC0

#define BLYNK_NO_DEFAULT_BANNER

#if defined(APP_DEBUG)
  #define DEBUG_PRINT(...) BLYNK_LOG1(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
#endif


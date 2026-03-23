// Settings.h - Cho ESP32-C5 (không màn hình)
#ifndef SETTINGS_H
#define SETTINGS_H

// ---------------------- General settings ---------------------- //
// Change to your Duino-Coin username
extern char *DUCO_USER = "your_username_here";
// Change to your mining key (if set in wallet, otherwise "None")
extern char *MINER_KEY = "None";
// Change if you want a custom miner name, use "Auto" to autogenerate
extern char *RIG_IDENTIFIER = "ESP32-C5";
// WiFi credentials
extern const char SSID[] = "your_wifi_ssid";
extern const char PASSWORD[] = "your_wifi_password";
// -------------------------------------------------------------- //

// -------------------- Advanced options ------------------------ //
// Uncomment to host web dashboard (available at ESP's IP)
#define WEB_DASHBOARD

// Comment to disable LED blinking
#define LED_BLINKING

// Uncomment to use LAN8720 Ethernet (WLAN will be ignored)
// #define USE_LAN

// Comment to disable Serial printing (slightly faster)
#define SERIAL_PRINTING

// Serial baud rate (lower may reduce performance)
#define SERIAL_BAUDRATE 500000

// Uncomment to disable brownout detector
#define DISABLE_BROWNOUT

// Uncomment to enable WiFiManager captive portal (first time setup)
// #define CAPTIVE_PORTAL
// -------------------------------------------------------------- //

// ------------------------ Displays ---------------------------- //
// ALL DISPLAYS DISABLED - MAXIMUM PERFORMANCE FOR MINING
// #define DISPLAY_SSD1306
// #define DISPLAY_16X2
// #define DISPLAY_TFT_ST7789
// -------------------------------------------------------------- //

// ---------------------- IoT options -------------------------- //
// Uncomment to use sensors (will slightly reduce hashrate)
// #define USE_INTERNAL_SENSOR
// #define USE_DS18B20
// #define USE_DHT
// -------------------------------------------------------------- //

// ---------------- Variables and definitions ------------------- //
// You generally do not need to edit below this line

// LED pin for ESP32-C5 (GPIO2 is common, adjust based on your board)
#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif

#define BLINK_SETUP_COMPLETE 2
#define BLINK_CLIENT_CONNECT 5

#define SOFTWARE_VERSION "4.3-C5"

extern unsigned int hashrate = 0;
extern unsigned int hashrate_core_two = 0;
extern unsigned int difficulty = 0;
extern unsigned long share_count = 0;
extern unsigned long accepted_share_count = 0;
extern String node_id = "";
extern String WALLET_ID = "";
extern unsigned int ping = 0;

// Sensor includes (only if enabled)
#if defined(USE_INTERNAL_SENSOR)
  #include "driver/temp_sensor.h"
#endif

#if defined(USE_DS18B20)
  #include <OneWire.h>
  #include <DallasTemperature.h>
  const int DSPIN = 12;
  OneWire oneWire(DSPIN);
  DallasTemperature sensors(&oneWire);
#endif

#if defined(USE_DHT)
  #include <DHT.h>
  #define DHTPIN 12
  #define DHTTYPE DHT11
  DHT dht(DHTPIN, DHTTYPE);
#endif

#endif  // End of SETTINGS_H

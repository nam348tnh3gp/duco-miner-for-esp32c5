/*
   ____  __  __  ____  _  _  _____       ___  _____  ____  _  _
  (  _ \(  )(  )(_  _)( \( )(  _  )___  / __)(  _  )(_  _)( \( )
   )(_) ))(__)(  _)(_  )  (  )(_)((___)( (__  )(_)(  _)(_  )  (
  (____/(______)(____)(_)\_)(_____)     \___)(_____)(____)(_)\_)
  Official Duino-Coin Miner for ESP32-C5
  Optimized - No Display - Maximum Performance

  The Duino-Coin Team & Community 2019-2024 © MIT Licensed
  https://duinocoin.com
*/

#pragma GCC optimize("-Ofast")

#include <ArduinoJson.h>

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266mDNS.h>
    #include <ESP8266HTTPClient.h>
    #include <ESP8266WebServer.h>
#else
    #include <ESPmDNS.h>
    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <WebServer.h>
    #include <WiFiClientSecure.h>
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "MiningJob.h"
#include "Settings.h"

#ifdef USE_LAN
  #include <ETH.h>
#endif

#if defined(WEB_DASHBOARD)
  #include "Dashboard.h"
#endif

// No display includes - for maximum performance

#if !defined(ESP8266) && defined(DISABLE_BROWNOUT)
    #include "soc/soc.h"
    #include "soc/rtc_cntl_reg.h"
#endif

// Core configuration - ESP32-C5 is single core RISC-V
#define CORE 1

#if defined(WEB_DASHBOARD)
    WebServer server(80);
#endif 

#if defined(CAPTIVE_PORTAL)
  #include <FS.h>
  #include <WiFiManager.h>
  #include <Preferences.h>
  char duco_username[40];
  char duco_password[40];
  char duco_rigid[24];
  WiFiManager wifiManager;
  Preferences preferences;
  WiFiManagerParameter custom_duco_username("duco_usr", "Duino-Coin username", duco_username, 40);
  WiFiManagerParameter custom_duco_password("duco_pwd", "Duino-Coin mining key (if enabled in the wallet)", duco_password, 40);
  WiFiManagerParameter custom_duco_rigid("duco_rig", "Custom miner identifier (optional)", duco_rigid, 24);
  
  void saveConfigCallback() {
    preferences.begin("duino_config", false);
    preferences.putString("duco_username", custom_duco_username.getValue());
    preferences.putString("duco_password", custom_duco_password.getValue());
    preferences.putString("duco_rigid", custom_duco_rigid.getValue());
    preferences.end();
    RestartESP("Settings saved");
  }

  void reset_settings() {
    server.send(200, "text/html", "Settings have been erased. Please redo the configuration.");
    delay(500);
    wifiManager.resetSettings();
    RestartESP("Manual settings reset");
  }
#endif

void RestartESP(String msg) {
  #if defined(SERIAL_PRINTING)
    Serial.println(msg);
    Serial.println("Restarting ESP...");
  #endif

  #if defined(ESP8266)
    ESP.reset();
  #else
    ESP.restart();
    abort();
  #endif
}

namespace {
    MiningConfig *configuration = new MiningConfig(
        DUCO_USER,
        RIG_IDENTIFIER,
        MINER_KEY
    );

    #ifdef USE_LAN
      static bool eth_connected = false;
    #endif

    void UpdateHostPort(String input) {
        DynamicJsonDocument doc(256);
        deserializeJson(doc, input);
        const char *name = doc["name"];

        configuration->host = doc["ip"].as<String>().c_str();
        configuration->port = doc["port"].as<int>();
        node_id = String(name);

        #if defined(SERIAL_PRINTING)
          Serial.println("Selected node: " + node_id);
        #endif
    }

    void VerifyWifi() {
      #ifdef USE_LAN
        while ((!eth_connected) || (ETH.localIP() == IPAddress(0, 0, 0, 0))) {
          #if defined(SERIAL_PRINTING)
            Serial.println("Ethernet reconnecting...");
          #endif
          SetupWifi();
        }
      #else
        while (WiFi.status() != WL_CONNECTED 
                || WiFi.localIP() == IPAddress(0, 0, 0, 0)
                || WiFi.localIP() == IPAddress(192, 168, 4, 2) 
                || WiFi.localIP() == IPAddress(192, 168, 4, 3)) {
            #if defined(SERIAL_PRINTING)
              Serial.println("WiFi reconnecting...");
            #endif
            WiFi.disconnect();
            delay(500);
            WiFi.reconnect();
            delay(500);
        }
      #endif
    }

    String httpGetString(String URL) {
        String payload = "";
        
        WiFiClientSecure client;
        HTTPClient https;
        client.setInsecure();

        https.begin(client, URL);
        https.addHeader("Accept", "*/*");
        
        int httpCode = https.GET();
        #if defined(SERIAL_PRINTING)
            Serial.printf("HTTP Response: %d\n", httpCode);
        #endif

        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            payload = https.getString();
        } else {
            #if defined(SERIAL_PRINTING)
               Serial.printf("Error: %s\n", https.errorToString(httpCode).c_str());
               VerifyWifi();
            #endif
        }
        https.end();
        return payload;
    }

    void SelectNode() {
        String input = "";
        int waitTime = 1;

        while (input == "") {
            #if defined(SERIAL_PRINTING)
              Serial.println("Fetching node in " + String(waitTime) + "s");
            #endif
            delay(waitTime * 1000);
            
            input = httpGetString("https://server.duinocoin.com/getPool");
            
            waitTime *= 2;
            if (waitTime > 32) 
                RestartESP("Node fetch failed");
        }

        UpdateHostPort(input);
    }

    #ifdef USE_LAN
        void WiFiEvent(WiFiEvent_t event) {
            switch (event) {
              case ARDUINO_EVENT_ETH_START:
                #if defined(SERIAL_PRINTING)
                    Serial.println("ETH Started");
                #endif
                ETH.setHostname("esp32-c5-miner");
                break;
            case ARDUINO_EVENT_ETH_CONNECTED:
                #if defined(SERIAL_PRINTING)
                    Serial.println("ETH Connected");
                #endif
                break;
            case ARDUINO_EVENT_ETH_GOT_IP:
                #if defined(SERIAL_PRINTING)
                    Serial.println("ETH Got IP");
                #endif
                eth_connected = true;
                break;
            case ARDUINO_EVENT_ETH_DISCONNECTED:
                #if defined(SERIAL_PRINTING)
                    Serial.println("ETH Disconnected");
                #endif
                eth_connected = false;
                break;
            default:
                break;
            }
        }
    #endif

    void SetupWifi() {
      #ifdef USE_LAN
        #if defined(SERIAL_PRINTING)
            Serial.println("Connecting to Ethernet...");
        #endif
        WiFi.onEvent(WiFiEvent);
        ETH.begin();
        
        while (!eth_connected) {
            delay(500);
            #if defined(SERIAL_PRINTING)
                Serial.print(".");
            #endif
        }

        #if defined(SERIAL_PRINTING)
            Serial.println("\nEthernet connected");
            Serial.println("IP: " + ETH.localIP().toString());
            Serial.println("Rig: " + String(RIG_IDENTIFIER));
        #endif

      #else
        #if defined(SERIAL_PRINTING)
            Serial.println("Connecting to: " + String(SSID));
        #endif
        
        WiFi.begin(SSID, PASSWORD);
        while(WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(100);
        }
        VerifyWifi();

        #if defined(SERIAL_PRINTING)
            Serial.println("\nWiFi connected");
            Serial.println("IP: " + WiFi.localIP().toString());
            Serial.println("Rig: " + String(RIG_IDENTIFIER));
        #endif
      #endif

      SelectNode();
    }

    void SetupOTA() {
        ArduinoOTA.onStart([]()
                           { 
                             #if defined(SERIAL_PRINTING)
                               Serial.println("OTA Start"); 
                             #endif
                           });
        ArduinoOTA.onEnd([]()
                         { 
                            #if defined(SERIAL_PRINTING)
                              Serial.println("\nOTA End"); 
                            #endif
                         });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                              { 
                                 #if defined(SERIAL_PRINTING)
                                   Serial.printf("Progress: %u%%\r", (progress / (total / 100))); 
                                 #endif
                              });
        ArduinoOTA.onError([](ota_error_t error)
                           {
                                Serial.printf("Error[%u]: ", error);
                                #if defined(SERIAL_PRINTING)
                                  if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                                  else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                                  else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                                  else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                                  else if (error == OTA_END_ERROR) Serial.println("End Failed");
                                #endif
                          });

        ArduinoOTA.setHostname(RIG_IDENTIFIER);
        ArduinoOTA.begin();
    }

    #if defined(WEB_DASHBOARD)
        void dashboard() {
             String s = WEBSITE;
             #ifdef USE_LAN
              s.replace("@@IP_ADDR@@", ETH.localIP().toString());
             #else
              s.replace("@@IP_ADDR@@", WiFi.localIP().toString());
             #endif
  
             s.replace("@@HASHRATE@@", String((hashrate + hashrate_core_two) / 1000));
             s.replace("@@DIFF@@", String(difficulty / 100));
             s.replace("@@SHARES@@", String(share_count));
             s.replace("@@NODE@@", String(node_id));
             
             s.replace("@@DEVICE@@", "ESP32-C5");
             s.replace("@@ID@@", String(RIG_IDENTIFIER));
             s.replace("@@MEMORY@@", String(ESP.getFreeHeap()));
             s.replace("@@VERSION@@", String(SOFTWARE_VERSION));

             #if defined(CAPTIVE_PORTAL)
                 s.replace("@@RESET_SETTINGS@@", "&bull; <a href='/reset'>Reset settings</a>");
             #else
                 s.replace("@@RESET_SETTINGS@@", "");
             #endif

             #if defined(USE_DS18B20)
                 sensors.requestTemperatures(); 
                 float temp = sensors.getTempCByIndex(0);
                 s.replace("@@SENSOR@@", "DS18B20: " + String(temp) + "*C");
             #elif defined(USE_DHT)
                 float temp = dht.readTemperature();
                 float hum = dht.readHumidity();
                 s.replace("@@SENSOR@@", "DHT: " + String(temp) + "*C, " + String(hum) + "%");
             #elif defined(USE_INTERNAL_SENSOR)
                 float temp = 0;
                 temp_sensor_read_celsius(&temp);
                 s.replace("@@SENSOR@@", "CPU: " + String(temp) + "*C");
             #else
                 s.replace("@@SENSOR@@", "None");
             #endif
                 
             server.send(200, "text/html", s);
        }
    #endif

} // End of namespace

MiningJob *job;

void setup() {
    #if !defined(ESP8266) && defined(DISABLE_BROWNOUT)
        WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    #endif
    
    #if defined(SERIAL_PRINTING)
        Serial.begin(SERIAL_BAUDRATE);
        Serial.println("\nDuino-Coin " + String(configuration->MINER_VER));
        Serial.println("ESP32-C5 Miner - No Display Mode");
    #endif
    
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    WALLET_ID = String(random(0, 2811));
    job = new MiningJob(0, configuration);

    // Sensor initialization (if enabled)
    #if defined(USE_DHT)
        dht.begin();
    #endif

    #if defined(USE_DS18B20)
        sensors.begin();
        sensors.requestTemperatures(); 
    #endif

    #if defined(USE_INTERNAL_SENSOR)
       temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
       temp_sensor.dac_offset = TSENS_DAC_L2;
       temp_sensor_set_config(temp_sensor);
       temp_sensor_start();
    #endif

    WiFi.mode(WIFI_STA);
    #if defined(ESP8266)
        WiFi.setSleepMode(WIFI_NONE_SLEEP);
    #else
        WiFi.setSleep(false);
    #endif
    
    #if defined(CAPTIVE_PORTAL)
        preferences.begin("duino_config", false);
        strcpy(duco_username, preferences.getString("duco_username", "username").c_str());
        strcpy(duco_password, preferences.getString("duco_password", "None").c_str());
        strcpy(duco_rigid, preferences.getString("duco_rigid", "None").c_str());
        preferences.end();
        configuration->DUCO_USER = duco_username;
        configuration->RIG_IDENTIFIER = duco_rigid;
        configuration->MINER_KEY = duco_password;
        RIG_IDENTIFIER = duco_rigid;

        wifiManager.setSaveConfigCallback(saveConfigCallback);
        wifiManager.addParameter(&custom_duco_username);
        wifiManager.addParameter(&custom_duco_password);
        wifiManager.addParameter(&custom_duco_rigid);

        wifiManager.autoConnect("Duino-Coin");
        delay(1000);
        VerifyWifi();
        SelectNode();
    #else
        SetupWifi();
    #endif
    
    SetupOTA();

    #if defined(WEB_DASHBOARD)
      if (!MDNS.begin(RIG_IDENTIFIER)) {
        #if defined(SERIAL_PRINTING)
          Serial.println("mDNS unavailable");
        #endif
      }
      MDNS.addService("http", "tcp", 80);
      #if defined(SERIAL_PRINTING)
        #ifdef USE_LAN
          Serial.println("Dashboard: http://" + ETH.localIP().toString());
        #else
          Serial.println("Dashboard: http://" + WiFi.localIP().toString());
        #endif
      #endif

      server.on("/", dashboard);
      #if defined(CAPTIVE_PORTAL)
        server.on("/reset", reset_settings);
      #endif
      server.begin();
    #endif

    // Set CPU to max frequency for ESP32-C5 (240 MHz)
    setCpuFrequencyMhz(240);
    
    #if defined(SERIAL_PRINTING)
        Serial.println("Setup complete - Mining started");
    #endif
    
    job->blink(BLINK_SETUP_COMPLETE);
}

void loop() {
    job->mine();
    
    // Feed watchdog and handle events
    yield();
    ArduinoOTA.handle();
    #if defined(WEB_DASHBOARD) 
        server.handleClient();
    #endif
    
    // Periodic WiFi check
    static unsigned long lastWifiCheck = 0;
    if (millis() - lastWifiCheck > 30000) {
        VerifyWifi();
        lastWifiCheck = millis();
    }
    
    // Print mining stats every 60 seconds
    #if defined(SERIAL_PRINTING)
    static unsigned long lastStatsPrint = 0;
    if (millis() - lastStatsPrint > 60000) {
        float total_hr = (hashrate + hashrate_core_two) / 1000.0;
        float accept_rate = (share_count > 0) ? (accepted_share_count * 100.0 / share_count) : 0;
        Serial.printf("Stats - HR: %.1f kH/s | Shares: %lu/%lu (%.1f%%) | Diff: %u | Ping: %ums\n",
                      total_hr, accepted_share_count, share_count, accept_rate, difficulty / 100, ping);
        lastStatsPrint = millis();
    }
    #endif
}

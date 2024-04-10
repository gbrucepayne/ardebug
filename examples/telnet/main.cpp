/**
 * @brief Example of remote/telnet ardebug library use.
 */
#include <Arduino.h>
#define ARDEBUG_ENABLED
#define ARDEBUG_WIFI
#include "ardebug.h"
#include "secrets.h"

#if defined(ESP32) || defined(ESP8266)
#define USE_MDNS true
#include <DNSServer.h>
#if defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else // ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif // ESP8266
#else
#error "The board must be ESP32 or ESP8266"
#endif  // ESP

#ifndef LED_BUILTIN
#define LED_BUILTIN (2)
#endif

// WiFi Station credentials
#define HOST_NAME "remotedebug"
const char* ssid = WIFI_SSID;   // from secrets.h
const char* password = WIFI_PASS;   // from secrets.h

ardebug::DebugContext& debug = ardebug::DebugContext::get();

uint32_t last_tick = 0;
uint32_t runtime_s = 0;

void foo();
void initWifi();

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(">>> Starting remote debug example...");
  // Configure LED for visual status
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  initWifi();
  debug.begin(nullptr, HOST_NAME);
  debug.showColors(true);
  Serial.print("* Started remote debug Telnet server: ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(ARDEBUG_TELNET_PORT);
}

void loop() {
  if ((millis() - last_tick) >= 1000) {   // every second
    last_tick = millis();
    runtime_s++;
    // Blink the led
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    // Print test output every 5 seconds
    if (runtime_s % 5 == 0) {
      AR_LOGV("* This is a message of debug level VERBOSE (%d)", ardebug::VERBOSE);
      AR_LOGD("* This is a message of debug level DEBUG (%d)", ardebug::DEBUG);
      AR_LOGI("* This is a message of debug level INFO (%d)", ardebug::INFO);
      AR_LOGW("* This is a message of debug level WARNING (%d)", ardebug::WARNING);
      AR_LOGE("* This is a message of debug level ERROR (%d)", ardebug::ERROR);
      foo();   // test function with tag

      uint8_t old_level = debug.logLevel();
      uint8_t new_level = old_level < ardebug::ERROR ? old_level + 1 : 0;
      bool toggled = ((old_level >= ardebug::INFO && new_level < ardebug::INFO) ||
          (old_level < ardebug::INFO && new_level >= ardebug::INFO));
      debug.dprintf("Changing to log level %d%s\n", new_level,
          toggled ? " (toggling time)" : "");
      debug.setLogLevel(new_level);
      debug.showTime(new_level < ardebug::INFO);
    }
  }
  debug.handle();   // alternatively ARD_HANDLE()
  yield();
}

void initWifi() {
  Serial.print("* Connecting to WiFi AP");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.print("\r\nConnected to IP address: ");
  Serial.println(WiFi.localIP());
  // Register host name in WiFi and mDNS
  WiFi.hostname(String(HOST_NAME));
  if (MDNS.begin(HOST_NAME)) {
    MDNS.addService("telnet", "tcp", ARDEBUG_TELNET_PORT);
    Serial.print("* MDNS responder started for hostname: ");
    Serial.println(HOST_NAME);
  }
  Serial.println("* Please use a telnet client to connect for remote debug.");
}

// Function example to show a different function name
void foo() {
  debugI("This is info from the foo function");
}

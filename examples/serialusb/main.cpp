/**
 * @brief Example of Serial ardebug usage
 */

// #define ARDEBUG_DISABLED

#include <Arduino.h>
#include "ardebug.h"
#if defined(ESP32) || defined(ESP8266)
#include "esp_log.h"
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN (2)
#endif

uint32_t last_tick = 0;
uint32_t runtime_s = 0;
const char* TAG = "TestTag";
const char* AR_TAG = "[TestTag]";

// Function example to show a different function name
void foo();

void setup() {
  // Initialize the Serial (use only in setup codes)
  Serial.begin(115200);
  delay(500);
  ardebugBegin(&Serial, nullptr, nullptr);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("\r\n>>> Starting with log_level: ");
  Serial.println(ardebugGetLevel());
#if defined(ESP32) || defined(ESP8266)
  ESP_LOGI(TAG, "This is a %s message for comparison.", "esp_log");
#endif
  AR_LOGI("This is a %s log", "ardebug");
  AR_LOGI("%s ardebug log with a esp_log style tag", AR_TAG);
}

void loop() {
#if defined(ESP32) || defined(ESP8266)
    if (last_tick > 999999) ESP_LOGI(TAG, "big time!");
#endif
  if ((millis() - last_tick) >= 1000) {   // every second
    last_tick = millis();
    runtime_s++;
    // Blink the led
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    // Print test output every 5 seconds
    if (runtime_s % 5 == 0) {
      AR_LOGV("This is a message of debug level VERBOSE (%d)", ARDEBUG_V);
      AR_LOGD("This is a message of debug level DEBUG (%d)", ARDEBUG_D);
      AR_LOGI("This is a message of debug level INFO (%d)", ARDEBUG_I);
      AR_LOGW("This is a message of debug level WARNING (%d)", ARDEBUG_W);
      AR_LOGE("This is a message of debug level ERROR (%d)", ARDEBUG_E);
      foo();   // test function with tag
#ifndef ARDEBUG_DISABLED
      uint8_t old_level = ardebugGetLevel();
      uint8_t new_level = old_level < ARDEBUG_V ? old_level + 1 : 0;
      bool toggled = ((old_level >= ARDEBUG_I && new_level < ARDEBUG_I) ||
          (old_level < ARDEBUG_I && new_level >= ARDEBUG_I));
      ardprintf("Changing to log level %d%s\n", new_level,
          toggled ? " (toggling time)" : "");
      ardebugSetLevel(new_level);
      ardebugTime(new_level < ARDEBUG_I);
#endif
    }
  }
}

void foo() {
  AR_LOGI("This is info from the foo function");
}
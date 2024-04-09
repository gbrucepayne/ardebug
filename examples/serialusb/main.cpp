/**
 * @brief Example of Serial ardebug usage
 */
#include <Arduino.h>

#include "ardebug.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN (2)
#endif

ardebug::DebugContext& debug = ardebug::DebugContext::get();

uint32_t last_tick = 0;
uint32_t runtime_s = 0;

// Function example to show a different function name
void foo() {
  AR_LOGI("This is info from the foo function");
}

void setup() {
  // Initialize the Serial (use only in setup codes)
  Serial.begin(115200);
  delay(500);
  debug.begin(&Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("\r\n>>> Starting with log_level: ");
  Serial.println(debug.logLevel());
  debug.showColors(true);
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
}

/**
 * @brief Defines supported board capabilities
*/

#ifndef ARDEBUG_BOARDS_H
#define ARDEBUG_BOARDS_H

#if defined(ESP32) || defined(ESP8266)
#define BOARD_WIFI
#define ARDEBUG_BUFFER_SIZE (128)
#include <Print.h>
#if defined(ESP32)
#include <WiFi.h>
#define BOARD_MULTI_CORE
#else
#include <ESP8266WIFI.H>
extern "C" { bool system_update_cpu_freq(uint8_t freq); }
#endif
#else
#define BOARD_AVR
#define BOARD_LOW_MEMORY
#define ARDEBUG_BUFFER_SIZE (64)
#include "avrprint.h"
#endif

#endif
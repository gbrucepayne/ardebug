/**
 * @brief Defines supported board capabilities
*/

#ifndef ARDEBUG_BOARDS_H
#define ARDEBUG_BOARDS_H

#if defined(ESP32) || defined(ESP8266)
#define BOARD_WIFI
#include <DNSServer.h>
// #include <Print.h>
#if defined(ESP32)
#define BOARD_MULTI_CORE
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#include <ESP8266WIFI.H>
#include <ESP8266mDNS.h>
extern "C" { bool system_update_cpu_freq(uint8_t freq); }
#endif
#else
#define BOARD_LOW_MEMORY
#endif

#endif
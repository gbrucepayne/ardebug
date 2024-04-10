#ifndef ARDEBUG_H
#define ARDEBUG_H

// #define ARDEBUG_ENABLED   // uncomment for testing
// #define ARDEBUG_WIFI   // uncomment for wifi testing

#ifdef ARDEBUG_ENABLED

#define _ARDEBUG_VERSION_ "4.0.0"

#include <Arduino.h>
#include <stdint.h>
#include "boards.h"

namespace ardebug {

#if defined(BOARD_WIFI) && defined(ARDEBUG_WIFI)
// #define DEBUG_DISABLE_AUTO_FUNC  // uncomment to remove function names
#define ARDEBUG_TELNET_PORT 23
#define ARDEBUG_MAX_PWD_ATTEMPTS 3
#endif  // BOARD_WIFI

// ANSI Colors
#define ARD_COLOR_RESET "\x1B[0m"
#define ARD_COLOR_RED "31"
#define ARD_COLOR_GREEN "32"
#define ARD_COLOR_YELLOW "33"
#define ARD_COLOR_BLUE "34"
#define ARD_COLOR_MAGENTA "35"
#define ARD_COLOR_CYAN "36"
#define ARD_COLOR_WHITE "37"
#define ARD_COLOR(COLOR) "\x1B[0;" COLOR "m"
#define ARD_BOLD(COLOR) "\x1B[1;" COLOR "m"

#define ARD_COLOR_V ARD_COLOR(ARD_COLOR_MAGENTA)
#define ARD_COLOR_D ARD_COLOR(ARD_COLOR_BLUE)
#define ARD_COLOR_I ARD_COLOR(ARD_COLOR_GREEN)
#define ARD_COLOR_W ARD_COLOR(ARD_COLOR_YELLOW)
#define ARD_COLOR_E ARD_COLOR(ARD_COLOR_RED)

#define ARDEBUG_CMD_BUFFER (16)

// Log levels
const uint8_t PROFILER = 0;  // Include time of code block execution
const uint8_t VERBOSE = 1;
const uint8_t DEBUG = 2;
const uint8_t INFO = 3;
const uint8_t WARNING = 4;
const uint8_t ERROR = 5;

// Auto function for debug macros?
#define ardebugV(fmt, ...) \
    ardebug::DebugContext::get().debugf(ardebug::VERBOSE, __func__, fmt, ##__VA_ARGS__)
#define ardebugD(fmt, ...) \
    ardebug::DebugContext::get().debugf(ardebug::DEBUG, __func__, fmt, ##__VA_ARGS__)
#define ardebugI(fmt, ...) \
    ardebug::DebugContext::get().debugf(ardebug::INFO, __func__, fmt, ##__VA_ARGS__)
#define ardebugW(fmt, ...) \
    ardebug::DebugContext::get().debugf(ardebug::WARNING, __func__, fmt, ##__VA_ARGS__)
#define ardebugE(fmt, ...) \
    ardebug::DebugContext::get().debugf(ardebug::ERROR, __func__, fmt, ##__VA_ARGS__)

#define ardprintf(fmt, ...) ardebug::DebugContext::get().dprintf(fmt, ##__VA_ARGS__)

// With newline
#define ardebugVln(fmt, ...) ardebugV(fmt "\n", ##__VA_ARGS__)
#define ardebugDln(fmt, ...) ardebugD(fmt "\n", ##__VA_ARGS__)
#define ardebugIln(fmt, ...) ardebugI(fmt "\n", ##__VA_ARGS__)
#define ardebugWln(fmt, ...) ardebugW(fmt "\n", ##__VA_ARGS__)
#define ardebugEln(fmt, ...) ardebugE(fmt "\n", ##__VA_ARGS__)

#define AR_LOGV(fmt, ...) ardebugVln(fmt, ##__VA_ARGS__)
#define AR_LOGD(fmt, ...) ardebugDln(fmt, ##__VA_ARGS__)
#define AR_LOGI(fmt, ...) ardebugIln(fmt, ##__VA_ARGS__)
#define AR_LOGW(fmt, ...) ardebugWln(fmt, ##__VA_ARGS__)
#define AR_LOGE(fmt, ...) ardebugEln(fmt, ##__VA_ARGS__)

#define debugV(fmt, ...) ardebugVln(fmt, ##__VA_ARGS__)
#define debugD(fmt, ...) ardebugDln(fmt, ##__VA_ARGS__)
#define debugI(fmt, ...) ardebugIln(fmt, ##__VA_ARGS__)
#define debugW(fmt, ...) ardebugWln(fmt, ##__VA_ARGS__)
#define debugE(fmt, ...) ardebugEln(fmt, ##__VA_ARGS__)

#define ardebugHandle() ardebug::DebugContext::get().handle()
#define ardebugLogLevel() ardebug::DebugContext::get().logLevel()
#define ardebugSetLogLevel(level) ardebug::DebugContext::get().setLogLevel(level)

class DebugContext {
  private:
    uint8_t log_level_ = ardebug::INFO;
    boolean serial_enabled_ = false;
    Stream* serial_ = nullptr;
    boolean telnet_enabled_ = false;
    boolean telnet_listening_ = false;
    boolean file_enabled_ = false;
    boolean show_func_ = true;
    boolean show_millis_ = false;
    boolean show_color_ = false;
#if defined(BOARD_WIFI) && defined(ARDEBUG_WIFI)
    char hostname[32] = {0};
    char password_[21] = {0};
    boolean password_ok_ = false;
    uint8_t password_attempt_ = 0;
    char telnet_cmd_[ARDEBUG_CMD_BUFFER] = {0};
#endif // BOARD_WIFI
    void showHelp();
    void processCommand();
    void onConnect();
    
    DebugContext() {}   // private for singleton

  public:
    // singleton
    static DebugContext& get() {
      static DebugContext instance;
      return instance;
    }
    DebugContext(const DebugContext&) = delete;
    void operator=(const DebugContext&) = delete;
    ~DebugContext();

    bool begin(Stream* stream = nullptr, const char* host_name = nullptr, const char* file_name = nullptr);
    void stop();

    bool setPassword(const char* password);
    boolean isConnected();
    void handle();
    void disconnect();

    size_t dprintf(const char* fmt, ...);
    size_t debugf(uint8_t level, const char* caller, const char* fmt, ...);

    uint8_t logLevel() { return log_level_; }
    void setLogLevel(uint8_t level) { if (level <= ERROR) log_level_ = level; }
    void enableSerial(boolean enable) { serial_enabled_ = enable; }
    void showFunction(boolean show) { show_func_ = show; }
    void showTime(boolean show) { show_millis_ = show; }
    void showColors(boolean show) { show_color_ = show; }
    uint32_t getFreeMemory();

    boolean isActive(uint8_t debugLevel = DEBUG) { return false; }

};

} // namespace ardebug

#else  // DEBUG_DISABLED

#define ardebugV(...)
#define ardebugD(...)
#define ardebugI(...)
#define ardebugW(...)
#define ardebugE(...)

#define ardprintf(...)

#define ardebugVln(...)
#define ardebugDln(...)
#define ardebugIln(...)
#define ardebugWln(...)
#define ardebugEln(...)

#define AR_LOGV(...)
#define AR_LOGD(...)
#define AR_LOGI(...)
#define AR_LOGW(...)
#define AR_LOGE(...)

#define debugV(...)
#define debugD(...)
#define debugI(...)
#define debugW(...)
#define debugE(...)

#define ardebugHandle()
#define ardebugLogLevel()
#define ardebugSetLogLevel(level)

#endif  // DEBUG_DISABLED



#endif  // ARDEBUG_H
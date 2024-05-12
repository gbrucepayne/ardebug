#ifndef ARDEBUG_H
#define ARDEBUG_H

#define ARDEBUG_VERSION "1.0.0"

#define ARDEBUG_V 4
#define ARDEBUG_D 3
#define ARDEBUG_I 2
#define ARDEBUG_W 1
#define ARDEBUG_E 0

#define ARDEBUG_TELNET_PORT 23

#ifndef ARDEBUG_DISABLED
// #if defined(ARDEBUG_ENABLE)

#include <Arduino.h>
#include <stdint.h>
#include "boards.h"

#define ARDEBUG_MAX_PWD_ATTEMPTS 3

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

#define ARD_COLOR_V ARD_BOLD(ARD_COLOR_MAGENTA)
#define ARD_COLOR_D ARD_BOLD(ARD_COLOR_BLUE)
#define ARD_COLOR_I ARD_BOLD(ARD_COLOR_GREEN)
#define ARD_COLOR_W ARD_BOLD(ARD_COLOR_YELLOW)
#define ARD_COLOR_E ARD_BOLD(ARD_COLOR_RED)
#define ARD_COLORSIZE 10

// Buffers

// Character sizing for debug log prefixes ...total max 64-byte buffer
#define ARDEBUG_LEVEL_TAG_SIZE 3  // [x]
#define ARDEBUG_MILLIS_SIZE 12  // "[4294967295]"
#define ARDEBUG_FILENAME_SIZE 20  // "[<filename>" ...seems reasonable
#define ARDEBUG_LINENO_SIZE 6  // ":9999]" ...seems reasonable
#define ARDEBUG_FUNCNAME_SIZE 20  // " <funcname>()" ...seems reasonable
#define ARDEBUG_DELIM_SIZE 2   // ": "
#define ARDEBUG_MIN_PREFIX_SIZE (ARDEBUG_LEVEL_TAG_SIZE + ARDEBUG_DELIM_SIZE)
#ifdef BOARD_LOW_MEMORY
#define ARDEBUG_MAX_PREFIX_SIZE ARDEBUG_MIN_PREFIX_SIZE
#else
#define ARDEBUG_MAX_PREFIX_SIZE (ARDEBUG_MIN_PREFIX_SIZE + \
    ARDEBUG_MILLIS_SIZE + ARDEBUG_FILENAME_SIZE + ARDEBUG_LINENO_SIZE + \
    ARDEBUG_FUNCNAME_SIZE)
#endif

// Buffer for message output to include prefix
#ifndef ARDEBUG_BUFFER_SIZE
#ifdef BOARD_LOW_MEMORY
#define ARDEBUG_BUFFER_SIZE ARDEBUG_MIN_PREFIX_SIZE + 42 + 1  // 48 bytes
#else
#define ARDEBUG_BUFFER_SIZE ARDEBUG_MAX_PREFIX_SIZE + 128 + 1  // 192 bytes
#endif
#endif // ARDEBUG_BUFFER_SIZE
#if ARDEBUG_BUFFER_SIZE < 32
#error "ARDEBUG_BUFFER_SIZE must be at least 32 bytes"
#endif

// Buffer for telnet command input
#define ARDEBUG_CMD_BUFFER 8  // max size of telnet command 7 chars

namespace ardebug {

struct DebugMessage {
  uint8_t level;
  uint32_t millis;
  char filename[ARDEBUG_FILENAME_SIZE];
  uint32_t lineno;
  char func[ARDEBUG_FUNCNAME_SIZE];
  char message[ARDEBUG_BUFFER_SIZE];
};

/**
 * @brief A debug logging context that allows USB or Telnet monitoring
*/
class DebugContext {
  private:
    uint8_t log_level_ = ARDEBUG_I;
    boolean low_memory_ = false;
    boolean serial_enabled_ = false;
    Stream* serial_ = nullptr;
    boolean telnet_enabled_ = false;
    boolean telnet_listening_ = false;
    boolean file_enabled_ = false;
    boolean show_millis_ = true;
    boolean show_line_ = true;
    boolean show_func_ = true;
    boolean show_core_ = true;
    boolean show_color_ = true;
#if defined(BOARD_WIFI) && !defined(ARDEBUG_WIFI_DISABLED)
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

    bool begin(Stream* stream = nullptr,
               const char* host_name = nullptr,
               const char* file_name = nullptr);
    void stop();

    bool setPassword(const char* password);
    boolean isConnected();
    void handle();
    void disconnect();

    size_t dprintf(const char* fmt, ...);
    size_t debugf(uint8_t level,
                  const char* caller,
                  const char* filename,
                  uint32_t lineno,
                  const char* fmt, ...);

    uint8_t logLevel() { return log_level_; }
    void setLogLevel(uint8_t level) { if (level <= ARDEBUG_V) log_level_ = level; }
    void enableSerial(boolean enable) { serial_enabled_ = enable; }  // redundant?
    void showTime(boolean show) { if (!low_memory_) show_millis_ = show; }
    void showLine(boolean show) { if (!low_memory_) show_line_ = show; }
    void showFunc(boolean show) { if (!low_memory_) show_func_ = show; }
    void showCore(boolean show) { if (!low_memory_) show_core_ = show; }
    void showColors(boolean show) { if (!low_memory_) show_color_ = show; }
    uint32_t getFreeMemory();

};

} // namespace ardebug

// Macros
#define ardebugV(fmt, ...) \
    ardebug::DebugContext::get().debugf(ARDEBUG_V, __func__, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define ardebugD(fmt, ...) \
    ardebug::DebugContext::get().debugf(ARDEBUG_D, __func__, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define ardebugI(fmt, ...) \
    ardebug::DebugContext::get().debugf(ARDEBUG_I, __func__, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define ardebugW(fmt, ...) \
    ardebug::DebugContext::get().debugf(ARDEBUG_W, __func__, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define ardebugE(fmt, ...) \
    ardebug::DebugContext::get().debugf(ARDEBUG_E, __func__, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)

#define ardprintf(fmt, ...) ardebug::DebugContext::get().dprintf(fmt, ##__VA_ARGS__)

// With newline
#define ardebugVln(fmt, ...) ardebugV(fmt "\n", ##__VA_ARGS__)
#define ardebugDln(fmt, ...) ardebugD(fmt "\n", ##__VA_ARGS__)
#define ardebugIln(fmt, ...) ardebugI(fmt "\n", ##__VA_ARGS__)
#define ardebugWln(fmt, ...) ardebugW(fmt "\n", ##__VA_ARGS__)
#define ardebugEln(fmt, ...) ardebugE(fmt "\n", ##__VA_ARGS__)

// ESP-IDF like
#define AR_LOGV(fmt, ...) ardebugVln(fmt, ##__VA_ARGS__)
#define AR_LOGD(fmt, ...) ardebugDln(fmt, ##__VA_ARGS__)
#define AR_LOGI(fmt, ...) ardebugIln(fmt, ##__VA_ARGS__)
#define AR_LOGW(fmt, ...) ardebugWln(fmt, ##__VA_ARGS__)
#define AR_LOGE(fmt, ...) ardebugEln(fmt, ##__VA_ARGS__)

// RemoteDebug like
#define debugV(fmt, ...) ardebugVln(fmt, ##__VA_ARGS__)
#define debugD(fmt, ...) ardebugDln(fmt, ##__VA_ARGS__)
#define debugI(fmt, ...) ardebugIln(fmt, ##__VA_ARGS__)
#define debugW(fmt, ...) ardebugWln(fmt, ##__VA_ARGS__)
#define debugE(fmt, ...) ardebugEln(fmt, ##__VA_ARGS__)

#define ardebugBegin(serialptr, hostnameptr, filenameptr) \
    ardebug::DebugContext::get().begin(serialptr, hostnameptr, filenameptr)
#define ardebugHandle() ardebug::DebugContext::get().handle()
#define ardebugGetLevel() ardebug::DebugContext::get().logLevel()
#define ardebugSetLevel(lvl) ardebug::DebugContext::get().setLogLevel(lvl)
#define ardebugTime(bool) ardebug::DebugContext::get().showTime(bool)
#define ardebugLine(bool) ardebug::DebugContext::get().showLine(bool)
#define ardebugFunc(bool) ardebug::DebugContext::get().showFunction(bool)
#define ardebugCore(bool) ardebug::DebugContext::get().showCore(bool)

#else  // ARDEBUG_DISABLED

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

#define ardebugBegin(...)
#define ardebugHandle()
#define ardebugGetLevel() -1
#define ardebugSetLevel(...)
#define ardebugTime(...)
#define ardebugLine(...)
#define ardebugFunc(...)
#define ardebugCore(...)

#endif  // ARDEBUG_DISABLED

#endif  // ARDEBUG_H
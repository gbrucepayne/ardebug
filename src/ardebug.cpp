#include "ardebug.h"

#ifndef ARDEBUG_DISABLED
// #if defined(ARDEBUG_ENABLE)

namespace ardebug {

#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
static WiFiServer server(ARDEBUG_TELNET_PORT, 1);  // @suppress("Abstract class cannot be instantiated")
static WiFiClient client;
#endif

DebugContext::~DebugContext() {
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
  if (client && client.connected()) {
    client.flush();
  }
#endif // BOARD_WIFI
  stop();
}

// iterate looking for [x] pattern
static int8_t isDebug(const char* candidate) {
  if (strlen(candidate) > 0 && candidate[0] == '[') {
    char prev = candidate[0];
    for (size_t i = 1; i < strlen(candidate); i++) {
      char c = candidate[i];
      char next = i < strlen(candidate) - 1 ? candidate[i + 1] : ' ';
      if (prev == '[' && next == ']') {
        switch (c) {
          case 'V': return ARDEBUG_V;
          case 'D': return ARDEBUG_D;
          case 'I': return ARDEBUG_I;
          case 'W': return ARDEBUG_W;
          case 'E': return ARDEBUG_E;
        }
      }
      prev = c;
    }
  }
  return -1;
}

static char* debugColor(uint8_t level) {
  switch (level) {
    case ARDEBUG_V: return (char*)ARD_COLOR_V;
    case ARDEBUG_D: return (char*)ARD_COLOR_D;
    case ARDEBUG_I: return (char*)ARD_COLOR_I;
    case ARDEBUG_W: return (char*)ARD_COLOR_W;
    case ARDEBUG_E: return (char*)ARD_COLOR_E;
    default: return (char*)ARD_COLOR_RESET;
  }
}

static void colorize(char* str, const size_t buffer_size, const char* color) {
  if (strlen(str) <= buffer_size - (2 * ARD_COLORSIZE)) {
    char tmp[buffer_size];
    strncpy(tmp, str, buffer_size);
    snprintf(str, buffer_size, "%s%s%s", color, tmp, ARD_COLOR_RESET);
  }
}

size_t DebugContext::dprintf(const char* fmt, ...) {
  if (!serial_enabled_ && !isConnected() && !file_enabled_) return 0;
  char buffer[ARDEBUG_BUFFER_SIZE];
  char* temp = buffer;
  va_list args;
  va_list copy;
  va_start(args, fmt);
  va_copy(copy, args);
  int len = vsnprintf(temp, ARDEBUG_BUFFER_SIZE, fmt, copy);
  va_end(copy);
  if (len < 0) {
      va_end(args);
      return 0;
  }
#if defined(ARDEBUG_FLEXBUFFER)
  if (len >= ARDEBUG_BUFFER_SIZE) {
      temp = new char[len + 1];
      if (temp == NULL) {
          va_end(args);
          return 0;
      }
      len = vsnprintf(temp, len + 1, fmt, args);
  }
#else
  if (len >= ARDEBUG_BUFFER_SIZE - 1) {
    int offset = strlen(temp) - 1;
    if (fmt[strlen(fmt) - 1] == '\n' || temp[strlen(temp) - 1] == '\n') {
      temp[strlen(temp) - 1] = '\n';
      offset--;
    }
    for (int i = offset; i > offset - 3; i--) temp[i] = '.';
    len = strlen(temp);
  }
#endif
  if (serial_enabled_ && serial_) {
    serial_->write((const char*)temp, len);
  }
  // if (file_enabled_) File.write((const char*)temp, len);
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
  int8_t is_debug = isDebug(temp);
  if (telnet_enabled_ && client) {
    if (show_color_ && is_debug >= ARDEBUG_E) {
      colorize(temp, ARDEBUG_BUFFER_SIZE, debugColor(is_debug));
      len = strlen(temp);
    }
    client.write((const char*)temp, len);
  }
#endif // BOARD_WIFI
  va_end(args);
#if defined(ARDEBUG_FLEXBUFFER)
  if (temp != buffer) delete[] temp;
#endif
  return (size_t)len;
}

size_t DebugContext::debugf(uint8_t level, const char* caller, const char* filename, uint32_t lineno, const char* fmt, ...) {
  if (level > log_level_) return 0;
  bool lf_required = fmt[strlen(fmt) - 1] == '\n';
  DebugMessage msg{level};
  msg.millis = millis();
  strncpy(msg.func, caller, ARDEBUG_FUNCNAME_SIZE);
  strncpy(msg.filename, filename, ARDEBUG_FILENAME_SIZE);
  msg.lineno = lineno;
  const size_t max_prefix_len = ARDEBUG_MAX_PREFIX_SIZE + 1;
  char prefix[max_prefix_len] = {0};
  size_t offset = 0;
  char level_label;
  switch (level) {
      case ARDEBUG_D:
          level_label = 'D';
          break;
      case ARDEBUG_I:
          level_label = 'I';
          break;
      case ARDEBUG_W:
          level_label = 'W';
          break;
      case ARDEBUG_E:
          level_label = 'E';
          break;
      default:
          level_label = 'V';
  }
  if (show_millis_) {
    offset += snprintf(prefix + offset, max_prefix_len, "[%*d]", 6, millis());
  }
  offset += snprintf(prefix + offset, max_prefix_len, "%s[%c]", prefix + offset, level_label);
  if (show_line_) {
    offset += snprintf(prefix + offset, max_prefix_len, "%s[%s:%d]", prefix + offset, filename, lineno);
  }
#ifdef BOARD_MULTI_CORE
  if (show_core_) {
    offset += snprintf(prefix + offset, max_prefix_len, "%s[C%d]", prefix + offset, xPortGetCoreID());
  }
#endif
  if (show_func_ && caller) {
      offset += snprintf(prefix + offset, max_prefix_len, "%s %s()", prefix + offset, caller);
  }
  offset += snprintf(prefix + offset, max_prefix_len, "%s: ", prefix + offset);
  char buffer[ARDEBUG_BUFFER_SIZE];
  // memset(buffer, 0, ARDEBUG_BUFFER_SIZE);
  char* temp = buffer;
  va_list args;
  va_list copy;
  va_start(args, fmt);
  va_copy(copy, args);
  int len = vsnprintf(temp, ARDEBUG_BUFFER_SIZE, fmt, copy);
  va_end(copy);
  if (len < 0) {
      va_end(args);
      return 0;
  }
#if defined(ARDEBUG_FLEXBUFFER)
  if (len >= ARDEBUG_BUFFER_SIZE) {
      temp = new char[len + 1];
      if (temp == NULL) {
          va_end(args);
          return 0;
      }
      len = vsnprintf(temp, len + 1, fmt, args);
  }
  len = (int)dprintf("%s%s", prefix, temp);
  if (temp != buffer) delete[] temp;
#else
  if (lf_required) {
    if (buffer[strlen(buffer) - 1] == '\n')
      buffer[strlen(buffer) - 1] = 0;
    len = (int)dprintf("%s%s\n", prefix, buffer);
  } else {
    len = (int)dprintf("%s%s", prefix, buffer);
  }
#endif  
  va_end(args);
  return (size_t)len;
}

bool DebugContext::begin(Stream* stream, const char* host_name, const char* file_name) {
  if (stream == nullptr && host_name == nullptr && file_name == nullptr)
    return false;
  if (stream != nullptr) {
    serial_enabled_ = true;
    serial_ = stream;
  }
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
  if (host_name && strlen(host_name) > 0) {
    telnet_enabled_ = true;
    strncpy(hostname, host_name, 32);
    if (WiFi.isConnected()) {
      server.begin();
      telnet_listening_ = true;
    }
  }
#else // no wifi = no host
  if (host_name != nullptr) return false;
#endif // BOARD_WIFI
#ifdef BOARD_LOW_MEMORY
    low_memory_ = true;
    show_millis_ = false;
    show_line_ = false;
    show_func_ = false;
    show_core_ = false;
    show_color_ = false;
#endif
  // if (file_name != nullptr) {
  //   file_enabled_ = true;
  // }
  return true;
}

bool DebugContext::setPassword(const char* password) {
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)    
  if (telnet_enabled_ && strlen(password) > 0) {
    strncpy(password_, password, 21);
    return true;
  }
#endif
  return true;
}

void DebugContext::stop() {
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
  if (telnet_enabled_) {
    if (client) client.stop();
    server.stop();
    telnet_listening_ = false;
  }
#endif // BOARD_WIFI
}

void DebugContext::handle() {
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
  if (telnet_enabled_) {
    if (!telnet_listening_) {
      if (WiFi.isConnected()) {
        server.begin();
        telnet_listening_ = true;
      }
    } else {
      if (!client && server.hasClient()) {
        client = server.available();
        client.setNoDelay(true);
        client.flush();
        onConnect();
      } else if (client.connected()) {
        while (client.available() > 0) {
          char c = client.read();
          if (c == '\r' || strlen(telnet_cmd_) == ARDEBUG_CMD_BUFFER - 1) {
            processCommand();
          } else if (isPrintable(c) && c != '\n') {
            telnet_cmd_[strlen(telnet_cmd_)] = c;
          }
        }
      } else {
        client.stop();
      }
    }
  }
#endif //BOARD_WIFI
}

void DebugContext::disconnect() {
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
  if (telnet_enabled_ && client) {
    if (client.connected())
      client.print("Closing client connection.\n");
    client.stop();
  }
#endif // BOARD_WIFI
}

void DebugContext::onConnect() {
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
  if (strlen(password_) > 0) {
    password_ok_ = false;
    password_attempt_ = 1;
  }
  showHelp();
#endif // BOARD_WIFI
}

boolean DebugContext::isConnected() {
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
  return (telnet_enabled_ && client.connected());
#endif // BOARD_WIFI
  return false;
}

void DebugContext::showHelp() {
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)
  bool en = serial_enabled_;
  if (en) serial_enabled_ = false;
  if (strlen(password_) > 0 && !password_ok_) {
    dprintf("Enter password > \r\n");
  } else {
    String help = "";
    help.concat(ARD_COLOR_I);
    help.concat("\r\n**************************************************");
    help.concat("\r\n* Remote debug over telnet - version ");
    help.concat(ARDEBUG_VERSION);
    help.concat("\r\n* Host name: ");
    help.concat(hostname);
    help.concat("\r\n* IP:");
    help.concat(WiFi.localIP().toString());
    help.concat("\r\n* MAC: ");
    help.concat(WiFi.macAddress());
    help.concat("\r\n* Free Heap RAM: ");
    help.concat(getFreeMemory());
#if defined(ESP32) || defined(ESP8266)
    help.concat("\r\n* ESP SDK version: ");
    help.concat(ESP.getSdkVersion());
#endif // ESP specific
    help.concat("\r\n---------------------------------------------------");
    help.concat("\r\n* Commands:");
    help.concat("\r\n*\t m -> display memory available");
    help.concat("\r\n*\t v -> set debug level to verbose");
    help.concat("\r\n*\t d -> set debug level to debug");
    help.concat("\r\n*\t i -> set debug level to info");
    help.concat("\r\n*\t w -> set debug level to warning");
    help.concat("\r\n*\t e -> set debug level to errors");
    help.concat("\r\n*\t l -> show debug level");
    help.concat("\r\n*\t t -> show time (millis)");
    help.concat("\r\n*\t c -> show colors");
    help.concat("\r\n*\t q -> quit (close this connection)");
    help.concat("\r\n*\t ? or help -> display these help of commands");
    help.concat("\r\n*");
    help.concat("\r\n**************************************************\r\n");
    help.concat(ARD_COLOR_RESET);
    dprintf(help.c_str());
    if (en) serial_enabled_ = true;
  }
#endif // BOARD_WIFI
}

void DebugContext::processCommand() {
#if defined(BOARD_WIFI) // && !defined(ARDEBUG_WIFI_DISABLED)  
  if (strlen(password_) > 0 && !password_ok_) {  // Process the password - 18/08/18 - adjust in 04/09/08 and 2018-10-19
    if (strcmp(telnet_cmd_, password_) == 0) {
      dprintf("* Password ok, allowing access now...\n");
      password_ok_ = true;
      showHelp();
    } else {
      dprintf("*** Invalid password ***\n");
      password_attempt_++;
      if (password_attempt_ > ARDEBUG_MAX_PWD_ATTEMPTS) {
        dprintf("*** Too many attempts ***\n");
        disconnect();
      }
    }
  } else {
    if (strcmp(telnet_cmd_, "h") == 0 || strcmp(telnet_cmd_, "?") == 0) {
      showHelp();
    } else if (strcmp(telnet_cmd_, "q") == 0) {
      disconnect();
    } else if (strcmp(telnet_cmd_, "m") == 0) {
      dprintf("Free heap RAM: %d", getFreeMemory());
#if defined(ESP8266)
    } else if (strcmp(telnet_cmd_, "cpu80") == 0) {
      system_update_cpu_freq(80);
      dprintf("CPU ESP8266 changed to: 80 MHz");
    } else if (strcmp(telnet_cmd_, "cpu160") == 0) {
      system_update_cpu_freq(160);
      dprintf("CPU ESP8266 changed to: 160 MHz");
#endif
    } else if (strcmp(telnet_cmd_, "v") == 0) {
      log_level_ = ARDEBUG_V;
      dprintf("Log level set to VERBOSE\n");
    } else if (strcmp(telnet_cmd_, "d") == 0) {
      log_level_ = ARDEBUG_D;
      dprintf("Log level set to DEBUG\n");
    } else if (strcmp(telnet_cmd_, "i") == 0) {
      log_level_ = ARDEBUG_I;
      dprintf("Log level set to INFO\n");
    } else if (strcmp(telnet_cmd_, "w") == 0) {
      log_level_ = ARDEBUG_W;
      dprintf("Log level set to WARNING\n");
    } else if (strcmp(telnet_cmd_, "e") == 0) {
      log_level_ = ARDEBUG_E;
      dprintf("Log level set to ERROR\n");
    } else if (strcmp(telnet_cmd_, "l") == 0) {
      dprintf("Log level: %d\n", log_level_);
    } else if (strcmp(telnet_cmd_, "t") == 0) {
      show_millis_ = !show_millis_;
      dprintf("* Include time: %s\r\n", (show_millis_) ? "On" : "Off");
    } else if (strcmp(telnet_cmd_, "c") == 0) {
      show_color_ = !show_color_;
      dprintf("* Show colors: %s\r\n", (show_color_) ? "On" : "Off");
    } else {
      dprintf("Unknown command: %s\n", telnet_cmd_);
    }
  }
  memset(telnet_cmd_, 0, ARDEBUG_CMD_BUFFER);
#endif // BOARD_WIFI
}

uint32_t DebugContext::getFreeMemory() {
  uint32_t free = 0;
#if defined(ESP32) || defined(ESP8266)
  free = ESP.getFreeHeap();
#endif
  return free;
}

} // namespace ardebug

#endif  // DEBUG_DISABLED
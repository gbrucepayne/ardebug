#ifndef DEBUG_DISABLED
#include "ardebug.h"

namespace ardebug {

#ifdef BOARD_WIFI
static WiFiServer server(ARDEBUG_TELNET_PORT, 1);  // @suppress("Abstract class cannot be instantiated")
static WiFiClient client;
#endif

DebugContext::~DebugContext() {
#ifdef BOARD_WIFI
  if (client && client.connected()) {
    client.flush();
  }
#endif // BOARD_WIFI
  stop();
}

static bool isDebug(const char* candidate) {
  if (strlen(candidate) > 0 &&
      candidate[0] == '(' && candidate[2] == ')')
    return true;
  return false;
}

static void colorize(char* str, const size_t buffer_size) {
  if (strlen(str) <= buffer_size - 16) {
    String temp = "";
    temp.reserve(buffer_size);
    char log_level = str[1];
    switch (log_level) {
      case 'V':
        temp += String(ARD_COLOR_V);
        break;
      case 'D':
        temp += String(ARD_COLOR_D);
        break;
      case 'I':
        temp += String(ARD_COLOR_I);
        break;
      case 'W':
        temp += String(ARD_COLOR_W);
        break;
      case 'E':
        temp += String(ARD_COLOR_E);
        break;
    }
    temp += String(str) + String(ARD_COLOR_RESET);
    strncpy(str, temp.c_str(), buffer_size);
  }
}

size_t DebugContext::dprintf(const char* fmt, ...) {
  if (!serial_enabled_ && !isConnected() && !file_enabled_) return 0;
  const size_t buffer_size = ARDEBUG_BUFFER_SIZE;
  char buffer[buffer_size] = {0};
  char* temp = buffer;
  va_list arg;
  va_list copy;
  va_start(arg, fmt);
  va_copy(copy, arg);
  int len = vsnprintf(temp, buffer_size, fmt, copy);
  va_end(copy);
  if (len < 0) {
      va_end(arg);
      return 0;
  }
  if (len >= buffer_size) {
      temp = (char*)malloc(len + 1);
      if (temp == NULL) {
          va_end(arg);
          return 0;
      }
      len = vsnprintf(temp, len + 1, fmt, arg);
  }
  va_end(arg);
  boolean is_debug = isDebug(temp);
  if (serial_enabled_ && serial_) {
    serial_->write((const char*)temp, len);
  }
#ifdef BOARD_WIFI
  if (telnet_enabled_ && client) {
    if (show_color_ && is_debug) {
      colorize(temp, buffer_size);
      len = strlen(temp);
    }
    client.write((const char*)temp, len);
  }
#endif // BOARD_WIFI
  // if (file_enabled_) File.write((const char*)temp, len);
  if (temp != buffer) free(temp);
  return len;
}

size_t DebugContext::debugf(uint8_t level, const char* caller, const char* fmt, ...) {
    if (level < log_level_) return 0;
    size_t len = 0;
    const size_t max_prefix_len = 32;
    char prefix[max_prefix_len] = {0};
    size_t offset = 0;
    char level_label;
    switch (level) {
        case DEBUG:
            level_label = 'D';
            break;
        case INFO:
            level_label = 'I';
            break;
        case WARNING:
            level_label = 'W';
            break;
        case ERROR:
            level_label = 'E';
            break;
        default:
            level_label = 'V';
    }
    offset += snprintf(prefix, max_prefix_len, "(%c)", level_label);
    if (show_millis_) {
        offset += snprintf(prefix + offset, max_prefix_len, "%s(t:%d)", prefix + offset, millis());
    }
    if (caller != nullptr) {
        offset += snprintf(prefix + offset, max_prefix_len, "%s(%s)", prefix + offset, caller);
    }
#ifdef BOARD_MULTI_CORE
    offset += snprintf(prefix + offset, max_prefix_len, "%s(C%d)", prefix + offset, xPortGetCoreID());
#endif
    offset += snprintf(prefix + offset, max_prefix_len, "%s ", prefix + offset);
    len = dprintf(prefix);
    char* tmp = {0};
    va_list args;
    va_start(args, fmt);
    vasprintf(&tmp, fmt, args);
    len += dprintf("%s", tmp);
    free(tmp);
    va_end(args);
    return len;
}

bool DebugContext::begin(Stream* stream, const char* host_name, const char* file_name) {
  if (stream == nullptr && host_name == nullptr && file_name == nullptr)
    return false;
  if (stream != nullptr) {
    serial_enabled_ = true;
    serial_ = stream;
  }
#ifdef BOARD_WIFI
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
  // if (file_name != nullptr) {
  //   file_enabled_ = true;
  // }
  return true;
}

bool DebugContext::setPassword(const char* password) {
#ifdef BOARD_WIFI    
  if (telnet_enabled_ && strlen(password) > 0) {
    strncpy(password_, password, 21);
    return true;
  }
#endif
  return true;
}

void DebugContext::stop() {
#ifdef BOARD_WIFI
  if (telnet_enabled_) {
    if (client) client.stop();
    server.stop();
    telnet_listening_ = false;
  }
#endif // BOARD_WIFI
}

void DebugContext::handle() {
#ifdef BOARD_WIFI
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
#ifdef BOARD_WIFI
  if (telnet_enabled_ && client) {
    if (client.connected())
      client.print("Closing client connection.\n");
    client.stop();
  }
#endif // BOARD_WIFI
}

void DebugContext::onConnect() {
#ifdef BOARD_WIFI
  if (strlen(password_) > 0) {
    password_ok_ = false;
    password_attempt_ = 1;
  }
  showHelp();
#endif // BOARD_WIFI
}

boolean DebugContext::isConnected() {
#ifdef BOARD_WIFI
  return (telnet_enabled_ && client.connected());
#endif // BOARD_WIFI
  return false;
}

void DebugContext::showHelp() {
#ifdef BOARD_WIFI
  bool en = serial_enabled_;
  if (en) serial_enabled_ = false;
  if (strlen(password_) > 0 && !password_ok_) {
    dprintf("Enter password > \r\n");
  } else {
    String help = "";
    help.concat("*** Remote debug - over telnet - version ");
    help.concat(_ARDEBUG_VERSION_);
    help.concat("\r\n");
    help.concat("* Host name: ");
    help.concat(String(hostname));
    help.concat(" IP:");
    help.concat(WiFi.localIP().toString());
    help.concat(" Mac address:");
    help.concat(WiFi.macAddress());
    help.concat("\r\n");
    help.concat("* Free Heap RAM: ");
    help.concat(getFreeMemory());
    help.concat("\r\n");
#if defined(ESP32) || defined(ESP8266)
    help.concat("* ESP SDK version: ");
    help.concat(ESP.getSdkVersion());
    help.concat("\r\n");
#endif // ESP specific
    help.concat("******************************************************\r\n");
    help.concat("* Commands:\r\n");
    help.concat("    ? or help -> display these help of commands\r\n");
    help.concat("    q -> quit (close this connection)\r\n");
    help.concat("    m -> display memory available\r\n");
    help.concat("    v -> set debug level to verbose\r\n");
    help.concat("    d -> set debug level to debug\r\n");
    help.concat("    i -> set debug level to info\r\n");
    help.concat("    w -> set debug level to warning\r\n");
    help.concat("    e -> set debug level to errors\r\n");
    help.concat("    l -> show debug level\r\n");
    help.concat("    t -> show time (millis)\r\n");
    help.concat("    c -> show colors\r\n");
    help.concat("\r\n");
    help.concat("* Please type the command and press enter to execute.(? or h for this help)\r\n");
    help.concat("***\r\n");
    dprintf(help.c_str());
    if (en) serial_enabled_ = true;
  }
#endif // BOARD_WIFI
}

void DebugContext::processCommand() {
#ifdef BOARD_WIFI  
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
      log_level_ = VERBOSE;
      dprintf("Log level set to VERBOSE\n");
    } else if (strcmp(telnet_cmd_, "d") == 0) {
      log_level_ = DEBUG;
      dprintf("Log level set to DEBUG\n");
    } else if (strcmp(telnet_cmd_, "i") == 0) {
      log_level_ = INFO;
      dprintf("Log level set to INFO\n");
    } else if (strcmp(telnet_cmd_, "w") == 0) {
      log_level_ = WARNING;
      dprintf("Log level set to WARNING\n");
    } else if (strcmp(telnet_cmd_, "e") == 0) {
      log_level_ = ERROR;
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
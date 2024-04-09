# `ardebug` Adruino Debug Logging Library

A simplified library for real-time debugging over Serial/USB, WiFi/Telnet.
Future plans to enable file storage.

Combines concepts of [RemoteDebug](https://github.com/JoaoLopesF/RemoteDebug)
by Joao Lopes and Karol Brejna, [SerialDebug](https://github.com/JoaoLopesF/SerialDebug/tree/master)
by Joao Lopes, and [DebugLog](https://github.com/hideakitai/DebugLog) by Hideaki Tai

Allows:
* When not explicitly enabled, avoids compiling unused code
* Use of Serial (USB) when physically connected to a device
* A TCP/IP Telnet server to connect to via your local WiFi network
* (Future) file output when appropriate storage/peripherals are available when not connected
* `printf`-style output for efficient single-line commands

## Background

* Removes the websocket feature/dependency from **`RemoteDebug`**
* Removes custom commands and advanced profiler interaction to simplify code base
* Simplifies field entry and allows arbitrary delimiters compared to **`DebugLog`**
* Intended to support low-memory boards such as AVR/ATMega
* Intended to support other WiFi or non-WiFi capable boards like Pi Pico, STM32
* Intended compatibilty with ESP logging facility

## Usage

* Include:
    ```cpp
    #define ARDEBUG_ENABLED
    #include "ardebug.h"
    ```
* Create an instance (singleton) of the DebugContext class
(or it will be instantiated with the first macro call)
* To use WiFi/Telnet you must:
    * Setup a WiFi Station client similar to the example
    * Call the handle function in your loop

## Limitations

The following limitations are inherited from the original library:
* doesn't support SSL (technical limitation of the underlying library)

The library has no tests, nor CI/CD.

## Acknowledgements

Many thanks to the ideas and original code done by Joao Lopes and Hidekai Tai,
and the updates from Karol Brejna.

Substantial changes were made from the orignal code but some vestiges may remain.
# `ardebug` Adruino Debug Logging Library

A simplified library for real-time debugging over Serial/USB, WiFi/Telnet.
Future plans to enable file storage.

Produces output like:
```
[   123][I][main.cpp:56][C1] loop(): This is a message of debug level INFO (2)
```

Combines concepts of [RemoteDebug](https://github.com/JoaoLopesF/RemoteDebug)
by Joao Lopes and Karol Brejna, [SerialDebug](https://github.com/JoaoLopesF/SerialDebug/tree/master)
by Joao Lopes, and [DebugLog](https://github.com/hideakitai/DebugLog) by Hideaki Tai

Allows:
* Macro-driven functions
* When not using, `#define ARDEBUG_DISABLED` avoids compiling unused code
* Use of Serial (USB) when physically connected to a device
* A TCP/IP Telnet server to connect to via your local WiFi network
* `printf`-style single-line commands
* `esp_log`-style output
* ***TODO*** file output when appropriate storage/peripherals are available when not connected

## Background / Features

While some great libraries exist for doing most of what I wanted, none of them
quite fit my needs. So I borrowed concepts from a few different places.

* Removes the websocket feature/dependency from **`RemoteDebug`**
* Removes custom commands and advanced profiler interaction to simplify code base
* Simplifies field entry and allows arbitrary delimiters compared to **`DebugLog`**
* ***TODO*** File output
* ***TODO*** Intended to support low-memory boards such as AVR/ATMega
* ***TODO*** Intended to support other WiFi or non-WiFi capable boards like Pi Pico, STM32
* ***TODO*** compatibilty with ESP `TAG` call structure

## Usage

Defines the following log levels:
* `ARDEBUG_V` 4
* `ARDEBUG_D` 3
* `ARDEBUG_I` 2
* `ARDEBUG_W` 1
* `ARDEBUG_E` 0

* Include:
    ```cpp
    #define ARDEBUG_WIFI   // optional
    #include "ardebug.h"
    ```
* To use WiFi/Telnet you must:
    * Setup a WiFi Station client similar to the example
    * Call the handle function in your loop
* Call `ardebugBegin()` with a combination of `&Serial` or `"<hostname>"` in
your `setup()`
* Use `ardebugGetLevel()` or `ardebugSetLevel(<n>)`
    > [!NOTE]
    > `ardebugGetLevel()` returns -1 if `ARDEBUG_DISABLED` is defined
* Generate logs in your code using `AR_LOG<x>(<fmt>, ...)`
where `<x>` is the level tag:
    * **E**rror (0)
    * **W**arning (1)
    * **I**nfo (2)
    * **D**ebug (3)
    * **V**erbose (4)
* To connect remotely to a WiFi-enabled microcontroller, use a telnet program
such as terminal `telnet` for Linux, or PuTTY for Windows.

## Limitations

* Some `%` character sequences in variadic char* arguments may produce strange
formatting (e.g. `%U`, `%G`)
* Doesn't support SSL (technical limitation of the underlying library)

> [!IMPORTANT]
> The library has no tests, nor CI/CD.

## Acknowledgements

Many thanks to the ideas and original code done by Joao Lopes and Hidekai Tai,
and the updates from Karol Brejna.

Substantial changes were made from the orignal code but some vestiges may remain.
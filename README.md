# esp32-base

Evolution of the [ESP8266 base](https://github.com/funnybrum/esp8266-base).

A common set of functionality needed to bring up a project on an ESP32 that exposes:
 * Web server
 * OTA update API
 * WiFi management
 * InfluxDB integration
 * Logger with in-memory buffer that exposes the collected logs via the web server
 * Settings management with UI exposed via the web server

## Logger

The Logger class is a logger that uses in-memory buffer for storing the data. This is useful for debugging projects that doesn't have serial connectivity. Usually exposed on http://{hostname}/logs

## Settings

Common class used to save and load settings from the EEPROM. Pass in the structure of the settings and the provided implementation will take care for saving and loading. The settings are guarded by checksum and are loaded only if it is correct.

Settings UI is usually exposed on http://{hostname}/settings

## WebServerBase

Web server base with build-in OTA update mechanism. Check the (ESP32Blank)[https://github.com/funnybrum/ESP32Blank] project for an example.

Below is an example for doing an OTA update with curl:
```
curl -F "image=@.pio/build/esp32/firmware.bin" http://esp32-blank/update
```


## WiFiManager

The WiFi Manager will take care for the WiFi connectivity. Integrated with the other tools. Settings are stored in the EEPROM and details like SSID and password can be kept between restarts. If the configured WiFi SSID/password are invalid - the microcontroller will switch to AP mode and expose an open network with the controller hostname used as SSID. The user can connect to it and open 192.168.0.1 to configure the correct SSID/password.

## InfluxDBCollector

A tool to automate the data publishing to InfluxDB. Requires DB that is not password protected. Data is collected in in-memory buffer and pushed once the buffer is full or the time for a push has come.

Several parameters can be configured, but the main one are - push interval, collect interval and InfluxDB address. If all of them are valid - the microcontroller will keep the WiFi off while data is being collected on regular intervals. Once the time for push has come - WiFi will be turned on, data will be pushed to the InfluxDB and the WiFi will be turned off again.

TODO: Check the statement about the WiFi being shutdown when data is pushed. IIRC this has been disabled.

# Usage

Check the (ESP32Blank)[https://github.com/funnybrum/ESP32Blank] project for an example.
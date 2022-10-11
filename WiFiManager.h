#pragma once

#include <WiFi.h>


struct NetworkSettings {
    char hostname[64];
    char ssid[32];
    char password[32];
};

#include "Logger.h"
#include "HttpServerBase.h"

const char NETWORK_CONFIG_PAGE[] PROGMEM = R"=====(
<fieldset style='display: inline-block; width: 300px'>
<legend>Network settings</legend>
Hostname:<br>
<input type="text" name="hostname" value="%s"><br>
<small><em>from 4 to 63 characters lenght, can contain chars, digits and '-'</em></small><br><br>
SSID:<br>
<input type="text" name="ssid" value="%s"><br>
<small><em>WiFi network to connect to</em></small><br><br>
Password:<br>
<input type="password" name="password"><br>
<small><em>WiFi network password</em></small><br>
</fieldset>
)=====";

enum _WiFiState {
    CONNECTING,
    CONNECTED,
    DISCONNECTED,
    AP
};

#define WIFI_CONNECT_TIMEOUT 10000  // 10 seconds

class WiFiManager {
    public:
        WiFiManager(Logger* logger, NetworkSettings* settings) {
            _logger = logger;
            _settings = settings;
        }

        void begin() {
            // https://github.com/espressif/arduino-esp32/issues/6700
            // Resolution - before doing any call to WiFi set the hostname.
            WiFi.hostname(_settings->hostname);
            WiFi.persistent(false);
            WiFi.mode(WIFI_STA);
            _setState(DISCONNECTED);
        }

        void loop() {
            switch(_state) {
                case CONNECTED:
                    // Do nothing.
                    break;
                case CONNECTING:
                    if (WiFi.status() == WL_CONNECTED) {
                        if (_logger != NULL) {
                            _logger->log("Connected in %.1f seconds, IP address is %s",
                                        (millis() - _lastStateSetAt)/1000.0f,
                                        WiFi.localIP().toString().c_str());
                        }

                        _setState(CONNECTED);
                    } else if (millis() - _lastStateSetAt > WIFI_CONNECT_TIMEOUT || strlen(_settings->ssid)==0) {
                        if (_logger != NULL) {
                            _logger->log("Connection failed, going in AP mode");
                        }

                        // For setup and debug purposes.
                        WiFi.disconnect();
                        WiFi.softAPConfig(
                            IPAddress(192, 168, 0, 1),
                            IPAddress(192, 168, 0, 1),
                            IPAddress(255, 255, 255, 0)); 
                        WiFi.softAP(_settings->hostname);
                        _setState(AP);
                    }
                    break;
                case DISCONNECTED:
                    // Do nothing.
                    break;
                case AP:
                    // If there is network configured - don't stay in AP mode for more than 5
                    // minutes. Try to reconnect to the configured network.
                    if (strlen(_settings->ssid) > 1 &&
                        millis() - _lastStateSetAt > 5 * 60 * 1000) {
                        disconnect();
                        delay(1000);
                        connect();
                    }
            }
        }

        void connect() {
            if (_state != DISCONNECTED) {
                return;
            }
            WiFi.mode(WIFI_STA);
            _connect();
        }

        void disconnect() {
            if (_state == DISCONNECTED) {
                return;
            }
            WiFi.mode(WIFI_OFF);
            _setState(DISCONNECTED);
        }

        bool isConnected() {
            return WiFi.status() == WL_CONNECTED && _state == CONNECTED;
        }

        bool isInAPMode() {
            return _state == AP;
        }

        void get_config_page(char* buffer) {
            sprintf_P(
                buffer,
                NETWORK_CONFIG_PAGE,
                _settings->hostname,
                _settings->ssid);
        }

        void parse_config_params(HttpServerBase* webServer) {
            webServer->process_setting("hostname", _settings->hostname, sizeof(_settings->hostname));
            webServer->process_setting("ssid", _settings->ssid, sizeof(_settings->ssid));
            webServer->process_setting("password", _settings->password, sizeof(_settings->password));
        }

    private:
        void _connect() {
            if (_logger != NULL) {
                _logger->log("Hostname is %s", _settings->hostname);
            }
            // https://github.com/espressif/arduino-esp32/issues/6700
            // To change the host name a restart is likely needed.
            WiFi.hostname(_settings->hostname);

            _setState(CONNECTING);


            if (strlen(_settings->ssid)==0) {
                // Skip connecting attempts and directly go to AP mode for enabling configuration
                // through the web UI.
                return;
            }

            WiFi.begin(_settings->ssid, _settings->password);
        }

        void _setState(_WiFiState state) {
            _state = state;
            _lastStateSetAt = millis();
        }

        _WiFiState _state;
        unsigned long _lastStateSetAt;

        Logger* _logger = NULL;
        NetworkSettings* _settings = NULL;
};

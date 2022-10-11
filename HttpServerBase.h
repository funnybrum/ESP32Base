#pragma once

#include <WebServer.h>
#include <HTTPUpdateServer.h>
#include <ESPmDNS.h>

#include "Logger.h"
#include "WiFi.h"

class HttpServerBase {
    public:
        HttpServerBase(NetworkSettings* networkSettings, Logger* logger) {
            this->logger = logger;
            this->networkSettings = networkSettings;

        };
 
        void begin() {
            if (server != NULL) {
                // Already invoked.
                return;
            }

            server = new WebServer(80);
            server->on("/restart", std::bind(&HttpServerBase::handle_restart, this));
            server->on("/logs", std::bind(&HttpServerBase::handle_logs, this));
            registerHandlers();

            httpUpdater = new HTTPUpdateServer(true);
            httpUpdater->setup(server);

            server->begin();
        }

        void loop() {
            server->handleClient();
        }

        virtual void registerHandlers() = 0;

        void process_setting(const char* name, char* destination, uint8_t max_size) {
            if (server->hasArg(name)) {
                String new_value = server->arg(name);
                if (new_value.length() > 2 && new_value.length()+1 < max_size) {
                    strcpy(destination, new_value.c_str());
                }
            }
        }

        void process_setting(const char* name, int16_t& destination) {
            if (server->hasArg(name)) {
                destination = server->arg(name).toInt();
            }
        }

        void process_setting(const char* name, uint16_t& destination) {
            if (server->hasArg(name)) {
                destination = server->arg(name).toInt();
            }
        }

        void process_setting(const char* name, int8_t& destination) {
            if (server->hasArg(name)) {
                destination = server->arg(name).toInt();
            }
        }

        void process_setting(const char* name, uint8_t& destination) {
            if (server->hasArg(name)) {
                destination = server->arg(name).toInt();
            }
        }

        void process_setting(const char* name, float& destination) {
            if (server->hasArg(name)) {
                destination = atof(server->arg(name).c_str());
            }
        }

        void process_setting(const char* name, bool& destination) {
            if (server->hasArg(name)) {
                String val = server->arg(name);
                if (val.compareTo("true") == 0) {
                    destination = true;
                } else if (val.compareTo("false") == 0) {
                    destination = false;
                }
            }
        }

    protected:
        Logger* logger = NULL;
        WebServer *server = NULL;

    private:
        HTTPUpdateServer *httpUpdater;
        NetworkSettings* networkSettings = NULL;

        void handle_restart() {
            server->send(200, "text/plain", "Restarting...");
            delay(1000);
            ESP.restart();
        }

        void handle_logs() {
            server->send(200, "text/plain", logger->getLogs());
        }
};

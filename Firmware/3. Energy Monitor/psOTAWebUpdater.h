#ifndef _PSOTAWebUpdater_h
#define _PSOTAWebUpdater_h

#include <Arduino.h>
#include "psGlobalConfig.h"
#include "psUtils.h"

#include <WiFi.h>
#include <Update.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>

#include "FS.h"
#include "stdlib_noniso.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"

#include "elegantWebpage.h"

bool UpdateInProgress = false;

class OTAWebUpdaterClass {

public:
	void begin(AsyncWebServer* server, const char* username = "", const char* password = "") {
		_server = server;

		if (strlen(username) > 0) {
			_authRequired = true;
			_username = username;
			_password = password;
		}
		else {
			_authRequired = false;
			_username = "";
			_password = "";
		}

		_server->on("/update", HTTP_GET, [&](AsyncWebServerRequest* request) {
			if (_authRequired) {
				if (!request->authenticate(_username.c_str(), _password.c_str())) {
					return request->requestAuthentication();
				}
			}
			//AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", ELEGANT_HTML, ELEGANT_HTML_SIZE);
			//response->addHeader("Content-Encoding", "gzip");
			//request->send(response);
			request->send(SPIFFS, "/update.html");
			});

		_server->on("/update", HTTP_POST, [&](AsyncWebServerRequest* request) {
			if (_authRequired) {
				if (!request->authenticate(_username.c_str(), _password.c_str())) {
					return request->requestAuthentication();
				}
			}
			// the request handler is triggered after the upload has finished... 
			// create the response, add header, and send response
			AsyncWebServerResponse* response = request->beginResponse((Update.hasError()) ? 500 : 200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
			response->addHeader("Connection", "close");
			response->addHeader("Access-Control-Allow-Origin", "*");
			request->send(response);
			restart();
			}, [&](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
				//Upload handler chunks in data
				if (_authRequired) {
					if (!request->authenticate(_username.c_str(), _password.c_str())) {
						return request->requestAuthentication();
					}
				}

				if (!index) {
					if (!request->hasParam("MD5", true)) {
						_errMsg = String("Update failed. MD5 parameter is missing.");
						Log(_errMsg);
						return request->send(400, "text/plain", _errMsg);
					}

					if (!Update.setMD5(request->getParam("MD5", true)->value().c_str())) {
						_errMsg = String("Update failed. MD5 parameter is invalid.");
						Log(_errMsg);
						return request->send(400, "text/plain", _errMsg);
					}
					int cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
					if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
						_errMsg = String("Update failed. ") + Update.errorString();
						Log(_errMsg);
						return request->send(400, "text/plain", _errMsg);
					}
					else {
						UpdateInProgress = true;
						_errMsg = String("Firmware update has been started: ") + filename;
						Log(_errMsg);
					}
				}

				// Write chunked data to the free sketch space
				if (len) {
					if (Update.write(data, len) != len) {
						_errMsg = String("Update failed. Data write error.");
						Log(_errMsg);
						return request->send(400, "text/plain", _errMsg);
					}
				}

				if (final) { // if the final flag is set then this is the last frame of data
					if (!Update.end(true)) { //true to set the size to the current progress
						_errMsg = String("Update failed. ") + Update.errorString();
						Log(_errMsg);
						return request->send(400, "text/plain", _errMsg);
					}
					else {
						Log("Firmware update completed successfully.");
					}
				}
				else {
					return;
				}
			});
	}

	// deprecated, keeping for backward compatibility
	void loop() {
	}

	void restart() {
		Log("Device is being restarted.");
		SaveConfiguration(SPIFFS);
		yield();
		delay(1000);
		yield();
		ESP.restart();
	}

private:
	AsyncWebServer* _server;
	String _username = "";
	String _password = "";
	String _errMsg = "";
	bool _authRequired = false;

};

OTAWebUpdaterClass OTAWebUpdater;
#endif  // _PSOTAWebUpdater_h

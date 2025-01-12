#ifndef _PSWEBSERVER_H
#define _PSWEBSERVER_H

#include <Arduino.h>
#include "psGlobalConfig.h"
#include "psUtils.h"

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include "psSensors.h"


// Create AsyncWebServer object on port 80
AsyncWebServer HTTPServer(http_port);


void setup_WebServer() {
	// Initialize SPIFFS
	if (!SPIFFSInitialized) return;

	// Route for root / web page
	HTTPServer.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/status.html");
		});

	// Route for configuration page
	HTTPServer.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/config.html");
		});
	HTTPServer.on("/config", HTTP_POST, [](AsyncWebServerRequest* request) {
		String jsonData;  bool bRes = false;
		if (request->hasParam("new_data_config", true)) { jsonData = request->getParam("new_data_config", true)->value(); }
		if (UpdateConfiguration(jsonData)) {
			bRes = true;
			Log("Device is being restarted.");
			SaveConfiguration(SPIFFS);
			yield();
			delay(2000);
			yield();
			ESP.restart();
		}

		AsyncWebServerResponse* response = request->beginResponse((bRes) ? 200 : 500, "text/plain", (bRes) ? "The configuration has been successfully updated." : "Updating configuration failed!");
		response->addHeader("Connection", "close");
		response->addHeader("Access-Control-Allow-Origin", "*");
		request->send(response);
		});

	HTTPServer.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/data.html");
		});
	HTTPServer.on("/status", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/status.html");
		});
	HTTPServer.on("/log", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/log.html");
		});
	HTTPServer.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/update.html");
		});

	// Route for various data and data log information
	HTTPServer.on("/device/last10minutes", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send_P(200, "text/plain; charset=utf-8", getCSVData(last10Min).c_str());
		});
	HTTPServer.on("/device/last2hours", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send_P(200, "text/plain; charset=utf-8", getCSVData(last2Hours).c_str());
		});
	HTTPServer.on("/device/last6hours", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send_P(200, "text/plain; charset=utf-8", getCSVData(last6Hours).c_str());
		});
	HTTPServer.on("/device/last24hours", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send_P(200, "text/plain; charset=utf-8", getCSVData(last24Hours).c_str());
		});
	HTTPServer.on("/device/infolog", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send_P(200, "text/plain; charset=utf-8", getLogData().c_str());
		});
	HTTPServer.on("/device/info", HTTP_GET, [&](AsyncWebServerRequest* request) {
		request->send(200, "application/json", getDeviceInfo().c_str());
		});
	HTTPServer.on("/device/infoex", HTTP_GET, [&](AsyncWebServerRequest* request) {
		request->send(200, "application/json", getDeviceInfoEx().c_str());
		});
	HTTPServer.on("/device/configfile", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, configFilePath);
		});


	HTTPServer.on("/jsondata", HTTP_GET, [](AsyncWebServerRequest* request) {
		//  if(_authRequired){
		//    if(!request->authenticate(_username.c_str(), _password.c_str())){
		//    return request->requestAuthentication();
		//    }
		//  }
		request->send_P(200, "text/plain; charset=utf-8", sJsonData.c_str());
		});
}


#endif // _PSWEBSERVER_H

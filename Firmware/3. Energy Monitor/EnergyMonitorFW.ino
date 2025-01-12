
#include "time.h"
#include "psGlobalConfig.h"
#include "psSensors.h"
#include "psWiFi.h"
#include "psMQTT.h"
#include "psWebServer.h"
#include "psOTAWebUpdater.h"
#include "psUtils.h"


void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);

  delay(3000);

	pinMode(Yellow_LED_Pin, OUTPUT);
	pinMode(Red_LED_Pin, OUTPUT);
	pinMode(Green_LED_Pin, OUTPUT);
	pinMode(Blue_LED_Pin, OUTPUT);

	digitalWrite(Red_LED_Pin, LOW);
	digitalWrite(Blue_LED_Pin, LOW);
	digitalWrite(Green_LED_Pin, LOW);
	digitalWrite(Yellow_LED_Pin, HIGH);  // Yellow lighst while booting

  Serial.println();
  Serial.println();
	String sBuild = build;
	sBuild = " Build version: " + sBuild;
	Log(sBuild);  
	Log("===============================================");

	InitializeSPIFFS();
	LoadConfiguration(SPIFFS);
	GetESPDeviceInfo();
  setup_WiFi();
	setup_Sensors();
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	Log("-----------------------------------------------");
  Log("Last boot: " + GetLocalTime());

	setup_mqtt();
	setup_WebServer();

	OTAWebUpdater.begin(&HTTPServer);  // First start the OTAWebUpdater
	HTTPServer.begin();                // Then start the Web Server
	Log("HTTP server started");

	digitalWrite(Yellow_LED_Pin, LOW);   // Green lighst while booting
}


void loop() {
	delay(2);//allow the cpu to switch to other tasks

	if (!UpdateInProgress) {
		loop_WiFi_CheckStatus();
		loop_mqtt();
		loop_Sensors_GetData();
	}
}

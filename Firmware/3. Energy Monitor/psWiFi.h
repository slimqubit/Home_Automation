#ifndef _PSWIFI_H
#define _PSWIFI_H

#include <Arduino.h>
#include "psGlobalConfig.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include "psUtils.h"

// extern void setup_WiFi();
// extern void loop_WiFi_CheckStatus();

const byte WiFi_led_flash_no = 3;
const int  WiFi_OK_led_on_delay = 100;
const int  WiFi_NOK_led_on_delay = 1000;
const int  WiFi_Check_delay = 10000;
unsigned long WiFi_Check_millis = millis();

byte WiFi_led_state = LOW;
byte WiFi_led_state_change_count = 0;
bool WiFi_connection_Ok = false;
bool WiFi_connection_Lost = false;


IPAddress IPAddr, IPMask, IPGetw, IPDns1, IPDns2;

String WiFiStrength(int nlev) {
	String slev = "None";
	if (nlev < -70) { slev = "Weak"; }
	else if (nlev < -60) { slev = "Fair"; }
	else if (nlev <= -50) { slev = "Good"; }
	else { slev = "Excellent"; }
	return slev;
}


void ScanWiFiNetworks() {
	char buff[128];
	String ScanResult;
	int n = WiFi.scanNetworks();
	if (n == 0) {
		ScanResult = "No WiFi networks were found.";
	}
	else {
		ScanResult = "Following WiFi networks are available:";
		for (int i = 0; i < n; ++i) {
			char buff[128];
			sprintf(buff, "\n    %s (%ddB %s) Enc: %s", WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFiStrength(WiFi.RSSI(i)).c_str(), (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
			ScanResult = ScanResult + String(buff);
		}
	}
	Log(ScanResult);
}


void setup_WiFi() {
  ScanWiFiNetworks();
  Serial.println();
	Serial.print("Connecting to ");
	Serial.print(config.ssid);

	// Connect to WiFi network
	// WiFi.mode(WIFI_AP_STA);
	IPAddr.fromString(config.ip_addr);
	IPMask.fromString(config.ip_mask);
	IPGetw.fromString(config.ip_getw);
	IPDns1.fromString(config.ip_dns1);
	IPDns2.fromString(config.ip_dns2);

  /*
  Serial.println("Some Logs: ");
  Serial.println(config.ip_addr);
  Serial.println(config.ip_mask);
  Serial.println(config.ip_getw);
  Serial.println(config.ip_addr);
  Serial.println(config.ip_dns1);
  Serial.println(config.ip_dns2);
  Serial.println(config.ssid);
  Serial.println(config.ssidpwd);
  */

	if (!WiFi.config(IPAddr, IPGetw, IPMask, IPDns1, IPDns2)) {
		Log("STA Failed to configure");
	}
	WiFi.begin(config.ssid, config.ssidpwd);

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	logWiFiStrength = WiFiStrength(WiFi.RSSI()).c_str();
  Serial.println();
	char buff[256];
	sprintf(buff, "Connected to \"%s\" (signal: %s) IP Address: %s", config.ssid, logWiFiStrength, WiFi.localIP().toString());
	Log(String(buff));
}

void loop_WiFi_CheckStatus() {
	if (millis() > WiFi_Check_millis)
	{
		if (WiFi_led_state_change_count == 0)          // Check WiFi status
		{
			// Serial.println("Now we check WiFi status... ");
			WiFi_connection_Ok = WiFi.status() == WL_CONNECTED;

			if (!WiFi_connection_Ok)
			{
        if (!WiFi_connection_Lost)
        {
				  Log("WiFi connection was lost; we'll try to reconnect... ");
          WiFi_connection_Lost = true;
        }
				WiFi.reconnect();
			}
			else
			{
        if (WiFi_connection_Lost)
        {
          Log("WiFi connection was restored.");
          WiFi_connection_Lost = false;
        }
//				Serial.print("Available memory: ");
//				Serial.println(ESP.getFreeHeap());
				logWiFiStrength = WiFiStrength(WiFi.RSSI()).c_str();
			}
			WiFi_led_state_change_count++;
			WiFi_led_state = !WiFi_led_state;
			digitalWrite(Blue_LED_Pin, WiFi_led_state);
			if (WiFi_connection_Ok)
				WiFi_Check_millis = millis() + WiFi_OK_led_on_delay;
			else
				WiFi_Check_millis = millis() + WiFi_NOK_led_on_delay;
		}
		else
		{
			WiFi_led_state_change_count++;
			WiFi_led_state = !WiFi_led_state;
			digitalWrite(Blue_LED_Pin, WiFi_led_state);
			if (WiFi_led_state_change_count == 2 * WiFi_led_flash_no)
			{
				WiFi_led_state_change_count = 0;
				WiFi_Check_millis = millis() + WiFi_Check_delay;
			}
			else
			{
				if (WiFi_connection_Ok)
					WiFi_Check_millis = millis() + WiFi_OK_led_on_delay;
				else
					WiFi_Check_millis = millis() + WiFi_NOK_led_on_delay;
			}
		}
	}
}


#endif // _PSWIFI_H

#ifndef _PSUTILS_H
#define _PSUTILS_H

#include <Arduino.h>
#include "psGlobalConfig.h"

#include <vector>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "FS.h"

struct SensorGroup {
  bool enabled;
  double ical;
  char lbct1[16];
  char lbct2[16];
  char lbct3[16];
};

struct ESPConfig {
	char hostname[32];
	char ssid[32];
	char ssidpwd[32];

	char ip_addr[16];
	char ip_mask[16];
	char ip_getw[16];
	char ip_dns1[16];
	char ip_dns2[16];

	char mqtt_srv[16];
	uint16_t mqtt_port;
	int  http_port;

	SensorGroup sg1, sg2, sg3, sg4, sg5, sg6;
};

struct ESPDevice {
	char chipModel[32];
	char chipId[8];
	char mqttCleintId[32];
};

const char* configFilePath = "/config.json";   // <- SD library uses 8.3 filenames
ESPConfig config;                              // <- global configuration object
ESPDevice device;                              // <- global configuration object

std::vector<String> v_lastLog;

bool SPIFFSInitialized = false;
String logWiFiStrength;

void Log(String logMessage) {
	Serial.println(logMessage);

	// Save the log to log vector
	//char buff[256];
	//sprintf(buff, "%s", String(logMessage));
	if (v_lastLog.size() >= 120) v_lastLog.erase(v_lastLog.begin());
	v_lastLog.push_back(logMessage);
}



void InitializeSPIFFS() {
  char buff[128];
	SPIFFSInitialized = SPIFFS.begin();
	if (!SPIFFSInitialized) 
	  Log("An Error has occurred while mounting SPIFFS");
  else
    Log("SPIFFS Initialized successfully.");
      
  int tBytes = SPIFFS.totalBytes()/1024;
  int uBytes = SPIFFS.usedBytes()/1204;
  sprintf(buff, "Storage status: %d Kb used, from a total of %d Kb capacity.", uBytes, tBytes);
  Log(String(buff));  
}

String getLogData() {
	String S = "       Last log info\n------------------------------------------------";
	for (String s : v_lastLog) { S = S + "\n" + s; }
	return S;
}


void GetESPDeviceInfo() {
	uint32_t id = 0;
	String cId = "";
	for (int i = 0; i < 17; i = i + 8) {
		id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
	cId = String(id, HEX);
	cId.toUpperCase();

	sprintf(device.chipModel, "%s rev. %d", ESP.getChipModel(), ESP.getChipRevision());
	strlcpy(device.chipId, cId.c_str(), sizeof(device.chipId));
	sprintf(device.mqttCleintId, "%s-%s", config.hostname, cId);
}


void SaveConfiguration(fs::FS& fs) {
	if (!SPIFFSInitialized) return;
	File file = fs.open(configFilePath, FILE_WRITE);
	if (!file) {
		Log("Failed to open file for writing");
		return;
	}

	StaticJsonDocument<2048> doc;
	doc["hostname"] = config.hostname;
	doc["ssid"] = config.ssid;
	doc["ssidpwd"] = config.ssidpwd;
	doc["ip_addr"] = config.ip_addr;
	doc["ip_mask"] = config.ip_mask;
	doc["ip_getw"] = config.ip_getw;
	doc["ip_dns1"] = config.ip_dns1;
	doc["ip_dns2"] = config.ip_dns2;

	doc["mqtt_srv"] = config.mqtt_srv;
	doc["mqtt_port"] = config.mqtt_port;
	doc["http_port"] = config.http_port;

  doc["sgroup1"]["enabled"] = config.sg1.enabled;
  doc["sgroup2"]["enabled"] = config.sg2.enabled;
  doc["sgroup3"]["enabled"] = config.sg3.enabled;
  doc["sgroup4"]["enabled"] = config.sg4.enabled;
  doc["sgroup5"]["enabled"] = config.sg5.enabled;
  doc["sgroup6"]["enabled"] = config.sg6.enabled;

  doc["sgroup1"]["icalib"] = config.sg1.ical;
  doc["sgroup2"]["icalib"] = config.sg2.ical;
  doc["sgroup3"]["icalib"] = config.sg3.ical;
  doc["sgroup4"]["icalib"] = config.sg4.ical;
  doc["sgroup5"]["icalib"] = config.sg5.ical;
  doc["sgroup6"]["icalib"] = config.sg6.ical;

  doc["sgroup1"]["lbct1"] = config.sg1.lbct1;
  doc["sgroup1"]["lbct2"] = config.sg1.lbct2;
  doc["sgroup1"]["lbct3"] = config.sg1.lbct3;
  doc["sgroup2"]["lbct1"] = config.sg2.lbct1;
  doc["sgroup2"]["lbct2"] = config.sg2.lbct2;
  doc["sgroup2"]["lbct3"] = config.sg2.lbct3;
  doc["sgroup3"]["lbct1"] = config.sg3.lbct1;
  doc["sgroup3"]["lbct2"] = config.sg3.lbct2;
  doc["sgroup3"]["lbct3"] = config.sg3.lbct3;
  doc["sgroup4"]["lbct1"] = config.sg4.lbct1;
  doc["sgroup4"]["lbct2"] = config.sg4.lbct2;
  doc["sgroup4"]["lbct3"] = config.sg4.lbct3;
  doc["sgroup5"]["lbct1"] = config.sg5.lbct1;
  doc["sgroup5"]["lbct2"] = config.sg5.lbct2;
  doc["sgroup5"]["lbct3"] = config.sg5.lbct3;
  doc["sgroup6"]["lbct1"] = config.sg6.lbct1;
  doc["sgroup6"]["lbct2"] = config.sg6.lbct2;
  doc["sgroup6"]["lbct3"] = config.sg6.lbct3;

	// Serialize JSON to file
	if (serializeJson(doc, file) == 0) {
		Log("Failed to write to file");
	}

	file.close();
	Log("Configuration saved successfully.");
}

void UpdateESPConfig(const JsonDocument& doc) {
	strlcpy(config.hostname, doc["hostname"] | def_host, sizeof(config.hostname));
	strlcpy(config.ssid, doc["ssid"] | def_ssid, sizeof(config.ssid));
	strlcpy(config.ssidpwd, doc["ssidpwd"] | def_ssidpwd, sizeof(config.ssidpwd));
	strlcpy(config.ip_addr, doc["ip_addr"] | def_ip_addr, sizeof(config.ip_addr));
	strlcpy(config.ip_mask, doc["ip_mask"] | def_ip_mask, sizeof(config.ip_mask));
	strlcpy(config.ip_getw, doc["ip_getw"] | def_ip_getw, sizeof(config.ip_getw));
	strlcpy(config.ip_dns1, doc["ip_dns1"] | def_ip_dns1, sizeof(config.ip_dns1));
	strlcpy(config.ip_dns2, doc["ip_dns2"] | def_ip_dns2, sizeof(config.ip_dns2));

	strlcpy(config.mqtt_srv, doc["mqtt_srv"] | def_mqtt_srv, sizeof(config.mqtt_srv));
	config.mqtt_port = doc["mqtt_port"] | def_mqtt_port;
	config.http_port = doc["http_port"] | def_http_port;

  config.sg1.enabled = doc["sgroup1"]["enabled"] | true;
  config.sg2.enabled = doc["sgroup2"]["enabled"] | true;
  config.sg3.enabled = doc["sgroup3"]["enabled"] | true;
  config.sg4.enabled = doc["sgroup4"]["enabled"] | true;
  config.sg5.enabled = doc["sgroup5"]["enabled"] | true;
  config.sg6.enabled = doc["sgroup6"]["enabled"] | true;

  config.sg1.ical = doc["sgroup1"]["icalib"] | def_ical1;
  config.sg2.ical = doc["sgroup2"]["icalib"] | def_ical1;
  config.sg3.ical = doc["sgroup3"]["icalib"] | def_ical1;
  config.sg4.ical = doc["sgroup4"]["icalib"] | def_ical2;
  config.sg5.ical = doc["sgroup5"]["icalib"] | def_ical2;
  config.sg6.ical = doc["sgroup6"]["icalib"] | def_ical2;

	strlcpy(config.sg1.lbct1, doc["sgroup1"]["lbct1"] | def_lbctg1l1, sizeof(config.sg1.lbct1));
	strlcpy(config.sg1.lbct2, doc["sgroup1"]["lbct2"] | def_lbctg1l2, sizeof(config.sg1.lbct2));
	strlcpy(config.sg1.lbct3, doc["sgroup1"]["lbct3"] | def_lbctg1l3, sizeof(config.sg1.lbct3));
	strlcpy(config.sg2.lbct1, doc["sgroup2"]["lbct1"] | def_lbctg2l1, sizeof(config.sg2.lbct1));
	strlcpy(config.sg2.lbct2, doc["sgroup2"]["lbct2"] | def_lbctg2l2, sizeof(config.sg2.lbct2));
	strlcpy(config.sg2.lbct3, doc["sgroup2"]["lbct3"] | def_lbctg2l3, sizeof(config.sg2.lbct3));
	strlcpy(config.sg3.lbct1, doc["sgroup3"]["lbct1"] | def_lbctg3l1, sizeof(config.sg3.lbct1));
	strlcpy(config.sg3.lbct2, doc["sgroup3"]["lbct2"] | def_lbctg3l2, sizeof(config.sg3.lbct2));
	strlcpy(config.sg3.lbct3, doc["sgroup3"]["lbct3"] | def_lbctg3l3, sizeof(config.sg3.lbct3));
	strlcpy(config.sg4.lbct1, doc["sgroup4"]["lbct1"] | def_lbctg4l1, sizeof(config.sg4.lbct1));
	strlcpy(config.sg4.lbct2, doc["sgroup4"]["lbct2"] | def_lbctg4l2, sizeof(config.sg4.lbct2));
	strlcpy(config.sg4.lbct3, doc["sgroup4"]["lbct3"] | def_lbctg4l3, sizeof(config.sg4.lbct3));
	strlcpy(config.sg5.lbct1, doc["sgroup5"]["lbct1"] | def_lbctg5l1, sizeof(config.sg5.lbct1));
	strlcpy(config.sg5.lbct2, doc["sgroup5"]["lbct2"] | def_lbctg5l2, sizeof(config.sg5.lbct2));
	strlcpy(config.sg5.lbct3, doc["sgroup5"]["lbct3"] | def_lbctg5l3, sizeof(config.sg5.lbct3));
	strlcpy(config.sg6.lbct1, doc["sgroup6"]["lbct1"] | def_lbctg6l1, sizeof(config.sg6.lbct1));
	strlcpy(config.sg6.lbct2, doc["sgroup6"]["lbct2"] | def_lbctg6l2, sizeof(config.sg6.lbct2));
	strlcpy(config.sg6.lbct3, doc["sgroup6"]["lbct3"] | def_lbctg6l3, sizeof(config.sg6.lbct3));
}

void LoadConfiguration(fs::FS& fs) {
	bool fNotFound = false;
	if (!SPIFFSInitialized) return;
	File file = fs.open(configFilePath);
	if (!file || file.isDirectory()) {
		Log("Failed to open file for reading");
		fNotFound = true;
	}

	StaticJsonDocument<2048> doc;
	DeserializationError error = deserializeJson(doc, file);
	if (error) {
		Log("Failed to read file. Using default configuration!");
    Log(error.c_str());
	}
	UpdateESPConfig(doc);

	if (fNotFound)
	{
		SaveConfiguration(fs);
	}
	else
		file.close();

	Log("Configuration loaded successfully.");
}

bool UpdateConfiguration(String sJson) {
	StaticJsonDocument<2048> doc;
	DeserializationError error = deserializeJson(doc, sJson.c_str());
	if (error) {
		Log("Failed to parse serialzed json data!");
    Log(error.c_str());
		return false;
	}
	UpdateESPConfig(doc);
	Log("Configuration updated successfully.");
	return true;
}


String getDeviceInfo()
{
	String devInfo = "{\"hardware\": \"" + String(device.chipModel) + "\", "
		"\"id\": \"" + String(device.chipId) + "\", "
		"\"hostname\": \"" + String(config.hostname) + "\", "
		"\"appver\": \"" + build + "\" "
		"}";
	return devInfo;
}

String getDeviceInfoEx()
{
	String devInfo = "{\"hardware\": \"" + String(device.chipModel) + "\", "
		"\"id\": \"" + String(device.chipId) + "\", "
		"\"mqttCleintId\": \"" + String(device.mqttCleintId) + "\", "
		"\"hostname\": \"" + String(config.hostname) + "\", "
		"\"appver\": \"" + build + "\", "
		"\"ip_address\": \"" + WiFi.localIP().toString().c_str() + "\", "
		"\"wifi_ssid\": \"" + WiFi.SSID().c_str() + "\", "
		"\"wifi_mac\": \"" + WiFi.BSSIDstr().c_str() + "\", "
		"\"wifi_channel\": \"" + WiFi.channel() + "\", "
		"\"wifi_level\": \"" + logWiFiStrength + "\", "
		"\"http_port\": \"" + http_port + "\", "
		"\"mqtt_server\": \"" + config.mqtt_srv + "\", "
		"\"mqtt_port\": \"" + config.mqtt_port + "\", "
		"\"ntp_server\": \"" + ntpServer + "\" "
		"}";

	return devInfo;
}


String GetLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "Failed to obtain time";
  }
  char timeStringBuff[50]; //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  return (String)timeStringBuff;
}


#endif // _PSWIFI_H

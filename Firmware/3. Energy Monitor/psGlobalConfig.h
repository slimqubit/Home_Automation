#ifndef _PSGLOBALCONFIG_H
#define _PSGLOBALCONFIG_H

const char* build = "ver. 1.20 (build 22 Mar. 2023)";

#if CONFIG_IDF_TARGET_ESP32S2
  const byte Yellow_LED_Pin = 14;
  const byte Red_LED_Pin = 15;
  const byte Green_LED_Pin = 16;
  const byte Blue_LED_Pin = 17;
#elif CONFIG_IDF_TARGET_ESP32S3
  const byte Yellow_LED_Pin = 45;
  const byte Red_LED_Pin = 46;
  const byte Green_LED_Pin = 47;
  const byte Blue_LED_Pin = 48;
#endif




const char* def_host = "esp32-d001";
const char* def_ssid = "your_ssid";
const char* def_ssidpwd = "your_ssid_password";

// IP configuration
const char* def_ip_addr = "";
const char* def_ip_mask = "";
const char* def_ip_getw = "";
const char* def_ip_dns1 = "";
const char* def_ip_dns2 = "";

// WebServer Configuration
const uint16_t http_port = 80;
const uint16_t def_http_port = 80;

// MQTT protocol
const char* def_mqtt_srv = "";
const uint16_t def_mqtt_port = 1883;
const int      MQTT_Reconnect_delay = 5000;

// Time configuration
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200; /// = 0;
const int   daylightOffset_sec = 3600;

// Sensors default
const double  def_ical1 = 16.3;
const double  def_ical2 = 26.3;
const  char* def_lbctg1l1 = "G1L1";
const  char* def_lbctg1l2 = "G1L2";
const  char* def_lbctg1l3 = "G1L3";
const  char* def_lbctg2l1 = "G2L1";
const  char* def_lbctg2l2 = "G2L2";
const  char* def_lbctg2l3 = "G2L3";
const  char* def_lbctg3l1 = "G3L1";
const  char* def_lbctg3l2 = "G3L2";
const  char* def_lbctg3l3 = "G3L3";
const  char* def_lbctg4l1 = "G4L1";
const  char* def_lbctg4l2 = "G4L2";
const  char* def_lbctg4l3 = "G4L3";
const  char* def_lbctg5l1 = "G5L1";
const  char* def_lbctg5l2 = "G5L2";
const  char* def_lbctg5l3 = "G5L3";
const  char* def_lbctg6l1 = "G6L1";
const  char* def_lbctg6l2 = "G6L2";
const  char* def_lbctg6l3 = "G6L3";

#endif  // _PSGLOBALCONFIG_H

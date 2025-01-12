#ifndef _PSSENSORS_H
#define _PSSENSORS_H

#include <Arduino.h>
#include "psGlobalConfig.h"

#include <vector>
#include <driver/adc.h>
#include <driver/adc_common.h>
#include <esp_adc_cal.h>
#include "psMQTT.h"
#include "psEmonLibESP.h"   
#include "psUtils.h"


// Calibration Value: 1000 turns / 100 Ohm burden = 10
// http://openenergymonitor.org/emon/buildingblocks/calibration
// 
//   AP-2000:   2000 turns / 50 Ohm burden   => Ical = 31.0
//   China mic  1000 turns / 50 Ohm burden   => Ical = 15.5
// cb

byte min_startup_cycles = 25;
const int no_of_samples = 2144;   // 50Hz: 1 cycle takes 50ms (no: 1492 - 100ms )
byte Vrms = 232;            // Vrms for apparent power readings (when no AC-AC voltage sample is present)

EnergyMonESP SG1L1, SG1L2, SG1L3, SG2L1, SG2L2, SG2L3, SG3L1, SG3L2, SG3L3;
EnergyMonESP SG4L1, SG4L2, SG4L3, SG5L1, SG5L2, SG5L3, SG6L1, SG6L2, SG6L3;
esp_adc_cal_characteristics_t adc1_chars, adc2_chars;


std::vector<String> v_last10Min;
std::vector<String> v_last2Hours;
std::vector<String> v_last6Hours;
std::vector<String> v_last24Hours;

enum data_range { last10Min, last2Hours, last6Hours, last24Hours };

const int Sensors_Check_delay = 3200;
unsigned long Sensors_Check_millis = millis();

const byte no_of_groups = 6;
const byte no_of_sensors = 18;

byte SGPins[no_of_sensors] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};  
double values[no_of_sensors];
double avg_10M[no_of_sensors];
double avg_2H[no_of_sensors];
double avg_6H[no_of_sensors];
double avg_24H[no_of_sensors];

byte i10M, i2H, i6H, i24H;
float prevSecond, prevMinute;

String csvData;
String sJsonData;


void resetvar_values() { for (byte i = 0; i < no_of_sensors; i++) { values[i] = 0; }}
void resetvar_2H() { i2H = 0; for (byte i = 0; i < no_of_sensors; i++) { avg_2H[i] = 0; }}
void resetvar_6H() { i6H = 0; for (byte i = 0; i < no_of_sensors; i++) { avg_6H[i] = 0; }}
void resetvar_24H() { i24H = 0; for (byte i = 0; i < no_of_sensors; i++) { avg_24H[i] = 0; }}

void readSensorsData() {
  if (config.sg1.enabled) {
    values[0]  = SG1L1.calcIrmsEx(200);  //if (valG1L1 <= 0.02) valG1L1 = 0;
    values[1]  = SG1L2.calcIrmsEx(200);  //if (valG1L2 <= 0.02) valG1L2 = 0;
    values[2]  = SG1L3.calcIrmsEx(200);  //if (valG1L3 <= 0.02) valG1L3 = 0;
  } else {for (byte i = 0; i <= 2; i++) { values[i] = 0; }}
    
  if (config.sg2.enabled) {
    values[3]  = SG2L1.calcIrmsEx(200);  //if (valG2L1 <= 0.02) valG2L1 = 0;
    values[4]  = SG2L2.calcIrmsEx(200);  //if (valG2L2 <= 0.02) valG2L2 = 0;
    values[5]  = SG2L3.calcIrmsEx(200);  //if (valG2L3 <= 0.02) valG2L3 = 0;
  } else {for (byte i = 3; i <= 5; i++) { values[i] = 0; }}

  if (config.sg3.enabled) {
    values[6]  = SG3L1.calcIrmsEx(200);  //if (valG3L1 <= 0.02) valG3L1 = 0;
    values[7]  = SG3L2.calcIrmsEx(200);  //if (valG3L2 <= 0.02) valG3L2 = 0;
    values[8]  = SG3L3.calcIrmsEx(200);  //if (valG3L3 <= 0.02) valG3L3 = 0;
  } else {for (byte i = 6; i <= 8; i++) { values[i] = 0; }}

  if (config.sg4.enabled) {
    values[9]  = SG4L1.calcIrmsEx(200);  //if (valG1L1 <= 0.02) valG1L1 = 0;
    values[10] = SG4L2.calcIrmsEx(200);  //if (valG1L2 <= 0.02) valG1L2 = 0;
    values[11] = SG4L3.calcIrmsEx(200);  //if (valG1L3 <= 0.02) valG1L3 = 0;
  } else {for (byte i = 9; i <= 11; i++) { values[i] = 0; }}

  if (config.sg5.enabled) {
    values[12] = SG5L1.calcIrmsEx(200);  //if (valG2L1 <= 0.02) valG2L1 = 0;
    values[13] = SG5L2.calcIrmsEx(200);  //if (valG2L2 <= 0.02) valG2L2 = 0;
    values[14] = SG5L3.calcIrmsEx(200);  //if (valG2L3 <= 0.02) valG2L3 = 0;
  } else {for (byte i = 12; i <= 14; i++) { values[i] = 0; }}
  
  if (config.sg6.enabled) {
    values[15] = SG6L1.calcIrmsEx(200);  //if (valG3L1 <= 0.02) valG3L1 = 0;
    values[16] = SG6L2.calcIrmsEx(200);  //if (valG3L2 <= 0.02) valG3L2 = 0;
    values[17] = SG6L3.calcIrmsEx(200);  //if (valG3L3 <= 0.02) valG3L3 = 0;
  } else {for (byte i = 15; i <= 17; i++) { values[i] = 0; }}
}

void calculateAvgData() {
  for (byte i = 0; i < no_of_sensors; i++) 
  { 
    avg_2H[i] = avg_2H[i] + values[i];
    avg_6H[i] = avg_6H[i] + values[i];
    avg_24H[i] = avg_24H[i] + values[i];
  }
  i2H++;   i6H++;   i24H++;
}

String getCSVData(data_range dataRange) {
  char buff[256];
  sprintf(buff, "Time, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s", 
        config.sg1.lbct1, config.sg1.lbct2, config.sg1.lbct3, 
        config.sg2.lbct1, config.sg2.lbct2, config.sg2.lbct3, 
        config.sg3.lbct1, config.sg3.lbct2, config.sg3.lbct3, 
        config.sg4.lbct1, config.sg4.lbct2, config.sg4.lbct3, 
        config.sg5.lbct1, config.sg5.lbct2, config.sg5.lbct3, 
        config.sg6.lbct1, config.sg6.lbct2, config.sg6.lbct3);
  String S = String(buff);
  switch (dataRange) {
  case last10Min:
    for (String s : v_last10Min) { S = S + "\n" + s; }
    break;
  case last2Hours:
    for (String s : v_last2Hours) { S = S + "\n" + s; }
    break;
  case last6Hours:
    for (String s : v_last6Hours) { S = S + "\n" + s; }
    break;
  case last24Hours:
    for (String s : v_last24Hours) { S = S + "\n" + s; }
    break;
  }
  return S;
}

String getAvgData(char* cTime, double val[no_of_sensors], byte Count) {
  char buff[128];
  sprintf(buff, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", cTime, 
    String(val[0] / Count), String(val[1] / Count), String(val[2] / Count),
    String(val[3] / Count), String(val[4] / Count), String(val[5] / Count), 
    String(val[6] / Count), String(val[7] / Count), String(val[8] / Count), 
    String(val[9] / Count), String(val[10] / Count), String(val[11] / Count),
    String(val[12] / Count), String(val[13] / Count), String(val[14] / Count), 
    String(val[15] / Count), String(val[16] / Count), String(val[17] / Count));
  return String(buff);
}

String getJsonAvgData(char* cTime, double val[no_of_sensors], byte Count) {
  char buff[256];
  sprintf(buff, "{\"timestamp\":\"%s\",\"G1L1\":%s,\"G1L2\":%s,\"G1L3\":%s,\"G2L1\":%s,\"G2L2\":%s,\"G2L3\":%s,\"G3L1\":%s,\"G3L2\":%s,\"G3L3\":%s,\"G4L1\":%s,\"G4L2\":%s,\"G4L3\":%s,\"G5L1\":%s,\"G5L2\":%s,\"G5L3\":%s,\"G6L1\":%s,\"G6L2\":%s,\"G6L3\":%s}", cTime, 
    String(val[0] / Count), String(val[1] / Count), String(val[2] / Count),
    String(val[3] / Count), String(val[4] / Count), String(val[5] / Count), 
    String(val[6] / Count), String(val[7] / Count), String(val[8] / Count), 
    String(val[9] / Count), String(val[10] / Count), String(val[11] / Count),
    String(val[12] / Count), String(val[13] / Count), String(val[14] / Count), 
    String(val[15] / Count), String(val[16] / Count), String(val[17] / Count));
  return String(buff);
}




void setup_Sensors() {
  resetvar_values();
  resetvar_2H();
  resetvar_6H();
  resetvar_24H();

  pinMode(SGPins[0], INPUT);    pinMode(SGPins[1], INPUT);    pinMode(SGPins[2], INPUT);
  pinMode(SGPins[3], INPUT);    pinMode(SGPins[4], INPUT);    pinMode(SGPins[5], INPUT);
  pinMode(SGPins[6], INPUT);    pinMode(SGPins[7], INPUT);    pinMode(SGPins[8], INPUT);
  pinMode(SGPins[9], INPUT);    pinMode(SGPins[10], INPUT);   pinMode(SGPins[11], INPUT);
  pinMode(SGPins[12], INPUT);   pinMode(SGPins[13], INPUT);   pinMode(SGPins[14], INPUT);
  pinMode(SGPins[15], INPUT);   pinMode(SGPins[16], INPUT);   pinMode(SGPins[17], INPUT);

  // Setup the ADC
  //adc1_config_width(ADC_WIDTH_BIT_12);
  analogReadResolution(ESP_ADC_BITS);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_8, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_9, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_1, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_2, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_3, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_4, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_5, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_6, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_7, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_8, ADC_ATTEN_DB_11);
  adc2_config_channel_atten(ADC2_CHANNEL_9, ADC_ATTEN_DB_11);

  #if CONFIG_IDF_TARGET_ESP32S2
    esp_adc_cal_value_t val1_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_13, 1100, &adc1_chars);
    esp_adc_cal_value_t val2_type = esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_13, 1100, &adc2_chars);
  #elif CONFIG_IDF_TARGET_ESP32S3
    esp_adc_cal_value_t val1_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc1_chars);
    esp_adc_cal_value_t val2_type = esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc2_chars);
  #endif
  
  SG1L1.current(SGPins[0],  config.sg1.ical);  SG1L2.current(SGPins[1],  config.sg1.ical);  SG1L3.current(SGPins[2],  config.sg1.ical);
  SG2L1.current(SGPins[3],  config.sg2.ical);  SG2L2.current(SGPins[4],  config.sg2.ical);  SG2L3.current(SGPins[5],  config.sg2.ical);
  SG3L1.current(SGPins[6],  config.sg3.ical);  SG3L2.current(SGPins[7],  config.sg3.ical);  SG3L3.current(SGPins[8],  config.sg3.ical);
  SG4L1.current(SGPins[9],  config.sg4.ical);  SG4L2.current(SGPins[10], config.sg4.ical);  SG4L3.current(SGPins[11], config.sg4.ical);
  SG5L1.current(SGPins[12], config.sg5.ical);  SG5L2.current(SGPins[13], config.sg5.ical);  SG5L3.current(SGPins[14], config.sg5.ical);
  SG6L1.current(SGPins[15], config.sg6.ical);  SG6L2.current(SGPins[16], config.sg6.ical);  SG6L3.current(SGPins[17], config.sg6.ical);

  // getting time spend by a sample
  unsigned long tNow = micros();
  double Idmy = SG1L1.calcIrms(no_of_samples);  // Calculate Irms only
  unsigned long tSpent = micros() - tNow;
  char buff[128];
  sprintf(buff, "Time spent to get a sample %u us.", tSpent);
  Log(String(buff));
}

void loop_Sensors_GetData() {
  struct tm timeinfo;
  char cTime1[25];
  char cTime2[25];
  char cTime3[25];
  String sData;

  if (millis() > Sensors_Check_millis) {
    // Read sensors data
    readSensorsData();
    if (min_startup_cycles > 0) { min_startup_cycles--; return; };

    if (!getLocalTime(&timeinfo)) {
      Log("Failed to obtain time while getting sensors data.");
      return;
    }
    byte cSecond = timeinfo.tm_sec;
    byte cMinute = timeinfo.tm_min;

    strftime(cTime1, sizeof(cTime1), "%Y-%m-%d %H:%M:%S", &timeinfo);
    strftime(cTime2, sizeof(cTime2), "%Y-%m-%d %H:%M:00", &timeinfo);

    // Update the vector with latest values
    char buff1[128];
    sprintf(buff1, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", cTime1, 
      String(values[0]), String(values[1]), String(values[2]),
      String(values[3]), String(values[4]), String(values[5]),
      String(values[6]), String(values[7]), String(values[8]),
      String(values[9]), String(values[10]), String(values[11]),
      String(values[12]), String(values[13]), String(values[14]),
      String(values[15]), String(values[16]), String(values[17]));

    if (v_last10Min.size() >= 120) v_last10Min.erase(v_last10Min.begin());
    v_last10Min.push_back(buff1);

    // Send MQtt Data at everyy 20 seconds
    byte xSec;
    if ((cSecond >= 0) && (cSecond < 20)) { xSec = 10; strftime(cTime3, sizeof(cTime3), "%Y-%m-%d %H:%M:10", &timeinfo); }
    if ((cSecond >= 20) && (cSecond < 40)) { xSec = 30; strftime(cTime3, sizeof(cTime3), "%Y-%m-%d %H:%M:30", &timeinfo); }
    if ((cSecond >= 40) && (cSecond < 60)) { xSec = 50; strftime(cTime3, sizeof(cTime3), "%Y-%m-%d %H:%M:50", &timeinfo); }
    if (xSec != prevSecond) {
      prevSecond = xSec;
      sJsonData = getJsonAvgData(cTime3, values, 1);
      mqtt_publish(device.mqttCleintId, sJsonData);
    }

    // Calculate the average data
    calculateAvgData();

    if (cMinute != prevMinute) {
      prevMinute = cMinute;
      sData = getAvgData(cTime2, avg_2H, i2H);

      resetvar_2H();
      if (v_last2Hours.size() >= 120) v_last2Hours.erase(v_last2Hours.begin());
      v_last2Hours.push_back(sData);

      if (cMinute % 3 == 0) {
        sData = getAvgData(cTime2, avg_6H, i6H);
        resetvar_6H();
        if (v_last6Hours.size() >= 120) v_last6Hours.erase(v_last6Hours.begin());
        v_last6Hours.push_back(sData);
      }

      if (cMinute % 5 == 0) {
        sData = getAvgData(cTime2, avg_24H, i24H);
        resetvar_24H();
        if (v_last24Hours.size() >= 288) v_last24Hours.erase(v_last24Hours.begin());
        v_last24Hours.push_back(sData);
      }
    }

    Sensors_Check_millis = millis() + Sensors_Check_delay;
  }
}


#endif // _PSSENSORS_H

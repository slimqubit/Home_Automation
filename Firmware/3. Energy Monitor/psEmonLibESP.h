

/*
  Emon.h - Library for openenergymonitor
  Created by Trystan Lea, April 27 2010
  GNU GPL
  modified to use up to 12 bits ADC resolution (ex. Arduino Due)
  by boredman@boredomprojects.net 26.12.2013
  Low Pass filter for offset removal replaces HP filter 1/1/2015 - RW
*/

#ifndef psEmonLibESP_h
#define psEmonLibESP_h

#include "Arduino.h"

// to enable 12/13-bit ADC resolution on ESP32, include the following line in main sketch inside setup() function:
#if CONFIG_IDF_TARGET_ESP32S2
	#define ESP_ADC_BITS    13
#elif CONFIG_IDF_TARGET_ESP32S3
	#define ESP_ADC_BITS    12
#endif

#define ESP_ADC_COUNTS  (1<<ESP_ADC_BITS)   //    analogReadResolution(ADC_BITS);    otherwise will default to 10 bits, as in regular Arduino-based boards.


class EnergyMonESP
{
public:

  //--------------------------------------------------------------------------------------
  // Sets the pins to be used for voltage and current sensors
  //--------------------------------------------------------------------------------------
  void current(unsigned int _inPinI, double _ICAL)
  {
    inPinI = _inPinI;
    ICAL = _ICAL;
    offsetI = ESP_ADC_COUNTS >> 1;
    sumI = 0;
    sqI = 0;
    filteredI = 0;
  }



  //--------------------------------------------------------------------------------------
  // Calculate Irms based on number of samples to be taken
  //--------------------------------------------------------------------------------------
  double calcIrms(unsigned int Number_of_Samples)
  {
    int SupplyVoltage = 3300;
    for (unsigned int n = 0; n < Number_of_Samples; n++)
    {
      sampleI = analogRead(inPinI);

      // Digital low pass filter extracts the 2.5 V or 1.65 V dc offset,
      //  then subtract this - signal is now centered on 0 counts.
      offsetI = (offsetI + (sampleI - offsetI) / 1024);
      filteredI = sampleI - offsetI;

      // Root-mean-square method current
      // 1) square current values
      sqI = filteredI * filteredI;
      // 2) sum
      sumI += sqI;
    }

    double I_RATIO = ICAL * ((SupplyVoltage / 1000.0) / (ESP_ADC_COUNTS));
    Irms = I_RATIO * sqrt(sumI / Number_of_Samples);

    //Reset accumulators
    sumI = 0;
    //--------------------------------------------------------------------------------------

    return Irms;
  }

  //--------------------------------------------------------------------------------------
  // Calculate Irms based on desired timeframe to take samples
  //--------------------------------------------------------------------------------------
  double calcIrmsEx(unsigned int millisSpent)
  {
    int SupplyVoltage = 3300;

    unsigned int Number_of_Samples = 0;
    unsigned long sNow = millis();
    while (millis() - sNow <= millisSpent)
    {
      sampleI = analogRead(inPinI);

      // Digital low pass filter extracts the 2.5 V or 1.65 V dc offset,
      //  then subtract this - signal is now centered on 0 counts.
      offsetI = (offsetI + (sampleI - offsetI) / ESP_ADC_COUNTS);
      filteredI = sampleI - offsetI;

      // Root-mean-square method current
      // 1) square current values
      sqI = filteredI * filteredI;
      // 2) sum
      sumI += sqI;
      Number_of_Samples++;
    }
    double I_RATIO = ICAL * ((SupplyVoltage / 1000.0) / (ESP_ADC_COUNTS));
    Irms = I_RATIO * sqrt(sumI / Number_of_Samples);

    //Reset accumulators
    sumI = 0;
    //--------------------------------------------------------------------------------------

    return Irms;
  }

  double getDC()
  {
    double vDC = (double)analogReadMilliVolts(inPinI)/1000.00;
    return vDC;
  }
  

  //Useful value variables
  double Irms;


private:
  unsigned int inPinI;    //Set Current input pins
  double ICAL;            //Calibration coefficients
  int sampleI;
  double filteredI;
  double offsetI;         //Low-pass filter output
  double sqI, sumI;       //sq = squared, sum = Sum

};

#endif

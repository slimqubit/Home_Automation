#ifndef _PSMQTT_H
#define _PSMQTT_H

#include <Arduino.h>
#include "psGlobalConfig.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include "psUtils.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long MQTT_Check_millis = millis();


void callback(char* topic, byte* payload, unsigned int length) {
  char buff[256];
  Log("Message arrived via MQTT connection: ");
  sprintf(buff, "   Topic: %s, Message: %d", topic, payload);
  Log(buff);


	// Switch on the LED if an 1 was received as first character
	if ((char)payload[0] == '1') {
		// digitalWrite(WiFi_led_pin, LOW);   // Turn the LED on (Note that LOW is the voltage level
		// but actually the LED is on; this is because
		// it is active low on the ESP-01)
	}
	else {
		// digitalWrite(WiFi_led_pin, HIGH);  // Turn the LED off by making the voltage HIGH
	}
}

void reconnect() {
	// Loop until we're reconnected
	Serial.println("Attempting MQTT connection... ");

	String clientId = String(config.hostname) + "-" + String(device.chipId);
	if (mqttClient.connect(clientId.c_str())) {
    Log("MQTT client connected to server. MQTT client id: " + clientId);
		//// Once connected, publish an announcement...
		//mqttClient.publish("outTopic", "hello world");
		//// ... and resubscribe
		//mqttClient.subscribe("inTopic");
	}
	else {
    String cState = String(mqttClient.state());
    Log("MQTT client connection failed, will try again in 5 sec. Client state: " + cState);
	}
}


void setup_mqtt() {
	mqttClient.setServer(config.mqtt_srv, config.mqtt_port);
	mqttClient.setCallback(callback);
  mqttClient.setBufferSize(512);
}


void loop_mqtt() {
	if (millis() > MQTT_Check_millis) {
		if (!mqttClient.connected()) {
			reconnect();
		}
		MQTT_Check_millis = millis() + MQTT_Reconnect_delay;
	}
	mqttClient.loop();
}


void mqtt_publish(String sTopic, String sData) {
	if (!mqttClient.connected()) {
    Log("Failed to publish data. MQTT client not connected!");
		return;
	} 
	mqttClient.publish(sTopic.c_str(), sData.c_str());
}


#endif // _PSMQTT_H

#pragma once
#include "DHT\DHT.h"
void green_led(byte data);
void yellow_led(byte data);
void dht_data(byte data);

DHT dht(2, DHT11);
void push_data(byte addr, byte data) {
	byte c = constrain(data, 50, 255);//преобразование к требуемому виду
	if (addr == 0x00) {
		green_led(c);
	}
	if (addr == 0x80) {
		yellow_led(c);
	}
	if (addr == 0xc0) {
		dht_data(data);
	}
}
void green_led(byte data) {
	//byte c = constrain(data, 50, 255);
	analogWrite(green, data);
	Serial.print("Green LED is on");
	delay(1000);
	analogWrite(green, 0);
	Serial.print("Green LED is off");
}
void yellow_led(byte data) {
	//byte c = constrain(data, 50, 255);
	analogWrite(yellow, data);
	Serial.print("Yellow LED is on");
	delay(1000);
	analogWrite(yellow, 0);
	Serial.print("Yellow LED is off");
}
void dht_data(byte data) {
	dht.begin();
	float h = dht.readHumidity();
	float t = dht.readTemperature();
	boolean is_data = !(isnan(t) || isnan(h));
	switch (data) {
	case 0x01: {
		if (is_data) {
			Serial.print("Humidity = ");
			Serial.print(h);
			Serial.print(" %\t");
		}
		else
		{
			Serial.print("Unable to get data...");
		}
		
	}break;
	case 0x02: {
		if (is_data) {
			Serial.print("Temperature: ");
			Serial.print(t);
			Serial.println(" *C");
		}
		else{
			Serial.print("Unable to get data...");
		}
	}break;
	case 0x04:{
		if (is_data) {
			Serial.print("Humidity = ");
			Serial.print(h);
			Serial.print(" %\t");
			Serial.print("Temperature: ");
			Serial.print(t);
			Serial.println(" *C");
		}
		else {
			Serial.print("Unable to get data...");
		}
	}break;
	default: {
		Serial.print("Unknown command...");
	}
			 break;
	}
}
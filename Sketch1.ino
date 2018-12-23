#include "DHT\DHT.h"

const short int green = 9;
const short int yellow = 10;
const short int baudrate = 9600;
DHT dht(2, DHT11);

const int size = 3;
char incoming_symbol;
char string_data[size];
int i = 0;
byte int_data = 0;
byte mask = 192;
byte wildcard = 63;

String in;
byte inc = 0;
void green_led(byte data);
void yellow_led(byte data);
void dht_data(byte data);

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
	byte c = constrain(data, 50, 255);
	analogWrite(green, data);
	Serial.println("Green LED is on");
	delay(1000);
	analogWrite(green, 0);
	Serial.println("Green LED is off");
}
void yellow_led(byte data) {
	byte c = constrain(data, 50, 255);
	analogWrite(yellow, data);
	Serial.println("Yellow LED is on");
	delay(1000);
	analogWrite(yellow, 0);
	Serial.println("Yellow LED is off");
}
void dht_data(byte data) {
	
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
		else {
			Serial.print("Unable to get data...");
		}
	}break;
	case 0x03: {
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
void setup() {
	Serial.begin(baudrate);//открытие порта	
	//инициализация портов
	pinMode(green, OUTPUT);
	pinMode(yellow, OUTPUT);
	dht.begin();//включить датчик
}
//получение адреса устройства
byte HWAddr(byte data) {
	//byte mask = 192;
	byte merge = data&mask;
	return merge;
}
//получение данных
byte HWData(byte data){
	//byte mask = 63;
	byte merge = data&wildcard;
	return merge;
}
//отправка данных на нужное устройство
void loop() {
	/*//если на входе имеем некоторые данные
	while (Serial.available()) {
		incoming_symbol = Serial.read();
		if (incoming_symbol < '0' || incoming_symbol > '9') break;
		string_data[i] = incoming_symbol;
		i++;
	}
	if (i > 2) //на данные отводим три знака
	{
		string_data[i] = 0;
		int_data = (byte)atoi(string_data);
		Serial.println(int_data);//отладочная информация
		i = 0;//обнуление счетчика*/
	//int i;
	if (Serial.available()) {
		inc = Serial.parseInt();
		Serial.flush();
		Serial.println(inc);
		push_data(HWAddr(inc), HWData(inc));
	}
	
}

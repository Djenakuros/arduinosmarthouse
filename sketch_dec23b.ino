#include <DHT.h>

//инициализация пинов
const short int green = 6;
const short int yellow = 10;
const short int photores_power = 9;
const short int photores = A0;
const short int reset = 11;
const short int pir = 4;
const short int dht_port = 3;


DHT dht(dht_port, DHT11);//инициализация датчика
//битовые маски

const unsigned int mask = 192;
const unsigned int wildcard = 63;

const short int baudrate = 9600;//скорость порта


//вспомогательные переменные
String in;
unsigned int inc = 0u;
bool is_enabled_led = false;
bool is_dark_room = false;

void yellow_led(byte data);
void dht_data(byte data);

//answer definition
const unsigned int OK = 0x0000u;
const unsigned int NO_COMMAND = 0x0001u;
const unsigned int NO_DATA = 0x0002u;
const unsigned int NO_DEVICE = 0x0003u;
const unsigned int REBOOT = 0x0004u;

//device definition
const unsigned int dev_photores = 0x0040u;
const unsigned int dev_lamp = 0x0080u;
const unsigned int dev_sensor_temp = 0x00C0u;
const unsigned int dev_sensor_pir = 0x0000u;
const unsigned int dev_root_reboot = 0x00FFu;

void send_system_message(unsigned short int code) {
    //функция вывода сообщений
    switch (code) {
    case OK: {
      Serial.println("OK");
    }
    break;
    case NO_COMMAND: {
    Serial.println("COMMAND DOES NOT EXIST");
     }
    break;
    case NO_DATA: {
    Serial.println("DATA NOT AVAILABLE");
    }
    break;
    case NO_DEVICE: {
       Serial.println("DEVICE NOT FOUND");
    }
    case REBOOT: {
       Serial.println("DEVICE WILL BE REBOOT");
    }
    default: {
       Serial.println("SYSTEM ERROR");
    }
    break;
   }
}

void push_data(unsigned short int addr, unsigned short int data) { 
  /*
  if (addr == dev_lamp) {
  }
  if (addr == dev_sensor_temp) {
    dht_data(data);
  }
  if (addr == dev_photores) {
    get_light_power();
  }
  if (addr == dev_root_reboot) {
    reboot();
  }
  */

 //функция подачи команд на устройства
  switch(addr){
      case dev_lamp:{
      led(data);
      }
      break;
      case dev_sensor_temp:{
      dht_data(data);
      }
      break;
      case dev_photores:{
      get_light_power();
      }
      break;
      case dev_root_reboot:{
      reboot();
      }
      break;
      default:{
        send_system_message(NO_DEVICE);
      }
  }
}




void led(unsigned short int data) {//функция зажигания освещения
    switch (data){
     case 0x0001: {
       is_enabled_led = false;
       send_system_message(OK);
    }
    break;
      case 0x0003: {
       is_enabled_led = true;
       send_system_message(OK);
       }
       break;
      default: {
       send_system_message(NO_COMMAND);
      }
     break;
  }
  if (is_enabled_led) {
    digitalWrite(yellow, HIGH);
  }
  else {
    digitalWrite(yellow, LOW);
  }
}


void dht_data(unsigned short int data) { //функция получения данных с датчика
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  bool is_data = !(isnan(t) || isnan(h));

  switch (data) {
    case 0x0001: {
      if (is_data) {
        send_system_message(OK);
        Serial.print("Humidity = ");
        Serial.print(h);
        Serial.print(" %\t");
       }
       else {
         send_system_message(NO_DATA);
         }
    }
    break;
    case 0x0002: {
        if (is_data) {
            send_system_message(OK);
            Serial.print("Temperature: ");
            Serial.print(t);
            Serial.println(" *C");
            }
        else {
            send_system_message(NO_DATA);
            }
    }
    break;
    case 0x0003: {
        if (is_data) {
            send_system_message(OK);
            Serial.print("Humidity = ");
            Serial.print(h);
            Serial.print(" %\t");
            Serial.print("Temperature: ");
            Serial.print(t);
            Serial.println(" *C");
        }
        else {
            send_system_message(NO_DATA);
        }
    }
    break;
    default:{
        send_system_message(NO_COMMAND);
        }
    break;
    }
}

void get_light_power() { // функция получения уровня освещенности
    Serial.print(analogRead(photores));
    send_system_message(OK);
}
void pir_sensor() {//функция управления ИК-датчиком
    if (analogRead(photores) > 450) {
        is_dark_room = true;
        }
    else {
        is_dark_room = false;
    }
    if ((is_dark_room == true) && (digitalRead(pir) == HIGH)) {
        pir_sensor_alarm();
        Serial.println("Interloper!!!");
    }
    else {
        return;
        }
}

void pir_sensor_alarm() { //функция подачи тревоги
    for (int i = 0; i < 4; i++) {
        digitalWrite(green, HIGH);
        delay(1000);
        digitalWrite(green, LOW);
        delay(1000);
}
}

void reboot() { // функция перезагрузки контроллера
    send_system_message(REBOOT);
    delay(5000);
    digitalWrite(reset, LOW);
}


void setup() {
   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.begin(baudrate);//открытие порта  
  Serial.println("Arduino OS v0.2a");
  Serial.println("System loading...");
  delay(1000);
  Serial.println("Reboot trigger enabling...");
  digitalWrite(reset, HIGH);
  Serial.println("Reboot trigger enable...");
  
  Serial.println("Pin setup...");
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(reset, OUTPUT);
  Serial.println("Pins enabled...");
  
  Serial.println("DHT configuring...");
  dht.begin();//включить датчик
  Serial.println("DHT Configured...");
  
  Serial.println("Photoresistor trigger configuring...");
  digitalWrite(photores_power, HIGH);
  Serial.println("Photoresistor trigger enabled...");

  Serial.println("Welcome to Arduino OS! Please, enter command:");
    }

unsigned short int HWAddr(unsigned short int data) {
    //получение адреса устройства
    unsigned short int merge = data & mask;
    return merge;
}

unsigned short int HWData(unsigned short int data){
    //получение данных
    unsigned short int merge = data & wildcard;
    return merge;
}

//отправка данных на нужное устройство
String input_string = "";
void loop() {
    if (Serial.available() > 0) {// получение данных с клиента
      while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n') { 
          inc = input_string.toInt();
          Serial.print("Input_string is: ");
          Serial.println(input_string);
          Serial.print("Input_string integer is: ");
          Serial.println(inc);
          input_string = "";
          push_data(HWAddr(inc), HWData(inc));
          } 
        else {
          input_string += c;
    }
  }
    }
    pir_sensor();
    //inc = inString.toInt();
    //Serial.println(inc); //debug info
    //Serial.flush();
    //if(inc !=0 ){
    
    //}
     // контроль присутствия
}

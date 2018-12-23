#include <DHT.h>

//initializing pins
//it's not a green, but it happens historically:) If be seriously, it's for alarm lamp
const short int green = 9;
//for flashlight
const short int yellow = 10; 
//Arduino Uno need a more 5V DC Out
const short int photores_power = 3; 
//for getting data from photoresistor
const short int photores = A0;
//for programm reset
const short int reset = 11;
//for getting data from PIR sensor
const short int pir = 4;
//for detting data from DHT11
const short int dht_port = 3

//initializing DHT
DHT dht(dht_port, DHT11);

//byte mask
const short unsigned int mask = 192; //for address of device
const short unsigned int wildcard = 63; //for command to device

const short int baudrate = 9600;//port speed

//additional variables
byte inc = 0;
bool is_enabled_led = false;
bool is_dark_room = false;

//prototyping functions
void yellow_led(byte data);
void dht_data(byte data);

//answer definition
const short unsigned int OK = 0x0000
const short unsigned int NO_COMMAND = 0x0001
const short unsigned int NO_DATA = 0x0002
const short unsigned int NO_DEVICE = 0x0003
const short unsigned int REBOOT = 0x00FF

//device definition
const short unsigned int dev_photores = 0x0040
const short unsigned int dev_lamp = 0x0080
const short unsigned int dev_sensor_temp = 0x00C0
const short unsigned int dev_sensor_pir = 0x0000
const short unsigned int dev_root_reboot = 0x00FF

void send_system_message(unsigned short int code) {
    /*This function interprete a return code to string message.
    Arguments:

    * code: code of message
    */
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
 /*This function send data to particular device.
  Arguments:
    * addr: device address, definited in "device definition" block
    * data: command to device
 */
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

void led(unsigned short int data) {
    /*
    This function manages a LED
    */
   switch (data){
       case 0x01: {
           is_enabled_led = false;
           send_system_message(OK);
           }
        break;
        case 0x03: {
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

void dht_data(unsigned short int data) {
    /*
    This function get data on demand from DHT.
    Arguments:
    * data: command to DHT
    */
   //checking availability of DHT
   float h = dht.readHumidity();
   float t = dht.readTemperature();
   bool is_data = !(isnan(t) || isnan(h));
   
   switch (data) {
       case 0x01: {
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
        case 0x02: {
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
        case 0x03: {
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

void get_light_power() { 
    /*
    Function for getting data from photoresistor.

    Function has no arguments
    */
   Serial.print(analogRead(photores));
   send_system_message(OK);
}
void pir_sensor() {
    /*
    This function using for detect an intruder when no light in room
    */
   //This variable definite a level of illumination when exceeding it will be triggered alarm
   unsigned int threshold = 200;
   if (analogRead(photores) > threshold) {
       is_dark_room = true;
       }
    else {
        is_dark_room = false;
        }

    // Enabling alarm
    if ((is_dark_room == true) && (digitalRead(pir) == HIGH)) {
        pir_sensor_alarm();
        Serial.println("Intruder!!!");
    }
    else {
        return;
    }
}

void pir_sensor_alarm() {
    /*
    This function using for alarm
    */
    for (int i = 0; i < 10; i++) {
        digitalWrite(green, HIGH);
        delay(1000);
        digitalWrite(green, LOW);
        delay(1000);
}
}

void reboot() {
    /*
    This function using for Arduino Uno programm reboot 
    */
   //Time in miliseconds to rebooting
   unsigned int timeout = 5000;
   send_system_message(REBOOT);
   delay(timeout);
   digitalWrite(reset, LOW);
}

unsigned short int HWAddr(unsigned short int raw_data) {
    /* Function for get target device address

    Arguments:
    * raw_data: data from serial port
    */
    unsigned short int merge = raw_data & mask;
    return merge;
}

unsigned short int HWData(unsigned short int raw_data){
     /* Function for get target device address

    Arguments:
    * raw_data: data from serial port
    */
    unsigned short int merge = raw_data & wildcard;
    return merge;
}

void setup() {
    /*
    Default function of Arduino
    */
   // Open Serial
   Serial.begin(baudrate);
   //Set High level on Reset port
   digitalWrite(reset, HIGH);
   //Activating send data mode on pins
   pinMode(green, OUTPUT);
   pinMode(yellow, OUTPUT);
   pinMode(reset, OUTPUT);
   //Enable DHT
   dht.begin();
   //Power on photoresistor
   digitalWrite(photores_power, HIGH);
}


void loop() {
    if (Serial.available()) {// получение данных с клиента
    inc = Serial.parseInt();
    Serial.flush();
    Serial.println(inc); //debug info
    push_data(HWAddr(inc), HWData(inc));
    }
    pir_sensor(); // контроль присутствия
}
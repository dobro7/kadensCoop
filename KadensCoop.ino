/*
* Copyright 2017, James Clark (http://clarksite.com)
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* 
* This project was for my son, Kaden, and based off Dave Naves project (http://davenaves.com)
*Many thanks to Dave and everyone out there in the Arduino world for sharing.
* I'm hoping that if you use/modify this code, you will share your
* coop project - as I have done here.
* Dave Naves & Kaden are big on sharing.
*/

#include <DallasTemperature.h>
#include <SimpleTimer.h>                     // load the SimpleTimer library to make timers, instead of delays & too many millis statements
#include <OneWire.h>                         // load the onewire library for temp sensors
#include <DallasTemperature.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <FastIO.h>
#include <I2CIO.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // RESERVED MAC ADDRESS
EthernetClient client;

#define ONE_WIRE_BUS_PIN 2                //Digital pin 2 for one wire sensors
OneWire oneWire(ONE_WIRE_BUS_PIN);        // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);      // Tell Dallas Temperature Library to use oneWire Library
// Define the address of each sensor below - tutorial can be found here:
DeviceAddress coopTemp = { 0x28, 0xEE, 0x56, 0xFE, 0x1D, 0x16, 0x02, 0xE9 };      // Inside temp sensor
DeviceAddress runTemp = { 0x28, 0xEE, 0x75, 0xC9, 0x20, 0x16, 0x01, 0xBD };       // Outside temp sensor


/*----------------------------( Pin Assignments )-----------------------------*/
const int photocellPin = A0;                 // photocell connected to analog 1
const int relayHeat = 5;                     // heat lamp relay set to digital pin 5
const int relayFan = 6;                      // exhaust fan relay set to digital pin 6
const int enableCoopDoorMotorB = 7;          // enable motor b - pin 7
const int directionCloseCoopDoorMotorB = 8;  // direction close motor b - pin 8
const int directionOpenCoopDoorMotorB = 9;   // direction open motor b - pin 9
const int bottomSwitchPin = 29;              // bottom switch is connected to pin 26
const int topSwitchPin = 27;                 // top switch is connected to pin 27
const int coopDoorOpenLed = 40;              // led set to digital pin 40
const int coopDoorClosedLed = 41;            // led set to digital pin 41                                   
const int relayInteriorLight = 45;           // interior lights relay set to digital pin 45
const int waterHeaterRelay = 46;           // interior lights relay set to digital pin 45

/*----------------------------( Variables )-----------------------------*/
String doorState = "";                       // Values will be: closed, closing, open, opening
String doorStatePrev = "";                   // We will need to know this for determining opening or closing
String fanState = "";
String heatState = "";
String lightState = "";
String waterHeaterState = "";
String data;
float tInside;
float tOutside;
long previousMillis = 0;
unsigned long currentMillis = 0;
long interval = 120000; // READING INTERVAL
long interval2 = 700000; // DATA INTERVAL
int photocellReading;                  // analog reading of the photocel
int photocellReadingLevel;             // photocel reading levels (dark, twilight, light)
int topSwitchPinVal;                   // top switch var for reading the pin status
int topSwitchPinVal2;                  // top switch var for reading the pin delay/debounce status
int topSwitchState;                    // top switch var for to hold the switch state
int bottomSwitchPinVal;                // bottom switch var for reading the pin status
int bottomSwitchPinVal2;               // bottom switch var for reading the pin delay/debounce status
int bottomSwitchState;                 // bottom switch var for to hold the switch state
SimpleTimer coopPhotoCellTimer;
long lastDebounceTime = 0;
long debounceDelay = 100;
long lastTempCheckTime = 0;
long TempCheckDelay = 600000;           // 10 minutes
long lastTwilightTime = 0;
long TwilightDelay = 300000;           // 5 minutes

/*----------------------------( The Setup )-----------------------------*/
void setup(void) {
  Serial.begin(9600);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  }
  pinMode(10, OUTPUT);
  digitalWrite(10,HIGH);                            // disable w5100 SPI
  sensors.begin();                                  // Start up the DallasTemperature library 
  sensors.setResolution(coopTemp, 10);            // Inside temperature
  sensors.setResolution(runTemp, 10);             // Outside temperature
  data = "";
  pinMode(topSwitchPin, INPUT_PULLUP);             // Since the other end of the reed switch is connected to ground, we need to pull-up the reed switch pin internally.
  pinMode(bottomSwitchPin, INPUT_PULLUP);
  Serial.println(" Processes running:");
  pinMode(relayHeat, OUTPUT);                   //set heat lamp relay output
  pinMode(relayFan, OUTPUT);                    //set exhaust fan relay output 
  pinMode (enableCoopDoorMotorB, OUTPUT);           // enable motor pin = output
  pinMode (directionCloseCoopDoorMotorB, OUTPUT);   // motor close direction pin = output
  pinMode (directionOpenCoopDoorMotorB, OUTPUT);    // motor open direction pin = output
  pinMode (coopDoorOpenLed, OUTPUT);                // enable coopDoorOpenLed = output
  pinMode (coopDoorClosedLed, OUTPUT);              // enable coopDoorClosedLed = output
  pinMode(bottomSwitchPin, INPUT);                  // set bottom switch pin as input
  digitalWrite(bottomSwitchPin, HIGH);              // activate bottom switch resistor
  pinMode(topSwitchPin, INPUT);                     // set top switch pin as input
  digitalWrite(topSwitchPin, HIGH);                 // activate top switch resistor  
  pinMode(relayInteriorLight, OUTPUT);
  digitalWrite(relayInteriorLight, HIGH);
  pinMode(waterHeaterRelay, OUTPUT);
  digitalWrite(waterHeaterRelay, HIGH);
  coopPhotoCellTimer.setInterval(5000, readPhotoCell);   // read the photocell every 10 minutes 600000
}

/*----------------------------( Functions )-----------------------------*/
// if cold, turn on heat lamps
void doCoopHVACHeat() {
  float inTemp = sensors.getTempF(coopTemp);         // create inside temp variable (inTemp)
  if ((millis() - lastTempCheckTime) > TempCheckDelay) {    // check temperature every 10 minutes
    if (inTemp <= 40) {                                      // if temp drops below 40F turn on heat lamp(s) relay
      digitalWrite(relayHeat, LOW); 
      heatState = "Heater On";
    }
    else if (inTemp > 40) {
      digitalWrite(relayHeat, HIGH);                        // if temp remains above 40F turn off heat lamp(s) relay
      heatState = "Heater Off";
    }
  }
}

// if hot, turn on cooling fans
void doCoopHVACCool() {
  float outTemp = sensors.getTempF(runTemp);         // create outside temp variable (outTemp)
  if ((millis() - lastTempCheckTime) > TempCheckDelay) {    // check temperature every 10 minutes
    if (outTemp >= 90) {                                      // if temp rises above 85F turn on cooling fan(s) relay
      digitalWrite(relayFan, LOW);
      fanState = "Fan On";
  }
    else if (outTemp < 90) {
      digitalWrite(relayFan, HIGH);
      fanState = "Fan Off";
    }
  }
}

// photocell to read levels of exterior light
void readPhotoCell() {                            // function to be called repeatedly - per coopPhotoCellTimer set in setup
  photocellReading = analogRead(photocellPin);
  //  set photocell threshholds
  if (photocellReading >= 0 && photocellReading <= 3) {
    photocellReadingLevel = '1';
    }  
  else if (photocellReading  >= 4 && photocellReading <= 120){
    photocellReadingLevel = '2';
  }  
  else if (photocellReading  >= 125 ) {
    photocellReadingLevel = '3';
  }
}

//debounce bottom reed switch
void debounceBottomReedSwitch() { 
  bottomSwitchPinVal = digitalRead(bottomSwitchPin);       // read input value and store it in val
  if ((millis() - lastDebounceTime) > debounceDelay) {    // delay 10ms for consistent readings
    bottomSwitchPinVal2 = digitalRead(bottomSwitchPin);    // read input value again to check or bounce
    if (bottomSwitchPinVal == bottomSwitchPinVal2) {       // make sure we have 2 consistant readings
      if (bottomSwitchPinVal != bottomSwitchState) {       // the switch state has changed!
        bottomSwitchState = bottomSwitchPinVal;
      }
    }
  }
}

// debounce top reed switch
void debounceTopReedSwitch() {
  topSwitchPinVal = digitalRead(topSwitchPin);             // read input value and store it in val
  if ((millis() - lastDebounceTime) > debounceDelay) {     // delay 10ms for consistent readings
    topSwitchPinVal2 = digitalRead(topSwitchPin);          // read input value again to check or bounce
    if (topSwitchPinVal == topSwitchPinVal2) {             // make sure we have 2 consistant readings
      if (topSwitchPinVal != topSwitchState) {             // the button state has changed!
        topSwitchState = topSwitchPinVal;
      }
    }
  }
}

// stop the coop door motor
void stopCoopDoorMotorB(){
  digitalWrite (directionCloseCoopDoorMotorB, LOW);      // turn off motor close direction
  digitalWrite (directionOpenCoopDoorMotorB, LOW);       // turn on motor open direction
  analogWrite (enableCoopDoorMotorB, 0);                 // enable motor, 0 speed
}

// close the coop door motor (motor dir close = clockwise) 
void closeCoopDoorMotorB() {  
  digitalWrite (directionCloseCoopDoorMotorB, HIGH);     // turn on motor close direction
  digitalWrite (directionOpenCoopDoorMotorB, LOW);       // turn off motor open direction
  analogWrite (enableCoopDoorMotorB, 255);               // enable motor, full speed 
  if (bottomSwitchPinVal == 0) {                         // if bottom reed switch circuit is closed
    stopCoopDoorMotorB();
    doorState = "Door Closed";
  }
}

// open the coop door (motor dir open = counter-clockwise)
void openCoopDoorMotorB() { 
  digitalWrite(directionCloseCoopDoorMotorB, LOW);       // turn off motor close direction
  digitalWrite(directionOpenCoopDoorMotorB, HIGH);       // turn on motor open direction
  analogWrite(enableCoopDoorMotorB, 255);                // enable motor, full speed
  if (topSwitchPinVal == 0) {                            // if top reed switch circuit is closed
    stopCoopDoorMotorB();
    doorState = "Door Open";
  }
}

//  coop door status: red if open, green if closed, blinking red if stuck 
void doCoopDoorLed() {
  if (bottomSwitchPinVal == 0) {                         // if bottom reed switch circuit is closed
    digitalWrite (coopDoorClosedLed, 255);              // turns on coopDoorClosedLed (green)
    digitalWrite (coopDoorOpenLed, 0);                 // turns off coopDoorOpenLed (red)
  }
  else if(topSwitchPinVal == 0) {                        // if top reed switch circuit is closed 
    digitalWrite (coopDoorClosedLed, 0);               // turns off coopDoorClosedLed (green)
    digitalWrite (coopDoorOpenLed, 255);                // turns on coopDoorOpenLed (red)
  }
  else {
    digitalWrite (coopDoorClosedLed, 0);              // turns off coopDoorClosedLed (green)
    digitalWrite (coopDoorOpenLed, 0);                // turns off coopDoorOpenLed (red)
  }
}

//  turn on interior lights at dusk and turn off after door shuts

void doCoopInteriorLightDusk() {
  if ((millis() - lastTwilightTime) > TwilightDelay) {     // delay 5 mins
    readPhotoCell();
    bottomSwitchPinVal = digitalRead(bottomSwitchPin);
    if (bottomSwitchPinVal == 1 && photocellReading  >= 4 && photocellReading <= 120) {   // if bottom reed switch circuit is open and it's twilight
     digitalWrite (relayInteriorLight, LOW);
    lightState = "Light On";
    }
    else if (photocellReading  >= 125) {   // if bright out turn off light
      digitalWrite (relayInteriorLight, HIGH);
    lightState = "Light Off";
    }
  }
}   


// do the coop door
void doCoopDoor(){
  if (photocellReadingLevel  == '1') {              // if it's dark
    if (photocellReadingLevel != '2') {             // if it's not twilight
      if (photocellReadingLevel != '3') {           // if it's not light 
        debounceTopReedSwitch();                    // read and debounce the switches
        debounceBottomReedSwitch();
        closeCoopDoorMotorB();                      // close the door
      }
    }
  } 
  if (photocellReadingLevel  == '3') {              // if it's light
    if (photocellReadingLevel != '2') {             // if it's not twilight
      if (photocellReadingLevel != '1') {           // if it's not dark 
        debounceTopReedSwitch();                    // read and debounce the switches
        debounceBottomReedSwitch();
        openCoopDoorMotorB();                       // Open the door
      }
    }
  }
}

void getWebData(){
  tInside = sensors.getTempF(coopTemp); 
  tOutside = sensors.getTempF(runTemp);
    data = "temp1=";
    data.concat(tInside);
    data.concat("&temp2=");
    data.concat(tOutside);
    data.concat("&doorStatus=");
    data.concat(doorState);
    data.concat("&photocellStatus=");
    data.concat(photocellReading);
    data.concat("&heatStatus=");
    data.concat(heatState);
    data.concat("&fanStatus=");
    data.concat(fanState);
    data.concat("&lightStatus=");
    data.concat(lightState);
    data.concat("&waterHeaterStatus=");
    data.concat(waterHeaterState); 
}
/*----------------------------( The Loop )-----------------------------*/
void loop() {  
  currentMillis = millis();
  sensors.requestTemperatures(); // Send the command to get temperatures
  coopPhotoCellTimer.run();      // timer for readPhotoCell
  getWebData();
  doCoopHVACCool();
  doCoopHVACHeat();
  doCoopDoor();
  doCoopDoorLed();
  doCoopInteriorLightDusk();
  tInside = sensors.getTempF(coopTemp); 
  tOutside = sensors.getTempF(runTemp);
    data = "temp1=";
    data.concat(tInside);
    data.concat("&temp2=");
    data.concat(tOutside);
    data.concat("&doorStatus=");
    data.concat(doorState);
    data.concat("&photocellStatus=");
    data.concat(photocellReading);
    data.concat("&heatStatus=");
    data.concat(heatState);
    data.concat("&fanStatus=");
    data.concat(fanState);
    data.concat("&lightStatus=");
    data.concat(lightState);
    data.concat("&waterHeaterStatus=");
    data.concat(waterHeaterState); 
/* ----------------------------------------(begin web transmission )---------------------- */
if(currentMillis - previousMillis > interval2) { // DO ONLY ONCE PER INTERVAL
    previousMillis = currentMillis;
if (client.connect("yoursite.com",80)) {
    client.print("POST /addpost.php HTTP/1.1\n");
    client.print("Host: www.yoursite.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\n\n");
    client.print(data);
    }
  }
if (client.connected()) { 
    client.stop();  // DISCONNECT FROM THE SERVER
    }
}

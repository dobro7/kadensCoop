/*
* Copyright 2017, James Clark (http://clarksite.com & http://Redyetti.com)
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

#include <SimpleTimer.h>                     // load the SimpleTimer library to make timers, instead of delays & too many millis statements
#include <OneWire.h>                         // load the onewire library for temp sensors
#include <FastIO.h>
#include <I2CIO.h>							             // Get the LCD I2C Library here:  https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
#include <LCD.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_SR2W.h>
#include <LiquidCrystal_SR3W.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
// print debug messages or not to serial 
const boolean SerialDisplay = true;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // RESERVED MAC ADDRESS
EthernetClient client;

#define ONE_WIRE_BUS_PIN 2				        //Digital pin 2 for one wire sensors
OneWire oneWire(ONE_WIRE_BUS_PIN);		    // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);	    // Tell Dallas Temperature Library to use oneWire Library
// Define the address of each sensor below - tutorial can be found here:
DeviceAddress coopTemp = { 0x28, 0xEE, 0x56, 0xFE, 0x1D, 0x16, 0x02, 0xE9 };      // Inside temp sensor
DeviceAddress runTemp = { 0x28, 0xEE, 0x75, 0xC9, 0x20, 0x16, 0x01, 0xBD };       // Outside temp sensor

// pins assignments
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
String doorState = "";                       // Values will be: closed, closing, open, opening
String doorStatePrev = "";                   // We will need to know this for determining opening or closing
String fanState = "";
String heatState = "";
String lightState = "";
// Water Heater Relay (Run_temp < 45F turn relay on) (Run_temp =>50F turn relay off)
// Water Circulation Pump Relay (IF heater_relay is ON then wait 10 minutes and turn on circulation pump)(else if heater_relay is off wait 10 minutes and turn off circulation)
// Water Level High Detection (When triggered turn off solenoid valve)
// Water Level Low Detection ( When triggered turn on solenoid valve) AND (send message 
// Water Solenoid Valve (Turn on when low_level for X time then off) (if High level do not turn on)
// Feeder Motor (turn on for 15 seconds once a day - NEED RTC?)
// Feed Low Alarm (Send alarm to phone/web to get more feed)

/*-----( Web Data Variables )-----*/
    

   String data;
   float tInside;
   float tOutside;
   long previousMillis = 0;
   unsigned long currentMillis = 0;
   long interval = 120000; // READING INTERVAL
   long interval2 = 700000; // DATA INTERVAL
   
// photocell
int photocellReading;                  // analog reading of the photocel
int photocellReadingLevel;             // photocel reading levels (dark, twilight, light)

// top switch

int topSwitchPinVal;                   // top switch var for reading the pin status
int topSwitchPinVal2;                  // top switch var for reading the pin delay/debounce status
int topSwitchState;                    // top switch var for to hold the switch state

// bottom switch

int bottomSwitchPinVal;                // bottom switch var for reading the pin status
int bottomSwitchPinVal2;               // bottom switch var for reading the pin delay/debounce status
int bottomSwitchState;                 // bottom switch var for to hold the switch state

// SimpleTimer objects
SimpleTimer coopPhotoCellTimer;

// debounce delay
long lastDebounceTime = 0;
long debounceDelay = 100;


// temperature check delay
long lastTempCheckTime = 0;
long TempCheckDelay = 600000;           // 10 minutes


// interior lights twighlight delay
long lastTwilightTime = 0;
long TwilightDelay = 300000;           // 5 minutes

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


// ************************************** the setup **************************************

void setup(void) {

  Serial.begin(9600);                               // initialize serial port hardware
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  }
  lcd.begin(20,4);                                 // initialize the lcd for 20 chars 4 lines, turn on backlight 
  sensors.begin(); 		                             // Start up the DallasTemperature library 
  sensors.setResolution(coopTemp, 10);            // Inside temperature
  sensors.setResolution(runTemp, 10);	            // Outside temperature
  data = "";
  lcd.backlight();                                 //backlight on 
  pinMode(topSwitchPin, INPUT_PULLUP);             // Since the other end of the reed switch is connected to ground, we need to pull-up the reed switch pin internally.
  pinMode(bottomSwitchPin, INPUT_PULLUP);
  // welcome message
  if(SerialDisplay){
    Serial.println(" Processes running:");
  //  Serial.println(" Timer readPhotoCell every 10 minutes - light levels: open or close door");
  }
  
  // coop hvac
  pinMode(relayHeat, OUTPUT);                   //set heat lamp relay output
  pinMode(relayFan, OUTPUT);                    //set exhaust fan relay output 
  
  // coop door  
  // coop door motor
  pinMode (enableCoopDoorMotorB, OUTPUT);           // enable motor pin = output
  pinMode (directionCloseCoopDoorMotorB, OUTPUT);   // motor close direction pin = output
  pinMode (directionOpenCoopDoorMotorB, OUTPUT);    // motor open direction pin = output

  // coop door leds
  pinMode (coopDoorOpenLed, OUTPUT);                // enable coopDoorOpenLed = output
  pinMode (coopDoorClosedLed, OUTPUT);              // enable coopDoorClosedLed = output

  // coop door switches
  // bottom switch
  pinMode(bottomSwitchPin, INPUT);                  // set bottom switch pin as input
  digitalWrite(bottomSwitchPin, HIGH);              // activate bottom switch resistor

  // top switch
  pinMode(topSwitchPin, INPUT);                     // set top switch pin as input
  digitalWrite(topSwitchPin, HIGH);                 // activate top switch resistor  

  // interior lights relay
  pinMode(relayInteriorLight, OUTPUT);
  digitalWrite(relayInteriorLight, HIGH);

  // timed actions setup
  coopPhotoCellTimer.setInterval(5000, readPhotoCell);   // read the photocell every 10 minutes 600000
  
  //-------- Write characters on the display ------------------
  lcd.setCursor(1,0);                                   //Start at character 4 on line 0
  lcd.print("Welcome to Kaden's");
  lcd.setCursor(2,1);
  lcd.print("Automated Coop");
  lcd.setCursor(0,3);
  lcd.print("Requesting data...");
  delay(4000);
  lcd.clear();                                        // clear the screen

  if (client.connect("redyetti.com",80)) {
    client.print("POST /kaden/ardpost.php HTTP/1.1\n");
    client.print("Host: www.redyetti.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\n\n");
    client.print(data);
  }
}

// ************************************** functions **************************************

// coop hvac

void doCoopHVACHeat() {

  float inTemp = sensors.getTempF(coopTemp);         // create inside temp variable (inTemp)
  if ((millis() - lastTempCheckTime) > TempCheckDelay) {    // check temperature every 10 minutes

    // if cold, turn on heat lamps
    if (inTemp <= 40) {                                      // if temp drops below 40F turn on heat lamp(s) relay
      digitalWrite(relayHeat, HIGH); 
      heatState = "Heater On";
    }
    else if (inTemp > 40) {
      digitalWrite(relayHeat, LOW);                        // if temp remains above 40F turn off heat lamp(s) relay
      heatState = "Heater Off";
    }
  }
}
// if hot, turn on cooling fans

void doCoopHVACCool() {

  float outTemp = sensors.getTempF(runTemp);         // create outside temp variable (outTemp)
  if ((millis() - lastTempCheckTime) > TempCheckDelay) {    // check temperature every 10 minutes
    if (outTemp >= 90) {                                      // if temp rises above 85F turn on cooling fan(s) relay
      digitalWrite(relayFan, HIGH);
    fanState = "Fan On";
  }
    }

    else if (outTemp < 90) {
      digitalWrite(relayFan, LOW);
      fanState = "Fan Off";
    }
  }


// operate the coop door

// photocell to read levels of exterior light

void readPhotoCell() {                            // function to be called repeatedly - per coopPhotoCellTimer set in setup
  photocellReading = analogRead(photocellPin);
  //  set photocell threshholds
  if (photocellReading >= 0 && photocellReading <= 3) {
    photocellReadingLevel = '1';
  //    Serial.println(" - Dark");
 
  }  
  else if (photocellReading  >= 4 && photocellReading <= 120){
    photocellReadingLevel = '2';
   //   Serial.println(" - Twilight");
  }  
  else if (photocellReading  >= 125 ) {
    photocellReadingLevel = '3';
 //     Serial.println(" - Light");
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
//        Serial.print (" Bottom Switch Value: ");           // display "Bottom Switch Value:" 
//        Serial.println(digitalRead(bottomSwitchPin));      // display current value of bottom switch;
    }
  }
}

// debounce top reed switch
void debounceTopReedSwitch() {

  topSwitchPinVal = digitalRead(topSwitchPin);             // read input value and store it in val
  //  delay(10);

  if ((millis() - lastDebounceTime) > debounceDelay) {     // delay 10ms for consistent readings

    topSwitchPinVal2 = digitalRead(topSwitchPin);          // read input value again to check or bounce

    if (topSwitchPinVal == topSwitchPinVal2) {             // make sure we have 2 consistant readings
      if (topSwitchPinVal != topSwitchState) {             // the button state has changed!
        topSwitchState = topSwitchPinVal;
      }
//        Serial.print (" Top Switch Value: ");              // display "Bottom Switch Value:" 
//        Serial.println(digitalRead(topSwitchPin));         // display current value of bottom switch;
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
      digitalWrite (relayInteriorLight, HIGH);
    lightState = "Light On";
    }
    else if (bottomSwitchPinVal == 0) {
      digitalWrite (relayInteriorLight, LOW);
    lightState = "Light On";
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

void displayTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);

   if (tempC == -127.00) // Measurement failed or no device found
   {
    lcd.print("Temperature Error");
   } 
   else
   {
   lcd.print(DallasTemperature::toFahrenheit(tempC)); // Convert to F
   }
}

// ************************************** the loop **************************************
void loop() {  
  sensors.requestTemperatures(); // Send the command to get temperatures
  coopPhotoCellTimer.run();      // timer for readPhotoCell
  doCoopHVACCool();
  doCoopHVACHeat();
  doCoopDoor();
  doCoopDoorLed();
  doCoopInteriorLightDusk();
  tInside = sensors.getTempF(coopTemp); 
  tOutside = sensors.getTempF(runTemp);
   lcd.setCursor(0, 0);
   lcd.print("Coop: ");
   displayTemperature(coopTemp);    // display the coop temperature
   lcd.print(" F ");
   lcd.setCursor(0,1);   // move cursor to next line
   lcd.print("Run: ");
   displayTemperature(runTemp);      // display the run temperature
   lcd.print(" F ");
   lcd.setCursor(0,2);   // move cursor to next line
   if (photocellReading >= 0 && photocellReading <= 3) {
    photocellReadingLevel = '1';
      lcd.print("Dark     ");
    } else if (photocellReading  >= 4 && photocellReading <= 120){
    photocellReadingLevel = '2';
      lcd.print("Twilight");
    }else if (photocellReading  >= 125 ) {
    photocellReadingLevel = '3';
      lcd.print("Light    ");
    }
   lcd.setCursor(0,3);   // move cursor to next line
   if (topSwitchPinVal == 0) {            // display the door position
    lcd.print("Coop door open!!!  ");
   } else if (bottomSwitchPinVal == 0){
    lcd.print("Coop door closed   ");
   } else {
    lcd.print("Coop door traveling");
   }
   currentMillis = millis();
//  if(currentMillis - previousMillis > interval) { // READ ONLY ONCE PER INTERVAL
//    previousMillis = currentMillis;
//  tInside = 10;
//  tOutside = 10;
//  }
    data = "temp1=";
    data.concat(tInside);
    data.concat("&temp2=");
    data.concat(tOutside);
    data.concat("&doorStatus=");
    data.concat(doorState);
    data.concat("&photocellStatus=");
    data.concat(photocellReadingLevel);
    data.concat("&heatStatus=");
    data.concat(heatState);
    data.concat("&fanStatus=");
    data.concat(fanState);
    data.concat("&lightStatus=");
    data.concat(lightState);


    
/* --(begin web data )-- */
if(currentMillis - previousMillis > interval2) { // DO ONLY ONCE PER INTERVAL
    previousMillis = currentMillis;
if (client.connect("redyetti.com",80)) {
    client.print("POST /kaden/ardpost.php HTTP/1.1\n");
    client.print("Host: www.redyetti.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\n\n");
    client.print(data);
    Serial.print("POST /kaden/ardpost.php HTTP/1.1\n");
    Serial.print("Host: www.redyetti.com\n");
    Serial.print("Connection: close\n");
    Serial.print("Content-Type: application/x-www-form-urlencoded\n");
    Serial.print("Content-Length: ");
    Serial.print(data.length());
    Serial.print("\n\n");
    Serial.print(data);
    Serial.println("Successfull");
}/* --(end web data )-- */
 }
if (client.connected()) { 
    client.stop();  // DISCONNECT FROM THE SERVER
 
}
   } /*--(end loop )---*/

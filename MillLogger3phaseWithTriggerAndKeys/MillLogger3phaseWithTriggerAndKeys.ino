//#include <SdFat.h>  //Optional SD Library in case of stack heap collisions et cetera
//SdFat SD;           //(I think this requires FAT16 though)
#include "Arduino.h"
//#include "Keypad.h"
#include <OnewireKeypad.h>
#include <SD.h> //Original SD Library
#include <SPI.h> //Load SPI Library - MAY NOT NEED
#include <SoftwareSerial.h> //Load the Software Serial library
#include <RTClib.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
RTC_DS1307 RTC;            //define logger RTC object
#define chipSelect 10 //For SD logger. BTW- SD logger uses digital pins 10-13 and analog 4-5
#define LED   13   //Arduino onboard LED pin
#define grnLED 3   //green LED on logger
#define redLED 4   //red LED on logger
#define KeyPin A3
#define PIN 8 //RGB LED pin #
#define NUMPIXELS 1 //RGB LEDs
#define NUM_READS 100 //number of reads for mode averaged filtering
#define LOG_INTERVAL 499 //How long do we want between log intervals (in ms)?
                         //To make this accurate, see the millis column to confirm. 
                         //For example, 999ms might = 1 sec if it takes the Arduino
                         //1ms to do its reading and recording. So, the time it takes
                         //to take/record should be subtracted from the log interval.
File logFile; //Data object you will write your sensor data to
SoftwareSerial mySerial(6, 7); //Initialize the Software Serial port
uint32_t timer = millis(); //Sets milliseconds since Arduino on timer
int name_temp = 0;
float avgAmps = 0;
//******BEGIN RGB LED SETUP************
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//COLOR REFERENCE (R,G,B):
// black[3]  = { 0, 0, 0 };
// white[3]  = { 100, 100, 100 };
// red[3]    = { 100, 0, 0 };
// green[3]  = { 0, 100, 0 };
// blue[3]   = { 0, 0, 100 };
// yellow[3] = { 40, 95, 0 };
// dimWhite[3] = { 30, 30, 30 };
//EXAMPLE OF NEOPIXEL RGB LED:
//pixels.setPixelColor(0, pixels.Color(0,150,0)); // Moderately bright green color.
//pixels.show(); // This sends the updated pixel color to the hardware.
//*********END RGB LED SETUP

#define Sensor1 14 //Sensor1 uses pin A0
#define Sensor2 15 //Sensor2 uses pin A1
#define Sensor3 16 //Sensor3 uses pin A2
//#define takerinPin 2 //takerin uses pin Digital 2
int takerinPin = 2;
int takerinVal = 0;
int stopbuttonPin = 5; //big red stop button uses digital 5
int stopbuttonVal = 0;
int startbuttonPin = 9; //big black stop button uses digital 9
int startbuttonVal = 0;
int logreadingsVal = 0; //When this == 1, the readings will log to SD

//*******BEGIN KEYPAD SETUP****************
#define Rows 4
#define Cols 4
#define Row_Res 4700
#define Col_Res 1000
char KEYS[]= {
  '1','2','3','A',
  '4','5','6','B',
  '7','8','9','C',
  '*','0','#','D'
};
OnewireKeypad <Print, 16> Keypad(Serial, KEYS, Rows, Cols, KeyPin, Row_Res, Col_Res );
int j=0;
char name[20]; //this stores the entered key number
//***********END KEYPAD SETUP****************

void setup(){
  Serial.begin(57600);
  Wire.begin();
  RTC.begin();
  pinMode(LED,OUTPUT);
  pinMode(grnLED,OUTPUT);
  pinMode(redLED,OUTPUT); 
  pinMode(10, OUTPUT);  //this is for the SD logger
  pinMode(takerinPin, INPUT); //takerin pin
  pinMode(stopbuttonPin, INPUT); //stop button pin
  pinMode(startbuttonPin, INPUT);//start button pin
  Keypad.addEventKey(starKey, '*'); //adds function to keypad library to look for *key
  Keypad.addEventKey(poundKey, '#');
  //********BEGINRGB LED SETUP AND INITIAL VALUE***************//
  pixels.begin(); // This initializes the RGB NeoPixel library.
  pixels.setPixelColor(0, pixels.Color(0,5,0));
  pixels.show();
  //*******END RGB SETUP AND INITIAL VALUE*******************//
  
  //**************SD Logger File Setup***********************//
  //*To do: Make this a function instead.
  //see if the card is present and can be initialized. Spinlock if it fails.
  if (!SD.begin(chipSelect)) {
    digitalWrite(redLED, HIGH); 
    pixels.setPixelColor(0, pixels.Color(255,00,0));
    pixels.show();
    while(1){};
  }
   // create a NEW file
  char filename[] = "LOGGER00.CSV";     //INPUT a default filename here, replace LOGGER with something
    for (uint8_t i = 0; i < 100; i++) { //and make sure its the same legnth, like COTTON, not MILL 
      filename[6] = i/10 + '0';         //but MILLXX would work since 6 letters. You get the point (I hope).
      filename[7] = i%10 + '0';
      if (! SD.exists(filename)) {
        // only open a new file if it doesn't exist
        logFile = SD.open(filename, FILE_WRITE); 
        break;  // leave the loop!
       }
  }
  //open up the logFile, spinlock if it fails.
  logFile = SD.open(filename, FILE_WRITE);
  if( ! logFile ) {
    digitalWrite(redLED,HIGH);
    pixels.setPixelColor(0, pixels.Color(255,5,0));
    pixels.show();
    while(1){};
  }
  //Print header to file, keep this in order of what data you log in the loop.
  //This would be your first row on your Excel spreadsheet
  //Uncomment and enter comma separated values to your needs.
  logFile.println(F("run,millis,Date?,UnixTime,Sensor1,Sensor2,Sensor3,SensorAVG,AmpsAVG"));  

  //*************END SD Logger File Setup**********************//
}//END SETUP

void loop(){
Starter:
//********BEGIN KEYPAD STUFF***************

   char customKey  = Keypad.Getkey();
   char customKey1 = Keypad.Getkey();
   char customKey2 = Keypad.Getkey();
   char customKey3 = Keypad.Getkey();
   char customKey4 = Keypad.Getkey();

if (customKey==customKey1 && customKey1==customKey2 && customKey2==customKey3 && customKey3==customKey4)
   char customKey = customKey1;
else
   char customKey = '\0';

    int noiseCheck = analogRead(A3);

    if(customKey==NO_KEY) name_temp = 0;

   Keypad.ListenforEventKey(); // check if an EventKey is found
   if(customKey!=NO_KEY){  // Check for a valid key.
    if(noiseCheck > 50){
    if(customKey!='*'){
//      if(customKey!='D'){
        if(name_temp!=9999){
            pixels.setPixelColor(0, pixels.Color(150,0,0)); pixels.show();
             name_temp=customKey;
             name[j]=name_temp;
             name_temp=9999;
             delay(10);
           switch (j){
            case 20:
            j--;
            break;
            default:
            j++;
            break;
           }
            //increase this # along with name[j] array size because
            //this IF stops it from going >name[j]
            
        }
//      }
      }
    }
    if(j>1) {
      pixels.setPixelColor(0, pixels.Color(10,5,0)); pixels.show();
    }
    else {
    pixels.setPixelColor(0, pixels.Color(0,5,0)); pixels.show();
   }
delay(5);
    pixels.setPixelColor(0, pixels.Color(0,5,0)); pixels.show();
     }
//********END KEYPAD STUFF*****************
 
  
    takerinVal = digitalRead(takerinPin);
    stopbuttonVal = digitalRead(stopbuttonPin);
    startbuttonVal = digitalRead(startbuttonPin);
    if(stopbuttonVal==1) {
        pixels.setPixelColor(0, pixels.Color(255,0,0)); pixels.show();
        stopbuttonVal = digitalRead(stopbuttonPin);
    }
    if(startbuttonVal==1) {
        pixels.setPixelColor(0, pixels.Color(0,255,0)); pixels.show();
        startbuttonVal = digitalRead(startbuttonPin);
    }
    if(digitalRead(startbuttonPin)==1) logreadingsVal=1;
    if(digitalRead(stopbuttonPin)==1)  logreadingsVal=0;
    
//      pixels.setPixelColor(0, pixels.Color(0,5,0)); pixels.show();
// approximately every LOG_INTERVAL or so, print out the current stats
  if (millis() - timer > LOG_INTERVAL) { 
  timer = millis(); // reset the timer
  DateTime now;    //this retrieves a new date/time
  now = RTC.now(); //fetch RTC time

//****Begin Sensor Reading****//
//To do: make this a function
    int SensorADC1 = analogRead(Sensor1);
    int SensorADC2 = analogRead(Sensor2);
    int SensorADC3 = analogRead(Sensor3);
//    int SensorADC5 = analogRead(Sensor5);
//    int SensorADC6 = analogRead(Sensor6);     

//****End Sensor Reading****//

  if(logreadingsVal==1) {
  digitalWrite(grnLED,HIGH); //Set onboard LED HIGH while logging
  pixels.setPixelColor(0, pixels.Color(0,0,10)); pixels.show();
  switch (customKey){
    case '*':
      if(j>1){
    starKey();
      }
                 logFile.print(F(", "));
    break;
    default:
    logFile.print(F(", "));
    break;
  }
  
//  if(customKey!='*') logFile.print(", ");
//  else starKey();
//  Keypad.ListenforEventKey(); // check if an EventKey is found
  logFile.print(millis());              //millis since Arduino on
  logFile.print(F(", "));               //comma and space for Comma Separated Values
  logFile.print(now.unixtime());        //Epoch time: seconds since 1/1/1970 cell- 'stamp' unix epoch time (in Excel try =CELL/(60*60*24)+"1/1/1970" to convert)
  logFile.print(F(", "));
  logFile.print(now.hour(), DEC);       //RTD hour:minute:second time cell
  logFile.print(F(":"));
  logFile.print(now.minute(), DEC);     //RTD hour:minute:second time cell
  logFile.print(F(":"));
  logFile.print(now.second(), DEC);     //RTD hour:minute:second time cell
  logFile.print(F(", "));
  logFile.print(SensorADC1);               //Sensor 1 (connected to A0)
  logFile.print(F(", "));
  logFile.print(SensorADC2);               //Sensor 2 (connected to A1)
  logFile.print(F(", "));
  logFile.print(SensorADC3);               //Sensor 3 (connected to A3)
  logFile.print(F(", "));
  float avgADC = (SensorADC1+SensorADC2+SensorADC3);
  avgADC = (avgADC/3);
  logFile.print(avgADC);
  logFile.print(F(", "));
  avgAmps = (avgADC*20);
  avgAmps = (avgAmps/1023);
  logFile.print(avgAmps);
  
  logFile.println("");   //go to next line
  logFile.flush();       //flush logFile cache after write for next read
  digitalWrite(grnLED,LOW); //Set onboard LED LOW when done logging
  pixels.setPixelColor(0, pixels.Color(0,5,0)); pixels.show();
 }
  }//END if (millis() - timer > LOG_INTERVAL)
}
//END loop

void starKey(){
           if(logreadingsVal==1){
           if (name_temp != 9999){
               name_temp = 9999;
//           logFile.println("The number entered is");
//           logFile.println(name);
           logFile.print(name);
//           logFile.print(F(", "));
           memset(name,'\0',j);
           j = 0;    
           }
           else {
            //  logFile.print(F(", ")); delay(1000);
            }
           }

           
//           delay(1);
           pixels.setPixelColor(0, pixels.Color(0,5,0)); pixels.show();
}
void poundKey(){
           memset(name,'\0',j);
           j = 0;  
}

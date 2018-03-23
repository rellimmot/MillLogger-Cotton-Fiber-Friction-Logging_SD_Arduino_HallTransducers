//#include <SdFat.h>  //Optional SD Library in case of stack heap collisions et cetera
//SdFat SD;           //(I think this requires FAT16 though)
#include <SD.h> //Original SD Library
#include <SPI.h> //Load SPI Library - MAY NOT NEED
#include <SoftwareSerial.h> //Load the Software Serial library
#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>
RTC_DS1307 RTC;            //define logger RTC object
//#define chipSelect 10 //for SD logger
#define LED   13   //Arduino onboard LED pin
#define grnLED 3   //green LED on logger
#define redLED 4   //red LED on logger
#define LOG_INTERVAL 999 //how long do we want between log intervals (in ms)?
File logFile; //Data object you will write your sensor data to
SoftwareSerial mySerial(6, 7); //Initialize the Software Serial port
uint32_t timer = millis(); //Sets milliseconds since Arduino on timer

#define Sensor1 A8 //Sensor1 uses pin A8
#define Sensor2 A9 //Sensor2 uses pin A9
#define Sensor3 A10 //Sensor3 uses pin A10
#define Sensor4 A11 //Sensor4 uses pin A11
#define Sensor5 A12 //Sensor5 uses pin A12
#define Sensor6 A13 //Sensor6 uses pin A13


void setup(){
  Serial.begin(57600);
  Wire.begin();
  RTC.begin();
    if (!RTC.begin()) {
    Serial.println("RTC failed");
    Serial.println("RTC failed");
}
  DateTime now;    //this retrieves a new date/time
  now = RTC.now(); //fetch RTC time
  Serial.println(now.second(), DEC);
  Serial.println("RTC Initialized");
  pinMode(LED,OUTPUT);
  pinMode(grnLED,OUTPUT);
  pinMode(redLED,OUTPUT); 
  pinMode(10, OUTPUT);  //this is for the SD logger
  //**************SD Logger File Setup***********************//
  //*To do: Make this a function instead.
  //see if the card is present and can be initialized. Spinlock if it fails.
  if (!SD.begin(10,11,12,13)) {
    Serial.println("Cant initialize card");
    digitalWrite(redLED, HIGH); 
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
    Serial.println("can't open logFile");
    digitalWrite(redLED,HIGH);
    while(1){};
  }
  //Print header to file, keep this in order of what data you log in the loop.
  //This would be your first row on your Excel spreadsheet
  //Uncomment and enter comma separated values to your needs.
  logFile.println(F("millis,Date,Time,Sensor1,Sensor2,Sensor3"));  

  //*************END SD Logger File Setup**********************//
    Wire.begin();  
 
    
  
}//END SETUP

void loop(){
  DateTime now;    //this retrieves a new date/time
  now = RTC.now(); //fetch RTC time
  
  // approximately every 5 seconds or so, print out the current stats
  if (millis() - timer > LOG_INTERVAL) { 
  timer = millis(); // reset the timer
//  DateTime now;    //this retrieves a new date/time
//  now = RTC.now(); //fetch RTC time

//****Begin Sensor Reading****//
//To do: make this a function
    int SensorADC1 = analogRead(Sensor1);
    int SensorADC2 = analogRead(Sensor2);
    int SensorADC3 = analogRead(Sensor3);  
    int SensorADC4 = analogRead(Sensor4);
    int SensorADC5 = analogRead(Sensor5);
    int SensorADC6 = analogRead(Sensor6);     

//****End Sensor Reading****//
//if (SensorADC6 > 210){ //only read when can is spinning, trigger is can motor- Sensor6


  digitalWrite(grnLED,HIGH); //Set onboard LED HIGH while logging
  
//  Serial.println("");   //go to next line

  Serial.print(millis());              //millis since Arduino on
  Serial.print(F(", "));               //comma and space for Comma Separated Values
  Serial.print(now.unixtime());        //Epoch time: seconds since 1/1/1970 cell- 'stamp' unix epoch time (in Excel try =CELL/(60*60*24)+"1/1/1970" to convert)
  Serial.print(F(", "));
  Serial.print(now.hour(), DEC);       //RTD hour:minute:second time cell
  Serial.print(F(":"));
  Serial.print(now.minute(), DEC);     //RTD hour:minute:second time cell
  Serial.print(F(":"));
  Serial.print(now.second(), DEC);     //RTD hour:minute:second time cell
  Serial.print(F(", "));
  Serial.print(SensorADC1);               //Sensor 1 (connected to A0)
  Serial.print(F(", "));
  Serial.print(SensorADC2);               //Sensor 2 (connected to A1)
  Serial.print(F(", "));
  Serial.print(SensorADC3);               //Sensor 3 (connected to A2)
  Serial.print(F(", "));
  Serial.print(SensorADC4);               //Sensor 3 (connected to A2)
  Serial.print(F(", "));
  Serial.print(SensorADC5);               //Sensor 3 (connected to A2)
  Serial.print(F(", "));
  Serial.print(SensorADC6);               //Sensor 3 (connected to A2)
  Serial.print(F(", "));

  float SensorAmps1 = ((SensorADC1*50));
  float SensorAmps2 = ((SensorADC2*50));
  float SensorAmps3 = ((SensorADC3*50));
  float SensorAmps4 = ((SensorADC4*20));
  float SensorAmps5 = ((SensorADC5*20));
  float SensorAmps6 = ((SensorADC6*20));
  SensorAmps1 = SensorAmps1/1023;
  SensorAmps2 = SensorAmps2/1023;
  SensorAmps3 = SensorAmps3/1023;
  SensorAmps4 = SensorAmps4/1023;
  SensorAmps5 = SensorAmps5/1023;
  SensorAmps6 = SensorAmps6/1023;
  Serial.print(SensorAmps1);               //Sensor 1 (connected to A0)
  Serial.print(F(", "));
  Serial.print(SensorAmps2);               //Sensor 2 (connected to A1)
  Serial.print(F(", "));
  Serial.print(SensorAmps3);               //Sensor 3 (connected to A2)
  Serial.print(F(", "));
  Serial.print(SensorAmps4);               //Sensor 3 (connected to A2)
  Serial.print(F(", "));
  Serial.print(SensorAmps5);               //Sensor 3 (connected to A2)
  Serial.print(F(", "));
  Serial.print(SensorAmps6);               //Sensor 3 (connected to A2)
  Serial.print(F(", "));
//  long map(long x, long in_min, long in_max, long out_min, long out_max)
//  {
//  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
//  }
  
  Serial.println("");   //go to next line

  
  logFile.flush();       //flush logFile cache after write for next read
  digitalWrite(grnLED,LOW); //Set onboard LED LOW when done logging
//}
//else logFile.println("Can's not spinning");
  }//END if (millis() - timer > LOG_INTERVAL)
}//END loop


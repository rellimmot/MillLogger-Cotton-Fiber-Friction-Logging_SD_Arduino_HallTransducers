//#include <SdFat.h>  //Optional SD Library in case of stack heap collisions et cetera
//SdFat SD;           //(I think this requires FAT16 though)
#include <SD.h> //Original SD Library
#include <SPI.h> //Load SPI Library - MAY NOT NEED
#include <SoftwareSerial.h> //Load the Software Serial library
#include <RTClib.h>
#include <Wire.h>
RTC_DS1307 RTC;            //define logger RTC object
#define chipSelect 10 //For SD logger. BTW- SD logger uses digital pins 10-13 and analog 4-5
#define LED   13   //Arduino onboard LED pin
#define grnLED 3   //green LED on logger
#define redLED 4   //red LED on logger
#define LOG_INTERVAL 99 //How long do we want between log intervals (in ms)?
                         //To make this accurate, see the millis column to confirm. 
                         //For example, 999ms might = 1 sec if it takes the Arduino
                         //1ms to do its reading and recording. So, the time it takes
                         //to take/record should be subtracted from the log interval.
File logFile; //Data object you will write your sensor data to
SoftwareSerial mySerial(6, 7); //Initialize the Software Serial port
uint32_t timer = millis(); //Sets milliseconds since Arduino on timer


#define Sensor1 14 //Sensor1 uses pin A0
#define Sensor2 15 //Sensor2 uses pin A1
#define Sensor3 16 //Sensor3 uses pin A2
#define Trigger 17 //Trigger uses pin A3
//#define Sensor5 18 //Sensor5 uses pin A4
//#define Sensor6 19 //Sensor6 uses pin A5


void setup(){
  Serial.begin(38400);
  Wire.begin();
  RTC.begin();
  pinMode(LED,OUTPUT);
  pinMode(grnLED,OUTPUT);
  pinMode(redLED,OUTPUT); 
  pinMode(10, OUTPUT);  //this is for the SD logger
  
  //**************SD Logger File Setup***********************//
  //*To do: Make this a function instead.
  //see if the card is present and can be initialized. Spinlock if it fails.
  if (!SD.begin(chipSelect)) {
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
    digitalWrite(redLED,HIGH);
    while(1){};
  }
  //Print header to file, keep this in order of what data you log in the loop.
  //This would be your first row on your Excel spreadsheet
  //Uncomment and enter comma separated values to your needs.
  logFile.println(F("millis,Date,Time,Sensor1,Sensor2,Sensor3,Trigger"));//,Sensor1Volts,Sensor1Amps,Sensor2Volts,Sensor2Amps,Sensor3Volts,Sensor3Amps,TriggerVolts"));  

  //*************END SD Logger File Setup**********************//
 
}//END SETUP

void loop(){


  // approximately every 5 seconds or so, print out the current stats
  if (millis() - timer > LOG_INTERVAL) { 
  timer = millis(); // reset the timer
  DateTime now;    //this retrieves a new date/time
  now = RTC.now(); //fetch RTC time

//****Begin Sensor Reading****//
//To do: make this a function
    int SensorADC1 = analogRead(Sensor1);
    int SensorADC2 = analogRead(Sensor2);
    int SensorADC3 = analogRead(Sensor3);  
    int SensorADC4 = analogRead(Trigger);
//    int SensorADC5 = analogRead(Sensor5);
//    int SensorADC6 = analogRead(Sensor6);
//****End Sensor Reading****//

Serial.print("    ADC1: "); Serial.print(SensorADC1);
Serial.print("  \tADC2: "); Serial.print(SensorADC2);
Serial.print("  \tADC3: "); Serial.print(SensorADC3);
Serial.print("  \tTRIG: "); Serial.println(SensorADC4);

if(SensorADC4 > 100){ //This is the Trigger, so this would set it to log only when the machine
                  //is running and feeding the Trigger input a signal.
  delay(100);
  
  digitalWrite(grnLED,HIGH); //Set onboard LED HIGH while logging
  digitalWrite(redLED,LOW);

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
  logFile.print(SensorADC4);               //Sensor 3 (connected to A4)
  logFile.print(F(", "));
//  logFile.print(SensorADC5);               //Sensor 3 (connected to A2)
//  logFile.print(F(", "));
//  logFile.print(SensorADC6);               //Sensor 3 (connected to A2)
//  logFile.print(F(", "));
//  float SensorVolts1 = SensorADC1 * (5.0 / 1023.0);   //ADC-to-Volts math
//  logFile.print(SensorVolts1);                         //Volts1
//  logFile.print(F(", "));  
//  float SensorAmps1 = map(SensorADC1, 0, 1023, 0, 20);//ADC-to-Amps scaled math
//  logFile.print(SensorAmps1);                          //Amps1
//  logFile.print(F(", "));  
//  float SensorVolts2 = SensorADC2 * (5.0 / 1023.0);   //ADC-to-Volts math
//  logFile.print(SensorVolts2);                         //Volts2
//  logFile.print(F(", "));  
//  float SensorAmps2 = map(SensorADC2, 0, 1023, 0, 20);//ADC-to-Amps scaled math
//  logFile.print(SensorAmps2);                          //Amps2
//  logFile.print(F(", "));  
//  float SensorVolts3 = SensorADC3 * (5.0 / 1023.0);   //ADC-to-Volts math
//  logFile.print(SensorVolts3);                         //Volts3
//  logFile.print(F(", "));  
//  float SensorAmps3 = map(SensorADC3, 0, 1023, 0, 20);//ADC-to-Amps scaled math
//  logFile.print(SensorAmps3);                          //Amps3
//  logFile.print(F(", "));  
//  float SensorVolts4 = SensorADC4 * (5.0 / 1023.0);   //ADC-to-Volts math
//  logFile.print(SensorVolts4);                         //Volts4
//  logFile.print(F(", "));  
//  float SensorAmps4 = map(SensorADC4, 0, 1023, 0, 20);//ADC-to-Amps scaled math
//  logFile.print(SensorAmps4);                          //Amps4
//  logFile.print(F(", "));  
//  float SensorVolts5 = SensorADC5 * (5.0 / 1023.0);   //ADC-to-Volts math
//  logFile.print(SensorVolts5);                         //Volts5
//  logFile.print(F(", "));  
//  float SensorAmps5 = map(SensorADC5, 0, 1023, 0, 20);//ADC-to-Amps scaled math
//  logFile.print(SensorAmps5);                          //Amps5
//  logFile.print(F(", "));  
//  float SensorVolts6 = SensorADC6 * (5.0 / 1023.0);   //ADC-to-Volts math
//  logFile.print(SensorVolts6);                         //Volts6
//  logFile.print(F(", "));  
//  float SensorAmps6 = map(SensorADC6, 0, 1023, 0, 20);//ADC-to-Amps scaled math
//  logFile.print(SensorAmps6);                          //Amps6
//  logFile.print(F(", "));  
//  long map(long x, long in_min, long in_max, long out_min, long out_max)
//  {
//  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
//  }
  
  logFile.println("");   //go to next line

  
  logFile.flush();       //flush logFile cache after write for next read
//  digitalWrite(grnLED,LOW); //Set onboard LED LOW when done logging

 }//END if(Trigger > xx)
else{
  digitalWrite(redLED,HIGH);
  digitalWrite(grnLED,LOW);
}
  }//END if (millis() - timer > LOG_INTERVAL)
}//END loop


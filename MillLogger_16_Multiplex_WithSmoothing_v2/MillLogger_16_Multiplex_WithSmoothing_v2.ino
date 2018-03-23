//#include <SdFat.h>  //Optional SD Library in case of stack heap collisions et cetera
//SdFat SD;           //(I think this requires FAT16 though)
#include <SD.h> //Original SD Library
#include <SPI.h> //Load SPI Library - MAY NOT NEED
#include <SoftwareSerial.h> //Load the Software Serial library
#include <RTClib.h>
#include <Wire.h>
RTC_DS1307 RTC;            //define logger RTC object
#define chipSelect 10 //for SD logger
#define LED   13   //Arduino onboard LED pin
#define grnLED 3   //green LED on logger
#define redLED 4   //red LED on logger
#define NUM_READS 100
#define LOG_INTERVAL 3000 //how long do we want between log intervals (in ms)?
File logFile; //Data object you will write your sensor data to
SoftwareSerial mySerial(6, 7); //Initialize the Software Serial port
uint32_t timer = millis(); //Sets milliseconds since Arduino on timer

//Mux control pins
int s0 = 0;
int s1 = 0;
int s2 = 0;
int s3 = 0;
//Mux in "SIG" pin
int SIG_pin = A0;


void setup(){

  //Multiplexer Pin Setup
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  //END Multiplexer Pin Setup
  
  Serial.begin(57600);
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
    digitalWrite(grnLED, HIGH); 
    digitalWrite(redLED,HIGH);
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
    digitalWrite(grnLED,HIGH);
    while(1){};
  }
  //Print header to file, keep this in order of what data you log in the loop.
  //This would be your first row on your Excel spreadsheet
  //Uncomment and enter comma separated values to your needs.
  logFile.println(F("millis,Date,Time,Sensor1,Sensor2,Sensor3"));  

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
    float SensorADC1 = readMux(0);
    float SensorADC2 = readMux(1);
    float SensorADC3 = readMux(2);  
    float SensorADC4 = readMux(3);
    float SensorADC5 = readMux(4);
    float SensorADC6 = readMux(5);
    SensorADC1 = SensorADC1*20/1023;
    SensorADC2 = SensorADC2*20/1023;
    SensorADC3 = SensorADC3*20/1023;
    SensorADC4 = SensorADC4*50/1023;
    SensorADC5 = SensorADC5*50/1023;
    SensorADC6 = SensorADC6*50/1023;  

//****End Sensor Reading****//



  digitalWrite(grnLED,HIGH); //Set onboard LED HIGH while logging
  
  logFile.println("");   //go to next line

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
  Serial.print(SensorADC1);               //Sensor 1 (connected to MUX0)
  Serial.print(F(", "));
  Serial.print(SensorADC2);               //Sensor 2 (connected to MUX1)
  Serial.print(F(", "));
  Serial.print(SensorADC3);               //Sensor 3 (connected to MUX2)
  Serial.print(F(", "));
  Serial.print(SensorADC4);               //Sensor 4 (connected to MUX3)
  Serial.print(F(", "));
  Serial.print(SensorADC5);               //Sensor 5 (connected to MUX4)
  Serial.print(F(", "));
  Serial.print(SensorADC6);               //Sensor 6 (connected to MUX5)
  Serial.print(F(", "));  
//  long map(long x, long in_min, long in_max, long out_min, long out_max)
//  {
//  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
//  }
  
  Serial.println("");   //go to next line

  
  logFile.flush();       //flush logFile cache after write for next read
  digitalWrite(grnLED,LOW); //Set onboard LED LOW when done logging
  }//END if (millis() - timer > LOG_INTERVAL)

  
  //Loop through and read all 16 values
  //Reports back Value at channel 6 is: 346
//  for(int i = 0; i < 16; i ++){
//    Serial.print("Value at channel ");
//    Serial.print(i);
//    Serial.print("is : ");
//    Serial.println(readMux(i));
//    delay(1000);
//  }

}


int readMux(int channel){
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = readMean(SIG_pin);

  //return the value
  return val;
}

float readMean(int SIG_pin){
   // read multiple values and sort them to take the mode
   int sortedValues[NUM_READS];
   for(int i=0;i<NUM_READS;i++){
     int value = analogRead(SIG_pin);
     int j;
     if(value<sortedValues[0] || i==0){
        j=0; //insert at first position
     }
     else{
       for(j=1;j<i;j++){
          if(sortedValues[j-1]<=value && sortedValues[j]>=value){
            // j is insert position
            break;
          }
       }
     }
     for(int k=i;k>j;k--){
       // move all values higher than current reading up one position
       sortedValues[k]=sortedValues[k-1];
     }
     sortedValues[j]=value; //insert current reading
   }
   //return scaled mode of 10 values
   float returnval = 0;
   for(int i=NUM_READS/2-5;i<(NUM_READS/2+5);i++){
     returnval +=sortedValues[i];
   }
   returnval = returnval/10;
   return returnval*1100/1023;
}

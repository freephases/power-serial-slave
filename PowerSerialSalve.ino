/**
* Simple program to send values from Emon object to software serial ports 6 (rx) and 7 (tx) 
*
*  RX is connected to Mega serial 1 TX
*  TX is connected to Mega serial 1 RX
*
*  Version 0.0.1.3
*/

/**
* OnOff is simple class to manage digital ports see: https://github.com/freephases/arduino-onoff-lib
*/
#include <OnOff.h>

#include <SoftwareSerial.h>
#include <math.h> 
#include <EmonLib.h>


#define DEBUG_TO_SERIAL 1
OnOff led(13);

// *****************************
SoftwareSerial master(6,7);

EnergyMonitor ACsensor; 
const unsigned long readPowerInterval = 10000;//read every 2.5 sec, calcVI times out at 2 secs max
unsigned long readPowerMillis = 0; // last milli secs since last avg reading
typedef struct { float power, Vrms, Irms;} PayloadTX;      // create structure - a neat way of packaging data for RF comms
PayloadTX emontx;  
PayloadTX powerAvg;  
int avgCount = 0;
const int avgTimes = 10;

void setupPower()
{
  ACsensor.current(1, 138.6);//60.606);    //111.1  //155.5
  ACsensor.voltage(0, 256.97, 1.7);   //268.97
  //calc power a few times to level out
  for(int i=0; i<10; i++) {
    ACsensor.calcVI(30, 2000);  // Calculate   
  }
}

void readPower2() {
   
   ACsensor.calcVI(60, 3000);  // Calculate 
   emontx.power = ACsensor.realPower;
   if (emontx.power < 1.9) emontx.power = 0.00;
   emontx.Vrms = ACsensor.Vrms;//*100;    
   emontx.Irms = ACsensor.Irms;    
   if (emontx.power < 1.9) emontx.Irms = 0.00;
   
   sendPower();
}

void readPower() {
  
   ACsensor.calcVI(60, 3000);  // Calculate 
  powerAvg.Vrms += ACsensor.Vrms;//*100;    
  if (ACsensor.realPower>1.9) { // sensor can not go lower then 2W as using 100A CT, have to work on that or get a smaller CT sensor
   powerAvg.power += ACsensor.realPower;   
   powerAvg.Irms += ACsensor.Irms;    
   }
   avgCount++;
   
   if (avgCount==avgTimes) {     
     emontx.power = powerAvg.power/avgTimes;
     if (emontx.power < 2) emontx.power = 0;
     emontx.Vrms = powerAvg.Vrms/avgTimes;//*100;    
     emontx.Irms = powerAvg.Irms/avgTimes;    
     if (emontx.power < 2) emontx.Irms = 0;
     sendPower();
     resetAvg();
   }

  
}

void resetAvg() {
   powerAvg.power = 0.00;
   powerAvg.Vrms = 0.00;    
   powerAvg.Irms = 0.00;
   avgCount = 0;
}

char buf[100];

void sendPower() {
  led.on();
  String tempStr = String(emontx.power);//map float to a string
  char powerBuf[24];
  tempStr.toCharArray(powerBuf, 24);//map float to char array
  
  String tempStr2 = String(emontx.Vrms);//map float to a string
  char VrmsBuf[24];
  tempStr2.toCharArray(VrmsBuf, 24);//map float to char array
  
  String tempStr3 = String(emontx.Irms);//map float to a string
  char IrmsBuf[24];
  tempStr3.toCharArray(IrmsBuf, 24);//map float to char array
  
   
  //build output up to send to master
  sprintf(buf, "R|%s|%s|%s|!", 
    powerBuf, 
    VrmsBuf,
    IrmsBuf);
  master.println(buf); //send
  
  if (DEBUG_TO_SERIAL==1) {
    Serial.println(buf);
  }
  led.off();
 // delay(100);  
}

void setup() {
  led.on();
  Serial.begin(9600);
  Serial.println("Emon power");
  delay(100); 
  master.begin(9600);
 // master.println("S|Started!-");
  setupPower();
  led.off(); 
}

void loop() {
  readPower();
 // sendPower();
}




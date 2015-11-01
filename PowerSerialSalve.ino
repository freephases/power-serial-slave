#include <OnOff.h>

#include <SoftwareSerial.h>



/**
 */

#include <math.h> 
#include <EmonLib.h>
#define DEBUG_TO_SERIAL 1
#define ROB_WS_MAX_STRING_DATA_LENGTH 100
int newlineCount = 0;
int serialBufferLength = 0;

boolean started = true;
unsigned long lastRequestMillis = 0;

short pos = 0; // position in read serialBuffer
char serialBuffer[ROB_WS_MAX_STRING_DATA_LENGTH+1];
OnOff led(13);
// *****************************
SoftwareSerial masterMaga(6,7);

EnergyMonitor ACsensor; 
const unsigned long readPowerInterval = 10000;//read every 2.5 sec, calcVI times out at 2 secs max
unsigned long readPowerMillis = 0; // last milli secs since last avg reading
typedef struct { float power, Vrms, Irms;} PayloadTX;      // create structure - a neat way of packaging data for RF comms
PayloadTX emontx;  

void setupPower()
{
  ACsensor.current(1, 60.606);//60.606);    //111.1  //155.5
  ACsensor.voltage(0, 268.97, 1.7);   
  //calc power a few times to level out
  for(int i=0; i<5; i++) {
    ACsensor.calcVI(20, 2000);  // Calculate 
  
  }
}


void readPower() {
  //if (isWaitingForResponse()) return;
  
   //if (millis()-readPowerMillis>=readPowerInterval) {
   // readPowerMillis = millis();
   led.on();
   ACsensor.calcVI(20, 2000);  // Calculate 
   emontx.power = ACsensor.realPower;
   emontx.Vrms = ACsensor.Vrms;//*100;    
   emontx.Irms = ACsensor.Irms;    
   led.off();
    
 
}

char buf[100];

void sendPower() {
  
  String tempStr = String(emontx.power);//map float to a string
  char powerBuf[24];
  tempStr.toCharArray(powerBuf, 24);//map float to char array
  
  String tempStr2 = String(emontx.Vrms);//map float to a string
  char VrmsBuf[24];
  tempStr2.toCharArray(VrmsBuf, 24);//map float to char array
  
  String tempStr3 = String(emontx.Irms);//map float to a string
  char IrmsBuf[24];
  tempStr3.toCharArray(IrmsBuf, 24);//map float to char array
  
   
  //build request for slave
  sprintf(buf, "R|%s|%s|%s|-", 
    powerBuf, 
    VrmsBuf,
    IrmsBuf);
  masterMaga.println(buf);
  if (DEBUG_TO_SERIAL==1) {
    Serial.println(buf);
  }
  delay(64);
  
}

void setup() {
  led.on();
  Serial.begin(9600);
  Serial.println("Emon power");
  delay(60);
  masterMaga.begin(9600);
  masterMaga.println("R|0.0000|-");
  setupPower();
  led.off(); 
}

void loop() {
  readPower();
  sendPower();
}





/*////////////////////////////////////////////////////////////////
***************** Aquarium Control System************************
////////////////////////////////////////////////////////////////*/
#define BLYNK_TEMPLATE_ID "TMPLXv2hwvRM"
#define BLYNK_DEVICE_NAME "Aquarium Control System"
#define BLYNK_AUTH_TOKEN "zfZVpxVtkid8FZlcOEo4NgGZGha7vDkg"
#define BLYNK_FIRMWARE_VERSION        "1.2.0"
#define BLYNK_PRINT Serial
#define USE_NODE_MCU_BOARD
/*////////////////////////////////////////////////////////////////
********************** Blynk Definitions**************************
////////////////////////////////////////////////////////////////*/
 
#define ONE_WIRE_BUS    D5
#define filter_BUS      D7
#define light_BUS       D6
#define autoFeeder_BUS  D8
/*////////////////////////////////////////////////////////////////
*******************NODEMCU GPIO Definitions***********************
////////////////////////////////////////////////////////////////*/

#include "BlynkEdgent.h"
#include "time.h"
#include "DallasTemperature.h"
#include "OneWire.h"
#include <math.h>
/*////////////////////////////////////////////////////////////////
**********************Library Inclusions**************************
////////////////////////////////////////////////////////////////*/

bool filterFlag = 0;
bool lightFlag = 0;
bool logCounterWaterTemp = 0;
bool logCounterLiquidFert = 0;
bool logCounterWaterChange = 0;
bool logCounterFilterMaint = 0;
bool autoFeederCheckCounter = 0;


int autoFeederFlag = 3;
int autoFeederArming = 0;
int autoFeederCount = 1;
int light_duration = 8;
int bootFlag = 0;

int cloudLightFlag = 0;
int cloudFilterFlag = 0;
int timerID10Sec, timerID1Sec, timerID10Min;

double water_temp = 0;

uint8_t mins = 0;
uint8_t hours = 0;
uint8_t secs = 0;
uint8_t date = 1;
uint8_t weekdays = 1;
/*////////////////////////////////////////////////////////////////
*****************Global Variable Declarations*********************
////////////////////////////////////////////////////////////////*/

OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
time_t cloudtime;
struct tm  ts;
BlynkTimer timer; 
/*////////////////////////////////////////////////////////////////
***********************Object Creations***************************
////////////////////////////////////////////////////////////////*/

void dailyReset()
{
  if ((hours >= 2) && (hours < 6))
  {
    Blynk.virtualWrite(V1, 0);
    autoFeederFlag = 0;
    cloudLightFlag = 0;
    cloudFilterFlag = 0;
  }
}
/*////////////////////////////////////////////////////////////////
Function to Daily reset all concerned Flags at various time 
intervals.
////////////////////////////////////////////////////////////////*/

void flagReset()
{
  cloudLightFlag = 0;
  cloudFilterFlag = 0;
}
/*////////////////////////////////////////////////////////////////
Function to reset all internal flags triggered from the cloud.
////////////////////////////////////////////////////////////////*/

void systemReset()
{
  ESP.restart();
}
/*////////////////////////////////////////////////////////////////
Function to reset the NODEMCU triggered from the cloud.
////////////////////////////////////////////////////////////////*/

void updateWaterTemp()
{
  sensors.requestTemperatures();
  water_temp = sensors.getTempCByIndex(0);
  if(water_temp >= 0)
  {
    Blynk.virtualWrite(V0, water_temp);
  }
  
  if(water_temp > 32)
  {
    if(logCounterWaterTemp == 0)
    {
      Blynk.logEvent("high_water_temp");
      logCounterWaterTemp = 1;
    }
    timer.setTimeout(60000L, []() 
    {  
      logCounterWaterTemp = 0;
    });
  }

  if(water_temp < 27)
  {
    if(logCounterWaterTemp == 0)
    {
      Blynk.logEvent("low_water_temp");
      logCounterWaterTemp = 1;
    }
    timer.setTimeout(60000L, []() 
    {  
      logCounterWaterTemp = 0;
    });
  }

  Serial.println(water_temp);
}
/*////////////////////////////////////////////////////////////////
Function to request for Water Temperature from the DS18b20 sensor
using the One Wire data transfer protocol. The function also 
updates the latest temperature value to the cloud.
////////////////////////////////////////////////////////////////*/

void startFilter()
{
  if(filterFlag == 0)
  {
    Blynk.virtualWrite(V3, 1);
    filterFlag = 1;
    digitalWrite(filter_BUS, HIGH);
    Serial.println("Filter is ON");
    Blynk.logEvent("filter_on");
  }
}
/*////////////////////////////////////////////////////////////////
Function that starts the Aquarium Filter whenever required.
////////////////////////////////////////////////////////////////*/

void stopFilter()
{
  if(filterFlag == 1)
  {
    Blynk.virtualWrite(V3, 0);
    filterFlag = 0;
    digitalWrite(filter_BUS, LOW);
    Serial.println("Filter is OFF"); 
    Blynk.logEvent("filter_off");
  }
}
/*////////////////////////////////////////////////////////////////
Function that stops the Aquarium Filter whenever required.
////////////////////////////////////////////////////////////////*/

void startLight()
{
  if (lightFlag == 0)
  {
    Blynk.virtualWrite(V4, 1);
    lightFlag = 1;
    digitalWrite(light_BUS, LOW);
    Serial.println("Light is ON");
    Blynk.logEvent("lights_on");
  }
}
/*////////////////////////////////////////////////////////////////
Function that starts the Aquarium Light whenever required.
////////////////////////////////////////////////////////////////*/

void stopLight()
{
  if (lightFlag == 1)
  {
    Blynk.virtualWrite(V4,0);
    lightFlag = 0;
    digitalWrite(light_BUS, HIGH);
    Serial.println("Light is OFF");
    Blynk.logEvent("lights_off");
  }
}
/*////////////////////////////////////////////////////////////////
Function that stops the Aquarium Light whenever required.
////////////////////////////////////////////////////////////////*/

void startAutoFeeder()
{ 
  autoFeederFlag++; 
  autoFeederCheckCounter = 1;
  timer.restartTimer(timerID10Min);
  timer.restartTimer(timerID10Sec);
  digitalWrite(autoFeeder_BUS, HIGH);
  Serial.println("AutoFeeder is ON");
  Blynk.logEvent("auto_feeder_triggered");
  Blynk.virtualWrite(V1, autoFeederFlag);
}
/*////////////////////////////////////////////////////////////////
Function that starts the Aquarium AutoFeeder whenever required.
////////////////////////////////////////////////////////////////*/

void maintenanceAlarms()
{
   if(weekdays == 1 || weekdays == 4)
   {
      if(hours == 9 && mins == 0)
      {
        if(logCounterLiquidFert == 0)
        {
          Blynk.logEvent("fetilizer_low");
          logCounterLiquidFert = 1;
        }
        timer.setTimeout(60000L, []() 
        {  
          logCounterLiquidFert = 0;
        });
      }
   }

   if(weekdays == 0)
   {
      if(hours == 9 && mins == 0)
      {
        if(logCounterWaterChange == 0)
        {
          Blynk.logEvent("water_change");
          logCounterWaterChange = 1;
        }
        timer.setTimeout(60000L, []() 
        {  
          logCounterWaterChange = 0;
        });
      }
   }

   if (date >= 10 && date <= 17 && weekdays == 0)
   {
      if(hours == 9 && mins == 0)
      {
        if(logCounterFilterMaint == 0)
        {
          Blynk.logEvent("filter_maintenance");
          logCounterFilterMaint = 1;
        }
        timer.setTimeout(60000L, []() 
        {  
          logCounterFilterMaint = 0;
        });
      }
   }
}
/*////////////////////////////////////////////////////////////////
Function that checks and triggers maintenance alarms.
////////////////////////////////////////////////////////////////*/

void checkAutoFeeder()
{
  if(autoFeederCheckCounter > 0)
  {
    Blynk.virtualWrite(V2, 0);
    digitalWrite(autoFeeder_BUS, LOW);
    Serial.println("AutoFeeder is OFF");
    autoFeederCheckCounter = 0;
  }
  
  if(autoFeederArming == 1)
  {
    switch(autoFeederCount)
    {
      case 1:
      
        if((hours >= 12) && (hours < 14) && (autoFeederFlag == 0))
        {
          stopFilter();
          timer.restartTimer(timerID10Min);  
        }
        
        if((hours >= 12) && (hours < 14) && (mins >= 2) && (autoFeederFlag == 0) && (bootFlag > 5))
        {
          startAutoFeeder();   
        }
        break;
        
      case 2:
      
        if((hours >= 12) && (hours < 14) && (autoFeederFlag == 0))
        {
          stopFilter();
          timer.restartTimer(timerID10Min);  
        }
        
        if((hours >= 12) && (hours < 14) && (mins >= 2) && (autoFeederFlag == 0) && (bootFlag > 5))
        {
          startAutoFeeder();   
        }

        if((hours >= 18) && (hours < 20) && (autoFeederFlag <= 1))
        {
          stopFilter();
          timer.restartTimer(timerID10Min);    
        }
        
        if((hours >= 18) && (hours < 20) && (mins >= 2) && (autoFeederFlag <= 1) && (bootFlag > 5))
        {
          startAutoFeeder();   
        }
        break;
    }
  }
  if(bootFlag < 10)
  {
    bootFlag++; 
  }
}
/*////////////////////////////////////////////////////////////////
Function that checks if AutoFeeder needs to br triggered or not.
////////////////////////////////////////////////////////////////*/
void checkFilter()
{
  if (cloudFilterFlag < 1)
  {
    startFilter();
  }
  Serial.println(cloudFilterFlag);
}
/*////////////////////////////////////////////////////////////////
Function that checks if the Aquarium Filter needs to be On or Off.
////////////////////////////////////////////////////////////////*/
void checkLight()
{
  if (cloudLightFlag < 1)
  {
    if((hours >= 14) && (hours < (light_duration + 14)))
    {
      startLight();   
    }
    
    else
    {
      stopLight();
    }
  }
  Serial.println(cloudLightFlag);
}
/*////////////////////////////////////////////////////////////////
Function that checks if the Aquarium Light needs to be On or Off.
////////////////////////////////////////////////////////////////*/

void updateTimeCloud()
{
  Blynk.sendInternal("rtc", "sync");
}
/*////////////////////////////////////////////////////////////////
Function that requests the current time from the cloud when connected.
////////////////////////////////////////////////////////////////*/
void scheduler10Min()
{
  checkFilter();
}
/*////////////////////////////////////////////////////////////////
Function that schedules jobs with a frequency of 10 minutes.
////////////////////////////////////////////////////////////////*/
void scheduler10Sec()
{
  if(Blynk.connected())
  { 
    Serial.println("Device Connected");
    updateTimeCloud();
    checkLight();
    checkAutoFeeder();
    dailyReset();
    maintenanceAlarms();
  }
  else
  {
    Serial.println("Device Not Connected");
  }
}
/*////////////////////////////////////////////////////////////////
Function that schedules jobs with a frequency of 10 seconds.
////////////////////////////////////////////////////////////////*/

void scheduler1Sec()
{
  if(Blynk.connected())
  { 
    updateWaterTemp(); 
  }
}
/*////////////////////////////////////////////////////////////////
Function that schedules jobs with a frequency of 1 second.
////////////////////////////////////////////////////////////////*/
void setup()
{
  Serial.begin(115200);
  delay(7000);

  pinMode(filter_BUS, OUTPUT);
  pinMode(light_BUS, OUTPUT);
  pinMode(autoFeeder_BUS, OUTPUT);

  digitalWrite(filter_BUS, LOW);
  digitalWrite(light_BUS, HIGH);
  digitalWrite(autoFeeder_BUS, LOW);

  BlynkEdgent.begin();
  startFilter();
  timer.setTimeout(3600000L, [] () {} ); // dummy/sacrificial Function
  timerID10Min = timer.setInterval(600000L, scheduler10Min);  
  timerID10Sec = timer.setInterval(10000L, scheduler10Sec);
  timerID1Sec = timer.setInterval(1000L, scheduler1Sec);
}
/*////////////////////////////////////////////////////////////////
Setup and Initialization function for all Functionalities.
////////////////////////////////////////////////////////////////*/

void loop() 
{
  BlynkEdgent.run();
  timer.run();
}
/*////////////////////////////////////////////////////////////////
Infinite Loop function to continuosly update cloud and timers.
////////////////////////////////////////////////////////////////*/

BLYNK_CONNECTED()
{
  Blynk.syncAll();
}
/*////////////////////////////////////////////////////////////////
Function to sync all local variables with cloud variables whenever
connection to cloud is setup.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(InternalPinRTC) 
{
  cloudtime = param.asLong();
  ts = *localtime(&cloudtime);
  hours = ts.tm_hour;
  mins = ts.tm_min;
  secs = ts.tm_sec;
  date = ts.tm_mday;
  weekdays = fmod((floor(param.asLong()/86400)+4), 7);
  Serial.println(hours);
  Serial.println(mins);
  Serial.println(secs);
}
/*////////////////////////////////////////////////////////////////
Function that updates the RTC from cloud whenever connection is
established.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(V1)
{
  autoFeederFlag = param.asInt();
  Serial.println(autoFeederFlag);
}
/*////////////////////////////////////////////////////////////////
Interrupt Routine from cloud for Autofeeder status change.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(V2)   
{
  if(param.asInt() == 1)
  {
    if(autoFeederArming == 1)
    {
      startAutoFeeder();
    }
  }
  else
  {
    Blynk.virtualWrite(V2, 0);
    digitalWrite(autoFeeder_BUS, LOW);
  }
}
/*////////////////////////////////////////////////////////////////
Interrupt Routine from cloud for Autofeeder switch status change.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(V3)  
{
  if(param.asInt() == 1)
  {
    startFilter();
    cloudFilterFlag++;
  }
  else
  {
    stopFilter();
    cloudFilterFlag++;
  }
}
/*////////////////////////////////////////////////////////////////
Interrupt Routine from cloud for Filter status change.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(V4)// Add your code here to act upon LightStatus change
{
  if(param.asInt() == 1)
  {
    startLight();
    cloudLightFlag++;
  }
  else
  {
    stopLight();
    cloudLightFlag++;
  }
}
/*////////////////////////////////////////////////////////////////
Interrupt Routine from cloud for Light status change.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(V5)
{
  light_duration = param.asInt();
  Serial.print("Duration");
  Serial.print(light_duration);
}
/*////////////////////////////////////////////////////////////////
Interrupt Routine from cloud for Light Duration status change.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(V6)
{
  if(param.asInt() == 1)
  {
    Blynk.virtualWrite(V6, 0);
    Serial.println("All flags reset");
    flagReset();
  }
}
/*////////////////////////////////////////////////////////////////
Interrupt Routine from cloud for flag reset.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(V8)
{
  autoFeederArming = param.asInt();
}
/*////////////////////////////////////////////////////////////////
Interrupt Routine from cloud for Auto Feeder Arming switch.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(V9)
{
  autoFeederCount = param.asInt();
}
/*////////////////////////////////////////////////////////////////
Interrupt Routine from cloud for Auto Feeder count status.
////////////////////////////////////////////////////////////////*/

BLYNK_WRITE(V10)
{
  if(param.asInt() == 1)
  {
    stopFilter();
    timer.restartTimer(timerID10Min);
    Blynk.virtualWrite(V10, 0);
  }
}
/*////////////////////////////////////////////////////////////////
Interrupt Routine from cloud for stopping filter when doing manual
feeding.
////////////////////////////////////////////////////////////////*/

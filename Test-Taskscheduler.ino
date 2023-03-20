/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

//#define USE_STM32F103
#define DEB_IDLE_CALL
#define USE_US_CORRECTION

#define _TASK_PRIORITY
#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_EXTERNAL_TIME

uint32_t external_millis();
uint32_t  external_micros();

#ifdef _TASK_SLEEP_ON_IDLE_RUN
  #define myMillis() external_millis()
#else
  #define myMillis() millis()
#endif

#ifdef USE_STM32F103
#include <STM32LowPower.h>
#endif
//#include <TaskScheduler.h>
#include ".\TaskScheduler\src\TaskScheduler.h"

Scheduler ts;
#ifdef _TASK_PRIORITY
Scheduler hpts;
#endif

#define WAKEUP_TIME_US      14 //STM32F103 deepSleep wakeup time = 14 usec

uint32_t micros_in_low_power = 0;
uint32_t millis_in_low_power = 0;

void myIdle(unsigned long aT);
void lowPowerMsec(uint32_t msec);

#define PERIOD1 1000
#define DURATION 10000
void blink1CB();
Task tBlink1 ( PERIOD1 * TASK_MILLISECOND, TASK_FOREVER, &blink1CB, &ts, true );

void handler();
#ifdef _TASK_PRIORITY
Task tHandler ( TASK_IMMEDIATE, TASK_ONCE, &handler, &hpts, false);
#else
Task tHandler ( TASK_IMMEDIATE, TASK_ONCE, &handler, &ts, false);
#endif

void slave1();
Task tSlave1 ( TASK_IMMEDIATE, TASK_ONCE, &slave1, &ts, false);

void slave2();
Task tSlave2 ( TASK_IMMEDIATE, TASK_ONCE, &slave2, &ts, false);


char event[64] = "";

// the setup function runs once when you press reset or power the board
void setup() 
{
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Scheduler test");
  #ifdef _TASK_SLEEP_ON_IDLE_RUN
  ts.setSleepMethod( &myIdle );
  #endif
  #ifdef _TASK_PRIORITY  
  ts.setHighPriorityScheduler(&hpts);
  ts.enableAll(true);
  #endif
}

// the loop function runs over and over again forever
void loop() 
{
  ts.execute();
}


void putEvent(const char* p)
{
  strncpy(event, p, 64);
  tHandler.restart();
}

void handler()
{
  if(strlen(event) != 0)
  {
    Serial.print(" >>> Event: ");
    Serial.print(event);
    Serial.println("");
    strcpy(event, "");
  }
}
void slave1()
{
  Serial.print(millis());
  Serial.println(" > SLV-1");
  putEvent("SLAVE 1 done");
}

void slave2()
{
  Serial.print(millis());
  Serial.println(" > SLV-2");
  putEvent("SLAVE 2 done");
}

void blink1CB() 
{
  static bool flag = 0;  

  flag = !flag;
  if(flag) 
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(millis());
    Serial.println(" > LEDS: ON");
    putEvent("LED ON done");
  }
  else 
  {
    digitalWrite(LED_BUILTIN, LOW);  
    Serial.print(millis());
    Serial.println(" > LEDS: OFF");
    putEvent("LED OFF done");
  }

  tSlave1.restart(); 
  tSlave2.restart(); 
}

/*
 * LOW POWER HANDLE
 */

uint32_t external_millis()
{
  return(millis() + millis_in_low_power);
}

uint32_t  external_micros()
{
  return(micros() + micros_in_low_power);
}

void myIdle(unsigned long aT) 
{  
  uint32_t t = 0;

  #ifdef DEB_IDLE_CALL
  t = millis();
  Serial.print("myIdle:");
  Serial.println(aT);
  t = millis() - t; //Cálculo del tiempo perdido en enviar el mensaje:
  #endif
  
  if(aT > t)
  {
    #ifdef USE_STM32F103 //solo en caso de un verdadero Idle (ya que el timer queda suspendido)
    lowPowerMsec(aT - t);  
    #else
    delay(aT - t); //simula el Idle
    #endif
  }
}


void lowPowerMsec(uint32_t msec)
{
  //Here your lowpower function...

  #ifdef USE_US_CORRECTION
  static uint32_t UsCounter = 0;

  millis_in_low_power += msec;
  micros_in_low_power += ((uint32_t)msec * 1000);
  UsCounter += WAKEUP_TIME_US;  

  while(msec && UsCounter >= 1000)
  {
    UsCounter -= 1000;
    msec--; 
  }  
  #else
  millis_in_low_power += msec;
  micros_in_low_power += ((uint32_t)msec * 1000);
  #endif
  
  #ifdef USE_STM32F103
  HAL_SuspendTick(); 
  LowPower.deepSleep(msec); 
  HAL_ResumeTick();
  #endif
}

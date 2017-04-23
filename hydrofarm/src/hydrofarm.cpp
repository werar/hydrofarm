#include <Arduino.h>
#include "hydrofarm.h"

/**
 * Software to control small hydrophonics farm
 *
 * TODO: implement RTC
 * TODO: implement water flow sensor https://www.dfrobot.com/wiki/index.php/Water_Flow_Sensor_-_1/8%22_SKU:_SEN0216
 */

/**
 * Select what module should be supported
 */
#define RTC_MODULE 0 //
#define NRF_MODULE 1//
#define WATER_FLOW_MODULE 0//

#if NRF_MODULE
  #define MY_DEBUG
  #define MY_RADIO_NRF24
  #define MY_TRANSPORT_WAIT_READY_MS 3000 ///try to connect 3sec if no sucess continue the loop
  #include <MySensors.h>
  #define CHILD_ID_FOR_PUMP_RELAY 1
  #define CHILD_ID_FOR_WATER_METER 2
  #define PUMP_STATUS 0
  MyMessage pumpMsg(CHILD_ID_FOR_PUMP_RELAY,V_STATUS);
  MyMessage waterSensorMsg(CHILD_ID_FOR_WATER_METER,V_LIGHT);

  void presentation()
  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Hydrophonics farm", "0.1");
  // Register this device as Waterflow sensor
  present(CHILD_ID_FOR_PUMP_RELAY, S_BINARY);
  present(CHILD_ID_FOR_WATER_METER,S_LIGHT);
  }

  int sendStatusesViaNRF()
  {
    //TODO implement that
    //send(volumeMsg.set(volume, 3));
    return -1;
  }

  void receive(const MyMessage &message)
  {
    // We only expect one type of message from controller. But we better check anyway.
    if (message.type==V_STATUS)
    {
      // This way of switching pump is independed on main pumpProcess. The pump status will be changed as normal, even if here was
      message.getBool()?turnPumpOn():turnPumpOff();
    }
  }
#endif

#ifdef DEBUG_ON
  #define DEBUG_PRINT(x)   Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif


#define PUMP_MOTOR_PIN 7
#define PERIOD_TO_RUN_PROCESS_MANAGER 1000 //in ms
unsigned long last_run_pm=0;

unsigned long period_to_turn_pump_on=5000;
unsigned long period_to_turn_pump_off=60000;
unsigned long last_pump_status_change=0;
#define RELAY_ON 1
#define RELAY_OFF 0
unsigned long current_time;

/**
Main idea is to use process flags as semaphores to know what is just working and what is not
*/
typedef struct process_flags_type
{
      boolean pump_on;
};

typedef struct sensors_type
{
      int external_temperature;
};

process_flags_type process_flags;

int turnPumpOff()
{
  digitalWrite(PUMP_MOTOR_PIN, RELAY_OFF);
  #if NRF_MODULE
    send(pumpMsg.setSensor(CHILD_ID_FOR_PUMP_RELAY).set(false), false); //TODO: check that
  #endif
  return 0;
}

void inversePumpStatus()
{
  last_pump_status_change=current_time;
  process_flags.pump_on=!process_flags.pump_on;
}

int turnPumpOn()
{
  digitalWrite(PUMP_MOTOR_PIN, RELAY_ON);
  #if NRF_MODULE
    send(pumpMsg.setSensor(CHILD_ID_FOR_PUMP_RELAY).set(true), false); //TODO: check that
  #endif
  return 0;
}

int processPump()
{
    if(process_flags.pump_on)
  {
    Serial.println("In processPump pump_on section");
    if(current_time>=last_pump_status_change+period_to_turn_pump_on)
    {
      turnPumpOff();
      inversePumpStatus();
    }
  }else
  {
    Serial.println("In processPump pump_off section");
   if(current_time>=last_pump_status_change+period_to_turn_pump_off)
    {
      turnPumpOn();
      inversePumpStatus();
    }
  }
  return 0;
}

int processManager()
{
  DEBUG_PRINTLN(F("in process Manager"));
  //pump manager
  processPump();
  return 0;
}


void cleanProcessFlags()
{
  process_flags.pump_on=false;
}

void setup()
{
  Serial.begin(115200);
  pinMode(PUMP_MOTOR_PIN, OUTPUT);
  cleanProcessFlags();
}

void loop()
{
  current_time = millis();
  if(current_time>=last_run_pm+PERIOD_TO_RUN_PROCESS_MANAGER)
  {
    last_run_pm=current_time;
    processManager();
  }

  #if NRF_MODULE
    sendStatusesViaNRF();
  #endif
}

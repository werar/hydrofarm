#include <Arduino.h>
#ifndef UNIT_TEST  // IMPORTANT LINE!
#include "hydrofarm.h"
#include "pump.h"
#include "serialConsole.h"

/**
 * Software to control small hydrophonics farm
 *
 * TODO: implement RTC
 * TODO: be sure that each process it turned on/off by.... either  flag or method. Why we need process manager is nrf is using directly turn pump on. Maybe better to have a singal/flag?
 * TODO: implement water flow sensor https://www.dfrobot.com/wiki/index.php/Water_Flow_Sensor_-_1/8%22_SKU:_SEN0216
 * TODO: Implement event handlers like desribed here: https://arobenko.gitbooks.io/bare_metal_cpp/content/basic_concepts/event_loop.html
 * TODO: mono switch enabling pump (manual mode)
 */

#if TM1637_MODULE
  #include <TM1637Display.h>
  // Module connection pins (Digital Pins)
  #define CLK A2
  #define DIO A3
  TM1637Display display(CLK, DIO);

  void show_time_on_LED(int value)
  {
    //display.showNumberDecEx(0, (0x80 >> k), true);
    if(value>9999)
    {
      value=value/60;
    }
    display.showNumberDec(value, false);
  }
#endif

#if NRF_MODULE
#include "nrf.h"
#include <MySensors.h>
MyMessage pumpMsg(CHILD_ID_FOR_PUMP_RELAY,V_STATUS);
#endif

#if WATER_FLOW_MODULE
#include "waterFlow.h"
#endif

#define PERIOD_TO_RUN_PROCESS_MANAGER 1000 //in ms
unsigned long last_run_pm=0;
unsigned long period_to_turn_pump_on=120000;
unsigned long period_to_turn_pump_off=7200000;
unsigned long last_pump_status_change=0;

unsigned long current_time;

process_flags_type process_flags;
sensors_type connected_sensors;

/**

*/
boolean processPump()
{
  if(!process_flags.pump_enabled) return false;
  if(isDark())
  {
      Serial.println("Is too dark. The pump will be to noisy.");
      return false;
  }
  if(process_flags.pump_is_on)
  {
    if(current_time>=last_pump_status_change+period_to_turn_pump_on)
    {
      turnPumpOff();
      last_pump_status_change=current_time;
      process_flags.pump_is_on=false;
    }
  }else
  {
   if(current_time>=last_pump_status_change+period_to_turn_pump_off)
    {
      turnPumpOn();
      last_pump_status_change=current_time;
      process_flags.pump_is_on=true;
    }
  }
  #if TM1637_MODULE
    //DEBUG_PRINTLN((last_pump_status_change+(process_flags.pump_is_on?period_to_turn_pump_on:period_to_turn_pump_off)-current_time)/1000);
    show_time_on_LED((last_pump_status_change+(process_flags.pump_is_on?period_to_turn_pump_on:period_to_turn_pump_off)-current_time)/1000);
  #endif
  return true;
}

boolean isDark()
{
  #if LIGHT_SENSOR_MODULE
  /*** If the night is comming to avoid running noisy pump lets disable it. What if some lamp will be enabled or a day will be quite dark. This is not perfect solution*/
  connected_sensors.digital_light_sensor=digitalRead(LIGHT_SENSOR_PIN);
  if(connected_sensors.digital_light_sensor==0)
  {
    return false;
  }else
  {
    return true;
  }
  #else
  return false;
  #endif
}

int processManager()
{
  //DEBUG_PRINTLN(F("in process Manager"));
  //pump manager
  processPump();
  #if WATER_FLOW_MODULE
  calculateWaterFlowRate();
  #endif
  #if NRF_MODULE
    //sendStatusesViaNRF();
  #endif
  return 0;
}


void cleanProcessFlags()
{
  process_flags.pump_is_on=false;
}

void setup()
{
  Serial.begin(9600); //for BLUETOOTH HC6
  //Serial.println("AT+BAUD8"); //http://42bots.com/tutorials/hc-06-bluetooth-module-datasheet-and-configuration-with-arduino/
  //Serial.begin(115200);
  pinMode(PUMP_MOTOR_PIN, OUTPUT);
  cleanProcessFlags();
  #if TM1637_MODULE
    display.setBrightness(0x0f);
  #endif
  #if LIGHT_SENSOR_MODULE
    pinMode(LIGHT_SENSOR_PIN,INPUT);
  #endif

  #if WATER_FLOW_MODULE
  initWaterFlow();
  #endif

  addSerialCommands();

  process_flags.pump_enabled=true;
  turnPumpOff();
  process_flags.pump_is_on=false;


}

void loop()
{
  current_time = millis();
  if(current_time>=last_run_pm+PERIOD_TO_RUN_PROCESS_MANAGER)
  {
    last_run_pm=current_time;
    processManager();
  }
  sCmd.readSerial();
}
#endif

#include <Arduino.h>
#include "hydrofarm.h"
#include "pump.h"
#include "nrf.h"

void disablePump()
{
  Serial.println("Pump is disabled");
  process_flags.pump_enabled=false;
}

void enablePump()
{
  Serial.println("Pump is enabled");
  process_flags.pump_enabled=true;
}

/**
*
*/
int turnPumpOff()
{
  digitalWrite(PUMP_MOTOR_PIN, RELAY_OFF);
  Serial.println("Pump is off");
  #if NRF_MODULE
    //send(pumpMsg.setSensor(CHILD_ID_FOR_PUMP_RELAY).set(false), false); //TODO: check that
  #endif
  return 0;
}

/**
*
*/
int turnPumpOn()
{
  digitalWrite(PUMP_MOTOR_PIN, RELAY_ON);
  Serial.println("Pump is on");
  #if NRF_MODULE
  //  send(pumpMsg.setSensor(CHILD_ID_FOR_PUMP_RELAY).set(true), false); //TODO: check that
  #endif
  return 0;
}

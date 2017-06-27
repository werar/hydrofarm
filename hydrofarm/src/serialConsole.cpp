#include <SerialCommand.h>

#include "serialConsole.h"
#include "hydrofarm.h"
#include "pump.h"
#include "waterFlow.h"
#include "config.h"
#include "soil.h"

SerialCommand sCmd;

void showStatus()
{
  if(process_flags.pump_is_on==true)
  { //TODO: the pump can be on even if pump_is_on is false -> if someone push directy pump on.
    Serial.print("The pump is on. Time to disable it:");
    Serial.println((last_pump_status_change+(process_flags.pump_is_on?config_in_ram.period_to_turn_pump_on:config_in_ram.period_to_turn_pump_off)-current_time)/1000);
    #if WATER_FLOW_MODULE
    calculateWaterFlowRate();
    #endif
  }else
  {
    Serial.print("The pump is off. Time to enable it:");
    Serial.println((last_pump_status_change+(process_flags.pump_is_on?config_in_ram.period_to_turn_pump_on:config_in_ram.period_to_turn_pump_off)-current_time)/1000);
  }
  Serial.print("Current pump port status:");
  digitalRead(PUMP_MOTOR_PIN)?Serial.println("The pump relay is disabled, the pump is not working"):Serial.println("The pump relay is enabled, the pump is working");
  Serial.print("Last last_pump_status_change: ");
  Serial.println(last_pump_status_change);
  #if SOIL_MODULE
  uint8_t soil_percentage= measureSoilPercentage();
  Serial.print("Soil % and average: ");
  Serial.print(soil_percentage);
  Serial.print("%, ");
  long soil_avg = soil_average();
  Serial.println(soil_avg);
  #endif
}

void pushToTurnPumpOn()
{
  turnPumpOn();
}

void pushToTurnPumpOff()
{
  turnPumpOff();
}

void setOnTime() {
  unsigned long secounds;
  char *arg;
  Serial.println("Changing pump ON time");
  arg = sCmd.next();
  if (arg != NULL) {
    secounds = atol(arg);    // Converts a char string to an integer
    Serial.print("Set to: ");
    Serial.println(secounds);
    config_in_ram.period_to_turn_pump_on=secounds*1000;
    Serial.println("Saved to eeprom");
    copy_ram_to_eem();
    Serial.println(config_in_ram.period_to_turn_pump_on/1000);
  }
  else {
    config_in_ram.period_to_turn_pump_on=999;
    copy_eem_to_ram(); //TODO: only for tests
    Serial.println(config_in_ram.period_to_turn_pump_on/1000);
  }
}

void setOffTime() {
  unsigned long seconds;
  char *arg;
  Serial.println("Changing pump OFF time");
  arg = sCmd.next();
  if (arg != NULL) {
    seconds = atol(arg);    // Converts a char string to an lon
    Serial.print("Set to: ");
    Serial.println(seconds);
    config_in_ram.period_to_turn_pump_off=seconds*1000;
    Serial.println("Saved to eeprom");
    copy_ram_to_eem();
  }
  else {
    Serial.println(config_in_ram.period_to_turn_pump_off/1000);
  }
}

// This gets set as the default handler, and gets called when no other command matches.
void unrecognized(const char *command)
{
 Serial.println("Supported commands:");
 Serial.println("disable_pump (d), enable_pump (e), status (s), pump_on (on), pump_of (off), o-time (ont) [sec], off-time (offt) [sec]");
}

void addSerialCommands()
{
//examples: https://github.com/kroimon/Arduino-SerialCommand/blob/master/examples/SerialCommandExample/SerialCommandExample.pde
sCmd.addCommand("disable_pump", disablePump);
sCmd.addCommand("d", disablePump);
sCmd.addCommand("enable_pump",  enablePump);
//TODO below
sCmd.addCommand("pump_on", pushToTurnPumpOn);
sCmd.addCommand("on", pushToTurnPumpOn);
sCmd.addCommand("pump_off", pushToTurnPumpOff);
sCmd.addCommand("off", pushToTurnPumpOff);

sCmd.addCommand("e",  enablePump);
sCmd.addCommand("status",  showStatus);
sCmd.addCommand("s",  showStatus);

sCmd.addCommand("o-time",  setOnTime);
sCmd.addCommand("ont",  setOnTime);
sCmd.addCommand("off-time",  setOffTime);
sCmd.addCommand("offt",  setOffTime);

sCmd.setDefaultHandler(unrecognized);
}

#include <SerialCommand.h>

#include "serialConsole.h"
#include "hydrofarm.h"
#include "pump.h"

SerialCommand sCmd;

void showStatus()
{
  if(process_flags.pump_is_on==true)
  {
    Serial.print("The pump is on time to disabe it:");
  }else
  {
    Serial.print("The pump is off time to enable it:");
  }
  Serial.println((last_pump_status_change+(process_flags.pump_is_on?period_to_turn_pump_on:period_to_turn_pump_off)-current_time)/1000);
}
 // This gets set as the default handler, and gets called when no other command matches.
void unrecognized(const char *command) {
  Serial.println("Supported commands:");
  Serial.println("disable_pump (d), enable_pump (e), status (s), pump_on (on), pump_of (off), set_on_time [sec], set_off_time [sec]");
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
    period_to_turn_pump_on=secounds*1000;
  }
  else {
    Serial.println("No arguments");
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
    period_to_turn_pump_off=seconds*1000;
    Serial.println(period_to_turn_pump_off);
  }
  else {
    Serial.println("No arguments");
  }
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

sCmd.addCommand("set_on_time",  setOnTime);
sCmd.addCommand("set_off_time",  setOffTime);

sCmd.setDefaultHandler(unrecognized);
}

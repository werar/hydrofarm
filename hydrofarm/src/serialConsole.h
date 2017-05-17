#ifndef SERIALCONSLOE_H_
#define SERIALCONSOLE_H_

#include <SerialCommand.h>

extern SerialCommand sCmd;

extern void addSerialCommands();

void pushToTurnPumpOff();
void pushToTurnPumpOn();
void setOffTime();
void setOnTime();
void showStatus();
void unrecognized(const char *command);


#endif

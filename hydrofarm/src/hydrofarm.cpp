#include <Arduino.h>
#include <SerialCommand.h>
#ifndef UNIT_TEST  // IMPORTANT LINE!
#include "hydrofarm.h"

/**
 * Software to control small hydrophonics farm
 *
 * TODO: implement RTC
 * TODO: be sure that each process it turned on/off by.... either  flag or method. Why we need process manager is nrf is using directly turn pump on. Maybe better to have a singal/flag?
 * TODO: implement water flow sensor https://www.dfrobot.com/wiki/index.php/Water_Flow_Sensor_-_1/8%22_SKU:_SEN0216
 * TODO: Implement event handlers like desribed here: https://arobenko.gitbooks.io/bare_metal_cpp/content/basic_concepts/event_loop.html
 * TODO: mono switch enabling pump (manual mode)
 */

#define DEBUG_ON
/**
 * Select what module should be supported
 */
#define RTC_MODULE 0 //
#define TM1637_MODULE 1 //
#define NRF_MODULE 1//
#define WATER_FLOW_MODULE 0//
#define LIGHT_SENSOR_MODULE 1//

#if NRF_MODULE
  #define MY_DEBUG
  #define MY_RADIO_NRF24
  #define MY_TRANSPORT_WAIT_READY_MS 3000 ///try to connect 3sec if no sucess continue the loop
  #include <MySensors.h>
  #define CHILD_ID_FOR_PUMP_RELAY 1
  #define CHILD_ID_FOR_WATER_METER 2
  #define PUMP_STATUS 0
  MyMessage pumpMsg(CHILD_ID_FOR_PUMP_RELAY,V_STATUS);
  //MyMessage waterSensorMsg(CHILD_ID_FOR_WATER_METER,V_LIGHT);

  void presentation()
  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Hydrophonics farm", "0.1");
  // Register this device as Waterflow sensor
  present(CHILD_ID_FOR_PUMP_RELAY, S_BINARY);
  //present(CHILD_ID_FOR_WATER_METER,S_LIGHT);
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
    if (message.type==V_STATUS) //TODO: if more  binary signals will be check here also CHILD_ID_FOR_PUMP_RELAY
    {
      // This way of switching pump is independed on main pumpProcess. For next period of time it will go as usual.
      // What if we will try to implement scheduler. This will not allow
      message.getBool()?turnPumpOn():turnPumpOff();
    }
  }
#endif

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


#if WATER_FLOW_MODULE
  volatile int NbTopsFan; //measuring the rising edges of the signal
  #define WATER_FLOW_PIN 6
  void rpm ()     //This is the function that the interupt calls
{
  NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal
}
void calculateWaterFlowRPM()
{
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
  sei();      //Enables interrupts
  delay (1000);   //Wait 1 second
  cli();      //Disable interrupts
  Calc = (NbTopsFan * 60 / 73); //(Pulse frequency x 60) / 73Q, = flow rate
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
unsigned long period_to_turn_pump_on=120000;
unsigned long period_to_turn_pump_off=7200000;
unsigned long last_pump_status_change=0;
#define RELAY_ON 0
#define RELAY_OFF 1
unsigned long current_time;

#if LIGHT_SENSOR_MODULE
  #define LIGHT_SENSOR_PIN 5
#endif

SerialCommand sCmd;

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

void pushToTurnPumpOff()
{
  turnPumpOff();
}

void pushToTurnPumpOn()
{
  turnPumpOn();
}


int turnPumpOff()
{
  digitalWrite(PUMP_MOTOR_PIN, RELAY_OFF);
  Serial.println("Pump is off");
  #if NRF_MODULE
    send(pumpMsg.setSensor(CHILD_ID_FOR_PUMP_RELAY).set(false), false); //TODO: check that
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
    send(pumpMsg.setSensor(CHILD_ID_FOR_PUMP_RELAY).set(true), false); //TODO: check that
  #endif
  return 0;
}
/**

*/
boolean processPump()
{
  if(!process_flags.pump_enabled) return false;
  if(isDark())
  {
      Serial.println("Is dark. Pump will be to noisy");
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
    pinMode(WATER_FLOW_PIN, INPUT); //initializes digital pin 2 as an input initialised,
    attachInterrupt(0, rpm, RISING); //and the interrupt is attached
    /***
    TODO implement that. Using delay is not a perfect solution.
    */
  #endif
  process_flags.pump_enabled=true;
  turnPumpOff();
  process_flags.pump_is_on=false;
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

void loop()
{
  current_time = millis();
  if(current_time>=last_run_pm+PERIOD_TO_RUN_PROCESS_MANAGER)
  {
    last_run_pm=current_time;
    processManager();
  }
  sCmd.readSerial();
  #if WATER_FLOW_MODULE
    calculateWaterFlowRPM();
  #endif
  #if NRF_MODULE
    sendStatusesViaNRF();
  #endif
}
#endif

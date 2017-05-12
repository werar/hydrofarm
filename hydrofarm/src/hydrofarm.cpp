#include <Arduino.h>
#ifndef UNIT_TEST  // IMPORTANT LINE!
#include "hydrofarm.h"

/**
 * Software to control small hydrophonics farm
 *
 * TODO: implement RTC
 * TODO: be sure that each process it turned on/off by.... either  flag or method. Why we need process manager is nrf is using directly turn pump on. Maybe better to have a singal/flag?
 * TODO: implement water flow sensor https://www.dfrobot.com/wiki/index.php/Water_Flow_Sensor_-_1/8%22_SKU:_SEN0216
 * TODO: Implement event handlers like desribed here: https://arobenko.gitbooks.io/bare_metal_cpp/content/basic_concepts/event_loop.html
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
unsigned long period_to_turn_pump_on=10000;
unsigned long period_to_turn_pump_off=60000;
unsigned long last_pump_status_change=0;
#define RELAY_ON 0
#define RELAY_OFF 1
unsigned long current_time;

#if LIGHT_SENSOR_MODULE
  #define LIGHT_SENSOR_PIN 5
#endif



/**
Main idea is to use process flags as semaphores to know what is just working and what is not
TODO: do we need struct?
*/
typedef struct process_flags_type
{
      boolean pump_enabled;
      boolean pump_is_on; //
};

typedef struct sensors_type
{
      int external_temperature;
      int digital_light_sensor;
};

process_flags_type process_flags;
sensors_type connected_sensors;

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
  process_flags.pump_is_on=!process_flags.pump_is_on;
}

/**
*
*/
int turnPumpOn()
{
  digitalWrite(PUMP_MOTOR_PIN, RELAY_ON);
  #if NRF_MODULE
    send(pumpMsg.setSensor(CHILD_ID_FOR_PUMP_RELAY).set(true), false); //TODO: check that
  #endif
  return 0;
}
/**

*/
int processPump()
{
  if(!process_flags.pump_enabled) return 0;
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
      #if LIGHT_SENSOR_MODULE
      /*** If the night is comming to avoid running noisy pump lets disable it. What if some lamp will be enabled or a day will be quite dark. This is not perfect solution*/
      connected_sensors.digital_light_sensor=digitalRead(LIGHT_SENSOR_PIN);
      if(connected_sensors.digital_light_sensor==1)
      {
          DEBUG_PRINTLN("Too dark to start the pump. Skiped");
      }else
        {
          turnPumpOn();
        }
      #else
          turnPumpOn();
      #endif
      last_pump_status_change=current_time;
      process_flags.pump_is_on=true;
    }
  }
  #if TM1637_MODULE
    DEBUG_PRINTLN((last_pump_status_change+(process_flags.pump_is_on?period_to_turn_pump_on:period_to_turn_pump_off)-current_time)/1000);
    show_time_on_LED((last_pump_status_change+(process_flags.pump_is_on?period_to_turn_pump_on:period_to_turn_pump_off)-current_time)/1000);
  #endif

  return 0;
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
  Serial.begin(115200);
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
}

void loop()
{
  current_time = millis();
  if(current_time>=last_run_pm+PERIOD_TO_RUN_PROCESS_MANAGER)
  {
    last_run_pm=current_time;
    processManager();
  }
  #if WATER_FLOW_MODULE
    calculateWaterFlowRPM();
  #endif
  #if NRF_MODULE
    sendStatusesViaNRF();
  #endif
}
#endif    // IMPORTANT LINE!

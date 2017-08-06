/**
 * Software to control small hydrophonics farm
 *
 * TODO: turn pump based on soil sensor state than cyclic mode
 * TODO: chnge function names convention _ _
 * TODO: fix bug with ont after restart
 * TODO: Implement event handlers like desribed here: https://arobenko.gitbooks.io/bare_metal_cpp/content/basic_concepts/event_loop.html
 * TODO:
 */

#include <Arduino.h>
#ifndef UNIT_TEST  // IMPORTANT LINE!
#include "hydrofarm.h"
#include "pump.h"
#include "serialConsole.h"
#include "eepromActions.h"
#include "soil.h"

#if NRF_MODULE
#include "nrf.h"
#include <MySensors.h>
#endif

#if WATER_FLOW_MODULE
#include "waterFlow.h"
#endif

#if SOIL_MODULE
#include "soil.h"
#endif

unsigned long last_run_pm=0;
unsigned long last_pump_status_change=0;
unsigned long current_time;

process_flags_type process_flags;
sensors_type connected_sensors;
volatile timers_type timers;

#if TM1637_MODULE
  #include <TM1637Display.h>
  // Module connection pins (Digital Pins)
  #define CLK A2
  #define DIO A3
  TM1637Display display(CLK, DIO);

  void show_report_on_LED()
  {
    if(timers.cyclic_report_counter>=CYCLIC_REPORT_SWITCH_REPORT_TIME)
    {
      if(timers.current_cyclic_report>=timers.max_cyclic_reports)
      {
        timers.current_cyclic_report=0;
      }else
      {
        timers.current_cyclic_report++;
      }
      timers.cyclic_report_counter=0;
    }
    switch(timers.current_cyclic_report)
    {
      case 0:
        show_time_on_LED((last_pump_status_change+(process_flags.pump_is_on?config_in_ram.period_to_turn_pump_on:config_in_ram.period_to_turn_pump_off)-current_time)/1000);
        break;
      case 1:
        show_soil_percentage();
        break;
    }
  }

  void show_time_on_LED(int value)
  {
    //display.showNumberDecEx(0, (0x80 >> k), true);
    if(value>9999)
    {
      value=value/60;
    }
    display.showNumberDec(value, false);
  }
  #if SOIL_MODULE
  void show_soil_percentage()
  {
    uint8_t soil_percentage=measureSoilPercentage();
    display.showNumberDec(soil_percentage, false);
  }
  #endif
#endif


/**
The pump can be
//TODO: consider situation to have more than one pump
*/
boolean processPump()
{
  if(!process_flags.pump_enabled) return false;

  if(process_flags.pump_is_on)
  {
    if(current_time>=last_pump_status_change+config_in_ram.period_to_turn_pump_on)
    {
      turnPumpOff();
      last_pump_status_change=current_time;
      process_flags.pump_is_on=false;
    }
  }else
  {
   if(current_time>=last_pump_status_change+config_in_ram.period_to_turn_pump_off)
    {
      if(isWet())
      {
          //return false;//TODO: soil measurment is non stable still!
      }
      if(isDark())
      {
          return false;
      }
      turnPumpOn();
      last_pump_status_change=current_time;
      process_flags.pump_is_on=true;
    }
  }
  #if TM1637_MODULE
   show_report_on_LED();
  #endif
  return true;
}

bool isDark()
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

bool isWet()
{
  uint8_t soil_percentage = measureSoilPercentage();
  if(soil_percentage>=MAX_WET_PERCENTAGE)
  {
    return true;
  }else
  {
    return false;
  }
}

int processManager()
{
  //pump manager
  processPump();
  #if SOIL_MODULE
  processSoil();
  #endif
  processReports();
  return 0;
}
void processSoil()
{
  uint8_t soil_percentage=measureSoilPercentage();
  connected_sensors.soil_percentage=soil_percentage;
}

void processReports()
{
  if(timers.counter_to_show_reports==0)
  {
    showReports();
    #if NRF_MODULE
    sendStatusesViaNRF();
    #endif
      timers.counter_to_show_reports=timers.count_to_show_reports;
  }else
  {
      timers.counter_to_show_reports--;
  }
}

void showReports()
{
  if(isDark())
  {
    Serial.println("Is to dark");
  }
  if(isWet())
  {
    Serial.println("Is to wet");
  }
  #if WATER_FLOW_MODULE
  calculateWaterFlowRate();
  #endif
  #if SOIL_MODULE
  uint8_t soil_percentage= measureSoilPercentage();
  Serial.print("Soil % and average: ");
  Serial.print(soil_percentage);
  Serial.print("%, ");
  long soil_avg = soil_average();
  Serial.println(soil_avg);
  #endif
}

/***
* TODO: move to common lib
*/
void initTimers()
{
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  TCCR1A |= (1<<WGM12); //CTC mode timer1
  TCCR1B |= (1<<CS11)|(1<<CS10); //prescaler64
  // 1 Hz (80000/((1249+1)*64))
  OCR1A = 1249; //(target time) = (timer resolution) * (# timer counts + 1)
  TIMSK1 |= (1<<OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect)
{
  PORTB ^= (1 << PB5); //bulit in led
  timers.cyclic_report_counter++;
}

void setup()
{
  Serial.begin(9600); //for BLUETOOTH HC6 //TODO set HC6 to use 115200
  //Serial.println("AT+BAUD8"); //http://42bots.com/tutorials/hc-06-bluetooth-module-datasheet-and-configuration-with-arduino/
  //Serial.begin(115200);

  pinMode(PUMP_MOTOR_PIN, OUTPUT);
  process_flags.pump_is_on=false;
  #if TM1637_MODULE
    display.setBrightness(0x0f);
  #endif
  #if LIGHT_SENSOR_MODULE
    pinMode(LIGHT_SENSOR_PIN,INPUT);
  #endif

  #if WATER_FLOW_MODULE
  initWaterFlow();
  #endif

  #if SOIL_MODULE
  initSoil();
  #endif
  pinMode(13, OUTPUT); //TODO magic number!
  addSerialCommands();

  load_default_config();  //TODO: use load_config after bug fix
  //load_config();
  //copy_eem_to_ram();
  Serial.println("Time off / Time on ");
  Serial.println(config_in_ram.period_to_turn_pump_off);
  Serial.println(config_in_ram.period_to_turn_pump_on);
  process_flags.pump_enabled=true;
  turnPumpOff();
  process_flags.pump_is_on=false;
  timers.counter_to_show_reports=timers.count_to_show_reports;
  initTimers();
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

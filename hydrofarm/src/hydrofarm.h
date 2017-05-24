#ifndef HYDROFARM_H_
#define HYDROFARM_H_

/**
 * Select what module should be supported
 */
#define RTC_MODULE 0 //
#define TM1637_MODULE 1 //
#define NRF_MODULE 1//
#define WATER_FLOW_MODULE 1//
#define LIGHT_SENSOR_MODULE 1//

#define DEBUG_ON

#ifdef DEBUG_ON
  #define DEBUG_PRINT(x)   Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#if LIGHT_SENSOR_MODULE
  #define LIGHT_SENSOR_PIN 5
#endif

#define RELAY_ON 0
#define RELAY_OFF 1

//
extern unsigned long period_to_turn_pump_on;
extern unsigned long period_to_turn_pump_off;
extern unsigned long last_pump_status_change;
extern unsigned long current_time;

/**
Main idea is to use process flags as semaphores to know what is just working and what is not
TODO: do we need struct?
*/
typedef struct
{
      bool pump_enabled;
      bool pump_is_on; //
} process_flags_type;

typedef struct sensors_type
{
      int external_temperature;
      int digital_light_sensor;
} ;

extern process_flags_type process_flags;
extern sensors_type connected_sensors;


int turnPumpOn();
int turnPumpOff();
bool processPump();
bool isDark();

void unrecognized(const char *command);
void LED_on();
#endif

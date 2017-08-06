#ifndef HYDROFARM_H_
#define HYDROFARM_H_

/**
 * Select what module should be supported
 */
#define RTC_MODULE 0 //
#define TM1637_MODULE 1 //
#define NRF_MODULE 1//TODO: linker has problem when set to 0
#define WATER_FLOW_MODULE 1//
#define LIGHT_SENSOR_MODULE 1//
#define SOIL_MODULE 1//


#define PERIOD_TO_RUN_PROCESS_MANAGER 1000 //in ms
#define PERIOD_TO_SHOW_SERIAL_REPORTS 60
#define MAX_WET_PERCENTAGE 100
#define MAX_CYCLIC_BUFFERS 1
#define CYCLIC_REPORT_SWITCH_REPORT_TIME 5

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
      uint8_t soil_percentage;
};

//TODO: move all timers related varabiles here
typedef struct timers_type
{
      unsigned long count_to_show_reports=PERIOD_TO_SHOW_SERIAL_REPORTS;
      unsigned long counter_to_show_reports;
      uint8_t current_cyclic_report=0;
      uint8_t max_cyclic_reports=MAX_CYCLIC_BUFFERS;
      uint32_t cyclic_report_counter=0;
};

extern process_flags_type process_flags;
extern sensors_type connected_sensors;

bool processPump();
bool isDark();
bool isWet();
void processSoil();
void processReports();
void show_time_on_LED(int value);
void show_soil_percentage();
void showReports();

void unrecognized(const char *command);
void LED_on();
#endif

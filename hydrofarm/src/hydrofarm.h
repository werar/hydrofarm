/**
Main idea is to use process flags as semaphores to know what is just working and what is not
TODO: do we need struct?
*/
typedef struct process_flags_type
{
      boolean pump_enabled;
      boolean pump_is_on; //
} ;

typedef struct sensors_type
{
      int external_temperature;
      int digital_light_sensor;
} ;

process_flags_type process_flags;
sensors_type connected_sensors;


int turnPumpOn();
int turnPumpOff();
boolean processPump();
boolean isDark();

void unrecognized(const char *command);
void LED_on();

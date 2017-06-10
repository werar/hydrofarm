#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#ifndef CONFIG_H_
#define CONFIG_H_


#define PERIOD_TO_TURN_PUMP_ON 180000 //in millis
#define PERIOD_TO_TURN_PUMP_OFF 14400000 //in millis


typedef struct {
  unsigned long period_to_turn_pump_on;
  unsigned long period_to_turn_pump_off;
} config_type;



extern const config_type config_in_progmem PROGMEM;		// dane w pami�ci FLASH
extern config_type config_in_eeprom EEMEM;		// dane w pami�ci EEPROM
extern config_type config;		// dane w pami�ci RAM

extern void load_default_config( void );

void load_config( void );
void copy_eem_to_ram( void );
void copy_ram_to_eem( void );
void copy_pgm_to_ram( void );


#endif

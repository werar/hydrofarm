#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <Arduino.h>

#include "config.h"

config_type config_in_eeprom EEMEM;
config_type config_in_ram;

const config_type config_in_progmem PROGMEM = {
  PERIOD_TO_TURN_PUMP_ON,
  PERIOD_TO_TURN_PUMP_OFF
};


void copy_eem_to_ram( void ) {
	eeprom_read_block( (void*)&config_in_ram, (void*)&config_in_eeprom, sizeof(config_in_ram) );
}

void copy_ram_to_eem( void ) {
	eeprom_write_block( (void*)&config_in_ram, (void*)&config_in_eeprom, sizeof(config_in_ram) );
}

void copy_pgm_to_ram( void ) {
	memcpy_P( &config_in_ram, &config_in_progmem, sizeof(config_in_ram) );
}

void load_default_config( void ) {
	copy_pgm_to_ram();
	copy_ram_to_eem();
}

void load_config( void ) {
	uint8_t i;
  uint8_t len = sizeof( config_in_ram );
	uint8_t * ram_wsk = (uint8_t*)&config_in_ram;

	copy_eem_to_ram();
  Serial.println(config_in_ram.period_to_turn_pump_on/1000);
  Serial.println(config_in_ram.period_to_turn_pump_off/1000);
	for(i=0; i<len; i++) {
		if( 0xff == *ram_wsk++ ) continue;
		break;
	}

	if( i == len ) {
		load_default_config();
	}
}

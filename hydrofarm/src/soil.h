#ifndef SOIL_H_
#define SOIL_H_

#include <Arduino.h>

// Setting up format for reading 3 soil sensors
#define NUM_READS 10    // Number of sensor reads for filtering

int const SOIL_POWER_1=6;
int const SOIL_POWER_2=7;
int const SOIL_AD_1=A1;
int const SOIL_AD_2=A2;

//The sensor measures resistance. But humidity will be sent as percentage.
//Set min and max resistance values for 0% and for 100% humidity range.
#define MIN_RESISTANCE 10700
#define MAX_RESISTANCE 115000

long const knownResistor = 4700;  /*!< Constant value of known resistor in Ohms */

void addReading(long resistance);
void soilMeasure (int phase_b, int phase_a, int analog_input);
long soil_average();
extern void initSoil();
extern uint8_t measureSoilPercentage();


#endif //for SOIL_H_

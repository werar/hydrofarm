/*! \brief Soil sensor based on gypsum
 *
 * This project is clone of the standard soil sensor example from mysensors.org
 * It uses gypsum (resistive) sensor. How to build gypsum sensor is described here: http://unpuntilloalambre.blogspot.com.es/2014/01/gypsum-block-for-soil-moisture-sensor.html
 * Addition circuit is needed to limit sensor degradation: http://vanderleevineyard.com/1/post/2012/08/-the-vinduino-project-3-make-a-low-cost-soil-moisture-sensor-reader.html
 * In spice notation it looks like that:
 * Diode1 D6 A1 1N4148
 * Diode2 D7 A2 1N4148
 * R1     A1 0 4.7k
 * R2     A2 0 4.7k
 * sensor A1 A2
 *
 * In original exampe centibar units are used, but this project is connected to domoticz automation system and emulates humdity sensor in presentage.
 *
 * The original project is described here: https://github.com/ReiniervdL/Vinduino/blob/master/Vinduino-R3/VDO_R3%20SCH.pdf
 * Somthing about centibar unit is here: http://www.irrometer.com/basics.html
 *            0-10 Saturated Soil. Occurs for a day or two after irrigation
 *            10-20 Soil is adequately wet (except coarse sands which are drying out at this range)
 *            30-60 Usual range to irrigate or water (except heavy clay soils).
 *            60-100 Usual range to irrigate heavy clay soils
 *            100-200 Soil is becoming dangerously dry for maximum production. Proceed with caution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
 //TODO: use used variable convention

#include <math.h>       // Conversion equation from resistance to %
#include "soil.h"

#define MY_DEBUG

int supplyVoltage;                /*!< Measured supply voltage */
int sensorVoltage;                /*!< Measured sensor voltage */

long buffer[NUM_READS];
int index;

// @brief Structure to be used in percentage and resistance values matrix to be filtered (have to be in pairs)
/*
typedef struct {
  int moisture; //!< Moisture
  long resistance; //!< Resistance
} values;*/

//values valueOf[NUM_READS];        // Calculated moisture percentages and resistances to be sorted and filtered

void initSoil() {
  pinMode(SOIL_POWER_1, OUTPUT);
  pinMode(SOIL_POWER_2, OUTPUT);
  analogReference(INTERNAL);
}

uint8_t measureSoilPercentage() {
  soilMeasure(SOIL_POWER_1,SOIL_POWER_2,SOIL_AD_2);
  long read1 = soil_average();
  soilMeasure(SOIL_POWER_2,SOIL_POWER_1,SOIL_AD_1);
  long read2= soil_average();
  long sensor1 = (read1 + read2)/2;
  #ifdef MY_DEBUG
    Serial.print ("resistance bias =" );
    Serial.println (read1-read2);
    Serial.print ("sensor bias compensated value = ");
    Serial.println (sensor1);
    Serial.println ();
  #endif
  long soil_value = constrain(sensor1, MIN_RESISTANCE, MAX_RESISTANCE); //limit measured resistance
  uint8_t soil_map = map(soil_value, MIN_RESISTANCE, MAX_RESISTANCE, 100, 0); //reverted MIN_RESISTANCE = 100%
  return soil_map;
}

void soilMeasure (int phase_b, int phase_a, int analog_input){
  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter
  for (int i=0; i<NUM_READS; i++) {
    // Read 1 pair of voltage values
    digitalWrite(phase_a, HIGH);                 // set the voltage supply on
    delayMicroseconds(25);
    supplyVoltage = analogRead(analog_input);   // read the supply voltage
    delayMicroseconds(25);
    digitalWrite(phase_a, LOW);                  // set the voltage supply off
    delay(1);
    digitalWrite(phase_b, HIGH);                 // set the voltage supply on
    delayMicroseconds(25);
    sensorVoltage = analogRead(analog_input);   // read the sensor voltage
    delayMicroseconds(25);
    digitalWrite(phase_b, LOW);                  // set the voltage supply off

    // Calculate resistance
    // the 0.5 add-term is used to round to the nearest integer
    // Tip: no need to transform 0-1023 voltage value to 0-5 range, due to following fraction
    long resistance = (knownResistor * (supplyVoltage - sensorVoltage ) / sensorVoltage) ;
    delay(1);
    addReading(resistance);
  }
}

// Averaging algorithm
void addReading(long resistance){
  buffer[index] = resistance;
  index++;
  if (index >= NUM_READS) index = 0;
}

long soil_average(){
  long sum = 0;
  for (int i = 0; i < NUM_READS; i++){
    sum += buffer[i];
  }
  return (long)(sum / NUM_READS);
}

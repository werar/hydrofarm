#include <Arduino.h>
#include <FlowMeter.h> // https://github.com/sekdiy/FlowMeter
#include "waterFlow.h"

// connect a flow meter to an interrupt pin (see notes on your Arduino model for pin numbers)
FlowMeter Meter = FlowMeter(WATER_FLOW_PIN);

// set the measurement update period to 1s (1000 ms)
const unsigned long period = 1000;

// define an 'interrupt service handler' (ISR) for every interrupt pin you use
void MeterISR() {
  // let our flow meter count the pulses
  Meter.count();
}

void initWaterFlow()
{
  // enable a call to the 'interrupt service handler' (ISR) on every rising edge at the interrupt pin
// do this setup step for every ISR you have defined, depending on how many interrupts you use
attachInterrupt(INT1, MeterISR, RISING);
// sometimes initializing the gear generates some pulses that we should ignore
Meter.reset();
}


void calculateWaterFlowRate()
{
  // process the (possibly) counted ticks
  Meter.tick(period);
  // output some measurement result
  Serial.println("Currently " + String(Meter.getCurrentFlowrate()) + " l/min, " + String(Meter.getTotalVolume())+ " l total.");
}

#ifndef WATERFLOW_H_
#define WATERFLOW_H_

#define WATER_FLOW_PIN 3 //INT1

void MeterISR();
extern void initWaterFlow();
extern void calculateWaterFlowRate();

#endif

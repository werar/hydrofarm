#ifndef NRF_H_
#define NRF_H_

#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_TRANSPORT_WAIT_READY_MS 3000 ///try to connect 3sec if no sucess continue the loop

#define CHILD_ID_FOR_PUMP_RELAY 1
#define CHILD_ID_FOR_WATER_METER 2
#define PUMP_STATUS 0

#include <core/MySensorsCore.h>
extern MyMessage pumpMsg;

int sendStatusesViaNRF();

#endif

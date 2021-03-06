
//#include "hydrofarm.h"
#include "nrf.h"
#include "hydrofarm.h"
#include "soil.h"
#include "pump.h"

  bool state = false;
  //MyMessage waterSensorMsg(CHILD_ID_FOR_WATER_METER,V_LIGHT);
  MyMessage soilMsg(CHILD_ID_FOR_SOIL, V_HUM);
  MyMessage pumpMsg(CHILD_ID_FOR_PUMP_RELAY,V_STATUS);

  void presentation()
  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Hydrophonics farm", "0.12");
  present(CHILD_ID_FOR_PUMP_RELAY, S_BINARY);
  present(CHILD_ID_FOR_SOIL, S_HUM);
  //present(CHILD_ID_FOR_WATER_METER,S_LIGHT);
  }

  int sendStatusesViaNRF()
  {
    uint8_t soil_percentage=measureSoilPercentage();
    send(soilMsg.set((long int)ceil(soil_percentage)));
    return 1;
  }

  void receive(const MyMessage &message)
  {
    if (message.type==V_STATUS && message.sensor==CHILD_ID_FOR_PUMP_RELAY) //TODO: if more  binary signals will be check here also CHILD_ID_FOR_PUMP_RELAY
    {
      // This way of switching pump is independed on main pumpProcess. For next period of time it will go as usual.
      // What if we will try to implement scheduler. This will not allow
      message.getBool()?turnPumpOn():turnPumpOff();
      //send(message.set(message.getBool()));
    }
}

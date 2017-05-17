
//#include "hydrofarm.h"
#include "nrf.h"


#if NRF_MODULE


  //MyMessage waterSensorMsg(CHILD_ID_FOR_WATER_METER,V_LIGHT);

  void presentation()
  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Hydrophonics farm", "0.1");
  // Register this device as Waterflow sensor
  present(CHILD_ID_FOR_PUMP_RELAY, S_BINARY);
  //present(CHILD_ID_FOR_WATER_METER,S_LIGHT);
  }

  int sendStatusesViaNRF()
  {
    //TODO implement that
    //send(volumeMsg.set(volume, 3));
    return -1;
  }

  void receive(const MyMessage &message)
  {
    // We only expect one type of message from controller. But we better check anyway.
    if (message.type==V_STATUS) //TODO: if more  binary signals will be check here also CHILD_ID_FOR_PUMP_RELAY
    {
      // This way of switching pump is independed on main pumpProcess. For next period of time it will go as usual.
      // What if we will try to implement scheduler. This will not allow
      message.getBool()?turnPumpOn():turnPumpOff();
    }
  }
#endif

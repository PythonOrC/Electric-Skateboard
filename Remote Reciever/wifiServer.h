#ifndef WFIISERVER_H
#define WIFISERVER_H

#include <cstdint>

class WifiServer
{
public:
    struct FaultCode
    {
        uint8_t faultCode;
        uint8_t faultSource;
    };
    struct DataPackage
    {
        float avgMotorCurrent;
        float avgInputCurrent;
        float dutyCycleNow;
        float rpm;
        float inpVoltage;
        float ampHours;
        float ampHoursCharged;
        float wattHours;
        float wattHoursCharged;
        long tachometer;
        long tachometerAbs;
        float tempMosfet;
        float tempMotor;
        float pidPos;
        uint8_t id;
        int faultCode;
        bool timedOut;
        bool timeoutSwitchActive;
    };
    WifiServer();
    void sendUDPMessage(WifiServer::DataPackage message);
    void receiveUDPMessage();

private:
};

#endif
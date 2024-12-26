#ifndef _VESCCOMM_h
#define _VESCCOMM_h

#include <cstdint>
#include <VescUart.h>
#include <SoftwareSerial.h>
class VescComm
{

public:
    struct VescData
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
        float pidPos;
        uint8_t id;
        mc_fault_code error;
        bool timedOut;
        bool timeoutSwitchActive;
    };
    VescComm();
    void setDuty(float duty);
    void setCurrent(float current);
    void setRPM(float rpm);
    void printVescValues();
    VescComm::VescData getData();
    VescData data;

private:
    void createBitmaskBits();
    uint32_t createBitmask(bool values[32]);
    VescUart vescUart;
    SoftwareSerial vescSerial;
    bool bit_mask[32] = {false};
    uint32_t mask;
    void toVescCommData();
};

#endif
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
        float tempMosfet;
        float tempMotor;
        float pidPos;
        uint8_t id;
        mc_fault_code error;
        bool timedOut;
        bool timeoutSwitchActive;
    };
    VescComm();
    void commInit();
    void sendUDPMessage(uint8_t message);
    void receiveUDPMessage();
    void setDuty(float duty);
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
};

#endif
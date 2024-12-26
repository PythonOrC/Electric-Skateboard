#ifndef WFIICLIENT_H
#define WFIICLIENT_H

#include <cstdint>

class WifiClient
{
public:
    enum ControlMode
    {
        DUTY = 0,
        CURRENT = 1,
        RPM = 2,
    };
    struct RemoteDataPackage
    {
        float dutyCycle;
        float current;
        ControlMode controlMode;
    };
    struct VescDataPackage
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
        int faultCode;
        bool timedOut;
        bool timeoutSwitchActive;
    };
    WifiClient();
    bool connectedToServer();
    void reconnect();
    bool tcpConnected();
    void sendTCPMessage(WifiClient::RemoteDataPackage message);
    void receiveTCPMessage();
    void printVescValues();
    VescDataPackage vescData;

private:
    void buffer_append_int32(uint8_t *buffer, int32_t number, int32_t *index);
    void buffer_append_float32(uint8_t *buffer, float number, float scale, int32_t *index);
    int32_t buffer_get_int32(const uint8_t *buffer, int32_t *index);
    float buffer_get_float32(const uint8_t *buffer, float scale, int32_t *index);
};

#endif
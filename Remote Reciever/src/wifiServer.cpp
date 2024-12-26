#include <WiFi.h>
#include "wifiServer.h"
#include <Arduino.h>

// TCP setup

WifiServer::WifiServer()
{
}


void WifiServer::sendTCPMessage(WifiServer::VescDataPackage message)
{
    // Create a packet to send
    uint8_t packet[255];
    // add the data to the packet
    int index = 0;
    buffer_append_float32(packet, message.avgMotorCurrent, 100.0, &index);
    buffer_append_float32(packet, message.avgInputCurrent, 100.0, &index);
    buffer_append_float32(packet, message.dutyCycleNow, 1000.0, &index);
    buffer_append_float32(packet, message.rpm, 1.0, &index);
    buffer_append_float32(packet, message.inpVoltage, 10.0, &index);
    buffer_append_float32(packet, message.ampHours, 10000.0, &index);
    buffer_append_float32(packet, message.ampHoursCharged, 10000.0, &index);
    buffer_append_float32(packet, message.wattHours, 10000.0, &index);
    buffer_append_float32(packet, message.wattHoursCharged, 10000.0, &index);
    buffer_append_int32(packet, message.tachometer, &index);
    buffer_append_int32(packet, message.tachometerAbs, &index);
    packet[index++] = message.faultCode;
    buffer_append_float32(packet, message.pidPos, 1000000.0, &index);
    packet[index++] = message.id;
    packet[index++] = message.timedOut ? 1 : 0;
    packet[index++] = message.timeoutSwitchActive ? 1 : 0;

    wifiClient.write(packet, index);
}

bool WifiServer::receiveTCPMessage()
{
    char incomingPacket[255]; // Buffer for incoming packets
                              // recieve the packet and get the size

    int packetSize = wifiClient.available();
    // read the remote packet into the remoteDataPackage struct
    if (packetSize)
    {
        Serial.printf("Received %d bytes from %s, port %d\n", packetSize, wifiClient.remoteIP().toString().c_str(), wifiClient.remotePort());
        wifiClient.read((uint8_t *)incomingPacket, packetSize);
        int index = 0;
        remoteData.dutyCycle = buffer_get_float32((uint8_t *)incomingPacket, 1000.0, &index);
        remoteData.current = buffer_get_float32((uint8_t *)incomingPacket, 100.0, &index);
        remoteData.controlMode = (ControlMode)incomingPacket[index];

        return true;
    }
    else
    {
        Serial.println("No data received");

        return false;
    }
}

void WifiServer::closeClient()
{
    // Close the client
    wifiClient.stop();
}
bool WifiServer::connectedToClient()
{
    // Check if a client is connected
    return wifiClient.connected();
}

void WifiServer::buffer_append_int32(uint8_t *buffer, int32_t number, int32_t *index)
{
    buffer[(*index)++] = (number >> 24) & 0xFF;
    buffer[(*index)++] = (number >> 16) & 0xFF;
    buffer[(*index)++] = (number >> 8) & 0xFF;
    buffer[(*index)++] = number & 0xFF;
}

void WifiServer::buffer_append_float32(uint8_t *buffer, float number, float scale, int32_t *index)
{
    int32_t intNumber = number * scale;
    buffer_append_int32(buffer, intNumber, index);
}

int32_t WifiServer::buffer_get_int32(const uint8_t *buffer, int32_t *index)
{
    int32_t res = ((int32_t)buffer[*index]) << 24 |
                  ((int32_t)buffer[*index + 1]) << 16 |
                  ((int32_t)buffer[*index + 2]) << 8 |
                  ((int32_t)buffer[*index + 3]);
    *index += 4;
    return res;
}

float WifiServer::buffer_get_float32(const uint8_t *buffer, float scale, int32_t *index)
{
    return (float)buffer_get_int32(buffer, index) / scale;
}
#include <WifiClient.h>
#include <stdint.h>
#include <WiFi.h>

// WiFi credentials
const char *ssid = "ESP32_AP";
const char *password = "password";

// UDP setup
WiFiClient tcpClient;
IPAddress serverIP(192, 168, 1, 1); // IP of the server
unsigned int serverPort = 4210;     // Port of the server

WifiClient::WifiClient()
{
    // init serial
    Serial.begin(9600);
    Serial.println();
    WiFi.mode(WIFI_STA);
    WiFi.setOutputPower(19.25);
    WiFi.disconnect();
    // connect to WPA2 network with SSID and password
    WiFi.begin("ESP8266_AP", "password");
}

bool WifiClient::connectedToServer()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connecting to WiFi");
        return false;
    }
    else
    {
        Serial.println("Connected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        // Begin TCP
        tcpClient.connect(serverIP, serverPort);
        return true;
    }
}

void WifiClient::reconnect()
{

    Serial.println("Reconnecting to server");
    tcpClient.connect(serverIP, serverPort);
}

bool WifiClient::tcpConnected()
{
    Serial.println("TCP is connected: " + String(tcpClient.connected()));
    return tcpClient.connected();
}

void WifiClient::sendTCPMessage(WifiClient::RemoteDataPackage message)
{
    // Create a packet to send
    uint8_t packet[255];
    // add the data to the packet
    int index = 0;
    buffer_append_float32(packet, message.dutyCycle, 1000.0, &index);
    buffer_append_float32(packet, message.current, 100.0, &index);
    packet[index++] = message.controlMode;

    // Send a response back to the server
    Serial.print("Sending packet of size: ");
    Serial.println(index);

    tcpClient.write(packet, index);
}

bool WifiClient::receiveTCPMessage()
{
    uint8_t incomingPacket[255]; // Buffer for incoming packets
    // recieve the packet and get the size
    int packetSize = tcpClient.available();

    // read the remote packet into the remoteDataPackage struct

    if (packetSize)
    {
        Serial.printf("Received %d bytes from %s, port %d\n", packetSize, tcpClient.remoteIP().toString().c_str(), tcpClient.remotePort());
        tcpClient.read(incomingPacket, packetSize);
        int index = 0;
        vescData.avgMotorCurrent = buffer_get_float32(incomingPacket, 100.0, &index);
        vescData.avgInputCurrent = buffer_get_float32(incomingPacket, 100.0, &index);
        vescData.dutyCycleNow = buffer_get_float32(incomingPacket, 1000.0, &index);
        vescData.rpm = buffer_get_float32(incomingPacket, 1.0, &index);
        vescData.inpVoltage = buffer_get_float32(incomingPacket, 10.0, &index);
        vescData.ampHours = buffer_get_float32(incomingPacket, 10000.0, &index);
        vescData.ampHoursCharged = buffer_get_float32(incomingPacket, 10000.0, &index);
        vescData.wattHours = buffer_get_float32(incomingPacket, 10000.0, &index);
        vescData.wattHoursCharged = buffer_get_float32(incomingPacket, 10000.0, &index);
        vescData.tachometer = buffer_get_int32((uint8_t *)incomingPacket, &index);
        vescData.tachometerAbs = buffer_get_int32((uint8_t *)incomingPacket, &index);
        vescData.pidPos = buffer_get_float32(incomingPacket, 1000000.0, &index);
        vescData.id = incomingPacket[index++];
        vescData.faultCode = incomingPacket[index++];
        vescData.timedOut = incomingPacket[index++] == 1;
        vescData.timeoutSwitchActive = incomingPacket[index++] == 1;

        return true;
    }
    else
    {
        Serial.println("No data received");

        return false;
    }
}

void WifiClient::buffer_append_int32(uint8_t *buffer, int32_t number, int32_t *index)
{
    buffer[(*index)++] = (number >> 24) & 0xFF;
    buffer[(*index)++] = (number >> 16) & 0xFF;
    buffer[(*index)++] = (number >> 8) & 0xFF;
    buffer[(*index)++] = number & 0xFF;
}

void WifiClient::buffer_append_float32(uint8_t *buffer, float number, float scale, int32_t *index)
{
    int32_t num = (int32_t)(number * scale);
    buffer_append_int32(buffer, num, index);
}

int32_t WifiClient::buffer_get_int32(const uint8_t *buffer, int32_t *index)
{
    uint32_t res = ((uint32_t)buffer[*index]) << 24 |
                   ((uint32_t)buffer[*index + 1]) << 16 |
                   ((uint32_t)buffer[*index + 2]) << 8 |
                   buffer[*index + 3];
    *index += 4;
    return res;
}

float WifiClient::buffer_get_float32(const uint8_t *buffer, float scale, int32_t *index)
{
    int32_t num = buffer_get_int32(buffer, index);
    return (float)num / scale;
}

void WifiClient::printVescValues()
{
    Serial.print("Motor Current: ");
    Serial.println(vescData.avgMotorCurrent);
    Serial.print("Input Current: ");
    Serial.println(vescData.avgInputCurrent);
    Serial.print("Duty Cycle: ");
    Serial.println(vescData.dutyCycleNow);
    Serial.print("RPM: ");
    Serial.println(vescData.rpm);
    Serial.print("Input Voltage: ");
    Serial.println(vescData.inpVoltage);
    Serial.print("Amp Hours: ");
    Serial.println(vescData.ampHours);
    Serial.print("Amp Hours Charged: ");
    Serial.println(vescData.ampHoursCharged);
    Serial.print("Watt Hours: ");
    Serial.println(vescData.wattHours);
    Serial.print("Watt Hours Charged: ");
    Serial.println(vescData.wattHoursCharged);
    Serial.print("Tachometer: ");
    Serial.println(vescData.tachometer);
    Serial.print("Tachometer Abs: ");
    Serial.println(vescData.tachometerAbs);
    Serial.print("PID Position: ");
    Serial.println(vescData.pidPos);
    Serial.print("ID: ");
    Serial.println(vescData.id);
    Serial.print("Fault Code: ");
    Serial.println(vescData.faultCode);
    Serial.print("Timed Out: ");
    Serial.println(vescData.timedOut);
    Serial.print("Timeout Switch Active: ");
    Serial.println(vescData.timeoutSwitchActive);
}
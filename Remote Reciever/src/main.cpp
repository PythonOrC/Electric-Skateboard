#include "vescComm.h"
#include "WiFi.h"
#include <Arduino.h>

// Enums
enum ControlMode
{
	DUTY = 0,
	CURRENT = 1,
	RPM = 2,
};
// Structs
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
// headers
void connectNewClient();
void sendTCPMessage(VescDataPackage message);
bool receiveTCPMessage();

// WiFi credentials
const char *ssid = "ESP32_AP";
const char *password = "password";

VescComm vescComm;
VescComm::VescData data;
VescDataPackage wifiData;
RemoteDataPackage remoteData;
WiFiClient wifiClient;
WiFiServer tcpServer(4210);
int LastTime = 0;

void setup()
{
	Serial.begin(9600);
	Serial.println("Starting WiFi Server");
	WiFi.softAP(ssid, password);
	IPAddress apIP = WiFi.softAPIP();
	Serial.println("Access Point started");
	Serial.print("IP Address: ");
	Serial.println(WiFi.softAPIP());

	// Begin TCP
	tcpServer.begin();
	Serial.printf("TCP Server started");
}

void connectNewClient()
{
	// print all active clients
	Serial.println("Available clients:");
	WiFiClient client = tcpServer.available();
	while (client)
	{
		Serial.println(client.remoteIP());
		client = tcpServer.available();
	}
	// Check if a client is trying to connect
	wifiClient = tcpServer.available();
	if (wifiClient)
	{
		Serial.print("New client connected: ");
		Serial.println(wifiClient.remoteIP());
	}
}
void sendTCPMessage(VescComm::VescData message)
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
	packet[index++] = message.error;
	buffer_append_float32(packet, message.pidPos, 1000000.0, &index);
	packet[index++] = message.id;
	packet[index++] = message.timedOut ? 1 : 0;
	packet[index++] = message.timeoutSwitchActive ? 1 : 0;

	wifiClient.write(packet, index);
}

bool receiveTCPMessage()
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

uint32_t createBitmask(bool values[32])
{
	uint32_t mask = 0;
	for (int i = 0; i < 32; i++)
	{
		if (values[i])
		{
			mask |= 1 << i;
		}
	}
	return mask;
}

void loop()
{
	connectNewClient();
	if (receiveTCPMessage())
	{
		switch (remoteData.controlMode)
		{
		case ControlMode::DUTY:
			vescComm.setDuty(remoteData.dutyCycle);
			break;

		case ControlMode::CURRENT:
			vescComm.setCurrent(remoteData.current);
			break;

		case ControlMode::RPM:
			vescComm.setRPM(remoteData.dutyCycle);
			break;
		default:
			vescComm.setDuty(0.0);
			break;
		}
	}
	data = vescComm.getData();
	sendTCPMessage(data);
}

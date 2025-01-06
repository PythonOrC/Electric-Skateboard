#include "vescComm.h"
#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Wire.h>

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
VescDataPackage toVescDataPackage(VescComm::VescData message);
void onDataSent(const uint8_t *mac, esp_now_send_status_t status);
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void readMacAddress();
uint32_t createBitmask(bool values[32]);
void printRemoteData();

// MAC Address of your receiver
// uint8_t broadcastAddress[] = {0x54, 0x43, 0xb2, 0xac, 0xee, 0xf8};
uint8_t broadcastAddress[] = {0xc8, 0xc9, 0xa3, 0xce, 0xff, 0xd0};

// Vesc Related Variables
VescComm vescComm;
VescComm::VescData data;

// ESP-Now Related Variables
VescDataPackage vescData;
RemoteDataPackage remoteData;
String success;
esp_now_peer_info_t peerInfo;

void setup()
{
	// Init Serial Monitor
	Serial.begin(9600);

	// Set device as a Wi-Fi Station
	WiFi.mode(WIFI_STA);
	Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
	readMacAddress();
	// Init ESP-NOW
	if (esp_now_init() != ESP_OK)
	{
		Serial.println("Error initializing ESP-NOW");
		return;
	}

	// Once ESPNow is successfully Init, we will register for Send CB to
	// get the status of Trasnmitted packet
	esp_now_register_send_cb(onDataSent);

	// Register peer
	memcpy(peerInfo.peer_addr, broadcastAddress, 6);
	peerInfo.channel = 0;
	peerInfo.encrypt = false;

	// Add peer
	if (esp_now_add_peer(&peerInfo) != ESP_OK)
	{
		Serial.println("Failed to add peer");
		return;
	}
	// Register for a callback function that will be called when data is received
	esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
}

void loop()
{
	// Send message via ESP-NOW
	vescData = toVescDataPackage(vescComm.getData());
	esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&vescData, sizeof(vescData));

	if (result == ESP_OK)
	{
		Serial.println("Sent with success");
	}
	else
	{
		Serial.println("Error sending the data");
	}
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
	Serial.print("\r\nLast Packet Send Status:\t");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
	if (status == 0)
	{
		success = "Delivery Success :)";
	}
	else
	{
		success = "Delivery Fail :(";
	}
}

// Callback when data is received
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
	Serial.println("Data received");

	memcpy(&remoteData, incomingData, sizeof(remoteData));

	printRemoteData();

	switch (remoteData.controlMode)
	{
	case ControlMode::DUTY:
		Serial.print("Setting Duty Cycle: ");
		Serial.println(remoteData.dutyCycle);
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

VescDataPackage toVescDataPackage(VescComm::VescData message)
{
	VescDataPackage package;
	package.avgMotorCurrent = message.avgMotorCurrent;
	package.avgInputCurrent = message.avgInputCurrent;
	package.dutyCycleNow = message.dutyCycleNow;
	package.rpm = message.rpm;
	package.inpVoltage = message.inpVoltage;
	package.ampHours = message.ampHours;
	package.ampHoursCharged = message.ampHoursCharged;
	package.wattHours = message.wattHours;
	package.wattHoursCharged = message.wattHoursCharged;
	package.tachometer = message.tachometer;
	package.tachometerAbs = message.tachometerAbs;
	package.pidPos = message.pidPos;
	package.id = message.id;
	package.faultCode = message.error;
	package.timedOut = message.timedOut;
	package.timeoutSwitchActive = message.timeoutSwitchActive;
	return package;
}

void readMacAddress()
{
	uint8_t baseMac[6];
	esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
	if (ret == ESP_OK)
	{
		Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
					  baseMac[0], baseMac[1], baseMac[2],
					  baseMac[3], baseMac[4], baseMac[5]);
	}
	else
	{
		Serial.println("Failed to read MAC address");
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

void printRemoteData()
{
	Serial.print("Duty Cycle: ");
	Serial.println(remoteData.dutyCycle);
	Serial.print("Current: ");
	Serial.println(remoteData.current);
	Serial.print("Control Mode: ");
	Serial.println(remoteData.controlMode);
}
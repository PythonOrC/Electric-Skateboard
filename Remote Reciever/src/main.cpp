#include "vescComm.h"
#include "wifiServer.h"
#include <Arduino.h>

WifiServer::VescDataPackage convertToWifiData(VescComm::VescData data);
// WiFi credentials
const char *ssid = "ESP32_AP";
const char *password = "password";

VescComm vescComm;
WifiServer wifiServer;
VescComm::VescData data;
WifiServer::VescDataPackage wifiData;
WiFiServer tcpServer(4210);
int LastTime = 0;
void setup()
{
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
	// Check if a client is trying to connect
	wifiClient = tcpServer.available();
	if (wifiClient)
	{
		Serial.println("New client connected");
	}
}

void loop() {}

// void loop()
// {
// 	wifiServer.connectNewClient();
// 	if (wifiServer.receiveTCPMessage())
// 	{
// 		switch (wifiServer.remoteData.controlMode)
// 		{
// 		case WifiServer::ControlMode::DUTY:
// 			vescComm.setDuty(wifiServer.remoteData.dutyCycle);
// 			break;

// 		case WifiServer::ControlMode::CURRENT:
// 			vescComm.setCurrent(wifiServer.remoteData.current);
// 			break;

// 		case WifiServer::ControlMode::RPM:
// 			vescComm.setRPM(wifiServer.remoteData.dutyCycle);
// 			break;
// 		default:
// 			vescComm.setDuty(0.0);
// 			break;
// 		}
// 	}
// 	data = vescComm.getData();
// 	wifiData = convertToWifiData(data);
// 	wifiServer.sendTCPMessage(wifiData);
// }

// WifiServer::VescDataPackage convertToWifiData(VescComm::VescData data)
// {
// 	// convert the data from the VescData struct to the DataPackage struct
// 	wifiData.avgMotorCurrent = data.avgMotorCurrent;
// 	wifiData.avgInputCurrent = data.avgInputCurrent;
// 	wifiData.dutyCycleNow = data.dutyCycleNow;
// 	wifiData.rpm = data.rpm;
// 	wifiData.inpVoltage = data.inpVoltage;
// 	wifiData.ampHours = data.ampHours;
// 	wifiData.ampHoursCharged = data.ampHoursCharged;
// 	wifiData.wattHours = data.wattHours;
// 	wifiData.wattHoursCharged = data.wattHoursCharged;
// 	wifiData.tachometer = data.tachometer;
// 	wifiData.tachometerAbs = data.tachometerAbs;
// 	wifiData.pidPos = data.pidPos;
// 	wifiData.id = data.id;
// 	wifiData.faultCode = data.error;
// 	wifiData.timedOut = data.timedOut;
// 	wifiData.timeoutSwitchActive = data.timeoutSwitchActive;

// 	return wifiData;
// }

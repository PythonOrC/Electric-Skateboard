
#include "vescComm.h"
#include "wifiServer.h"

WifiServer::VescDataPackage convertToWifiData(VescComm::VescData data);

VescComm vescComm;
WifiServer wifiServer;
VescComm::VescData data;
WifiServer::VescDataPackage wifiData;
int LastTime = 0;
float duty = 0.0;
float current = 0.0;
float dutyIncrement = 0.01;
float currentIncrement = 0.1;
void setup()
{
}

void loop()
{
	data = vescComm.getData();
	wifiData = convertToWifiData(data);
	wifiServer.sendUDPMessage(wifiData);
	wifiServer.receiveUDPMessage();
	switch (wifiServer.remoteData.controlMode)
	{
	case WifiServer::ControlMode::DUTY:
		vescComm.setDuty(wifiServer.remoteData.dutyCycle);
		break;

	case WifiServer::ControlMode::CURRENT:
		vescComm.setCurrent(wifiServer.remoteData.current);
		break;

	case WifiServer::ControlMode::RPM:
		vescComm.setRPM(wifiServer.remoteData.dutyCycle);
		break;
	default:
		vescComm.setDuty(0.0);
		break;
	}
	delay(50);
}

WifiServer::VescDataPackage convertToWifiData(VescComm::VescData data)
{
	// convert the data from the VescData struct to the DataPackage struct
	wifiData.avgMotorCurrent = data.avgMotorCurrent;
	wifiData.avgInputCurrent = data.avgInputCurrent;
	wifiData.dutyCycleNow = data.dutyCycleNow;
	wifiData.rpm = data.rpm;
	wifiData.inpVoltage = data.inpVoltage;
	wifiData.ampHours = data.ampHours;
	wifiData.ampHoursCharged = data.ampHoursCharged;
	wifiData.wattHours = data.wattHours;
	wifiData.wattHoursCharged = data.wattHoursCharged;
	wifiData.tachometer = data.tachometer;
	wifiData.tachometerAbs = data.tachometerAbs;
	wifiData.pidPos = data.pidPos;
	wifiData.id = data.id;
	wifiData.faultCode = data.error;
	wifiData.timedOut = data.timedOut;
	wifiData.timeoutSwitchActive = data.timeoutSwitchActive;

	return wifiData;
}

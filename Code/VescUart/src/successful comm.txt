
#include "vescComm.h"
// #include "wifiServer.h"

// WifiServer::DataPackage convertToWifiData(VescComm::VescData data);

VescComm vescComm;
// WifiServer wifiServer;
int LastTime = 0;
void setup()
{
}

void loop()
{

	vescComm.setDuty(0.25); // set the duty cycle to 25%
	// if (millis() - LastTime > 100)
	// {
	// 	LastTime = millis();
	// 	VescComm::VescData data = vescComm.getData();
	// 	// WifiServer::DataPackage wifiData = convertToWifiData(data);
	// 	// wifiServer.sendUDPMessage(wifiData);
	// }
	delay(50);
}

// WifiServer::DataPackage convertToWifiData(VescComm::VescData data)
// {
// 	// convert the data from the VescData struct to the DataPackage struct
// 	WifiServer::DataPackage wifiData;
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
// 	wifiData.tempMosfet = data.tempMosfet;
// 	wifiData.tempMotor = data.tempMotor;
// 	wifiData.pidPos = data.pidPos;
// 	wifiData.id = data.id;
// 	wifiData.faultCode = data.error;
// 	wifiData.timedOut = data.timedOut;
// 	wifiData.timeoutSwitchActive = data.timeoutSwitchActive;

// 	return wifiData;
// }
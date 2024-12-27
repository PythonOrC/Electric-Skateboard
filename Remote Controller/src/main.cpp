#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <stdint.h>
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

// function prototypes
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void readMacAddress();
void printVescValues();

// REPLACE WITH THE MAC Address of your receiver
// uint8_t broadcastAddress[] = {0x54, 0x43, 0xb2, 0xac, 0xee, 0xf8};
uint8_t broadcastAddress[] = {0xc8, 0xc9, 0xa3, 0xce, 0xff, 0xd0};
// Variable to store if sending data was successful
String success;
esp_now_peer_info_t peerInfo;
RemoteDataPackage remoteData;
VescDataPackage vescData;

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
	esp_now_register_send_cb(OnDataSent);

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
	esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
	remoteData.controlMode = ControlMode::DUTY;
}

void getReadings()
{
	remoteData.dutyCycle = 0.2;
	remoteData.controlMode = ControlMode::DUTY;
}

void loop()
{
	getReadings();
	esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&remoteData, sizeof(remoteData));

	if (result == ESP_OK)
	{
		Serial.println("Sent with success");
	}
	else
	{
		Serial.println("Error sending the data");
	}

}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
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
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
	Serial.println("Data received");
	// parse the received data, typecast it to uint8_t
	memcpy(&vescData, incomingData, sizeof(vescData));

	// print the received data
	printVescValues();
}

void printVescValues()
{
	// Serial.print("Motor Current: ");
	// Serial.println(vescData.avgMotorCurrent);
	// Serial.print("Input Current: ");
	// Serial.println(vescData.avgInputCurrent);
	// Serial.print("Duty Cycle: ");
	// Serial.println(vescData.dutyCycleNow);
	// Serial.print("RPM: ");
	// Serial.println(vescData.rpm);
	// Serial.print("Input Voltage: ");
	// Serial.println(vescData.inpVoltage);
	// Serial.print("Amp Hours: ");
	// Serial.println(vescData.ampHours);
	// Serial.print("Amp Hours Charged: ");
	// Serial.println(vescData.ampHoursCharged);
	// Serial.print("Watt Hours: ");
	// Serial.println(vescData.wattHours);
	// Serial.print("Watt Hours Charged: ");
	// Serial.println(vescData.wattHoursCharged);
	// Serial.print("Tachometer: ");
	// Serial.println(vescData.tachometer);
	// Serial.print("Tachometer Abs: ");
	// Serial.println(vescData.tachometerAbs);
	// Serial.print("PID Position: ");
	// Serial.println(vescData.pidPos);
	Serial.print("ID: ");
	Serial.println(vescData.id);
	// Serial.print("Fault Code: ");
	// Serial.println(vescData.faultCode);
	// Serial.print("Timed Out: ");
	// Serial.println(vescData.timedOut);
	// Serial.print("Timeout Switch Active: ");
	// Serial.println(vescData.timeoutSwitchActive);
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
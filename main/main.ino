// TODO GET READING TO BE WRITTEN, ALSO NEED TO ADD RPM MODE

#include <Wire.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <ezButton.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <stdint.h>

// define joystick pins
#define JOYSTICK_Y_PIN 0
#define JOYSTICK_X_PIN 1
#define JOYSTICK_BUTTON_PIN 2

enum ControlMode
{
	DUTY,
	CURRENT,
	RPM
};
struct UpdateArea
{
	int x;
	int y;
	int width;
	int height;
};

struct DisplayElement
{
	UpdateArea updateArea;
	String text;
	int size;
	int color;
};

struct DisplayData
{
	int batteryPercent;
	int throttlePercent;
	ControlMode controlMode;
	bool cruiseOn;
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
void drawText(int x, int y, String text);
void drawText(int x, int y, String text, int size);
void drawText(int x, int y, String text, int size, int color);
void basicOverlay();
void updateScreen();
void getThrottle();
int getJoystickPercent(int jotstickPin, int joystickCenter, int deadZone);
void getControlMode();
String getControlModeText(ControlMode mode);
void getCruise();
void calibrateCenter();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void readMacAddress();
void printVescValues();
void getReadings();

int lastTime = 0;
int screenRefreshMs = 50;
int deadZone = 30;
int xCenter;
int yCenter;
TFT_eSPI tft = TFT_eSPI(); // Invoke library
// define elements to be displayed on the screen
DisplayElement battery = {{118, 10, 46, 15}, "0%", 2, TFT_GREEN};
DisplayElement throttle = {{130, 30, 58, 15}, "0%", 2, TFT_GREEN};
DisplayElement controlMode = {{140, 70, 95, 16}, "Duty", 2, TFT_GREEN};
DisplayElement cruise = {{106, 90, 35, 15}, "OFF", 2, TFT_GREEN};

DisplayData displayData = {100, 0, DUTY, false};
bool xBackCenter = true;
ezButton button(JOYSTICK_BUTTON_PIN);

// REPLACE WITH THE MAC Address of your receiver
uint8_t broadcastAddress[] = {0x54, 0x43, 0xb2, 0xac, 0xee, 0xf8};
// uint8_t broadcastAddress[] = {0xc8, 0xc9, 0xa3, 0xce, 0xff, 0xd0};
// Variable to store if sending data was successful
String success;
esp_now_peer_info_t peerInfo;
RemoteDataPackage remoteData;
VescDataPackage vescData;

void setup()
{

	Serial.begin(115200);
	tft.begin();			   // Initialise the display
  tft.fillScreen(TFT_BLACK); // Black screen fill
  basicOverlay(); // Draw the basic overlay
	button.setDebounceTime(8);
	calibrateCenter();
	
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


}

void loop()
{
	button.loop();
	getThrottle();
	getControlMode();

	// update screen at desired refresh rate
	if (millis() - lastTime > screenRefreshMs)
	{
		lastTime = millis();
		// only update the portion of the screen that needs to be updated
		updateScreen();
	}

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

	delay(50);
}

void updateScreen()
{
	// only update if the value has changed
	if (battery.text != String(displayData.batteryPercent) + "%")
	{
		battery.text = String(displayData.batteryPercent) + "%";
		tft.fillRect(battery.updateArea.x, battery.updateArea.y, battery.updateArea.width, battery.updateArea.height, TFT_BLACK);
		drawText(battery.updateArea.x, battery.updateArea.y, battery.text, battery.size, battery.color);
	}

	if (throttle.text != String(displayData.throttlePercent) + "%")
	{
		throttle.text = String(displayData.throttlePercent) + "%";
		tft.fillRect(throttle.updateArea.x, throttle.updateArea.y, throttle.updateArea.width, throttle.updateArea.height, TFT_BLACK);
		drawText(throttle.updateArea.x, throttle.updateArea.y, throttle.text, throttle.size, throttle.color);
	}

	if (cruise.text != (displayData.cruiseOn ? "ON" : "OFF"))
	{
		cruise.text = displayData.cruiseOn ? "ON" : "OFF";
		tft.fillRect(cruise.updateArea.x, cruise.updateArea.y, cruise.updateArea.width, cruise.updateArea.height, TFT_BLACK);
		drawText(cruise.updateArea.x, cruise.updateArea.y, cruise.text, cruise.size, cruise.color);
	}

	if (getControlModeText(displayData.controlMode) != controlMode.text)
	{
		controlMode.text = getControlModeText(displayData.controlMode);
		tft.fillRect(controlMode.updateArea.x, controlMode.updateArea.y, controlMode.updateArea.width, controlMode.updateArea.height, TFT_BLACK);
		drawText(controlMode.updateArea.x, controlMode.updateArea.y, controlMode.text, controlMode.size, controlMode.color);
	}
}

void getCruise()
{
	if (button.isReleased())
	{
		displayData.cruiseOn = true;
	}
}

void getThrottle()
{

	getCruise();
	int throttle = getJoystickPercent(JOYSTICK_Y_PIN, yCenter, deadZone);
	if (throttle < 0)
	{
		displayData.cruiseOn = false;
		displayData.throttlePercent = throttle;
	}
	else if ((displayData.cruiseOn && throttle > displayData.throttlePercent) || !displayData.cruiseOn)
	{
		displayData.throttlePercent = throttle;
	}
}

String getControlModeText(ControlMode mode)
{
	switch (mode)
	{
	case DUTY:
		return "Duty";
	case CURRENT:
		return "Current";
	case RPM:
		return "Velocity";
	default:
		return "Duty";
	}
}

void calibrateCenter()
{
	xCenter = analogRead(JOYSTICK_X_PIN);
	yCenter = analogRead(JOYSTICK_Y_PIN);
}

void basicOverlay()
{
	// Draw a basic overlay
	drawText(10, 10, "Battery:", 2);
	drawText(10, 30, "Throttle:", 2);
	drawText(10, 50, "Control Mode:", 2);
	drawText(10, 90, "Cruise:", 2);

	// Draw the Display Elements
	drawText(battery.updateArea.x, battery.updateArea.y, battery.text, battery.size, battery.color);
	drawText(throttle.updateArea.x, throttle.updateArea.y, throttle.text, throttle.size, throttle.color);
	drawText(controlMode.updateArea.x, controlMode.updateArea.y, controlMode.text, controlMode.size, controlMode.color);
	drawText(cruise.updateArea.x, cruise.updateArea.y, cruise.text, cruise.size, cruise.color);
}

int getJoystickPercent(int joystickPin, int joystickCenter, int deadZone = 15)
{
	int joystickValue = analogRead(joystickPin);
	Serial.print("Pin: ");
	Serial.print(joystickPin);
	Serial.print(" Value: ");
	Serial.println(joystickValue);
	// convert the joystick value to a percentage value from -1.0 to 1.0
	int percentPos = map(joystickValue, joystickCenter, 4095, 0, 100);
	int percentNeg = map(joystickValue, 0, joystickCenter, -100, 0);

	if (joystickValue > joystickCenter + deadZone)
	{
		return percentPos;
	}
	else if (joystickValue < joystickCenter - deadZone)
	{
		return percentNeg;
	}
	else
	{
		return 0;
	}
}

void getControlMode()
{
	// read the joystick x value
	int joystickPercent = getJoystickPercent(JOYSTICK_X_PIN, xCenter, deadZone);
	// switch to the next control mode when the joystick is pushed to the rightmost 20% of the range
	// switch to the previous control mode when the joystick is pushed to the leftmost 20% of the range
	// the control mode only changes once per joystick movement

	if (joystickPercent > 80 && xBackCenter)
	{
		xBackCenter = false;
		switch (displayData.controlMode)
		{
		case DUTY:
			displayData.controlMode = CURRENT;
			break;
		case CURRENT:
			displayData.controlMode = RPM;
			break;
		case RPM:
			displayData.controlMode = DUTY;
			break;
		}
	}
	else if (joystickPercent < -80 && xBackCenter)
	{
		xBackCenter = false;
		switch (displayData.controlMode)
		{
		case DUTY:
			displayData.controlMode = RPM;
			break;
		case CURRENT:
			displayData.controlMode = DUTY;
			break;
		case RPM:
			displayData.controlMode = CURRENT;
			break;
		}
	}
	else if (joystickPercent > -80 && joystickPercent < 80)
	{
		xBackCenter = true;
	}
}

void drawText(int x, int y, String text)
{
	drawText(x, y, text, 1, TFT_WHITE);
}

void drawText(int x, int y, String text, int size)
{
	drawText(x, y, text, size, TFT_WHITE);
}

void drawText(int x, int y, String text, int size, int color)
{
	tft.setCursor(x, y);
	tft.setTextSize(size);
	tft.setTextColor(color);
	tft.print(text);
}

void getReadings()
{
	// TO BE WRITTEN
	switch (displayData.controlMode)
	{
	case DUTY:
		remoteData.dutyCycle = displayData.throttlePercent / 200.0;
		break;
	case CURRENT:
		remoteData.current = displayData.throttlePercent > 0 ? 1 : -1;
		break;
	case RPM:
		// TO BE WRITTEN
		break;

	default: // send duty cycle by default
		remoteData.dutyCycle = displayData.throttlePercent / 100.0;
		break;
	}

	remoteData.controlMode = displayData.controlMode;
}

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

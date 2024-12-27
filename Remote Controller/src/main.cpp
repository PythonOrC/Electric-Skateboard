#include <Wire.h>
#include <TFT_eSPI.h> // Hardware-specific library

// define joystick pins
#define JOYSTICK_Y_PIN 35
#define JOYSTICK_X_PIN 34

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

TFT_eSPI tft = TFT_eSPI(); // Invoke library

int lastTime = 0;
int screenRefreshMs = 50;
int deadZone = 30;
int xCenter = 1955;
int yCenter = 1960;
// define elements to be displayed on the screen
DisplayElement battery = {{118, 10, 46, 15}, "0%", 2, TFT_GREEN};
DisplayElement throttle = {{130, 30, 58, 15}, "0%", 2, TFT_GREEN};
DisplayElement controlMode = {{140, 70, 95, 16}, "Duty", 2, TFT_GREEN};
DisplayElement cruise = {{106, 90, 35, 15}, "OFF", 2, TFT_GREEN};

DisplayData displayData = {100, 0, DUTY, false};
bool xBackCenter = true;

void setup()
{

	Serial.begin(9600);
	tft.begin();			   // Initialise the display
	tft.fillScreen(TFT_BLACK); // Black screen fill
	basicOverlay();			   // Draw the basic overlay
}

void loop()
{
	getThrottle();
	getControlMode();

	// update screen at desired refresh rate
	if (millis() - lastTime > screenRefreshMs)
	{
		lastTime = millis();
		// only update the portion of the screen that needs to be updated
		updateScreen();
	}

	// delay(10);
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

void getThrottle()
{
	displayData.throttlePercent = getJoystickPercent(JOYSTICK_Y_PIN, yCenter, deadZone);
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
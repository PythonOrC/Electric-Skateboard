#include <stdint.h>
#include <VescUart.h>
#include <SoftwareSerial.h>

VescUart vescUart;
SoftwareSerial vescSerial(13, 15);
bool bit_mask[32] = {false};
uint32_t mask;
int lastTime = 0;
float dutyCycle = 0;

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

void createBitmaskBits()
{
	bit_mask[0] = false;  // FET temperature
	bit_mask[1] = false;  // Motor temperature
	bit_mask[2] = true;	  // Average motor current
	bit_mask[3] = true;	  // Average input current
	bit_mask[4] = false;  // Average D-axis current
	bit_mask[5] = false;  // Average Q-axis current
	bit_mask[6] = true;	  // Duty cycle now
	bit_mask[7] = true;	  // RPM
	bit_mask[8] = true;	  // Input voltage
	bit_mask[9] = true;	  // Amp hours
	bit_mask[10] = true;  // Amp hours charged
	bit_mask[11] = true;  // Watt hours
	bit_mask[12] = true;  // Watt hours charged
	bit_mask[13] = true;  // Tachometer
	bit_mask[14] = true;  // Tachometer absolute
	bit_mask[15] = true;  // Fault code
	bit_mask[16] = true;  // PID position
	bit_mask[17] = true;  // Controller ID
	bit_mask[18] = false; // MOS NTC temperature 1
	bit_mask[19] = false; // MOS NTC temperature 2
	bit_mask[20] = false; // MOS NTC temperature 3
	bit_mask[21] = true;  // Timeout status
}

void setup()
{

	/** Setup Serial port to display data */
	Serial.begin(9600);

	/** Setup SoftwareSerial port */
	vescSerial.begin(115200);

	/** Define which ports to use as UART */
	vescUart.setSerialPort(&vescSerial);
	vescUart.setDebugPort(&Serial);
	createBitmaskBits();
	mask = createBitmask(bit_mask);
	Serial.print("Mask: ");
	Serial.println(mask);
}

void loop()
{
	// set motor properties for the vesc controller
	if (millis() - lastTime > 100)
	{
		vescUart.setDuty(dutyCycle);
		dutyCycle += 0.01;
		if (dutyCycle > 0.3)
		{
			dutyCycle = 0.0;
		}
		lastTime = millis();
	}

	// retrieve data from the vesc controller
	vescUart.getVescValuesSelective(mask);

	delay(50);
}

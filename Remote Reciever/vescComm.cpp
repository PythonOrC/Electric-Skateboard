#include "vescComm.h"
#include <stdint.h>
#include <VescUart.h>
#include <SoftwareSerial.h>

#define DEBUG true
VescComm::VescComm() : vescSerial(13, 15)
{
    /** Setup Serial port to display data */
    Serial.begin(9600);

    /** Setup SoftwareSerial port */
    vescSerial.begin(115200);

    /** Define which ports to use as UART */
    vescUart.setSerialPort(&vescSerial);
    if (DEBUG)
    {
        vescUart.setDebugPort(&Serial);
    }
    createBitmaskBits();
    mask = createBitmask(bit_mask);
    Serial.print("Mask: ");
    Serial.println(mask);
}

uint32_t VescComm::createBitmask(bool values[32])
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

void VescComm::createBitmaskBits()
{
    bit_mask[0] = false;  // FET temperature
    bit_mask[1] = false;  // Motor temperature
    bit_mask[2] = true;   // Average motor current
    bit_mask[3] = true;   // Average input current
    bit_mask[4] = false;  // Average D-axis current
    bit_mask[5] = false;  // Average Q-axis current
    bit_mask[6] = true;   // Duty cycle now
    bit_mask[7] = true;   // RPM
    bit_mask[8] = true;   // Input voltage
    bit_mask[9] = true;   // Amp hours
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

void VescComm::setDuty(float duty)
{
    Serial.print("Setting Duty: ");
    Serial.println(duty);
    vescUart.setDuty(duty);
    Serial.print("Duty: ");
    Serial.println(duty);
}

VescComm::VescData VescComm::getData()
{
    vescUart.getVescValuesSelective(mask);
    if (DEBUG)
    {
        printVescValues();
    }
    return data;
}

void VescComm::printVescValues()
{
    Serial.print("Motor Current: ");
    Serial.println(data.avgMotorCurrent);
    Serial.print("Input Current: ");
    Serial.println(data.avgInputCurrent);
    Serial.print("Duty Cycle: ");
    Serial.println(data.dutyCycleNow);
    Serial.print("RPM: ");
    Serial.println(data.rpm);
    Serial.print("Input Voltage: ");
    Serial.println(data.inpVoltage);
    Serial.print("Amp Hours: ");
    Serial.println(data.ampHours);
    Serial.print("Amp Hours Charged: ");
    Serial.println(data.ampHoursCharged);
    Serial.print("Watt Hours: ");
    Serial.println(data.wattHours);
    Serial.print("Watt Hours Charged: ");
    Serial.println(data.wattHoursCharged);
    Serial.print("Tachometer: ");
    Serial.println(data.tachometer);
    Serial.print("Tachometer Absolute: ");
    Serial.println(data.tachometerAbs);
    Serial.print("MOSFET Temp: ");
    Serial.println(data.tempMosfet);
    Serial.print("Motor Temp: ");
    Serial.println(data.tempMotor);
    Serial.print("PID Position: ");
    Serial.println(data.pidPos);
    Serial.print("Controller ID: ");
    Serial.println(data.id);
    Serial.print("Fault Code: ");
    Serial.println(data.error);
    Serial.print("Timeout: ");
    Serial.println(data.timedOut);
    Serial.print("Timeout Switch Active: ");
    Serial.println(data.timeoutSwitchActive);
}
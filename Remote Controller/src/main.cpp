#include <WifiClient.h>
#include <Arduino.h>

WifiClient wifiClient;
WifiClient::RemoteDataPackage remoteData;

void setup()
{
  while (!wifiClient.connectedToServer())
  {
    delay(1000);
  }
  remoteData.controlMode = WifiClient::ControlMode::DUTY;
}

void loop()
{
  // continue only if the client is connected
  if (!wifiClient.tcpConnected())
  {
    Serial.println("Client not connected");
    wifiClient.reconnect();
  }
  else
  {
    Serial.println("Client connected");
    remoteData.dutyCycle = 0.2;
    wifiClient.sendTCPMessage(remoteData);
    wifiClient.receiveTCPMessage();
  }
  delay(50);
}

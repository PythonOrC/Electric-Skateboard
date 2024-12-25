#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "wifiServer.h"
// WiFi credentials
const char *ssid = "ESP8266_AP";
const char *password = "password";

// UDP setup
WiFiUDP udp;
unsigned int localPort = 4210; // Port for the server to listen on

WifiServer::WifiServer()
{
    // Set up the ESP8266 as an Access Point
    WiFi.softAP(ssid, password);
    IPAddress apIP(192, 168, 1, 1); // Static IP for the server
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    Serial.println("Access Point started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Begin UDP
    udp.begin(localPort);
    Serial.printf("Listening on UDP port %d\n", localPort);
}

void WifiServer::sendUDPMessage(WifiServer::DataPackage message)
{
    // Create a packet to send
    char packet[255];

    // Send a response back to the client
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(packet);
    udp.endPacket();
}

void WifiServer::receiveUDPMessage()
{
    char incomingPacket[255]; // Buffer for incoming packets
    int packetSize = udp.parsePacket();

    if (packetSize)
    {
        // Read the incoming packet
        int len = udp.read(incomingPacket, 255);
        if (len > 0)
        {
            incomingPacket[len] = 0; // Null-terminate the string
        }

        // Print received packet
        Serial.printf("Received packet: %s\n", incomingPacket);
    }
}
#include "Connection.h"
#include "Settings.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Create Wifi Connector
WifiConnector::WifiConnector(bool inDebugMode)
{
  debugMode = inDebugMode;
  tryingReconnect = false;
}

// Set params and try to connect
void WifiConnector::start()
{

  if (debugMode){
    Serial.println("Wifi) Start ");
    Serial.print("\tConnecting to ");
    Serial.println(WIFI_SSID);
  }

  // Setup Wifi
  WiFi.mode(WIFI_STA);
  WiFi.hostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);   

  // This funtion is called from main setup function,
  // Wait until unit has wifi before continuing
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  if (debugMode){
        Serial.print(".");
  }
  }
  if (debugMode){
    Serial.println("Wifi) Connected!");
    Serial.println("Wifi) IP: ");
    Serial.println(WiFi.localIP());
  }
  // Set wifi bool
  hasWIFI = true;
}

// If wifi connection is lost, try to reconnect
void WifiConnector::reconnect()
{

  if (debugMode){
    Serial.println("Wifi) Try to reconnect!");
  }
  
  // First hit, try to reconnect. 
  if(!tryingReconnect)
  {
    WiFi.reconnect();
    tryingReconnect = true;
  }

}

// handle function called from main loop
void WifiConnector::handle()
{
    
  // Check if WIfi is Connected
  if(!WiFi.isConnected() && !tryingReconnect){
        
    if (debugMode){
      Serial.println("Wifi) Disconnected!");
    }

    // Set bool false and try to reconnect
    hasWIFI = false;
    reconnect();
 
  }else if (WiFi.isConnected() && !hasWIFI){ // Wifi is Reconnected

    if (debugMode){
      Serial.println("Wifi) Reconnected!");
    }

    hasWIFI = true;
    tryingReconnect = false;

  }
        
}

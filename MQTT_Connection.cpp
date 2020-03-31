#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include "WiFiClient.h"
#include "MQTT_Connection.h"
#include "Settings.h"

bool RESET = false;
void callback(char* topic, byte* payload, unsigned int length);

WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);
String mac2Str(const uint8_t* mac){
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);}
  return result;
}
String ip2Str(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++) {
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

MQTTPublisher::MQTTPublisher(bool inDebugMode)
{
  debugMode = inDebugMode;
}

MQTTPublisher::~MQTTPublisher()
{
  client.publish("watermeter", "offline");
  client.disconnect();
}
void MQTTPublisher::start()
{
      String clientName;
      clientName +=  WIFI_HOSTNAME;
      clientName += "-";
      uint8_t mac[6];

  
  if (String(MQTT_SERVER).length() == 0 || MQTT_PORT == 0)
  {
    Serial.println("MQTT) disabled. No hostname or port set.");
    return; //not configured
  }
  if(debugMode){
    Serial.println("MQTT) enabled. Connecting.");
  }
      WiFi.macAddress(mac);
      long rssi= WiFi.RSSI();
      String ip = ip2Str(WiFi.localIP());
 if (client.connect (clientName.c_str(),MQTT_USER,MQTT_PASS)){
  } else reconnect(); //connect right away
 
  isStarted = true;
        client.subscribe((PUB_TOPIC "correct/reset"));
        client.subscribe((PUB_TOPIC "correct/Cold: "));
        client.subscribe((PUB_TOPIC "correct/Hot:  "));
        send_mqtt(PUB_TOPIC,"client_RSSI",String(rssi).c_str());
        send_mqtt(PUB_TOPIC,"client_IP" ,ip.c_str());
        send_mqtt(PUB_TOPIC,"client_mac" ,mac2Str(mac).c_str());
        send_mqtt(PUB_TOPIC,"client", WIFI_HOSTNAME);
        send_mqtt(PUB_TOPIC,"status", "alive");
        send_mqtt(PUB_TOPIC, "firmware",FIRMWARE_VERSION);
        send_mqtt(PUB_TOPIC, "step",String(STEP).c_str());
  
}
bool MQTTPublisher::send_mqtt(String prepend, String topic, String value)
{
  auto retVal =  client.publish((prepend.c_str() + topic).c_str(), value.c_str());
  yield();
  return retVal;
}


void callback(char* topic, byte* payload, unsigned int length) {

  unsigned  int HighBase,LowBase;
  String dataStr, topicStr,topicShort;

  for (int i=0 ; i<length; i++)dataStr += (char)payload[i];
    HighBase =dataStr.substring(0,dataStr.lastIndexOf(',')).toInt();
    LowBase = dataStr.substring(dataStr.lastIndexOf(',')+1).toInt();
      topicStr = String (topic); // Конвертируем Топик в строку
      topicShort = topicStr.substring(topicStr.lastIndexOf('/')+1);// Берем конец строки (сам топик без ветки)

if (topicShort == "reset" && HighBase == 1 ){

  client.publish((PUB_TOPIC "status"),"SET Enabled",true);
  RESET = true;
  }
if (topicShort == "Cold: " & RESET) {

        EEPROM_write_Int(CounterLowAddress[0], LowBase);
        EEPROM_write_Int(CounterHighAddress[0], HighBase);
        client.publish((PUB_TOPIC "status"),"set Cold: new data",true);
        client.publish((PUB_TOPIC "correct/reset"),"0",true);
        countersInit();
        RESET = false;
  }
  if (topicShort == "Hot:  " & RESET) {

        EEPROM_write_Int(CounterLowAddress[1], LowBase);
        EEPROM_write_Int(CounterHighAddress[1], HighBase);
        client.publish((PUB_TOPIC "status"),"set Hot: new data",true);
        client.publish((PUB_TOPIC "correct/reset"),"0",true);
        countersInit();
        RESET = false;
  }

}
void MQTTPublisher::handle()
{
  if (!isStarted)
    return;
     if (!client.connected()) {
    if (!reconnect()) return;
  }
   client.loop();
}
 

bool MQTTPublisher::reconnect()
{
  if (debugMode)
  {
    Serial.print("MQTT) Attempting connection to server: ");
    Serial.print(MQTT_SERVER);
    Serial.println("...");
  }

  // Create a random client ID
 String clientName;
      clientName +=  WIFI_HOSTNAME;
      clientName += "-";
      uint8_t mac[6];
      
  // Attempt to connect
  bool clientConnected;
  if (String(MQTT_USER).length())
  {
    Serial.println("MQTT) Connecting with credientials");
    clientConnected = client.connect(clientName.c_str(), MQTT_USER, MQTT_PASS);
  }
  else
  {
    Serial.println("MQTT) Connecting without credentials.");
    clientConnected = client.connect(clientName.c_str());
  }

  if (clientConnected)
  {
    if (debugMode) {
      Serial.println("MQTT) connected");
    }

    // Once connected, publish an announcement...
    send_mqtt(PUB_TOPIC,"status", "alive");

    return true;
  }
  else {
    if (debugMode)
    {
      Serial.print("MQTT) failed, rc=");
      Serial.print(client.state());
    }
  }

  return false;
}

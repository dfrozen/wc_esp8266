#pragma once 
#define RECONNECT_TIMEOUT 15000
extern int CounterHighAddress[];
extern int CounterLowAddress[];
extern void EEPROM_write_Int(int addr,unsigned int data);
extern void countersInit();
class MQTTPublisher
{
  private:
    bool debugMode;
    bool isStarted;
    bool reconnect();
    //void callback(char* topic, byte* payload, unsigned int length);
  public:
    MQTTPublisher(bool inDebugMode = false);
    ~MQTTPublisher();
    bool send_mqtt(String prepend, String topic, String value);
    void start();
    void stop();
    void handle();
};

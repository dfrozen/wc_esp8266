#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>             // MQTT PubSubClient
#include <Wire.h>                     // I2C
#include <LiquidCrystal_I2C.h>        // I2C LCD

#include <Bounce2.h>                  // Для обработки замыкания линий со счетчиков
#include "Settings.h"
#define LCD_COL           16                      // Разрешение экрана - колонки
#define LCD_ROW           2                       // Разрешение экрана - строки

LiquidCrystal_I2C lcd(0x3F,LCD_COL,LCD_ROW);      // Устанавливаем дисплей 0x3F - адрес

/////////////////////////////////////////////////////////////////////////////////////////////////

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

boolean SET = false;
//////////////////////////////////////////////////////////////////////////////////////////////

unsigned int CounterHighBase[COUNTERS] = {00000, 00000};     // Если значение отлично от нуля - то пишем его в качестве базового
unsigned int CounterLowBase[COUNTERS]  = {400,700};     // Если значение отлично от нуля - то пишем его в качестве базового
int counterReadDelay  = 0;                          // Текущая задержка считывания счетчика
                                                    // (нужна для уверенной отработки переключения счетчика)
int CounterPin[COUNTERS]         = {COLD_COUNTER_PIN, HOT_COUNTER_PIN};  // Пины
int CounterHighAddress[COUNTERS] = {0x10, 0x1a};     //Адреса EEPROM для младшего слова (кубометры)  5 байта
int CounterLowAddress[COUNTERS]  = {0x28, 0x23};     //Адреса EEPROM для младшего слова (литры) 2 байта
char *CounterName[COUNTERS]      = {"Cold: ", "Hot:  "};                 // Названия счетчиков для вывода на экран
Bounce CounterBouncer[COUNTERS]  = {};               // Формируем для счетчиков Bounce объекты
//////////////////////////////////////////////////////////////////////////////////////////////

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {

  unsigned  int HighBase,LowBase;
  String dataStr, topicStr,topicShort;

  for (int i=0 ; i<length; i++)dataStr += (char)payload[i];
    HighBase =dataStr.substring(0,dataStr.lastIndexOf(',')).toInt();
    LowBase = dataStr.substring(dataStr.lastIndexOf(',')+1).toInt();
      topicStr = String (topic); // Конвертируем Топик в строку
      topicShort = topicStr.substring(topicStr.lastIndexOf('/')+1);// Берем конец строки (сам топик без ветки)

if (topicShort == "reset" && HighBase == 1 ){
  #if DEBUG
  Serial.println("SET Enabled");
  #endif
  client.publish((PUB_TOPIC "status"),"SET Enabled",true);
  SET = true;
  }
if (topicShort == "Cold: " & SET) {
    #if DEBUG
         Serial.println("Message write Cold:");
    #endif
        EEPROM_write_Int(CounterLowAddress[0], LowBase);
        EEPROM_write_Int(CounterHighAddress[0], HighBase);
        countersInit();
        client.publish((PUB_TOPIC "status"),"set Cold: new data",true);
        client.publish((PUB_TOPIC "correct/reset"),"0",true);
    #if DEBUG
         Serial.println("SET Disabled");
    #endif
          SET = false;
  }
  if (topicShort == "Hot:  " & SET) {
        #if DEBUG
    Serial.println("Message write Hot:");
 #endif
        EEPROM_write_Int(CounterLowAddress[1], LowBase);
        EEPROM_write_Int(CounterHighAddress[1], HighBase);
        countersInit();
        client.publish((PUB_TOPIC "status"),"set Hot: new data",true);
        client.publish((PUB_TOPIC "correct/reset"),"0",true);
      #if DEBUG
        Serial.println("SET Disabled");
      #endif
        SET = false;
  }

}

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

void re_connect() {

  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
       #if DEBUG
          Serial.print("\tConnecting to ");
       #endif
   lcd.clear();
   lcd.print("Wi-Fi Connecting");

      #if DEBUG
        Serial.print(WIFI_SSID);
        Serial.println("...");
       #endif

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);}
    else {
      delay(5000);
    }


  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      #if DEBUG
      Serial.println("\tAttempting MQTT connection...");
      #endif
      lcd.clear();
      lcd.print("MQTT connection...");
      String clientName;
      clientName +=  WIFI_HOSTNAME;
      clientName += "-";
      uint8_t mac[6];
     
      WiFi.macAddress(mac);
      IPAddress local_ip = WiFi.localIP();
      long rssi= WiFi.RSSI();
      #if DEBUG

         Serial.print("IP Address: ");
         Serial.println(local_ip);
         Serial.print("signal strength (RSSI):");
         Serial.print(rssi);
         Serial.println(" dBm");
      #endif
      String ip = ip2Str(local_ip);
      clientName += mac2Str(mac);

      if (client.connect(clientName.c_str(),MQTT_USER,MQTT_PASS)) {
        #if DEBUG
        Serial.println("\tMQTT Connected");
        #endif
        lcd.clear();
        lcd.print("MQTT Connected");
        client.subscribe(SUB_TOPIC);
        //client.subscribe((PUB_TOPIC "correct/reset"));
        //client.subscribe((PUB_TOPIC "correct/Cold: "));
        //client.subscribe((PUB_TOPIC "correct/Hot:  "));
        client.publish ((PUB_TOPIC "client_RSSI"),String(rssi).c_str(), true);
        client.publish ((PUB_TOPIC "client_IP"),ip.c_str() , true);
        client.publish ((PUB_TOPIC "client_mac"),mac2Str(mac).c_str(), true);
        client.publish ((PUB_TOPIC "client"), WIFI_HOSTNAME, true);
        client.publish ((PUB_TOPIC "status"), "alive", true);
        client.publish ((PUB_TOPIC "firmware"),FIRMWARE_VERSION,true);
        client.publish ((PUB_TOPIC"step"),String(STEP).c_str(),true);
      }
      //otherwise print failed for debugging
      else { Serial.println("\tFailed."); abort(); }
    }
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void EEPROM_write_Int(int addr,unsigned int data)
  {
    byte buf[4];
      (int&)buf=data;
          for (int i=0; i<4; i++){
            EEPROM.write(addr+i,buf[i]);
             EEPROM.commit();
          }
  }
 ///////////////////////////////////////////////////////////////////////////////////////////////////
 unsigned int EEPROM_read_Int(int addr)
  {
    byte buf[4];
    for(int i = 0; i < 4; i++)buf[i] = EEPROM.read(addr+i);
    int &num = (int&)buf;
    return num;
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Publish (char *Dataname, unsigned int HighData,unsigned int LowData ) {
   String MQTT_node; 
  // char strh[5];
   char str1[3];
   MQTT_node += PUB_TOPIC;
   MQTT_node += Dataname;
   String MQTT_data;
  // sprintf(strh,"%0.5u",HighData);
   sprintf(str1,"%0.3u",LowData);
   #if DEBUG
   Serial.print("\t LowData.");Serial.println(str1);
   Serial.print("\t HighData.");Serial.println(HighData);
   #endif
   MQTT_data +=String(HighData).c_str(); MQTT_data +=","; MQTT_data +=String(str1).c_str();

   client.publish(String(MQTT_node).c_str(),String(MQTT_data).c_str(),true);
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void countersInit()
{
 lcd.clear();
  for (int i=0; i<COUNTERS; i++)
  {

   CounterHighBase[i] = EEPROM_read_Int(CounterHighAddress[i]); // Читаем начальные значения из EEPROM
   CounterLowBase[i]  = EEPROM_read_Int(CounterLowAddress[i]);  // Читаем начальные значения из EEPROM

  Publish (CounterName[i], CounterHighBase[i],CounterLowBase[i] );


    #if DEBUG
      Serial.print("Read form EEPROM "); Serial.print(i,DEC ); Serial.print(" counter. Name "); Serial.println(CounterName[i]);
      Serial.print(CounterHighAddress[i] ,HEX); Serial.print(" => "); Serial.println(CounterHighBase[i],DEC);
      Serial.print(CounterLowAddress[i]  ,HEX); Serial.print(" => "); Serial.println(CounterLowBase[i] ,DEC);
    #endif
        printPos(0,0,CounterName[0]);
        printPos(0,1,CounterName[1]);
        printHigh(7,i,CounterHighBase[i]);
        printPos(12,i,",");
        printLow(13,i,CounterLowBase[i]);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////Обрабатываются счетчики
void readCounter()
{
  if (counterReadDelay >= 100) // Если подошло время обработки
  {
    counterReadDelay = 0;
    for (int i=0; i<COUNTERS; i++)
    {
      CounterBouncer[i].update();
      if ( CounterBouncer[i].fell()) {

          if (CounterLowBase[i]<990) // если не произошло перехода на кубометры - увеличиваем счетчик литров на STEP литров
          {
            CounterLowBase[i]+=STEP;
            printLow(13,i,CounterLowBase[i]);
            EEPROM_write_Int(CounterLowAddress[i],CounterLowBase[i]);
            Publish (CounterName[i], CounterHighBase[i],CounterLowBase[i] );
            #if DEBUG
               Serial.print("Write to EEPROM "); Serial.print(i,DEC ); Serial.print(" counter. Name "); Serial.println(CounterName[i]);
               Serial.print(CounterHighAddress[i] ,HEX); Serial.print(" => "); Serial.println(CounterHighBase[i]);
               Serial.print(CounterLowAddress[i] ,HEX); Serial.print(" => "); Serial.println(CounterLowBase[i] );
           #endif
          }
          else  // иначе, если произошел переход - обнуляем счетчик литров и увеличиваем счетчик кубометров на 1
          {
              CounterLowBase[i] = 0;
              CounterHighBase[i]++;
              printHigh(7 ,i ,CounterHighBase[i]);
              printLow(13,i ,CounterLowBase[i] );
            EEPROM_write_Int( CounterLowAddress[i],  CounterLowBase[i] );
            EEPROM_write_Int( CounterHighAddress[i], CounterHighBase[i]);
            Publish (CounterName[i], CounterHighBase[i],CounterLowBase[i] );
            #if DEBUG
               Serial.print("Write to EEPROM "); Serial.print(i,DEC ); Serial.print(" counter. Name "); Serial.println( CounterName[i] );
               Serial.print(CounterHighAddress[i] ,HEX); Serial.print(" => "); Serial.println(CounterHighBase[i]);
               Serial.print(CounterLowAddress[i] ,HEX); Serial.print(" => "); Serial.println(CounterLowBase[i] );
            #endif
          }
      }
    }
  }
  else //Если время обработки еще не истекло
  {
    counterReadDelay++;
   // Serial.println(counterReadDelay);
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
  void printPos(byte col, byte row, char *str)
  {
    lcd.setCursor(col, row);
    lcd.print(str);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Выводит на LCD старший разряд счетчика, начиная с указаных координат
  void printHigh(byte col, byte row, int val)
  {
    char str1[5];
    row=row;
    lcd.setCursor(col, row);
    sprintf(str1,"%0.5u",val);
    lcd.print(str1);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Выводит на LCD младший разряд счетчика, начиная с указаных координат
  void printLow(byte col, byte row, int val)
  {
    char str1[3];
    row=row;
    lcd.setCursor(col, row);
    sprintf(str1,"%0.3u",val);
    lcd.print(str1);
  }
//////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  delay(10);
  #if DEBUG
    Serial.begin(9600);
    Serial.println( "Debug is ON...");
    Serial.println( "------- FLAGS ------");
  #endif

    EEPROM.begin(64);

 ////////////////////////////////////////////////////////////////////////////////////////////////////////
   for (int i=0; i<COUNTERS; i++)
  {
    pinMode(CounterPin[i], INPUT_PULLUP);                              // Инициализируем пин
    digitalWrite(CounterPin[i], HIGH);                          // Включаем подтягивающий резистор
    CounterBouncer[i].attach(CounterPin[i], INPUT_PULLUP);                    // Настраиваем Bouncer
    CounterBouncer[i].interval(25);                             // и прописываем ему интервал дребезга
  }
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////

 lcd.begin(4, 5);      // 4 - sda 5 - scl
 lcd.backlight();       // подсветка
 //lcd.blink();           // моргание пикселем
 lcd.print("System started"); // че то пишет

 pinMode(BUTTON_PIN, INPUT_PULLUP);              // Инициализация кнопки


 //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //start wifi subsystem
  WiFi.hostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //attempt to connect to the WIFI network and then connect to the MQTT server
  re_connect();
  countersInit();                                   // Инициализация начальных показаний счетчиков
  MDNS.begin(update_host);
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTP Update Server ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", update_host, update_path, update_username, update_password);

}

void loop() {
  // put your main code here, to run repeatedly:
 //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {
      re_connect();
      countersInit();                                   // Инициализация начальных показаний счетчиков
      }
  //maintain MQTT connection
   delay(200);
    httpServer.handleClient();
    readCounter();   // Читаем и обрабатываем значения счетчиков
    wdt_reset();
    client.loop();
}

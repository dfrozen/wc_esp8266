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
    
#define LCD_COL           16                      // Разрешение экрана - колонки
#define LCD_ROW           2                       // Разрешение экрана - строки

LiquidCrystal_I2C lcd(0x3F,LCD_COL,LCD_ROW);      // Устанавливаем дисплей 0x3F - адрес
/////////////////////////////////////////////////////////////////////////////////////////////////
const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";
const char* ssid = "Bestoloch";
const char* password = "511794sinikon";
//////////////////////////////////////////////////////////////////////////////////////////////////
#define MQTT_SERVER "10.10.100.14"  ///YourMQTTBroker'sIP
#define mqtt_port  1883
const char* mqtt_user="orangepi";
const char* mqtt_pass="orangepi";
/////////////////////////////////////////////////////////////////////////////////////////////////
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG            1   // Выдача отладочной информации в COM-порт
#define RESET            1   // Установка первоначального "0"

//////////////////////////////////////////////////////////////////////////////////////////////
// Пины для подключения устройств
#define BUTTON_PIN       16    //Пин с конпкой
#define HOT_COUNTER_PIN  14    //Пин счетчика горячей воды
#define COLD_COUNTER_PIN 12    //Пин счетчика холодной воды
#define COUNTERS 2            //Колличество счетчиков в системе
//////////////////////////////////////////////////////////////////////////////////////////////


unsigned int CounterHighBase[COUNTERS] = {77777,88888};     // Если значение отлично от нуля - то пишем его в качестве базового     
unsigned int CounterLowBase[COUNTERS]  = {333,666};     // Если значение отлично от нуля - то пишем его в качестве базового
int counterReadDelay  = 0;                          // Текущая задержка считывания счетчика 
                                                    // (нужна для уверенной отработки переключения счетчика) 
int CounterPin[COUNTERS]         = {COLD_COUNTER_PIN, HOT_COUNTER_PIN};  // Пины 
int CounterHighAddress[COUNTERS] = {0x30, 0x3a};     //Адреса EEPROM для младшего слова (кубометры)  5 байта
int CounterLowAddress[COUNTERS]  = {0x38, 0x43};     //Адреса EEPROM для младшего слова (литры) 2 байта
char *CounterName[COUNTERS]      = {"Cold: ", "Hot:  "};                 // Названия счетчиков для вывода на экран 
Bounce CounterBouncer[COUNTERS]  = {};               // Формируем для счетчиков Bounce объекты
//////////////////////////////////////////////////////////////////////////////////////////////

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);


#define BUFFER_SIZE 100

String macToStr(const uint8_t* mac){
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);}
  return result;
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
        Serial.print(ssid);
        Serial.println("...");
       #endif
       
    WiFi.begin(ssid, password);}
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
  
      if (client.connect("esp8266",mqtt_user,mqtt_pass)) {
        #if DEBUG
        Serial.println("\tMQTT Connected");
        #endif
        lcd.clear();  
        lcd.print("MQTT Connected");
      }
      //otherwise print failed for debugging
      else { Serial.println("\tFailed."); abort(); }
    }
  }
  lcd.clear();        
    for (int i=0; i<COUNTERS; i++)                  // Выводим на экран начальные значения    
    {

        printPos(0,0,CounterName[0]);
        printPos(0,1,CounterName[1]);
        printHigh(7,i,CounterHighBase[i]);
        printPos(12,i,",");
        printLow(13,i,CounterLowBase[i]);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void EEPROM_write_Int(int addr,unsigned int data)
  {  
    int _size = sizeof(data);
    int i;
    byte buf[4];
      (int&)buf=data;
          for (i=0; i<_size; i++)EEPROM.write(addr+i,buf[i]);
}
 ///////////////////////////////////////////////////////////////////////////////////////////////////
 unsigned int EEPROM_read_Int(int addr)
{
    byte buf[4];
    for(byte i = 0; i < 4; i++)buf[i] = EEPROM.read(addr+i);
    int &num = (int&)buf;
    return num;
  }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void countersInit()
{
 
  for (int i=0; i<COUNTERS; i++)
  {
    #if RESET // Установка значений по умолчанию - пишем его в EEPROM
      EEPROM_write_Int(CounterLowAddress[i], CounterLowBase[i]);
      EEPROM_write_Int(CounterHighAddress[i], CounterHighBase[i]);
    #endif  
   CounterHighBase[i] = EEPROM_read_Int(CounterHighAddress[i]); // Читаем начальные значения из EEPROM
   client.publish("home/sensors/watercount/CounterHighBase",String(CounterHighBase[i]).c_str());
   CounterLowBase[i]  = EEPROM_read_Int(CounterLowAddress[i]);  // Читаем начальные значения из EEPROM
   client.publish("home/sensors/watercount/CounterLowBase",String(CounterLowBase[i]).c_str());
    #if DEBUG
      Serial.print("Read form EEPROM "); Serial.print(i,DEC ); Serial.print(" counter. Name "); Serial.println(CounterName[i]); 
      Serial.print(CounterHighAddress[i] ,HEX); Serial.print(" => "); Serial.println(CounterHighBase[i],DEC);
      Serial.print(CounterLowAddress[i]  ,HEX); Serial.print(" => "); Serial.println(CounterLowBase[i] ,DEC);
    #endif
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Обрабатываются счетчики
void readCounter()
{
  if (counterReadDelay >= 100) // Если подошло время обработки
  {
    counterReadDelay = 0;
    for (int i=0; i<COUNTERS; i++) 
    {
      CounterBouncer[i].update();
      if ( CounterBouncer[i].fell()) {
         
          if (CounterLowBase[i]<990) // если не произошло перехода на кубометры - увеличиваем счетчик литров на 10
          {
            CounterLowBase[i]+=10;  
            printLow(13,i,CounterLowBase[i]);        
            EEPROM_write_Int(CounterLowAddress[i],CounterLowBase[i]);
            #if DEBUG
               Serial.print("Write to EEPROM "); Serial.print(i,DEC ); Serial.println(" counter. Name "); Serial.println(CounterName[i]); 
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
            #if DEBUG
               Serial.print("Write to EEPROM "); Serial.print(i,DEC ); Serial.println(" counter. Name "); Serial.println( CounterName[i] ); 
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
   
    //println(counterReadDelay); 
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
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  delay(100);
  #if DEBUG
    Serial.begin(9600);
    Serial.println( "Debug is ON...");
    Serial.println( "------- FLAGS ------");
  #endif
    
    EEPROM.begin(512);

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
  WiFi.begin(ssid, password); //attempt to connect to the WIFI network and then connect to the MQTT server
  re_connect();
   countersInit();                                   // Инициализация начальных показаний счетчиков  
  client.publish("home/sensors/watercount/alive","true");
  MDNS.begin(host);
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTP Update Server ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
 
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

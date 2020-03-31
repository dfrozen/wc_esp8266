#include <EEPROM.h>

#include <Wire.h>                     // I2C
#include <LiquidCrystal_I2C.h>        // I2C LCD

#include <Bounce2.h>                  // Для обработки замыкания линий со счетчиков
#include "Settings.h"
#include "Connection.h"
#include "HTTP_Server.h"
#include "MQTT_Connection.h"


#define LCD_COL           16                      // Разрешение экрана - колонки
#define LCD_ROW           2                       // Разрешение экрана - строки

LiquidCrystal_I2C lcd(0x3F,LCD_COL,LCD_ROW);      // Устанавливаем дисплей 0x3F - адрес

/////////////////////////////////////////////////////////////////////////////////////////////////


bool hasWIFI = false;

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

MQTTPublisher mqqtPublisher(DEBUG);
WifiConnector wifiConnector(DEBUG);
HTTP_Server http_server(DEBUG);


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
void Publish (char *Topic, unsigned int HighData,unsigned int LowData ) {
   String MQTT_node; 
  // char strh[5];
   char str1[3];
   String MQTT_data;
   sprintf(str1,"%0.3u",LowData);
   #if DEBUG
   Serial.print("\t LowData.");Serial.println(str1);
   Serial.print("\t HighData.");Serial.println(HighData);
   #endif
   MQTT_data +=String(HighData).c_str(); MQTT_data +=","; MQTT_data +=String(str1).c_str();
    mqqtPublisher.send_mqtt(PUB_TOPIC,Topic,MQTT_data);
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
  
  wifiConnector.start();

  http_server.start();
   // Setup MQTT
  mqqtPublisher.start();
  countersInit();                                   // Инициализация начальных показаний счетчиков
 
}

void loop() {
  wifiConnector.handle();
  yield();
   http_server.handle();
   yield();
     mqqtPublisher.handle();
  yield();
  delay(200);
    readCounter();   // Читаем и обрабатываем значения счетчиков
      for (int i=0; i<COUNTERS; i++)
  {
            printPos(0,0,CounterName[0]);
        printPos(0,1,CounterName[1]);
        printHigh(7,i,CounterHighBase[i]);
        printPos(12,i,",");
        printLow(13,i,CounterLowBase[i]);
    }
    wdt_reset();
 }

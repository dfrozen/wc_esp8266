
# wc_esp8266
ESP8266, LiquidCrystal_I2C water meter monitor with MQTT

The monitor for the watermeter, saves the measurements in EEPROM.
Have a reset mode to default measurements  Based on ESP8266 and Arduino IDE

## warning
garanted work with Bounce 2 ver 2.2.0


### ver 1.0.3 - 2020.03.31 - BugFix, Вынос WiFi, MQTT, Update Server во внешние процедуры.

### ver 1.0.2 - 2020.03.28 - BugFix, Вынос настроек в Settings.h (GitHub)

### ver 1.0.1 - 2020.03.25 - Первая версия кода на базе ESP8266


# HOW USE

## Change if you need for default config

//Hostname of ESP8266
#define WIFI_HOSTNAME "WaterMeter"

//Wifi SSID to connect to Leave empty to disable Wi-Fi.
#define WIFI_SSID "Bestoloch"

//Passowrd for WIFI
#define WIFI_PASSWORD "511794sinikon"

//set the mqqt host name or ip address to your mqqt host. Leave empty to disable mqtt.
#define MQTT_SERVER "10.10.100.14"
//mqtt port for the above host
#define MQTT_PORT  1883
//if authentication is enabled for mqtt, set the username below. Leave empty to disable authentication
#define MQTT_USER "orangepi"
#define MQTT_PASS "orangepi"

// Publish branch of topic to MQTT
#define PUB_TOPIC  "/home/watermeter/"
// Subscribe  branch of topic for Callback
#define SUB_TOPIC  "/home/watermeter/correct/#"

/////////////////////////webudate server//////////////////////////////////////////

#define update_host  "WaterMeter"

#define update_path  "/firmware"

#define update_username  "admin"

#define update_password  "admin"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG            true   // Выдача отладочной информации в COM-порт
#define BUFFER_SIZE 100

// Пины для подключения устройств
#define BUTTON_PIN       16    //Пин с кнопкой
#define HOT_COUNTER_PIN  14    //Пин счетчика горячей воды
#define COLD_COUNTER_PIN 12    //Пин счетчика холодной водыводы
#define COUNTERS 2            //Колличество счетчиков в системе
#define STEP 10 // Цена импульса - 1 на 10 литров, или 1 на литр

## Set  default or correction
 if You need correct data of measurements, to do:
 for sample you need correct Cold water measurements
 1. synchronize MQTT Topic "/watercount/correct/Hot:" to current measurements /watercount/Hot:  (if no sync maybe rewrite )
 2. Set MQTT Topic "/watercount/correct/reset" = 1
 3. Set MQTT Topic "/watercount/correct/Cold:"  correct data

If new data saved to EEPROM auto reset "/watercount/correct/reset" = 0 and /watercount/status "set Cold: new data"

## License
MIT License

Copyright (c) 2020 Andrey Kuznetsoff

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#define FIRMWARE_VERSION "1.0.4"

/*
Версии прошивки для ESP
1.0.4 - 2020.04.17 - Оптимизация задержки в процедурах.

1.0.3 - 2020.03.31 - BugFix, Вынос WiFi, MQTT, Update Server в внешние процедуры.

1.0.2 - 2020.03.28 - BugFix, Вынос настроек в Settings.h

1.0.1 - 2020.03.25 - Первая версия кода на базе ESP8266
*/

//Hostname of ESP8266
#define WIFI_HOSTNAME "develop"

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

// Publish branch of topic`s to MQTT
#define PUB_TOPIC  "/develop/watermeter/"
// Subscribe  branch of topic`s for Callback
#define SUB_TOPIC  "/develop/watermeter/correct/#"

/////////////////////////webudate server//////////////////////////////////////////

#define update_host  "watermeter-webupdate"

#define update_path  "/firmware"

#define update_username  "admin"

#define update_password  "admin"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG            true   // Выдача отладочной информации в COM-порт
#define BUFFER_SIZE 100

// Пины для подключения устройств
//#define BUTTON_PIN       16    //Пин с кнопкой
#define HOT_COUNTER_PIN  14    //Пин счетчика горячей воды
#define COLD_COUNTER_PIN 12    //Пин счетчика холодной водыводы
#define COUNTERS 2            //Колличество счетчиков в системе
#define STEP 10 // Цена импульса - 1 на 10 литров, или 1 на литр

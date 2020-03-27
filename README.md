
# wc_esp8266
ESP8266, LiquidCrystal_I2C water meter monitor with MQTT


## warning
garanted work with Bounce 2 ver 2.2.0

## ver 1.0.1
The monitor for the watermeter, saves the measurements in EEPROM.
Have a reset mode to default measurements  Based on ESP8266 and Arduino IDE

# HOW USE

## Change if you need for default config

#### #define DEBUG            1   //  Sent Debug information to COM-port


#### #define BUTTON_PIN       16    //Pin for feature use
#### #define HOT_COUNTER_PIN  14    //Hot water input PIN
#### #define COLD_COUNTER_PIN 12    //Cold Water input PIN
#### #define COUNTERS 2             //Do not change (set to feature use)
Setup Wi-Fi Connection
const char* ssid = "SSID_Name";
const char* password = "SSID_PASS";
//////////////////////////////////////////////////////////////////////////////////////////////////
#### #define MQTT_SERVER "10.10.100.14"  // Name or IP of MQTT Broker
#### #define mqtt_port  1883             // Port of MQTT Broker
const char* mqtt_user="orangepi";   // User name for MQTT Broker
const char* mqtt_pass="orangepi";   // User password for MQTT Broker

if you need  you did change name of MQTT topic
home/sensors/watercount/correct/reset // Set default flag
home/sensors/watercount/correct/Cold: // correction Cold Water
home/sensors/watercount/correct/Hot: // correction Hot Water
home/sensors/watercount/status       // Status of watermeter monitor
home/sensors/watercount/Cold:        // measurements Cold Water
home/sensors/watercount/Hot:         // measurements Hot Water

## Set  default or correction
 if You need correct data of measurements, to do:
 for sample you need correct Cold water measurements
 1. synchronize MQTT Topic "home/sensors/watercount/correct/Hot:" to current measurements home/sensors/watercount/Hot:  (if no sync maybe rewrite )
 2. Set MQTT Topic "home/sensors/watercount/correct/reset" = 1
 3. Set MQTT Topic "home/sensors/watercount/correct/Cold:"  correct data

If new data saved to EEPROM auto reset "home/sensors/watercount/correct/reset" = 0 and home/sensors/watercount/status "set Cold: new data"

## License
MIT License

Copyright (c) 2020 Andrey Kuznetsoff

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

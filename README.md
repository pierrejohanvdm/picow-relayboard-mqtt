# picow-relayboard-mqtt

Arduino IDE project for Raspberry Pi Pico W.

This controls a Waveshare relay board using a Pico W connected to an external MQTT server over WiFi.

Excluded from source control is a secrets.h file that should contain:

/************************* WiFi config ****************************************/

#define WLAN_SSID       "your wifi name here"
#define WLAN_PASS       "your wifi pw here"

/************************* Adafruit.io config *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "your username here"
#define AIO_KEY         "your api key here"

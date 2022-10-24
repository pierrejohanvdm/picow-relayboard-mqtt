# picow-relayboard-mqtt

Arduino IDE project for Raspberry Pi Pico W.

This controls a Waveshare relay board using a Pico W connected to an external MQTT server over WiFi.

Link to the relay board: https://www.waveshare.com/pico-relay-b.htm

Steps (Windows):
1. Install Arduino IDE
2. Add Pico W boards manager into Arduino IDE from https://github.com/earlephilhower/arduino-pico/ and install any packages as prompted by the IDE
3. Include Adafruit MQTT library into the project sketch and any other libraries that may be missing
4. Add a secrets.h file into the project as below.

Excluded from source control is a secrets.h file that should contain:

/************************* WiFi config ****************************************/

#define WLAN_SSID       "your wifi name here"
#define WLAN_PASS       "your wifi pw here"

/************************* Adafruit.io config *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "your username here"
#define AIO_KEY         "your api key here"

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
#define WLAN_SSID       "ssid"
#define WLAN_PASS       "password"

/************************* MQTT server config *********************************/
#define MQTT_SERVER     "192.168.0.131"
#define MQTT_PORT       1883
#define MQTT_USERNAME   "username"
#define MQTT_PASSWORD   "api key or password"

/************************* Board specific setup *******************************/
#define STATUS_FEED     "relays/status"
#define COMMAND_FEED    "relays/commands"
#define RELAY_COUNT     8
#define BOARD_ID        1

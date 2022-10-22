#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "secrets.h"

/************************* Board specific Setup ******************************/
#define DEVICES_FEED    "devices"
#define RELAYS_FEED     "relays"
#define RELAY_COUNT     8
#define BOARD_ID        1

// Create a WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
Adafruit_MQTT_Subscribe relayFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/" RELAYS_FEED, MQTT_QOS_1);
Adafruit_MQTT_Publish devicesFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/" DEVICES_FEED);

/*************************** Sketch Code ************************************/

bool ledState = 0;

void setup()
{
  // Setup LED.
  pinMode(LED_BUILTIN, OUTPUT);

  // Start blink.
  blink(1);

  // Setup relays.
  for (int i = 1; i <= RELAY_COUNT; i++)
  {
    int pin = getRelayPin(i);
    pinMode(pin, OUTPUT);
  }

  // Connect to WiFi.
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    blink(1);
  }
  
  // Setup MQTT.
  relayFeed.setCallback(relayFeedCallback);
  mqtt.subscribe(&relayFeed);

  String helloMsgStr = "Relay board '" + String(BOARD_ID) + "' connected. IP '" + WiFi.localIP().toString() + "', Mac '" + WiFi.macAddress() + "'.";
  const char* helloMsg = helloMsgStr.c_str();
  connectMqtt();
  devicesFeed.publish(helloMsg);

  // Finish blink.
  delay(1000);
  blink(2);
}

void loop()
{
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).
  connectMqtt();

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  mqtt.processPackets(2500);
  
  // ping the server to keep the mqtt connection alive (when not publishing before keep-alive expires)
  bool result = mqtt.ping();
  
  if(result == false)
  {
    mqtt.disconnect();
  }
}

void blink(int count)
{
  // Perform count amount of blinks.
  for(int i = 0; i < count; i++)
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }

  // Restore pre-blink LED state.
  digitalWrite(LED_BUILTIN, ledState);
}

void connectMqtt()
{
  // Function to connect and reconnect as necessary to the MQTT server.
  // Should be called in the loop function and it will take care if connecting.

  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }

  uint8_t retriesRemaining = 3;
  while (mqtt.connect() != 0 && retriesRemaining > 0) // connect will return 0 for connected
  {
    mqtt.disconnect();
    retriesRemaining--;
    delay(5000);
    blink(1);
  }
  
  if (retriesRemaining == 0)
  {
    errorRecovery();
  }
}

void relayFeedCallback(char *data, uint16_t len)
{
  if (len >= 3)
  {
    int boardId = data[0] - '0';
    if (boardId == 0 || boardId == BOARD_ID)
    {
      int relayChannel = data[1] - '0';
      int relayValue = data[2] - '0';
      if (relayValue == 0 || relayValue == 1)
      {
        setRelay(relayChannel, relayValue);
      }
    }
  }
}

void setRelay(int channel, int value)
{
  if (channel == 0)
  {
    for (int i = 1; i <= RELAY_COUNT; i++)
    {
      int pin = getRelayPin(i);
      digitalWrite(pin, value);
    }
    
    // temp. for debugging.
    blink(1);
  }
  else if (channel >= 1 && channel <= RELAY_COUNT)
  {
    int pin = getRelayPin(channel);
    digitalWrite(pin, value);

    // temp. for debugging.
    blink(1);
  }
}

int getRelayPin(int channel)
{
  return 22 - channel;
}

void errorRecovery()
{
  // Delay for 10s then reset device.
  delay(10000);
  watchdog_enable(1, 1);
  while(true);
}

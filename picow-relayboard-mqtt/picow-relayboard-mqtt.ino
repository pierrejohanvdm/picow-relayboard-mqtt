#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "secrets.h"

// Create a WiFiClient class to connect to the MQTT server.
WiFiClient wifiClient;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqttClient(&wifiClient, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

// Setup the MQTT feeds we will be publishing or subscribing to.
Adafruit_MQTT_Publish statusFeed = Adafruit_MQTT_Publish(&mqttClient, STATUS_FEED);
Adafruit_MQTT_Subscribe commandFeed = Adafruit_MQTT_Subscribe(&mqttClient, COMMAND_FEED, MQTT_QOS_1);

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
  connectWiFi();
  
  // Setup MQTT.
  commandFeed.setCallback(commandFeedCallback);
  mqttClient.subscribe(&commandFeed);

  String helloMsgStr = "Relay board " + String(BOARD_ID) + " connected. IP " + WiFi.localIP().toString() + ", Mac " + WiFi.macAddress() + ".";
  const char* helloMsg = helloMsgStr.c_str();
  connectMqtt();
  statusFeed.publish(helloMsg);

  // Finish blink.
  delay(1000);
  blink(3);
}

void loop()
{
  // Ensure the connection to the WiFi is alive.
  connectMqtt();

  // Ensure the connection to the MQTT server is alive.
  connectMqtt();

  // Wait for incoming subscription packets and process callback.
  mqttClient.processPackets(2500);
  
  // Ping the server to keep the MQTT connection alive. (when not publishing before keep-alive expires)
  bool pingSuccessful = mqttClient.ping();
  if(!pingSuccessful)
  {
    mqttClient.disconnect();
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
}

void connectWiFi()
{
  // Stop if already connected.
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  // Try to connect.
  uint8_t retriesRemaining = 3;
  while (retriesRemaining > 0)
  {
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    if (WiFi.status() == WL_CONNECTED) 
    {
      break;
    }
    else
    {
      retriesRemaining--;
      delay(5000);
      blink(2);
    }
  }
  
  if (retriesRemaining == 0)
  {
    errorRecovery();
  }
}

void connectMqtt()
{
  // Stop if already connected.
  if (mqttClient.connected())
  {
    return;
  }

  // Try to connect. It will return 0 for success.
  uint8_t retriesRemaining = 3;
  while (mqttClient.connect() != 0 && retriesRemaining > 0)
  {
    mqttClient.disconnect();
    retriesRemaining--;
    delay(5000);
    blink(2);
  }
  
  if (retriesRemaining == 0)
  {
    errorRecovery();
  }
}

void commandFeedCallback(char *data, uint16_t len)
{
  // Our command protocol has boardId at data[0] and commandId at data[1].
  // Accept commands for this BOARD_ID or 0 for any relay board.
  // Commands 0-9 are for relays and A-Z for ad-hoc functions.

  if (len >= 2)
  {
    int boardId = data[0] - '0';
    if (boardId == 0 || boardId == BOARD_ID)
    {
      if (data[1] >= '0' && data[1] <= '9')
      {
        int channel = data[1] - '0';
        bool isHigh = len >= 3 && data[2] == '1';
        setRelay(channel, isHigh);
      }
      else if (data[1] == 'S')
      {
        publishStatus();
      }
    }
  }
}

void setRelay(int channel, bool isHigh)
{
  if (channel == 0)
  {
    for (int i = 1; i <= RELAY_COUNT; i++)
    {
      int pin = getRelayPin(i);
      digitalWrite(pin, isHigh);
    }
  }
  else if (channel >= 1 && channel <= RELAY_COUNT)
  {
    int pin = getRelayPin(channel);
    digitalWrite(pin, isHigh);
  }
}

int getRelayPin(int channel)
{
  return 22 - channel;
}

void publishStatus()
{
  // respond with board number, relay count, then all the current relay pin values.
  int size = 2 + RELAY_COUNT + 1; // board_id, relay_count, pin values, null
  char statusMsg[size] = { BOARD_ID + '0', RELAY_COUNT + '0' };
  for (int i = 1; i <= RELAY_COUNT; i++)
  {
    int pinValue = digitalRead(getRelayPin(i));
    statusMsg[i + 1] = pinValue + '0';
  }
  statusMsg[size - 1] = 0;

  connectMqtt();
  statusFeed.publish(statusMsg);
}

void errorRecovery()
{
  // Delay for 10 blinks then reset device.
  blink(10);
  watchdog_enable(1, 1);
  while(true);
}

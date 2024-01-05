#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include <MQTT.h>
#include <TM1638Anode.h>
#include <TM1638.h>
#include <TM16xxDisplay.h>
#include <Wire.h>
#include <SPI.h>

MQTTClient client;
WiFiClient net;
TM1638 module1(DIO_PIN, CLK_PIN, STB_PIN, 8, true, 0); // DIO=8, CLK=9, STB=7, activateDisplay, intensity
TM16xxDisplay display1(&module1, 8);    // 8 characters
int loops = 0;
int displayMessages = 1;

String displayMessage = "012345678";

void messageReceived(String &topic, String &payload)
{
  displayMessage = payload;
  loops = 0;
  Serial.println("incoming: " + topic + " - " + payload);
}

void setup()
{
  #ifdef MOVEMENT_PIN
   pinMode(MOVEMENT_PIN, INPUT_PULLDOWN_16);
  #endif

  Serial.begin(9600);
  display1.flush();
  display1.println(F("HELLO !"));
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WLAN_SSID, WLAN_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  client.begin(MQTT_SERVER, net);
  Serial.print("\nconnecting...");
  int randomNumber = random(10000);

  // Append the random number to the client ID
  String clientId = "ESP8266Client" + String(randomNumber);
  while (!client.connect(clientId.c_str()))
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.print(".");
  delay(1000);
  client.onMessage(messageReceived);
  client.subscribe(TOPIC);
}

void loop()
{
  if(displayMessages>0)
    display1.println(displayMessage.c_str());
  else
    display1.clear();
  client.loop();
  loops++;
  delay(100);
  #ifdef MOVEMENT_PIN
  if (digitalRead(MOVEMENT_PIN) == HIGH)
  {
    Serial.print(".");
    displayMessages = 100; 
  }
  if(displayMessages>0)
  displayMessages--;
  #endif
  if (loops > 8000)
  {
    Serial.println("RESTARTING");
    delay(1000);
    ESP.restart();
  }
}

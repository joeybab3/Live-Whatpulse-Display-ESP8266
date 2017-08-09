//By Joey Babcock
//Find diagram and more here: http://www.joeybabcock.me/blog/projects/arduino-esp8266-live-subscriber-display/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>
#include <SPI.h>
#include "LedMatrix.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define NUMBER_OF_DEVICES 2
#define CS_PIN D4
LedMatrix ledMatrix = LedMatrix(NUMBER_OF_DEVICES, CS_PIN);

const char* host = "joeybabcock.me";
String username = "djnat";
const int httpPort = 80;
char ssid[] = "SSID_HERE"; // your network SSID (name)
char password[] = "PASSWORD"; // your network key
int api_mtbs = 20000; //mean time between api requests
long api_lasttime;   //last time api request has been done
long subs = 0;

void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  ArduinoOTA.setPassword((const char *)"123");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  ledMatrix.init();
  ledMatrix.setIntensity(10); // range is 0-15
  getStats();
}

void loop() {
  scroll();
  ArduinoOTA.handle();
  if (millis() > api_lasttime + api_mtbs)  {
    getStats();
    api_lasttime = millis();
  }
}

void scroll()
{
  ledMatrix.clear();
  ledMatrix.scrollTextLeft();
  ledMatrix.drawText();
  ledMatrix.commit();
  delay(50);
}

void getStats() {
  String json;
  boolean httpBody = false;
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection to my server failed...");
    return;
  }

  String url = "/tests/php/whatpulse.php?username="+username;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  Serial.println("Got the connection, parsing...");
  
  
  char c ;
  int index = 0;
  Serial.println("[");
  while (client.available()) 
  {
      String line = client.readStringUntil('\r');
      if (!httpBody && line.charAt(1) == '{') {
        httpBody = true;
      }
      if (httpBody) {
        json += line;
      }
  }
  Serial.println("]");
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  Serial.print(json);
  if(root.success()) {
    if (root.containsKey("clicks")) {
      String clicks = root["clicks"];
      String keys = root["keys"];
      String username = root["username"];
      String response = username + " has ";
      response += clicks + " clicks, and ";
      response += keys, " keys.";
      ledMatrix.setText(String(response));
    }
    else
    {
      ledMatrix.setText("JSON was bad.");
    }
  }
  else
  {
    ledMatrix.setText("Root failed.");
  }
}


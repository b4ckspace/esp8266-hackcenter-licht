#include <Wire.h>
#include <PCA9685.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>

#include "settings.h"
#include "static.h"

// Used to store the public values, because mapping the values 
// introduce inaccuracy / inconsistent responses
uint8_t lightValues[NUM_LIGHTS] = { 0 };
const PROGMEM char text_html[] = "text/html";

PCA9685 driver = PCA9685(0x00, PCA9685_MODE_N_DRIVER);
ESP8266WebServer server(80);

void getLights() {

  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  JsonArray& jsonEndpoints = root.createNestedArray("lights");

  for (uint8_t light = 0; light < NUM_LIGHTS; light++) {
    JsonObject& entry = jsonBuffer.createObject();
    entry["id"] = light;
    entry["value"] = lightValues[light];

    jsonEndpoints.add(entry);
  }

  String response = "";
  root.printTo(response);

  server.send(200, "application/json", response);
}

void setLight(uint8_t light) {;

  if(!server.hasArg("value")) {
    return server.send(400, "text/plain", "Missing parameter value");
  }

  uint8_t incomingValue = server.arg("value").toInt();
  if(incomingValue > 100 || incomingValue < 0) {
    return server.send(400, "text/plain", "Value out of range (Max: 100, Min: 0)");
  }
  
  uint16_t value = map(incomingValue, 0, 100, PCA9685_FULL_OFF, PCA9685_FULL_ON);
  driver.getPin(light).setValueAndWrite(value);

  lightValues[light] = incomingValue;
  
  return getLight(light);
}

void getLight(uint8_t light) {
  
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["id"] = light;
  root["value"] = lightValues[light];
    
  String response = "";
  root.printTo(response);

  server.send(200, "application/json", response);
}

void setup() {
  char routeHelper[12];
  
  Serial.begin(115200);

  WiFi.hostname("ESP-HackcenterLicht");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Wire.begin(I2C_SDA, I2C_SCL);
  driver.setup();

  server.on("/", []() {
    server.send_P(200, text_html, static_index_html);
  });
  
  server.on("/lights", HTTP_GET, getLights);

  for (uint8_t light = 0; light < NUM_LIGHTS; light++) {

    sprintf(routeHelper, "/lights/%d", light);
    
    server.on(routeHelper, HTTP_POST, [light]() {
      return setLight(light);
    });

    server.on(routeHelper, HTTP_GET, [light]() {
      return getLight(light);
    });
  }

  server.begin();
}

void loop() {
  server.handleClient();
}

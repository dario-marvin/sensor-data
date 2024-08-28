/*********
  Connect board to WiFi and send BME280 readings to a web server
  Author: Dario Marvin
  Year: 2022
*********/

#include "ESPAsyncWebServer.h"
#include <ESP8266WiFi.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Initialize I2C components
Adafruit_BME280 bme;

// WiFi network credentials
const char* ssid = "###";
const char* password = "###";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Helpful constants and variables
#define SEALEVELPRESSURE_HPA (1013.25)
unsigned long previousMillis = 0;
unsigned long interval = 10000;
unsigned long delayTime = 10000;


void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(500);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println(WiFi.localIP());
  //The ESP8266 tries to reconnect automatically when the connection is lost
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

String readTemp() {
  return String(bme.readTemperature());
}

String readHumi() {
  return String(bme.readHumidity());
}

String readPres() {
  return String(bme.readPressure() / 100.0F);
}


void printReadings() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("\n### Initializing components... ###");
  initWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connection successfully established");
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
  }

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.println("This Board is a: sender");

  Serial.println(F("Checking BME280 sensor..."));
  bool status = bme.begin(0x76);

  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, please check wiring");
  }
  else {
    Serial.println("Sensor BME280 OK");
  }

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readTemp().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readHumi().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readPres().c_str());
  });

  // Start server
  server.begin();

  Serial.println("### Done initializing components ###\n");
  Serial.println("### Starting loop... ###\n");
}


void loop() {
  printReadings();
  delay(delayTime);
}

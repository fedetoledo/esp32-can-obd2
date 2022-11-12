
// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <ArduinoJson.h>
#include <CAN.h> // the OBD2 library depends on the CAN library
#include <OBD2.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// SSID Credentials
const char* ssid = "Roxanawifi2";
const char* password = "pancho2021";

int pid = 0;

DynamicJsonDocument carData(4096);

WebServer server(80);

void setupWebServer() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.enableCORS();
  server.on("/", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/getData", sendData);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void setupOBD2() {
  // OBD2 Setup
  while (!Serial);

  while (!OBD2.begin()) {
    Serial.print(F("Attempting to connect to OBD2 CAN bus ... "));

    if (!OBD2.begin()) {
      Serial.println(F("failed!"));

      delay(1000);
    } else {
      Serial.println(F("success"));
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);

  setupWebServer();
  setupOBD2();
}

void loop() {
  // Server handler
  server.handleClient();
  delay(2);

  carData["speed"] = processPid(13);
  carData["rpm"] = processPid(12);
  carData["intake_air_temp"] = processPid(15);
  carData["throttle_position"] = processPid(17);
  carData["fuel_pressure"] = processPid(10);
  carData["run_time"] = processPid(31);
  carData["fuel_tank_level"] = processPid(47);

  delay(100);
}

void sendData() {
  String buffer1;
  serializeJson(carData, buffer1);
  server.send(200, "application/json", buffer1);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

float processPid(int pid) {
  Serial.print("PID: ");
  Serial.print(pid);
  Serial.print(", pid Name: ");
  Serial.print(OBD2.pidName(pid));
  Serial.print(", pid value: ");
   
  float pidValue = OBD2.pidRead(pid); 
  Serial.println(pidValue);
  return pidValue;
} 

#include <Arduino.h>
#include <stdio.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "Adafruit_TCS34725.h"
#include "wifi_credentials.h"

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

WebServer server(80);

uint16_t r, g, b, c, colorTemp, lux;
uint16_t attempts = 0;

void getData() {
    Serial.println("Read data");
    tcs.getRawData(&r, &g, &b, &c);
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
}

void getColorData() {
    DynamicJsonDocument doc(512);
    doc["r"] = r;
    doc["g"] = g;
    doc["b"] = b;
    doc["temp"] = colorTemp;
    doc["lux"] = lux;
    String buf;
    serializeJson(doc, buf);
    server.send(200, F("application/json"), buf);
}

void getState() {
    DynamicJsonDocument doc(512);
    if(b > g && b > r) {
        doc["state"] = "normal";
    } else if(g > b && g > r) {
        doc["state"] = "regeneration";
    } else if(r > b && r > g) {
        doc["state"] = "salt empty";
    }
    String buf;
    serializeJson(doc, buf);
    server.send(200, F("application/json"), buf);
}

// Manage not found URL
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

void connect_wifi() {
    Serial.println("Connect to Wifi");
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("BWT-REST-SENSOR");
    WiFi.begin(ssid, password);
    Serial.println("");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        attempts++;
        if(attempts > 20) {
            Serial.println("Too many Attempts, reboot now");
            ESP.restart();
        }
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("BWT-REST-SENSOR")) {
        Serial.println("MDNS responder started");
    }

}

void setup() {
    Serial.begin(115200);

    if (tcs.begin()) {
        Serial.println("Found sensor");
    } else {
        Serial.println("No TCS34725 found ... check your connections");
        while (1);
    }

    connect_wifi();

    server.on(F("/"), HTTP_GET, getColorData);
    server.on(F("/state"), HTTP_GET, getState);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    getData();
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi connection lost, reconnect ...");
        WiFi.mode(WIFI_OFF);
        connect_wifi();
    }
}

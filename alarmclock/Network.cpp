#include <Arduino.h>

// Different machines :)
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "This ain't a ESP8266 or ESP32!"
#endif

#include <Preferences.h>

#include <ESPmDNS.h>
#include "Network.h"


Network::Network() {}

void Network::connect() {
  // Get WiFi creds from preferences storage
  Preferences wifiCreds;
  wifiCreds.begin("wifiCreds", true);
  String ssid = wifiCreds.getString("ssid");
  String pwd = wifiCreds.getString("password");
  wifiCreds.end();

  WiFi.begin(ssid.c_str(), pwd.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String Network::ipAddress() {
  return WiFi.localIP().toString();
}

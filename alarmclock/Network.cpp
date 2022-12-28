#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

#include "Network.h"


Network::Network() {}

void Network::connect(){
  // Get WiFi creds from preferences storage
  Preferences wifiCreds;
  wifiCreds.begin("wifiCreds", true);
  String ssid = wifiCreds.getString("ssid");
  String pwd = wifiCreds.getString("password");
  wifiCreds.end();

  WiFi.begin(ssid.c_str(), pwd.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String Network::ipAddress() {
  return WiFi.localIP().toString();
}

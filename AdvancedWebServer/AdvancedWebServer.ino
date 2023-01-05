#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include "Alarm.h"

TaskHandle_t xHandle = NULL;
BaseType_t xReturned;

WebServer server(80);

const int led = 13;

AlarmEntry alarmFromServerArgs() {
  AlarmEntry alarmEntry = { "", "", "", false, false, false, false, false, false, false, false, false, false };
  
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "name") {
      alarmEntry.name = server.arg(i);
    } else if (server.argName(i) == "hour") {
      alarmEntry.hour = server.arg(i);
    } else if (server.argName(i) == "minute") {
      alarmEntry.minute = server.arg(i);
    } else if (server.argName(i) == "sunday") {
      alarmEntry.sunday = true;
    } else if (server.argName(i) == "monday") {
      alarmEntry.monday = true;
    } else if (server.argName(i) == "tuesday") {
      alarmEntry.tuesday = true;
    } else if (server.argName(i) == "wednesday") {
      alarmEntry.wednesday = true;
    } else if (server.argName(i) == "thursday") {
      alarmEntry.thursday = true;
    } else if (server.argName(i) == "friday") {
      alarmEntry.friday = true;
    } else if (server.argName(i) == "saturday") {
      alarmEntry.saturday = true;
    } else if (server.argName(i) == "skip_phols") {
      alarmEntry.skip_phols = true;
    } else if (server.argName(i) == "once") {
      alarmEntry.once = true;
    } else if (server.argName(i) == "enabled") {
      alarmEntry.enabled = true;
    }
  }

  return alarmEntry;
}


void handleShow() {

}


void handleNew() {
  AlarmEntry alarmEntry = alarmFromServerArgs();

  String message = "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  Alarm alarm;

  Serial.println(alarm.toString(alarmEntry));

  server.send(202, "text/plain", message);
}


void handleRoot() {
  digitalWrite(led, 1);
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP32!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

           hr, min % 60, sec % 60);
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
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
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);

  Preferences wifiCreds;
  wifiCreds.begin("wifiCreds", true);
  String ssid = wifiCreds.getString("ssid");
  String pwd = wifiCreds.getString("password");
  wifiCreds.end();


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pwd.c_str());
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Alarm alarms;
  alarms.init();


  server.on("/", handleRoot);
  server.on("/new", handleNew);
  server.on("/show", handleShow);
  server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  xReturned = xTaskCreate(
    webserverLoop,    /* Function that implements the task. */
    "webserverLoop",  /* Text name for the task. */
    10000,            /* Stack size in words, not bytes. */
    (void*)1,         /* Parameter passed into the task. */
    tskIDLE_PRIORITY, /* Priority at which the task is created. */
    &xHandle);
}

void loop(void) {
  delay(2);  //allow the cpu to switch to other tasks
}

void webserverLoop(void* pvParameters) {
  for (;;) {
    server.handleClient();
    delay(2);  //allow the cpu to switch to other tasks
  }
}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}

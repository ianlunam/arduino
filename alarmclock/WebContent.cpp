#include <Arduino.h>

// Different machines :)
#if defined(ESP32)
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#else
#error "This ain't a ESP8266 or ESP32!"
#endif

#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>

#include "WebContent.h"


#define WEATHER_URL "https://www.metservice.com/publicData/webdata/favourites/urban/tauranga"
#define PUBLIC_HOLIDAYS_URL "https://holidays.abstractapi.com/v1/"

String weatherCondition = "None";
bool isHoliday = false;
int lastDay = 0;
time_t lastWeather;


WebContent::WebContent() {}

void WebContent::init() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  lastWeather = mktime(&timeinfo);

  weather();
  holiday();
}

void WebContent::update() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  time_t now = mktime(&timeinfo);
  if (now > (lastWeather + (60 * 60))) {
    weather();
    lastWeather = now;
  }

  holiday(); // Only does it once a day. Check in function.
}

void WebContent::weather() {
  String payload = "";
  HTTPClient http;
  http.begin(WEATHER_URL);
  int httpCode = http.GET();
  if (httpCode > 0) {
    payload = http.getString();
  } else {
    Serial.println("Error on HTTP request");
    return;
  }
  http.end();

  char chars[payload.length() + 1];
  strcpy(chars, payload.c_str());

  DynamicJsonDocument weatherObject(2048);
  deserializeJson(weatherObject, chars);

  String result = weatherObject["value"]["condition"];
  result = convertCondition(result);

  result.concat(" ");
  result.concat(int(weatherObject["value"]["low"]));
  result.concat("/");
  result.concat(int(weatherObject["value"]["high"]));

  weatherCondition = result;
  Serial.println("Weather updated");
}


void WebContent::holiday() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    return;
  }

  if (lastDay == timeinfo.tm_mday) {
    return;  // Already up to date
  }
  lastDay = timeinfo.tm_mday;

  char ptr[40];
  int rc = strftime(ptr, 40, "?year=%Y\&month=%m\&day=%d", &timeinfo);

  String url = PUBLIC_HOLIDAYS_URL;
  url.concat(ptr);

  // Get Holiday Api creds from preferences storage
  Preferences holidayCreds;
  holidayCreds.begin("holidayApi", true);
  String apiKey = holidayCreds.getString("apiKey");
  holidayCreds.end();
  url.concat("&api_key=");
  url.concat(apiKey);
  url.concat("&country=NZ");

  HTTPClient http;
  http.begin(url);
  String webpage = "";

  int xhttpCode = http.GET();
  if (xhttpCode > 0) {  //Check for the returning code
    webpage = http.getString();
  } else {
    Serial.println("Error on HTTP request");
    return;
  }
  http.end();

  if (webpage.length() > 2) {  // It'll just be "[]" if it's not a holidy
    isHoliday = true;
  } else {
    isHoliday = false;
  }
  Serial.println("Holidays updated");
}

String WebContent::getWeatherString() {
  if (weatherCondition == "None") {
    init();
  }
  return weatherCondition;
}

bool WebContent::isTodayAHoliday() {
  return isHoliday;
}

String WebContent::convertCondition(String current) {
  String weatherTypes[] = { "cloudy", "drizzle", "few-showers-night", "few-showers", "fine-night", "fine", "frost", "hail", "partly-cloudy-night", "partly-cloudy", "rain", "showers", "snow", "thunder", "wind-rain"};
  String weatherTypesMap[] = { "Cloudy", "Drizz", "Showers", "Showers", "Fine", "Fine", "Frost", "Hail", "Cloud", "Cloud", "Rain", "Showers", "Snow", "Thund", "Wind-Rain"};
  for (int x = 0; x < 14; x++) {
    if (weatherTypes[x] == current) {
      return weatherTypesMap[x];
    }
  }
  return current;
}

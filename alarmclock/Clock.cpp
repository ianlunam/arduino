#include <Arduino.h>
#include <time.h>

#include "Clock.h"

const char* tz = "NZST-12NZDT,M9.5.0,M4.1.0/3";

struct tm timeinfo;
time_t lastUpdate;


Clock::Clock() {
  struct tm t = { 0 };
  t.tm_year = 112;  // This is year-1900, so 112 = 2012, randomly picked.
  t.tm_mon = 8;
  t.tm_mday = 15;
  t.tm_hour = 21;
  t.tm_min = 54;
  t.tm_sec = 13;
  lastUpdate = mktime(&t);
}


void Clock::init() {
  setTime();
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  lastUpdate = mktime(&timeinfo);
}


void Clock::update() {
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  time_t now = mktime(&timeinfo);
  if (now > (lastUpdate + (60 * 60 * 12))) {
    setTime();
    lastUpdate = now;
  }
}


void Clock::setTime() {
  configTzTime(tz, "nz.pool.ntp.org");
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  Serial.println("NTP updated");
}


String Clock::getTimeString() {
  if (!getLocalTime(&timeinfo)) {
    return "";
  }

  char ptr[20];
  int rc = strftime(ptr, 20, "%H:%M", &timeinfo);
  return ptr;
}

String Clock::getDateString() {
  if (!getLocalTime(&timeinfo)) {
    return "";
  }

  char ptr[20];
  int rc = strftime(ptr, 20, "%a %e %b", &timeinfo);
  return ptr;
}

String Clock::getYearString() {
  if (!getLocalTime(&timeinfo)) {
    return "";
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");

  char ptr[5];
  int rc = strftime(ptr, 5, "%Y", &timeinfo);
  return ptr;
}